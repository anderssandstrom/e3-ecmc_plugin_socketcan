/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcCANOpenDevice.cpp
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*      Credits to  https://github.com/sgreg/dynamic-loading 
*
\*************************************************************************/

// Needed to get headers in ecmc right...
#define ECMC_IS_PLUGIN

#include <sstream>
#include "ecmcCANOpenDevice.h"

/** 
 * ecmc ecmcCANOpenDevice class
*/
ecmcCANOpenDevice::ecmcCANOpenDevice(ecmcSocketCANWriteBuffer* writeBuffer,
                                     uint32_t nodeId,  // 0x580 + CobId
                                     int exeSampleTimeMs,
                                     int dbgMode) {

  writeBuffer_        = writeBuffer;
  nodeId_             = nodeId;
  exeSampleTimeMs_    = exeSampleTimeMs;
  exeCounter_         = 0;
  errorCode_          = 0;
  dbgMode_            = dbgMode;

  pdoCounter_ = 0;
  sdoCounter_ = 0;

  for(int i = 0 ; i<ECMC_CAN_DEVICE_PDO_MAX_COUNT;i++) {
    pdos[i] = NULL;
  }
  for(int i = 0 ; i<ECMC_CAN_DEVICE_SDO_MAX_COUNT;i++) {
    sdos[i] = NULL;
  }
}

ecmcCANOpenDevice::~ecmcCANOpenDevice() {
}

void ecmcCANOpenDevice::execute() {
  
  exeCounter_++;

  for(int i=0 ; i<pdoCounter_; i++) {
    if(pdos[i]) {
      pdos[i]->execute();
    }
  }

  for(int i=0 ; i<sdoCounter_; i++) {
    if(sdos[i]) {
      sdos[i]->execute();
    }
  }

  return;
}

// new rx frame recived!
void ecmcCANOpenDevice::newRxFrame(can_frame *frame) {

  if (!validateFrame(frame) {
    return;
  }

  // forward to pdos
  for(int i=0 ; i<pdoCounter_; i++) {
    if(pdos[i]) {
      pdos[i]->newRxFrame(frame);
    }
  }

  // forward to sdos
  for(int i=0 ; i<sdoCounter_; i++) {
    if(sdos[i]) {
      sdos[i]->newRxFrame(frame);
    }
  }

  return
}

// r 0x183 [8] 0x00 0x00 0x00 0x00 0x0B 0x40 0x04 0x20
int ecmcCANOpenDevice::validateFrame(can_frame *frame) {
  if(frame->can_id != cobId_) {
    return 0;
  }
  return 1;
}

int ecmcCANOpenDevice::addPDO(ecmc_can_direction rw,
                              int cobId,
                              uint32_t ODSize,
                              int readTimeoutMs,
                              int writeCycleMs,   //if <0 then write on demand..                              
                              ) {

  if(pdoCounter_>= ECMC_CAN_DEVICE_PDO_MAX_COUNT) {
    return ECMC_CAN_PDO_INDEX_OUT_OF_RANGE;
  }

  pdos[pdoCounter_] = new ecmcCANOpenPDO(writeBuffer_,
                                         cobId,
                                         rw,
                                         ODSize,
                                         readTimeoutMs,
                                         writeCycleMs,
                                         exeSampleTimeMs_,
                                         dbgMode_);
  pdoCounter_++;                                       
  return 0;
}

int ecmcCANOpenDevice::addSDO(uint32_t cobIdTx,    // 0x580 + CobId
                              uint32_t cobIdRx,    // 0x600 + Cobid
                              ecmc_can_direction rw,
                              uint16_t ODIndex,    // Object dictionary index
                              uint8_t ODSubIndex, // Object dictionary subindex
                              uint32_t ODSize,
                              int readSampleTimeMs) {

  if(sdoCounter_>= ECMC_CAN_DEVICE_SDO_MAX_COUNT) {
    return ECMC_CAN_SDO_INDEX_OUT_OF_RANGE;
  }

  sdos[sdoCounter_] = new ecmcCANOpenSDO(writeBuffer_,
                                         cobIdTx,
                                         cobIdRx,
                                         rw,
                                         ODIndex,
                                         ODSubIndex,
                                         ODSize,
                                         readSampleTimeMs,
                                         exeSampleTimeMs_,
                                         dbgMode_);
  sdoCounter_++;                                       
  return 0;
}
