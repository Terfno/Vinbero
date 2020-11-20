#define KBUILD_MODNAME "xdp_srv6_functions"
#include <stddef.h>
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/socket.h>
#include <linux/seg6.h>
#include <linux/seg6_local.h>
#include <linux/udp.h>
#include "bpf_helpers.h"
#include "bpf_endian.h"
#include "hook.h"

// linux/socket.h
#define AF_INET 2   /* Internet IP Protocol 	*/
#define AF_INET6 10 /* IP version 6			*/

// net/ipv6.h
#define NEXTHDR_ROUTING 43 /* Routing header. */

// #include "srv6_maps.h"
#include "srv6_structs.h"
#include "srv6_consts.h"
#include "srv6_helpers.h"

struct bpf_map_def SEC("maps") tx_port = {
    .type = BPF_MAP_TYPE_DEVMAP,
    .key_size = sizeof(int),
    .value_size = sizeof(int),
    .max_entries = 256,
};

struct bpf_map_def SEC("maps") transit_table_v4 = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(__u32),
    .value_size = sizeof(struct transit_behavior),
    .max_entries = MAX_TRANSIT_ENTRIES,
};

struct bpf_map_def SEC("maps") function_table = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(struct in6_addr),
    .value_size = sizeof(struct end_function),
    .max_entries = MAX_END_FUNCTION_ENTRIES,
};

// https://github.com/cloudflare/xdpcap
struct bpf_map_def SEC("maps") xdpcap_hook = XDPCAP_HOOK();

static inline int rewrite_nexthop(struct xdp_md *xdp)
{
    void *data = (void *)(long)xdp->data;
    void *data_end = (void *)(long)xdp->data_end;
    struct ethhdr *eth = data;

    if (data + sizeof(*eth) > data_end)
    {
        return XDP_PASS;
    }

    // struct lookup_result result = {};
    __u32 ifindex;
    unsigned short smac;
    unsigned short dmac;

    lookup_nexthop(xdp, &smac, &dmac, &ifindex);
    bpf_printk("check_lookup_result\n");
    if (check_lookup_result(&dmac))
    {
        bpf_printk("check_lookup_result match\n");
        set_src_dst_mac(data, &smac, &dmac);

        if (xdp->ingress_ifindex == ifindex)
        {
            bpf_printk("select TX\n");
            return XDP_TX;
        }
        bpf_printk("select bpf_redirect_map\n");
        // todo: fix bpf_redirect_map
        return bpf_redirect_map(&tx_port, ifindex, 0);
    }
    return XDP_PASS;
}

/* regular endpoint function */
static inline int action_end(struct xdp_md *xdp)
{
    bpf_printk("run action_end\n");
    void *data = (void *)(long)xdp->data;
    void *data_end = (void *)(long)xdp->data_end;

    struct ipv6_sr_hdr *srhdr = get_and_validate_srh(xdp);
    struct ipv6hdr *v6h = get_ipv6(xdp);

    if (!srhdr || !v6h)
    {
        return XDP_PASS;
    }
    if (!advance_nextseg(srhdr, &v6h->daddr, xdp))
    {
        return XDP_PASS;
    }

    return rewrite_nexthop(xdp);
}

/* regular endpoint function */
static inline int action_t_gtp4_d(struct xdp_md *xdp, struct transit_behavior *tb)
{
    // chack UDP/GTP packet
    void *data = (void *)(long)xdp->data;
    void *data_end = (void *)(long)xdp->data_end;
    struct iphdr *iph = get_ipv4(xdp);
    __u8 type;
    __u32 tid;
    __u16 seqNum;
    if (!iph)
    {
        return XDP_PASS;
    }
    __u16 inner_len = bpf_ntohs(iph->tot_len);

    // Check protocol
    bpf_printk("Check protocol\n");
    if (iph->protocol != IPPROTO_UDP)
    {
        return XDP_PASS;
    }
    struct gtp1hdr *gtp1h = data + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);

    if (data + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(struct gtp1hdr) > data_end)
    {
        return XDP_PASS;
    }

    // tid = gtp1h->tid;
    // type = gtp1h -> type;
    // if (type == ECHO_REQUEST || type == ECHO_RESPONSE){
    //     seqNum = data + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(struct gtp1hdr);

    //     if (data + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(struct gtp1hdr) + sizeof(u16) > data_end){
    //         return XDP_PASS;
    //     }
    // }

    // new seg6 headers
    struct ipv6hdr *hdr;
    struct ipv6_sr_hdr *srh;
    __u8 srh_len;
    bpf_printk("new seg6 headers start\n");

    if (tb->segment_length > MAX_SEGMENTS)
    {
        return XDP_PASS;
    }
    srh_len = sizeof(struct ipv6_sr_hdr) + sizeof(struct in6_addr) * tb->segment_length;
    if (bpf_xdp_adjust_head(xdp, 0 - (int)(sizeof(struct ipv6hdr) + srh_len)))
    {
        return XDP_PASS;
    }
    data = (void *)(long)xdp->data;
    data_end = (void *)(long)xdp->data_end;

    if (!iph)
    {
        return XDP_PASS;
    }
    bpf_printk("new seg6 make hdr\n");
    struct ethhdr *old_eth, *new_eth;
    new_eth = (void *)data;
    old_eth = (void *)(data + sizeof(struct ipv6hdr) + srh_len);
    if ((void *)((long)old_eth + sizeof(struct ethhdr)) > data_end)
    {
        return XDP_PASS;
    }
    if ((void *)((long)new_eth + sizeof(struct ethhdr)) > data_end)
    {
        return XDP_PASS;
    }
    __builtin_memcpy(&new_eth->h_source, &old_eth->h_dest, sizeof(unsigned char) * ETH_ALEN);
    __builtin_memcpy(&new_eth->h_dest, &old_eth->h_source, sizeof(unsigned char) * ETH_ALEN);
    new_eth->h_proto = bpf_htons(ETH_P_IPV6);

    // outer IPv6 header
    bpf_printk("outer IPv6 header\n");
    hdr = (void *)data + sizeof(struct ethhdr);
    if ((void *)(data + sizeof(struct ethhdr) + sizeof(struct ipv6hdr)) > data_end)
    {
        return XDP_PASS;
    }
    hdr->version = 6;
    hdr->priority = 0;
    hdr->nexthdr = NEXTHDR_ROUTING;
    hdr->hop_limit = 64;
    hdr->payload_len = bpf_htons(srh_len + inner_len);
    __builtin_memcpy(&hdr->saddr, &tb->saddr, sizeof(struct in6_addr));
    if (tb->segment_length == 0 || tb->segment_length > MAX_SEGMENTS)
    {
        return XDP_PASS;
    }
    __builtin_memcpy(&hdr->daddr, &tb->segments[tb->segment_length - 1], sizeof(struct in6_addr));

    srh = (void *)hdr + sizeof(struct ipv6hdr);
    if ((void *)(data + sizeof(struct ethhdr) + sizeof(struct ipv6hdr) + sizeof(struct ipv6_sr_hdr)) > data_end)
    {
        return XDP_PASS;
    }
    srh->nexthdr = IPPROTO_IPIP;
    srh->hdrlen = (srh_len / 8 - 1);
    srh->type = 4;
    srh->segments_left = tb->segment_length - 1;
    srh->first_segment = tb->segment_length - 1;
    srh->flags = 0;

    bpf_printk("loop write\n");
