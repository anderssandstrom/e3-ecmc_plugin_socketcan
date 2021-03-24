e3-ecmc_plugin_socketcan
======
ESS Site-specific EPICS module : ecmcPlugin_socketcan

# SocketCAN

This module adds some CAN support to ecmc based on SocketCAN:
https://en.wikipedia.org/wiki/SocketCAN

## Hardware

This module have been tested with Kvaser Leaf Light v2 usb can interface. 
More info about how to setup this hardware can be found here:

[Kvaser Leaf V2 setup](kvaser/readme.md)

# Install ecmc plugin module

## Dependencies
This module depends on ecmc and asyn.

## Configuration
Ensure that these two files are correct:

1.  configure/CONFIG_MODULE:
``` 
EPICS_MODULE_TAG:=master
E3_MODULE_VERSION:=master
ECMC_DEP_VERSION:=6.3.2
ASYN_DEP_VERSION:=4.41.0
``` 

2.  configure/RELEASE:
``` 
EPICS_BASE:=${HOME}/epics/base-7.0.4
E3_REQUIRE_VERSION:=3.4.0
``` 

## Install

The module is installed like any other e3 module by "make install":
``` 
make install
``` 
Now the module should be ready for use.

# CANOpen

This module implements a few of the CANOpen functionalities (but it is far from a full CANOpen Implementation).

The current implemented features are:

* Simple master:

    * LSS (basic)

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

