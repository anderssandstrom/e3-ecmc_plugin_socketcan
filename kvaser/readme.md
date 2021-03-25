# Use Kvaser Leaf Light v2

## Kernel 3.10

The socketcan drivers sources downloadable from kvaser webpage will not work for kernel 3.10 but a patched version can be used.

NOTE: This workflow will only work for the Kvaser Leaf Light v2 (and not the Kvaser Hydra).

### Pre reqs

Install kernel headers
```
sudo yum install kernel-devel
```
Note: make sure that correct kernel headers are installed for the current kernel, otherwise maybe a update is needed.

### Install kvaser driver

#### Use the correct source

Goto kvaser dir and untar the file: socketcan_kvaser_drivers_1.6.113_patch_asm_3.10.tar.gz

The "socketcan_kvaser_drivers_1.6.113_patch_asm_3.10.tar.gz" contains a patched version of the kvaser socketcan driver version 1.6.113.
Functionality not availbale in kernel 3.10 have been removed. For the driver for the Leaf Light v2 this includes:

1. Removing CAN FD support 

2. Adding sources provided by kvaser support for the "can_change_state()" function since it is not avilabe in the 3.10 kernel. 

NOTE: This source will only work for Kvaser Leaf Light v2 usb. It will not work for Kvaser Hydra.
``` 
cd kvaser
tar -xvf socketcan_kvaser_drivers_1.6.113_patch_asm_3.10.tar.gz 
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

Note: You will get some warnings from hydra source but like stated above the hydra hw or driver is NOT supported and should be avoided. So basically this driver ONLY supports the Kvaser Leaf Light v2.

###  DMESG 

#### Connect Kvaser Leaf Light v2 USB

dmesg using newest Kvaser socketcan_kvaser_drivers patched and added "can_change_state()" (socketcan_kvaser_drivers_1.6.113_patch_asm_3.10.tar.gz):

```
# Connecting leaf to usb:
[Mar24 11:26] usb 1-2: new high-speed USB device number 11 using xhci_hcd
[  +0.126349] usb 1-2: New USB device found, idVendor=0bfd, idProduct=0120, bcdDevice= 0.01
[  +0.000003] usb 1-2: New USB device strings: Mfr=1, Product=2, SerialNumber=0
[  +0.000002] usb 1-2: Product: Kvaser Leaf Light v2
[  +0.000002] usb 1-2: Manufacturer: Kvaser AB
[  +0.034801] usbcore: registered new interface driver kvaser_usb

Can interface is now visible with ip addr:

ip addr
8: can0: <NOARP,ECHO> mtu 16 qdisc noop state DOWN group default qlen 10
    link/can 

```
The interface is working and communication to can slave are possible.

#### Disconnect Kvaser Leaf Light v2 USB

The following errors will appear in dmesg if unplugging the device while link is up.
```
# Disconnecting leaf from usb:
[Mar24 10:31] usb 1-2: USB disconnect, device number 8
[  +0.000050] kvaser_usb 1-2:1.0 can0: Cannot flush queue, error -19
[  +0.000002] kvaser_usb 1-2:1.0 can0: Cannot reset card, error -19
[  +0.000002] kvaser_usb 1-2:1.0 can0: Cannot stop device, error -19
```

Checked with kvaser support and this is normal when unplugging the usb when the the link is up. So this is not an Issue.


### More info

More info on other driver supplied by Kvaser can be found here:
[Other driver (currently not working) ](obsolete/README.md)

## Kernel version >= 4.x
Go to kvaser webpage and download kvaser socket can drivers. Install according to instructions.


## Start services and configure interface

First connect the Kvaser Leaf Light v2 usb interface to the controller.

Start services:
```
$ sudo modprobe can_dev
$ sudo modprobe can
$ sudo modprobe can_raw
#$ sudo modprobe kvaser_usb                       # not  needed, will start as soon as leaf is plugged in
$ sudo ip link set can0 type can bitrate 125000   # 125000 is bitrate (works for pmu905)
$ sudo ip link set up can0
``` 
Now you should see the can0 interface, test with "ip addr"
``` 
ip addr
...
7: can0: <NOARP,UP,LOWER_UP,ECHO> mtu 16 qdisc pfifo_fast state UNKNOWN group default qlen 10
    link/can 
...
``` 

Check that you have the correct services running (lsmod):
``` 
lsmod | grep can
can_raw                17120  0 
can                    36567  1 can_raw
can_dev                20760  1 kvaser_usb
...
``` 

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

