/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcCANOpenDevice.h
*
*  Created on: Mar 08, 2021
*      Author: anderssandstrom
*
\*************************************************************************/
#ifndef ECMC_CANOPEN_DEVICE_H_
#define ECMC_CANOPEN_DEVICE_H_

#include <stdexcept>
#include "ecmcDataItem.h"
#include "ecmcAsynPortDriver.h"
#include "ecmcSocketCANDefs.h"
#include "ecmcCANOpenPDO.h"
#include "ecmcCANOpenSDO.h"
#include "ecmcCANOpenDevice.h"
#include "inttypes.h"
#include <string>
#include "ecmcSocketCANWriteBuffer.h"
#include "epicsMutex.h"

#include <linux/can.h>
#include <linux/can/raw.h>

#define ECMC_CAN_DEVICE_PDO_MAX_COUNT 8
#define ECMC_CAN_DEVICE_SDO_MAX_COUNT 8
#define ECMC_CAN_ERROR_PDO_TIMEOUT 100

#define ECMC_CAN_PDO_INDEX_OUT_OF_RANGE 1000
#define ECMC_CAN_SDO_INDEX_OUT_OF_RANGE 1001

class ecmcCANOpenDevice {
 public:
  ecmcCANOpenDevice(ecmcSocketCANWriteBuffer* writeBuffer,
                    uint32_t cobId,
                    int exeSampleTimeMs,
                    int dbgMode);
  ~ecmcCANOpenDevice();
  void execute();
  void newRxFrame(can_frame *frame);
  int addPDO(ecmc_can_direction rw,
             uint32_t cobId,
             uint32_t ODSize,
             int readTimeoutMs,
             int writeCycleMs);   //if <0 then write on demand.
  
  int addSDO(uint32_t cobIdTx,    // 0x580 + CobId
             uint32_t cobIdRx,    // 0x600 + Cobid
             ecmc_can_direction rw,
             uint16_t ODIndex,    // Object dictionary index
             uint8_t ODSubIndex, // Object dictionary subindex
             uint32_t ODSize,
             int readSampleTimeMs);
  
 private:
  int validateFrame(can_frame *frame);
  ecmcSocketCANWriteBuffer *writeBuffer_;
  uint32_t nodeId_;   // with cobid
  int exeSampleTimeMs_;
  int exeCounter_;
  int errorCode_;
  int dbgMode_;
  int pdoCounter_;
  int sdoCounter_;
  ecmcCANOpenSDO *pdos_[ECMC_CAN_DEVICE_PDO_MAX_COUNT];
  ecmcCANOpenPDO *sdos_[ECMC_CAN_DEVICE_SDO_MAX_COUNT];
};

#endif  /* ECMC_CANOPEN_DEVICE_H_ */

