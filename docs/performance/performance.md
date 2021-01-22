# Performance

## Install
in detel to check link
cf. [https://trex-tgn.cisco.com/trex/doc/trex_manual.html](https://trex-tgn.cisco.com/trex/doc/trex_manual.html)

This time I am using fedora33 and mellanox X5 EN 50GbE.
However, fedora33 is trapped by the validation of the mellanox driver installation, so we need to do a work-around hack!

```
yum -y install perl-File-Tail perl-File-Copy perl-File-Compare perl-sigtrap
yum -y install kernel-rpm-macros python-srpm-macros rpm-build python3-devel tk gcc-gfortran tcsh elfutils-libelf-devel
wget http://www.mellanox.com/downloads/ofed/MLNX_OFED-5.2-1.0.4.0/MLNX_OFED_LINUX-5.2-1.0.4.0-fc32-x86_64.tgz
tar xfvz MLNX_OFED_LINUX-5.2-1.0.4.0-fc32-x86_64.tgz
cd　MLNX_OFED_LINUX-5.2-1.0.4.0-fc32-x86_64/
```

`mlnxofedinstall` check and rewrite

```
} elsif ($dist_rpm =~ /fedora-release(|-common)-(\d+)/ and
        ($2 >= 12) and ($2 <= 33)) {
```

lets install
```
./mlnxofedinstall --add-kernel-support --distro fc32
# reboot after run
/etc/init.d/openibd restart
```

next, trex install
```
sudo su -
mkdir -p /opt/trex
cd /opt/trex
wget --no-check-certificate --no-cache https://trex-tgn.cisco.com/trex/release/v2.87.tar.gz
cd /opt/trex
tar xzvf v2.87.tar.gz
cd /opt/trex/v2.87
```

## Usage
```
./t-rex-64
```
## RouterSetup
use irqaffinity. this mellanox nic case.

```
git clone https://github.com/Mellanox/mlnx-tools.git
cd mlnx-tools/
./set_irq_affinity.sh ens4f0np0
./set_irq_affinity.sh ens4f1np1
```


## Tips
* https://github.com/tohojo/xdp-paper/blob/master/benchmarks/bench01_baseline.org

