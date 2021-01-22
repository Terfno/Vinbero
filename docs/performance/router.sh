#!/bin/bash

systemctl stop irqbalance
systemctl disable irqbalance

# addr configuraiton
ip link set lo up
ip addr add fc00:2::2/128 dev lo

ip link set ens4f0 up
ip addr add fc00:12::2/64 dev ens4f0
ip addr add 10.1.0.2/24 dev ens4f0

ip link set ens4f1 up
ip addr add fc00:21::2/64 dev ens4f1
ip addr add 10.2.0.2/24 dev ens4f1

ip -6 route add fc00:1::/48 via fc00:12::1
ip -6 route add fc00:3::/48 via fc00:21::1

sysctl net.ipv4.conf.all.rp_filter=0
sysctl net.ipv4.conf.all.forwarding=1
sysctl net.ipv6.conf.all.forwarding=1

# seg6
sysctl net.ipv6.conf.all.seg6_enabled=1

# offload disible
disblelist=( rx tx tso ufo gso gro lro tx nocache copy sg txvlan rxvlan receive-hashing ntuple-filters rx-vlan-filter tx-gre-segmentation tx-gre-csum-segmentation tx-ipxip4-segmentation tx-ipxip6-segmentation tx-udp_tnl-segmentation tx-udp_tnl-csum-segmentation tx-gso-partial tx-vlan-stag-hw-insert rx-udp_tunnel-port-offload )
for i in ${disblelist[@]}; do
       /sbin/ethtool -K ens4f0 $i off 2>&1 > /dev/null;
done
ethtool -L ens4f0 combined $(nproc --all)
ethtool -K ens4f0 rxhash on
ethtool -K ens4f0 ntuple on
# card balancing
for proto in tcp4 udp4 tcp6 udp6; do
   /sbin/ethtool -N ens4f0 rx-flow-hash $proto sd
done

for i in ${disblelist[@]}; do
       /sbin/ethtool -K ens4f1 $i off 2>&1 > /dev/null;
done

ethtool -L ens4f1 combined $(nproc --all)
ethtool -K ens4f1 rxhash on
ethtool -K ens4f1 ntuple on

for proto in tcp4 udp4 tcp6 udp6; do
   /sbin/ethtool -N ens4f1 rx-flow-hash $proto sd
done
cd mlnx-tools/ofed_scripts
./set_irq_affinity.sh ens4f0 ens4f1
sudo ethtool -G ens4f0 rx 4080 tx 4080
sudo ethtool -G ens4f1 rx 4080 tx 4080
sudo ethtool --set-priv-flags ens4f0 rx_cqe_compress on
sudo ethtool --set-priv-flags ens4f1 rx_cqe_compress on
