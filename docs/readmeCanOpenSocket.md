

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
## install mono
sudo apt-get install mono-complete
