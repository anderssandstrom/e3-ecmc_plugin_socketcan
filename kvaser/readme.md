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
