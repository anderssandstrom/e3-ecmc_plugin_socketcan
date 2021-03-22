e3-ecmc_plugin_socketcan
======
ESS Site-specific EPICS module : ecmcPlugin_socketcan

# ScoketCAN

This module adds some CAN support to ecmc based on SocketCAN:
https://en.wikipedia.org/wiki/SocketCAN


# CANOpen

This module implements a few of the CANOpen functionalities (but it is far from a full CANOpen Implementation).

The current implemented features are:

* Simple master:

    * Simple LSS

    * Heartbeat

    * Sync

* Simple generic (slave) device

* SDO segmented r/w

* PDO r/w

These functionalities are configured through iocsh cmds:

* ecmcCANOpenAddMaster

* ecmcCANOpenAddDevice

* ecmcCANOpenAddSDO

* ecmcCANOpenAddPDO


## ecmcCANOpenAddMaster

Issueing "ecmcCANOpenAddMaster -h" in the iocsh you will get the following help printout:
``` 
    ecmcCANOpenAddMaster -h
       Use ecmcCANOpenAddMaster(<name>, <node id>,....)
                 <name>                     : Name of master device.
                 <node id>                  : CANOpen node id of master.
                 <LSS sample time ms>       : Sample time for LSS.
                 <Sync sample time ms>      : Sample time for SYNC.
                 <Heartbeat sample time ms> : Sample time for Heartbeat.
``` 

Example to add a master with LSS, sync, heartbeat sample time of 1s:
``` 
ecmcCANOpenAddMaster("ecmcCANOpenMaster",0,1000,1000,1000)
``` 

Note:: You can only have one master in each ioc (one call to "ecmcCANOpenAddMaster").

## ecmcCANOpenAddDevice

Issueing "ecmcCANOpenAddDevice -h" in the iocsh you will get the following help printout:
``` 
    ecmcCANOpenAddDevice -h
      Use ecmcCANOpenAddDevice(<name>, <node id>)
          <name>                     : Name of device.
          <node id>                  : CANOpen node id of device.
          <NMT Heartbeat timeout ms> : Timeout for NMT Heartbeat.\n")

``` 
Example to add a device with node id 3 and a NMT heartbeat timeout of 3s:
``` 
ecmcCANOpenAddDevice("testDevice",3,3000)
``` 

Note:: You can only use the below commands (ecmcCANOpenAddSDO and ecmcCANOpenAddODO) to nodes of an existing device or master.

## ecmcCANOpenAddSDO

Issueing "ecmcCANOpenAddSDO -h" in the iocsh you will get the following help printout:
``` 
    ecmcCANOpenAddSDO -h
        ecmcCANOpenAddSDO( in the iocsh<name>, <node id>,.....)
         <name>            : Name of master device.
         <node id>         : CANOpen node id of device/master.
         <cob id tx>       : CANOpen cob id of Tx of slave SDO.
         <cob id rx>       : CANOpen cob id of Rx of slave SDO.
         <dir>             : Direction 1=write and 2=read.
         <ODIndex>         : OD index of SDO.
         <ODSubIndex>      : OD sub index of SDO.
         <ODSize>          : OS Siz
         <readSampleTimeMs>: Sample time for read in ms (write is always on demand).
``` 

A few examples:
``` 
# Example read SDO from device, will be cyclically read at an intervall of 7000ms:
ecmcCANOpenAddSDO("analogValues",3,0x583,0x603,2,0x2640,0x0,56,7000)
dbLoadRecords(ecmcPluginSocketCAN_SDO_input.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},NELM=${NELM=1},CH_ID=03,DEV_ID=3,suffix=AI03-Array,source=basicConfig,DTYP=asynInt8ArrayIn,FTVL=CHAR,NELM=7")

# Example write SDO to device, will be written on demand (when write data from epics):
ecmcCANOpenAddSDO("basicConfig",3,0x583,0x603,1,0x2690,0x1,7,0)
dbLoadRecords(ecmcPluginSocketCAN_SDO_output.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},NELM=${NELM=1},CH_ID=03,DEV_ID=3,suffix=BasicConfig,source=basicConfig,DTYP=asynInt8ArrayOut,FTVL=CHAR,NELM=7")

``` 
## ecmcCANOpenAddPDO

Issueing "ecmcCANOpenAddPDO -h" in the iocsh you will get the following help printout:
``` 
    ecmcCANOpenAddPDO -h
      Use "ecmcCANOpenAddPDO(<name>, <node id>
          <name>            : Name of master device.
          <node id>         : CANOpen node id of device/master.
          <cob id>          : CANOpen cob id of PDO.
          <dir>             : Direction 1=write and 2=read.
          <ODSize>          : Size of PDO (max 8 bytes).
          <readTimeoutMs>   : Readtimeout in ms.
          <writeCycleMs>    : Cycle time for write (if <= 0 then only write on change).

``` 
An example:

``` 
# Read PDO, if not recived pdo within 10s the PDO object will go into error state:
ecmcCANOpenAddPDO("status",3,0x183,2,8,10000,0)
dbLoadRecords(ecmcPluginSocketCAN_PDO_input.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},NELM=${NELM=1},CH_ID=03,DEV_ID=3,suffix=PDO01-Array,source=status,NELM=8")
``` 

