## History
Kvaser does not support socketcan for kernel 3.10.
However, Kvaser support supplied a custom driver for Kernel 3.10 for testing. 

## Files
In this dir you will find two files:

1. socketcan_kvaser_drivers_for_3.10.tar

2. socketcan_kvaser_drivers_for_3.10_patched.tar.gz  

The "socketcan_kvaser_drivers_for_3.10.tar" is the "raw" files recived from Kvaser support. This file did not compile. Kvaser suggested to remove some lines in the Makefile. This resulted in the "socketcan_kvaser_drivers_for_3.10_patched.tar.gz" file.

The "socketcan_kvaser_drivers_for_3.10_patched.tar.gz" driver compiles and installs just fine. But an error is generated when the Kvaser Leaf Light v2 interafce is connected to USB and the interface will not work:

dmesg using socketcan_kvaser_drivers_for_3.10.tar (with patched Makefile):
``` 
Connecting leaf to usb:
[Mar24 11:11] usbcore: deregistering interface driver kvaser_usb
[Mar24 11:12] usb 1-2: new high-speed USB device number 10 using xhci_hcd
[  +0.126425] usb 1-2: New USB device found, idVendor=0bfd, idProduct=0120, bcdDevice= 0.01
[  +0.000002] usb 1-2: New USB device strings: Mfr=1, Product=2, SerialNumber=0
[  +0.000002] usb 1-2: Product: Kvaser Leaf Light v2
[  +0.000002] usb 1-2: Manufacturer: Kvaser AB
[  +1.058714] kvaser_usb 1-2:1.0: Cannot get software infos, error -110
[  +0.000007] kvaser_usb: probe of 1-2:1.0 failed with error -110
[  +0.000027] usbcore: registered new interface driver kvaser_usb

The interface is not working (not visible with ip addr).

Disconnecting leaf from usb:
[Mar24 11:21] usb 1-2: USB disconnect, device number 10
[Mar24 11:23] usbcore: deregistering interface driver kvaser_usb

``` 

## CONCLUSION

Use file "kvaser/socketcan_kvaser_drivers_1.6.113_patch_asm_3.10.tar.gz"
This file contains a patched version of the newest Kvaser socketcan drivers. 
Basically everything that did not compile was removed, mostly related to CAN FD. A function can_change_state() was also not availble in kernel 3.10. Kvaser support supplied a can_change_state() which have been added to the code.

So the patched newest version of the Kvaser socketcan drivers is the only option for the time beeing. By using this driver, communiucation to a slave is possible.

One error is however encounterd during dereg of the device, see below.

dmesg from when this driver is used:
``` 
##########################################################################
dmesg using newest Kvaser socketcan_kvaser_drivers patched and added can_change_state()

Connecting leaf to usb:
[Mar24 11:26] usb 1-2: new high-speed USB device number 11 using xhci_hcd
[  +0.126349] usb 1-2: New USB device found, idVendor=0bfd, idProduct=0120, bcdDevice= 0.01
[  +0.000003] usb 1-2: New USB device strings: Mfr=1, Product=2, SerialNumber=0
[  +0.000002] usb 1-2: Product: Kvaser Leaf Light v2
[  +0.000002] usb 1-2: Manufacturer: Kvaser AB
[  +0.034801] usbcore: registered new interface driver kvaser_usb

Can intreface is now visible with ip addr:

ip addr
8: can0: <NOARP,ECHO> mtu 16 qdisc noop state DOWN group default qlen 10
    link/can 


And the interface is working!

Disconnecting leaf from usb:
[Mar24 10:31] usb 1-2: USB disconnect, device number 8
[  +0.000050] kvaser_usb 1-2:1.0 can0: Cannot flush queue, error -19
[  +0.000002] kvaser_usb 1-2:1.0 can0: Cannot reset card, error -19
[  +0.000002] kvaser_usb 1-2:1.0 can0: Cannot stop device, error -19


``` 
These error messages are normal when the usb is unplugged while link is up (confirmed with kvaser support).

So conclusion is that use of "kvaser/socketcan_kvaser_drivers_1.6.113_patch_asm_3.10.tar.gz" is the only way to make it work under kernel 3.10.
According to Kvaser support this approach is OK and should work just fine. Also test shows it works well.

