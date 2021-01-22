# Tips
## Print debug
```
sudo trace-cmd record -e 'xdp:*' -O trace_printk
sudo trace-cmd report > trace.log
```
## Xdpcap
Packets output at runtime can be saved in pcap.
See https://github.com/cloudflare/xdpcap

```
sudo apt-get install libpcap-dev
# centos/fedora case
# sudo yum install libpcap-devel
go get -u github.com/cloudflare/xdpcap/cmd/xdpcap

# run on each nodes
sudo mount bpffs /sys/fs/bpf -t bpf

# capture packets
xdpcap /sys/fs/bpf/xdpcap_hook "icmp"
```
