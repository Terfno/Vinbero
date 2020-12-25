#ifndef __SRV6_STRUCTS_H
#define __SRV6_STRUCTS_H
#include "srv6_consts.h"

#include <linux/types.h>
#include <linux/in6.h> /* For struct in6_addr. */

struct transit_behavior
{
    __u8 action;
    __u32 segment_length;
    struct in6_addr saddr;
    struct in6_addr segments[MAX_SEGMENTS];
};

struct lpm_key_v4
{
    __u32 prefixlen;
    __u32 addr;
};

struct lpm_key_v6
{
    __u32 prefixlen;
    struct in6_addr addr;
};

struct end_function
{
    __u32 saddr[4];
    // The reason why the "__u32 function" is not "__u8" is that it also serves as padding.
    // The cilium/ebpf package assumes that the go structure takes 4 bytes each and does not pack.
    __u32 function;
};

// Segment Routing Extension Header (SRH)
// https://datatracker.ietf.org/doc/draft-ietf-6man-segment-routing-header/
struct srhhdr
{
    __u8 nextHdr;
    __u8 hdrExtLen;
    __u8 routingType;
    __u8 segmentsLeft;
    __u8 lastEntry;
    __u8 flags;
    __u8 tag;
    // cf. 5.3. Encoding of Tags Field/ https://datatracker.ietf.org/doc/draft-murakami-dmm-user-plane-message-encoding
    __u8 gtpMessageType : 4; // least significant 4 bits of tag field
    struct in6_addr segments[0];
};

struct srv6_tlv
{
    __u8 type;
    __u8 len;
    __u8 data[0];
};

/* According to 3GPP TS 29.060. */
struct gtp1hdr
{
    __u8 version : 3;  // Version field: always 1 for GTPv1
    __u8 pt : 1;       // Protocol Type (PT): GTP(1), GTP'(0)
    __u8 reserved : 1; // always zero (0)
    __u8 e : 1;        // Extension Header flag (E)
    __u8 s : 1;        // Sequence number flag (S): not present(0), present(1)
    __u8 pn : 1;       // N-PDU Number flag (PN)

    __u8 type;
    __u16 length;
    __u32 tid;

    // options
    __u16 seq;       // Sequence Number
    __u8 npdu;       // N-PDU number
    __u8 nextExtHdr; // Next Extention Header Type
};

// https://tools.ietf.org/html/draft-ietf-dmm-srv6-mobile-uplane-05#section-6.1
struct args_mob_session
{
    __u8 qfi : 6;
    __u8 r : 1;
    __u8 u : 1;
    __u32 pdu_session_id;
};

#endif
