

git clone https://github.com/linux-can/can-utils
cd can-utils
make
sudo make install


git clone git https://github.com/CANopenNode/CANopenSocket.git
cd CANopenSocket
git submodule init
git submodule update
cd tools
./get_tools.sh


cd CANopenNode
make

update gcc if needed (if error at __has_include):
sudo yum install centos-release-scl
sudo yum install devtoolset-7-gcc*
scl enable devtoolset-7 bash
which gcc
gcc --version

fatal error: bits/getopt_core.h:
Modify CO_main_basic.c:
//#include <bits/getopt_core.h>
#include <getopt.h>


Follow this to test:
https://github.com/CANopenNode/CANopenNode/blob/master/doc/gettingStarted.md
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0


candump vcan0
./canopend --help
./canopend vcan0 -i 4 -s od4_storage -a od4_storage_auto
Seems not suppoer "-a" option so remove that:
./canopend vcan0 -i 4 -s od4_storage

update 


Checkout master of CanopenNode to make it work like in gettingStarted.md

needed to use this as the canid 1 device:
echo "-" > od1_storage
echo "-" > od1_storage_auto
./canopend vcan0 -i 1 -c "stdio" -s od1_storage -a od1_storage_auto


# EDSEditor (windows)
## install mono on raspian
sudo apt-get install mono-complete

# Setup kvaser leaf
## must install socketcan_kvaser_drivers.tar.gz from kvaser download.
$ tar -xvzf socketcan_kvaser_drivers.tar.gz
$ cd socketcan_kvaser_drivers
# IMPORTANT: must add #include <linux/version.h> to all source files to get linux version macros.
$ make
$ sudo make install
$ sudo make load

# seems kvaser need to use can_dev
$ modprobe can_dev
$ modprobe can
$ modprobe can_raw

# seems leaf0 is now called can0 (not sure why)
$ sudo ip link set can0 type can bitrate 125000
$ sudo ip link set up can0
$ ip link show can0

# check that interface is there
$ ip addr

# Connect pmu to canbus
# connect pmu to power

# monitor canbus with candump
$ candump can0
  can0  280   [0] 
  can0  280   [0] 
  can0  280   [0] 
  can0  280   [0] 
  can0  280   [0] 
  can0  703   [1]  05
  can0  280   [0] 
  can0  683   [4]  00 00 00 00
  can0  280   [0] 
  can0  280   [0] 
  can0  183   [8]  00 00 00 00 0B 40 04 20
  can0  280   [0] 
  can0  703   [1]  05
  can0  280   [0] 
  can0  280   [0] 
  can0  280   [0] 
  can0  703   [1]  05
  can0  280   [0] 
  can0  280   [0] 
  can0  280   [0] 
  
  ...
  can0  280   [0] 
  can0  280   [0] 
  can0  703   [1]  05
  can0  280   [0] 
  can0  280   [0] 
  can0  683   [4]  00 0NMT0 00 00 0B 40 04 20
  can0  703   [1]  05
  can0  280   [0] 
  ..

# 183 seems to be PDO status bytes
can0  183   [8]  00 00 00 00 0B 40 04 20

# 703 seems to be NMT hearbeat
can0  703   [1]  05

# 280 seems to be "alarm" that it has no basic configuration (needs to be transfeered over pdo (for all amplifiers) or sdo for the specific amplifier)
can0  280   [0] 

# 603 seems to not be described in manual.. Just described as third transmit pdo?!
can0  683   [4]  00 00 00 00