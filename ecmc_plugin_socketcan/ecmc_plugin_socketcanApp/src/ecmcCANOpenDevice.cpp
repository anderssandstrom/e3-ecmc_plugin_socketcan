/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcCANOpenDevice.cpp
*
*  Created on: Mar 08, 2021
*      Author: anderssandstrom
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
                                     const char* name,
                                     int dbgMode) {

  writeBuffer_        = writeBuffer;
  nodeId_             = nodeId;
  exeSampleTimeMs_    = exeSampleTimeMs;
  exeCounter_         = 0;
  errorCode_          = 0;
  dbgMode_            = dbgMode;
  name_               = strdup(name);
  isMaster_           = false;
  pdoCounter_ = 0;
  sdoCounter_ = 0;
  sdo1Lock_.test_and_set();   // make sure only one sdo is accessing the bus at the same time
  sdo1Lock_.clear();
  for(int i = 0 ; i<ECMC_CAN_DEVICE_PDO_MAX_COUNT;i++) {
    pdos_[i] = NULL;
  }
  for(int i = 0 ; i<ECMC_CAN_DEVICE_SDO_MAX_COUNT;i++) {
    sdos_[i] = NULL;
  }
}

ecmcCANOpenDevice::~ecmcCANOpenDevice() {
  for(int i = 0 ; i<ECMC_CAN_DEVICE_PDO_MAX_COUNT;i++) {
    delete pdos_[i];
  }
  for(int i = 0 ; i<ECMC_CAN_DEVICE_SDO_MAX_COUNT;i++) {
    delete sdos_[i];
  }

  free(name_);
}

void ecmcCANOpenDevice::execute() {
  
  exeCounter_++;

  for(int i=0 ; i<pdoCounter_; i++) {
    if(pdos_[i]) {
      pdos_[i]->execute();
    }
  }

  for(int i=0 ; i<sdoCounter_; i++) {
    if(sdos_[i]) {
      sdos_[i]->execute();
    }
  }

  return;
}

// new rx frame recived!
void ecmcCANOpenDevice::newRxFrame(can_frame *frame) {

  // only validate if not master
  if (!validateFrame(frame) && !isMaster_) {
    return;
  }

  // forward to pdos
  for(int i=0 ; i<pdoCounter_; i++) {
    if(pdos_[i]) {
      pdos_[i]->newRxFrame(frame);
    }
  }

  // forward to sdos
  for(int i=0 ; i<sdoCounter_; i++) {
    if(sdos_[i]) {
      sdos_[i]->newRxFrame(frame);
    }
  }

  return;
}

// r 0x183 [8] 0x00 0x00 0x00 0x00 0x0B 0x40 0x04 0x20
int ecmcCANOpenDevice::validateFrame(can_frame *frame) {
  
  // nodeid is always lower 7bits.. Need to check this calc.. byte order?!
  uint8_t tempNodeId = frame->can_id & 0x7F;
  
  if(tempNodeId != nodeId_) {
    return 0;
  }
  return 1;
}

int ecmcCANOpenDevice::addPDO(uint32_t cobId,
                              ecmc_can_direction rw,
                              uint32_t ODSize,
                              int readTimeoutMs,
                              int writeCycleMs,   //if <0 then write on demand..                              
                              const char* name) {

  if(pdoCounter_>= ECMC_CAN_DEVICE_PDO_MAX_COUNT) {
    return ECMC_CAN_PDO_INDEX_OUT_OF_RANGE;
  }

  pdos_[pdoCounter_] = new ecmcCANOpenPDO(writeBuffer_,
                                         nodeId_,
                                         cobId,
                                         rw,
                                         ODSize,
                                         readTimeoutMs,
                                         writeCycleMs,
                                         exeSampleTimeMs_,
                                         name,
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
                              int readSampleTimeMs,
                              const char* name) {

  if(sdoCounter_>= ECMC_CAN_DEVICE_SDO_MAX_COUNT) {
    return ECMC_CAN_SDO_INDEX_OUT_OF_RANGE;
  }

  sdos_[sdoCounter_] = new ecmcCANOpenSDO(writeBuffer_,
                                         nodeId_,
                                         cobIdTx,
                                         cobIdRx,
                                         rw,
                                         ODIndex,
                                         ODSubIndex,
                                         ODSize,
                                         readSampleTimeMs,
                                         exeSampleTimeMs_,
                                         name,
                                         &sdo1Lock_,
                                         sdoCounter_,
                                         dbgMode_);
  sdoCounter_++;                                       
  return 0;
}

uint32_t ecmcCANOpenDevice::getNodeId() {
  return nodeId_;
}