# Plugin

Plugin report printout:
``` 
Plugin info: 
  Index                = 0
  Name                 = ecmcPlugin_socketcan
  Description          = SocketCAN plugin for use with ecmc.
  Option description   = 
    DBG_PRINT=<1/0>    : Enables/disables printouts from plugin, default = disabled (=0).
    IF=<if name>         : Sets can interface (example: can0, vcan0..).
    CONNECT=<1/0>        : Auto connect to if at startup, default = autoconnect (=1).

  Filename             = /home/dev/epics/base-7.0.4/require/3.4.0/siteMods/ecmc_plugin_socketcan/master/lib/linux-x86_64/libecmc_plugin_socketcan.so
  Config string        = IF=vcan0;DBG_PRINT=1;
  Version              = 2
  Interface version    = 65536 (ecmc = 65536)
     max plc funcs     = 64
     max plc func args = 10
     max plc consts    = 64
  Construct func       = @0x7fb36b81d310
  Enter realtime func  = @0x7fb36b81d220
  Exit realtime func   = @0x7fb36b81d230
  Realtime func        = @0x7fb36b81d270
  Destruct func        = @0x7fb36b81d240
  dlhandle             = @0xa45a50
  Plc functions:
    funcs[00]:
      Name       = "can_connect();"
      Desc       = double can_connect() : Connect to can interface (from config str).
      Arg count  = 0
      func       = @0x7fb36b81d280
    funcs[01]:
      Name       = "can_connected();"
      Desc       = double can_connected() : Connected to can interface.
      Arg count  = 0
      func       = @0x7fb36b81d2a0
    funcs[02]:
      Name       = "can_add_write(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);"
      Desc       = double can_add_write(canId,len,data0..data7) : Add write frame to can interface output buffer.
      Arg count  = 10
      func       = @0x7fb36b81d2e0
    funcs[03]:
      Name       = "can_last_writes_error();"
      Desc       = double can_last_writes_error() : get error from last writes.
      Arg count  = 0
      func       = @0x7fb36b81d2c0
  Plc constants:
``` 
# ecmcByteToArray Asub record
This module implements an asub record that can merge bytes into an array (waveform)

Usage:
1. Link bytes to inputs A..S.

2. Link array to VALA.

3. Set size of output in NOVA. This also defines how many inputs will be used.

 Note: Max 18 bytes (input A..S) will be merged into the array.


EPICS database example of 7 bytes merged intio an array:
``` 
record(aSub,   "$(P)CAN${CH_ID}-BasicConfigPackArray_") {
  field(INAM,  "ecmcByteToArrayInit")
  field(SNAM,  "ecmcByteToArray")
  field(FTA,   "UCHAR")
  field(NOA,   "1")
  field(INPA,  "$(P)CAN${CH_ID}-BasicConfigB0_.VAL")    # Byte 0
  field(FTB,   "UCHAR")
  field(NOB,   "1")
  field(INPB,  "$(P)CAN${CH_ID}-VrefPwrCmdCalcB1_.VAL") # Byte 1
  field(FTC,   "UCHAR")
  field(NOC,   "1")
  field(INPC,  "$(P)CAN${CH_ID}-VrefPwrCmdCalcB2_.VAL") # Byte 2
  field(FTD,   "UCHAR")
  field(NOD,   "1")
  field(INPD,  "$(P)CAN${CH_ID}-VdcCtrlCmdCalcB3_.VAL") # Byte 3
  field(FTE,   "UCHAR")
  field(NOE,   "1")
  field(INPE,  "$(P)CAN${CH_ID}-VdcCtrlCmdCalcB4_.VAL") # Byte 4
  field(FTF,   "UCHAR")
  field(NOF,   "1")
  field(INPF,  "0")                                     # Byte 5
  field(FTG,   "UCHAR")
  field(NOG,   "1")
  field(INPG,  "0")                                     # Byte 6
  field(FTVA,  "UCHAR")
  field(OUTA,  "$(P)CAN${CH_ID}-SDO02-BasicConfig")
  field(NOVA,  "7")                                     # 7 bytes (0..6 corresponds to input A..G)
  field(FLNK,  "$(P)CAN${CH_ID}-SDO02-BasicConfig.PROC") # Send the data
}
``` 

# Testing
You can use a virtual can, vcan network for testing:

## Prerequirements

Install can utils:
``` 
   $ git clone https://github.com/linux-can/can-utils
   $ cd can-utils
   $ make
   $ make install
``` 

## vcan setup
 Start virt can 0 (vcan0) and candump:
 ``` 
   $ sudo modprobe vcan
   $ sudo ip link add dev vcan0 type vcan
   $ sudo ip link set up vcan0
   $ candump vcan0
``` 
candump will start to printout the raw can meassages on vcan0 network.

# Other intressting things

In future it could be an option to use one of these repos for the CANOpen support:
1. https://github.com/CANopenNode/CANopenNode
2. https://github.com/CANopenNode/CANopenSocket
3. https://github.com/marel-keytech/openCANopen

