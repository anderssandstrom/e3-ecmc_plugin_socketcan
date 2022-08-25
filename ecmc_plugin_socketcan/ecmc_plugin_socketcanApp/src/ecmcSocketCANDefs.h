/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcSocketCANDefs.h
*
*  Created on: March 02, 2021
*      Author: anderssandstrom
*
\*************************************************************************/

#ifndef ECMC_SOCKETCAN_DEFS_H_
#define ECMC_SOCKETCAN_DEFS_H_

// Options
#define ECMC_PLUGIN_DBG_PRINT_OPTION_CMD   "DBG_PRINT="
#define ECMC_PLUGIN_IF_OPTION_CMD          "IF="
#define ECMC_PLUGIN_CONNECT_OPTION_CMD     "CONNECT="

#define ECMC_CANOPEN_NMT_BASE              0x700
#define ECMC_CANOPEN_NMT_BOOT              0x0
#define ECMC_CANOPEN_NMT_STOP              0x4
#define ECMC_CANOPEN_NMT_OP                0x5
#define ECMC_CANOPEN_NMT_PREOP             0x7F

#define ECMC_SDO_REPLY_TIMOUT_MS           1000

#define ECMC_PLUGIN_ASYN_PREFIX            "plugin.can"

enum ecmc_can_direction {
    DIR_WRITE = 1,
    DIR_READ  = 2 };

enum ecmc_read_states {
    READ_IDLE,
    READ_REQ_TRANSFER,
    READ_WAIT_FOR_CONF,
    READ_WAIT_FOR_DATA
};

enum ecmc_write_states {
    WRITE_IDLE,
    WRITE_REQ_TRANSFER,
    WRITE_WAIT_FOR_CONF,
    WRITE_DATA,
};

enum ecmc_nmt_state_act {
    NMT_NOT_VALID = 0,
    NMT_BOOT_UP   = 1,
    NMT_STOPPED   = 2,
    NMT_OP        = 3,
    NMT_PREOP     = 4
};

struct ODIndexBytes {
    char byte0:8;
    char byte1:8;
};

struct ODLegthBytes {
    char byte0:8;
    char byte1:8;
    char byte2:8;
    char byte3:8;
};

struct writeCmdByte {
    char c:1;
    char nnn:3;
    char t:1;
    char notused:3;
};

/** Just one error code in "c" part of plugin 
(error handled with exceptions i c++ part) */
#define ECMC_PLUGIN_SOCKETCAN_ERROR_CODE 1

#endif  /* ECMC_SOCKETCAN_DEFS_H_ */
