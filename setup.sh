#/bin/sh

# install dependencies for building iproute2
apt update
DEBIAN_FRONTEND=noninteractive apt upgrade -y
apt install -y bison flex clang llvm libelf-dev bc libssl-dev tmux

# update iproute2
sudo apt install -y pkg-config bison flex make gcc
cd /tmp
wget https://mirrors.edge.kernel.org/pub/linux/utils/net/iproute2/iproute2-4.20.0.tar.gz
tar -xzvf ./iproute2-4.20.0.tar.gz
cd ./iproute2-4.20.0

sudo make && sudo make install

# enable gtp and install
sudo apt -y install libtalloc-dev libpcsclite-dev libmnl-dev autoconf libtool

git clone git://git.osmocom.org/libgtpnl.git
cd libgtpnl
autoreconf -fi
./configure
make
sudo make install
sudo ldconfig

sudo modprobe gtp
lsmod | grep gtp
