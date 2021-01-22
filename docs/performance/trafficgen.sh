#!/bin/bash
# addr configuraiton
ip link set lo up
ip addr add fc00:1::1/128 dev lo
ip addr add fc00:3::3/128 dev lo

ip link set ens2f0 up
ip addr add fc00:12::1/64 dev ens2f0
ip addr add 10.1.0.1/24 dev ens2f0

ip link set ens2f1 up
ip addr add fc00:21::1/64 dev ens2f1
ip addr add 10.2.0.1/24 dev ens2f1

# ip -6 route add fc00:21::/48 via fc00:21::2
# ip -6 route add fc00:12::/48 via fc00:12::2
ip -6 route add fc00:2::/48 via fc00:12::2

sysctl net.ipv4.conf.all.rp_filter=0
sysctl net.ipv4.conf.all.forwarding=1
sysctl net.ipv6.conf.all.forwarding=1
