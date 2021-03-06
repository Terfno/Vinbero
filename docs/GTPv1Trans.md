# SRv6 to GTPv1 Stateless Translation Function

## Subset
* T.M.GTP4.D
* T.M.GTP4.E

## Topology
![](./images/GTPv1TransTop.png)

## Run
```bash
# host1
> Create gtp device
# if failed modprobe, plz new kernel header and module
modprobe udp_tunnel
modprobe ip6_udp_tunnel
modprobe gtp
./gtp-link add gtp1

> Open a new console and configure tunnel (PDP session)
./gtp-tunnel add gtp1 v1 200 100 10.0.1.2 172.0.2.1
ip route add 10.0.1.2/32 dev gtp1
ip link set gtp1 mtu 1500

# host2
> Create gtp device
modprobe udp_tunnel
modprobe ip6_udp_tunnel
modprobe gtp
./gtp-link add gtp2

> Open a new console and configure tunnel (PDP session)
./gtp-tunnel add gtp2 v1 100 200 10.0.1.1 172.0.1.1
ip route add 10.0.1.1/32 dev gtp2
ip link set gtp2 mtu 1500
```
sudo ./gtptools --mode server --listen 172.0.2.1 --subscriber 10.0.1.2 --peer 172.0.1.1 --ote 200 --ite 100
