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
#include "ecmcAsynPortDriver.h"
#include "ecmcPluginClient.h"

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
  nmtState_           = NMT_NOT_VALID;
  nmtStateOld_        = NMT_NOT_VALID;
  nmtActParam_        = NULL;
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
  initAsyn();
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
  
  // NMT
  if(!isMaster_ && nmtActParam_) {
    checkNMT(frame);
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

int ecmcCANOpenDevice::checkNMT(can_frame *frame) {
  // check if NMT frame
  if(frame->can_id == (ECMC_CANOPEN_NMT_BASE + nodeId_)) {
    if(frame->can_dlc == 1){
      switch(frame->data[0]) {
        case ECMC_CANOPEN_NMT_BOOT:
          nmtState_ = NMT_BOOT_UP;
          break;
        case ECMC_CANOPEN_NMT_STOP:
          nmtState_ = NMT_STOPPED;
          break;
        case ECMC_CANOPEN_NMT_OP:
          nmtState_ = NMT_OP;
          break;
        case ECMC_CANOPEN_NMT_PREOP:
          nmtState_ = NMT_BOOT_UP;
          break;
        default:
          nmtState_ = NMT_NOT_VALID;
          break;
      }
    }
    if(nmtState_ != nmtStateOld_) {
      nmtActParam_->refreshParam(1);
    }
    nmtState_ = nmtStateOld_;
  }
  return 0;
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

void ecmcCANOpenDevice::initAsyn() {

   ecmcAsynPortDriver *ecmcAsynPort = (ecmcAsynPortDriver *)getEcmcAsynPortDriver();
   if(!ecmcAsynPort) {
     printf("ERROR: ecmcAsynPort NULL.");
     throw std::runtime_error( "ERROR: ecmcAsynPort NULL." );
   }

  // Add resultdata "plugin.can.dev%d.<name>"
  std::string paramName = ECMC_PLUGIN_ASYN_PREFIX + std::string(".dev") + 
                          to_string(nodeId_) + ".nmtstate";

  nmtActParam_ = ecmcAsynPort->addNewAvailParam(
                                          paramName.c_str(),    // name
                                          asynParamInt32,       // asyn type 
                                          (uint8_t*)&nmtState_,           // pointer to data
                                          sizeof(nmtState_),    // size of data
                                          ECMC_EC_S32,          // ecmc data type
                                          0);                   // die if fail

  if(!nmtActParam_) {
    printf("ERROR: Failed create asyn param for NMT state.");
    throw std::runtime_error( "ERROR: Failed create asyn param for data: " + paramName);
  }

  nmtActParam_->addSupportedAsynType(asynParamUInt32Digital);

  nmtActParam_->refreshParam(1); // read once into asyn param lib
  ecmcAsynPort->callParamCallbacks(ECMC_ASYN_DEFAULT_LIST, ECMC_ASYN_DEFAULT_ADDR);
}
// Avoid issues with std:to_string()
std::string ecmcCANOpenDevice::to_string(int value) {
  std::ostringstream os;
  os << value;
  return os.str();
}