#pragma clang loop unroll(full)
    for (int i = 0; i < MAX_SEGMENTS; i++)
    {
        if (tb->segment_length <= i)
        {
            break;
        }
        if (tb->segment_length == i - 1)
        {
            // todo :: convation ipv6 addr
        }
        if ((void *)(&srh->segments[i] + sizeof(struct in6_addr) + 1) > data_end)
            return XDP_PASS;

        __builtin_memcpy(&srh->segments[i], &tb->segments[i], sizeof(struct in6_addr));
    }

    bpf_printk("exec nexthop\n");
    return rewrite_nexthop(xdp);
}

SEC("xdp_prog")
int srv6_handler(struct xdp_md *xdp)
{
    bpf_printk("srv6_handler start");
    void *data = (void *)(long)xdp->data;
    void *data_end = (void *)(long)xdp->data_end;
    struct ethhdr *eth = data;
    struct iphdr *iph = get_ipv4(xdp);
    struct ipv6hdr *v6h = get_ipv6(xdp);
    struct end_function *ef_table;
    struct transit_behavior *tb;

    __u16 h_proto;

    if (data + sizeof(*eth) > data_end)
    {
        bpf_printk("data_end 1\n");
        return xdpcap_exit(xdp, &xdpcap_hook, XDP_PASS);
    }
    if (!iph || !v6h)
    {
        bpf_printk("data_end 2\n");
        return xdpcap_exit(xdp, &xdpcap_hook, XDP_PASS);
    }

    h_proto = eth->h_proto;
    bpf_printk("srv6_handler L3 check\n");
    if (h_proto == bpf_htons(ETH_P_IP))
    {
        // use encap
        bpf_printk("h_proto == bpf_htons(ETH_P_IP)\n");
        tb = bpf_map_lookup_elem(&transit_table_v4, &iph->daddr);
        if (tb)
        {
            bpf_printk("run transit_table_v4 lookup!\n");
            switch (tb->action)
            {
            case SEG6_IPTUN_MODE_ENCAP_T_M_GTP4_D:
                bpf_printk("run SEG6_IPTUN_MODE_ENCAP_T_M_GTP4_D\n");
                return xdpcap_exit(xdp, &xdpcap_hook, action_t_gtp4_d(xdp, tb));
            }
        }
    }
    else if (h_proto == bpf_htons(ETH_P_IPV6))
    {
        // use nexthop and exec to function or decap or encap
        // checkSRv6
        if (v6h->nexthdr == NEXTHDR_ROUTING)
        {
            bpf_printk("match v6h->nexthdr == NEXTHDR_ROUTING)\n");
            ef_table = bpf_map_lookup_elem(&function_table, &(v6h->daddr));
            if (ef_table)
            {
                bpf_printk("ef_table check %u", ef_table);
                switch (ef_table->function)
                {
                case SEG6_LOCAL_ACTION_END:
                    return xdpcap_exit(xdp, &xdpcap_hook, action_end(xdp));
                }
            }
        }
        else
        {
            // todo::
            // encap type code
            // encap check condtion
        }
    }
    bpf_printk("no match all\n");
    return xdpcap_exit(xdp, &xdpcap_hook, XDP_PASS);
}

SEC("xdp_pass")
int xdp_pass_func(struct xdp_md *ctx)
{
    return XDP_PASS;
}
char _license[] SEC("license") = "GPL";