# Load status records for device
dbLoadRecords(ecmcPluginSocketCAN_Dev.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,CH_ID=00,DEV_ID=0")
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

# Load status records for device
dbLoadRecords(ecmcPluginSocketCAN_Dev.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,CH_ID=03,DEV_ID=3")
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

# Load record for SDO data
dbLoadRecords(ecmcPluginSocketCAN_SDO_input.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},NELM=${NELM=1},CH_ID=03,DEV_ID=3,suffix=AI03-Array,source=basicConfig,DTYP=asynInt8ArrayIn,FTVL=CHAR,NELM=7")

# Load record for SDO error
dbLoadRecords(ecmcPluginSocketCAN_SDO_error.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},CH_ID=03,DEV_ID=3,suffix=SDO01-AnalogValuesErr,source=analogValues")

# Example write SDO to device, will be written on demand (when write data from epics):
ecmcCANOpenAddSDO("basicConfig",3,0x583,0x603,1,0x2690,0x1,7,0)

# Load record for SDO data
dbLoadRecords(ecmcPluginSocketCAN_SDO_output.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},NELM=${NELM=1},CH_ID=03,DEV_ID=3,suffix=BasicConfig,source=basicConfig,DTYP=asynInt8ArrayOut,FTVL=CHAR,NELM=7")

# Load record for SDO error
dbLoadRecords(ecmcPluginSocketCAN_SDO_error.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},CH_ID=03,DEV_ID=3,suffix=SDO02-BasicConfigErr,source=basicConfig")

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

# Load record for PDO data
dbLoadRecords(ecmcPluginSocketCAN_PDO_input.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},NELM=${NELM=1},CH_ID=03,DEV_ID=3,suffix=PDO01-Array,source=status,NELM=8")

# Load record for PDO error
dbLoadRecords(ecmcPluginSocketCAN_PDO_error.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},CH_ID=03,DEV_ID=3,suffix=PDO01-StatusErr,source=status")

``` 

# PVs

## Communication

The status of the CAN communication can be supervised by loading the "ecmcPluginSocketCAN_Com.template":

``` 
dbLoadRecords(ecmcPluginSocketCAN_Com.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1")

# Will load:
IOC_TEST:CAN-Stat-ComErr
IOC_TEST:CAN-Stat-Connected
``` 
The Connected Pv show if the plugin is connected to socketcan and the hardware.
The ComErr Pv shows error codes for reads and writes to socketcan socket.

## Device (also Master)

### ecmcPluginSocketCAN_Dev.template

The ecmcPluginSocketCAN_Dev.template loads a NMT PV:
``` 
IOC_TEST:CAN03-Stat-NMT
``` 
The PV tells wich state the device is in and can have the following values:
*  NMT_NOT_VALID : State not valid. Something is wrong.

*  NMT_BOOT_UP : Device is booting

*  NMT_STOPPED : Device is stopped

*  NMT_OP : Device is operational (this is the normal state when running the ioc). 

*  NMT_PREOP : Device is in pre operational state


## PDO

## ecmcPluginSocketCAN_PDO_input.template

The ecmcPluginSocketCAN_PDO_intput.template in used for input PDOS (data read from a device device to EPICS).
The template only contains one PV that holds the data:

Example:
``` 
dbLoadRecords(ecmcPluginSocketCAN_PDO_input.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},CH_ID=03,DEV_ID=3,suffix=PDO01-Status_,source=status,NELM=8")

# Loads this PV:
IOC_TEST:CAN03-PDO01-Status_
``` 
The data PV is an waveform of type uchar and length NELM.

## ecmcPluginSocketCAN_PDO_output.template

The ecmcPluginSocketCAN_PDO_output.template in used for output PDOS (data written from EPICS to device).
The template only contains one PV that holds the data:

Example:
``` 
dbLoadRecords(ecmcPluginSocketCAN_PDO_output.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},CH_ID=00,DEV_ID=0,suffix=PDO01-Reset_,  source=reset,NELM=8")

# Loads this PV:
IOC_TEST:CAN03-PDO01-Reset_
``` 
The data PV is an waveform of type uchar and length NELM.


## ecmcPluginSocketCAN_PDO_error.template

The ecmcPluginSocketCAN_PDO_error.template in used to read the PDO error code.

Example:
``` 
dbLoadRecords(ecmcPluginSocketCAN_PDO_error.template,  "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},CH_ID=00,DEV_ID=0,suffix=PDO01-ResetErr,source=reset")

# Loads this PV:
IOC_TEST:CAN03-PDO01-ResetErr
``` 

An error could for example be that the PDO data have not been recived within the correct time frame (readTimeoutMs).
The PDO is in error state if the error code is not 0.

## SDO

## ecmcPluginSocketCAN_SDO_input.template

The ecmcPluginSocketCAN_SDO_intput.template in used for input SDOS (data read from a device device to EPICS).
The template only contains one PV that holds the data:

Example:
``` 
dbLoadRecords(ecmcPluginSocketCAN_SDO_input.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},CH_ID=03,DEV_ID=3,suffix=SDO01-AnalogValues_,  source=analogValues,DTYP=asynInt16ArrayIn,FTVL=SHORT,NELM=28")

# Loads this PV:
IOC_TEST:CAN03-SDO01-AnalogValues_
``` 
The data PV is an waveform of type uchar and length NELM.

## ecmcPluginSocketCAN_SDO_output.template

The ecmcPluginSocketCAN_SDO_output.template in used for output SDOS (data written from EPICS to device).
The template only contains one PV that holds the data:

Example:
``` 
dbLoadRecords(ecmcPluginSocketCAN_SDO_output.template,"P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},CH_ID=03,DEV_ID=3,suffix=SDO02-BasicConfig_,  source=basicConfig,DTYP=asynInt8ArrayOut,FTVL=UCHAR,NELM=7")

# Loads this PV:
IOC_TEST:CAN03-SDO02-BasicConfig_
``` 
The data PV is an waveform of type uchar and length NELM.

## ecmcPluginSocketCAN_SDO_error.template

The ecmcPluginSocketCAN_PSO_error.template in used to read the SDO error code.

Example:
``` 
dbLoadRecords(ecmcPluginSocketCAN_SDO_error.template, "P=${ECMC_PREFIX},PORT=${ECMC_ASYN_PORT},ADDR=0,TIMEOUT=1,T_SMP_MS=${ECMC_SAMPLE_RATE_MS},TSE=${ECMC_TSE},CH_ID=03,DEV_ID=3,suffix=SDO02-BasicConfigErr,source=basicConfig")

# Loads this PV:
IOC_TEST:CAN03-SDO02-BasicConfigErr
``` 

An error could for example be that the SDO data have not been recived within the correct time frame or slave is not reposning like it should.
The SDO is in error state if the error code is not 0.

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

