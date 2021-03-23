# Use Kvaser Leaf Light v2

## Kernel 3.10

The socketcan drivers sources downloadable from kvaser webpage will not work for kernel 3.10 but a pathed version can be used.

### Pre reqs

Install kernel headers
```
sudo yum install kernel-devel
```
Note: make sure that correct kernel headers are installed for the current kernel, otherwise maybe a update is needed.

### Install kvaser driver

#### Use the correct source

Goto kvasser dir and untar the file: socketcan_kvaser_drivers_patch_asm_3.10.tar.gz
The "socketcan_kvaser_drivers_patch_asm_3.10.tar.gz" contains a pathed version of the kvaser socket can drivers were  functionality not availbale in kernel 3.10 have been removed.

NOTE: This source will only work for Kvaser leaf light v2 usb. It will not work for Kvaser Hydra.
``` 
cd kvaser
tar -xvf socketcan_kvaser_drivers_patch_asm_3.10.tar.gz 
cd socketcan_kvaser_drivers/
``` 
Note: Non patched source downladed from kvaser will not work with kernel 3.10. So use the source in kvaser dir of this repo.

#### make 
Read the README file in kvaser driver.
These commands are from that README:
```
# UNPLUGG KVASER USB DEVICE FIRST
$ sudo make uninstall
$ make
$ sudo make install
```
## Kernel version >= 4.x
Go to kvaser webpage and download kvaser socket can drivers. Install according to instructions.


## Start services and configure interface

Start services:
```
$ sudo modprobe can_dev
$ sudo modprobe can
$ sudo modprobe can_raw
#$ sudo modprobe kvaser_usb                        # not  needed, will start as soon as pluggen in
$ sudo ip link set can0 type can bitrate 125000   # 125000 is bitrate (works for pmu905)
$ sudo ip link set up can0
``` 
Now you should see the can0 interface, test with "ip addr"






# OBSOLETE NOTES BELOW:
#### 1. First test without install of driver and without kvaser leaf connected.

Check support before install of driver:

``` 
sudo cat /boot/config-3.10.0-1127.el7.x86_64  | grep KVASER
CONFIG_CAN_KVASER_PCI=m
CONFIG_CAN_KVASER_USB=m
``` 

Seems support for KVASER_USB already without installing.

Check ip addr:
``` 
ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: eno1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
    link/ether c0:3f:d5:66:25:44 brd ff:ff:ff:ff:ff:ff
    inet 172.30.2.34/27 brd 172.30.2.63 scope global noprefixroute dynamic eno1
       valid_lft 9999557sec preferred_lft 9999557sec
    inet6 fe80::c23f:d5ff:fe66:2544/64 scope link noprefixroute 
       valid_lft forever preferred_lft forever


``` 
Seems no can interface (and it's not connected).

#### 2. Connect kvaser leaf:

Check ip addr:
``` 
ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: eno1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
    link/ether c0:3f:d5:66:25:44 brd ff:ff:ff:ff:ff:ff
    inet 172.30.2.34/27 brd 172.30.2.63 scope global noprefixroute dynamic eno1
       valid_lft 9999557sec preferred_lft 9999557sec
    inet6 fe80::c23f:d5ff:fe66:2544/64 scope link noprefixroute 
       valid_lft forever preferred_lft forever


``` 
Still no can0...

``` 
dmesg:[  543.068096] usb 2-4: new high-speed USB device number 4 using xhci_hcd
[  543.191792] usb 2-4: New USB device found, idVendor=0bfd, idProduct=0120, bcdDevice= 0.01
[  543.191803] usb 2-4: New USB device strings: Mfr=1, Product=2, SerialNumber=0
[  543.191809] usb 2-4: Product: Kvaser Leaf Light v2
[  543.191815] usb 2-4: Manufacturer: Kvaser AB

``` 


#### 3. Try start services and conf if

###### Services
Start services:
```
$ sudo modprobe can_dev
$ sudo modprobe can
$ sudo modprobe can_raw
$ sudo ip link set can0 type can bitrate 125000

```
Conclusion:
Cannot find can0.. Need to install kvaser driver..


#### Insall kvaser socketcan 3.10

##### 1 unplug kvaser leaf

##### 2 untar driver source
```
cd kvaser
tar -xvf socketcan_kvaser_drivers_for_3.10.tar 
cd socketcan_kvaser_drivers/
```

NOTE: The socketcan_kvaser_drivers_for_3.10.tar  was recived on request from kvaser support. Their normal socketcan driver will not support kernel 3.10.

##### 3 make 
Read the README file in kvaser driver.
These commands are from that README:
```
$ sudo make uninstall
$ make
$ sudo make install
```

Fails at make...

sudo  yum install kernel-devel
The kernel sources are at the wrong place.. Sources for wrong kernel!!

Need to update

``` 
sudo yum update
``` 
Now the kernel sources are at the corerct location. patched version of new socketcan version builds and seems to work with candump and connected pmu905

#### THIS IS A SIDE NOTE for PEAK usb CAN interface !!!

Check if support for PEAK USB
``` 
sudo cat /boot/config-3.10.0-1127.el7.x86_64  | grep PEAK
CONFIG_CAN_PEAK_PCI=m
CONFIG_CAN_PEAK_PCIEC=y
CONFIG_CAN_PEAK_USB=m
CONFIG_SND_FIREWIRE_SPEAKERS=m
# CONFIG_SPEAKUP is not set
``` 

