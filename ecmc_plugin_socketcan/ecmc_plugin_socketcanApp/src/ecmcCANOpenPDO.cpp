/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcCANOpenPDO.cpp
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*
\*************************************************************************/

// Needed to get headers in ecmc right...
#define ECMC_IS_PLUGIN

#include <sstream>
#include "ecmcCANOpenPDO.h"
#include "ecmcAsynPortDriver.h"
#include "ecmcPluginClient.h"

// Calback function to init write from asyn
asynStatus asynWritePDOValue(void* data, size_t bytes, asynParamType asynParType,void *userObj) {
  // userobj = NULL
  if(!userObj) {
    printf("Error: asynWritePDOValue() fail, no user obj defined.\n");
    return asynError;
  }
  ecmcCANOpenPDO* pdo = (ecmcCANOpenPDO*)userObj;
  int bytesToCp = bytes;
  if (bytes > 8) {
    bytesToCp = 8;
  }
  uint64_t tempData = 0;

  memcpy(&tempData,data,bytesToCp);       
  pdo->setValue(tempData);
  pdo->writeValue();
  return asynSuccess;
}

/** 
 * ecmc ecmcCANOpenPDO class
*/
ecmcCANOpenPDO::ecmcCANOpenPDO(ecmcSocketCANWriteBuffer* writeBuffer,
                               uint32_t nodeId,
                               uint32_t cobId,  // 0x580 + CobId
                               ecmc_can_direction rw,
                               uint32_t ODSize,
                               int readTimeoutMs,
                               int writeCycleMs, 
                               int exeSampleTimeMs,
                               const char* name,
                               int dbgMode) {

  writeBuffer_        = writeBuffer;
  nodeId_             = nodeId;
  cobId_              = cobId;
  ODSize_             = ODSize;
  name_               = strdup(name);

  if(ODSize_ > 8)  {
    ODSize_ = 8;
  }

  readTimeoutMs_      = readTimeoutMs;
  writeCycleMs_       = writeCycleMs;
  exeSampleTimeMs_    = exeSampleTimeMs;
  rw_                 = rw;
  exeCounter_         = 0;
  busy_               = 0;
  errorCode_          = 0;
  dataBuffer_         = new uint8_t(ODSize_);
  dbgMode_            = dbgMode;

  writeFrame_.can_id  = cobId_;
  writeFrame_.can_dlc = ODSize;  // data length
  writeFrame_.data[0] = 0;       // request read cmd
  writeFrame_.data[1] = 0;
  writeFrame_.data[2] = 0;
  writeFrame_.data[3] = 0;
  writeFrame_.data[4] = 0;
  writeFrame_.data[5] = 0;
  writeFrame_.data[6] = 0;
  writeFrame_.data[7] = 0;

  dataMutex_ = epicsMutexCreate();
  initAsyn();
}

ecmcCANOpenPDO::~ecmcCANOpenPDO() {
  delete[] dataBuffer_;
  free(name_);
}

void ecmcCANOpenPDO::execute() {
  
  exeCounter_++;
  
  if(rw_ == DIR_READ) {
    if(exeCounter_* exeSampleTimeMs_ >= readTimeoutMs_) {
      errorCode_ = ECMC_CAN_ERROR_PDO_TIMEOUT;
      if(dbgMode_) {
        printf("ECMC_CAN_ERROR_PDO_TIMEOUT (0x%x)\n",errorCode_);
      }
      exeCounter_ = 0;
    }
  }
  else {  //DIR_WRITE
    if(writeCycleMs_<=0) {  // Only write on demand if cycle is less than 0
      exeCounter_ = 0;
      return;
    }
    if(exeCounter_* exeSampleTimeMs_ >= writeCycleMs_) {      
      writeValue();  // write in defined cycle
      exeCounter_ = 0;
    }
  }
  return;
}

// new rx frame recived!
void ecmcCANOpenPDO::newRxFrame(can_frame *frame) {
  // Wait for:
  if(rw_ == DIR_READ) {
    if(validateFrame(frame)) {
      epicsMutexLock(dataMutex_);
      memset(dataBuffer_,0,ODSize_);
      memcpy(dataBuffer_, &(frame->data[0]),frame->can_dlc);
      epicsMutexUnlock(dataMutex_);
      errorCode_ = 0;
      if(dbgMode_) {
        printBuffer();
      }
    }
  }
}

void ecmcCANOpenPDO::printBuffer() {
  if(!dataBuffer_) {
    return;
  }

  for(uint32_t i = 0; i < ODSize_; i = i + 2) {
    uint16_t test;
    memcpy(&test,&dataBuffer_[i],2);
    printf("data[%02d]: %u\n",i/2,test);
  }
}

// r 0x183 [8] 0x00 0x00 0x00 0x00 0x0B 0x40 0x04 0x20
int ecmcCANOpenPDO::validateFrame(can_frame *frame) {
  if(frame->can_id != cobId_) {
    return 0;
  }
  if(frame->can_dlc != ODSize_) {
    return 0;
  }
  return 1;
}

void ecmcCANOpenPDO::setValue(uint64_t data) {
  epicsMutexLock(dataMutex_);
  memcpy(dataBuffer_, &data, ODSize_);
  epicsMutexUnlock(dataMutex_);
}

int ecmcCANOpenPDO::writeValue() {
  if(writeFrame_.can_dlc > 0) {
    epicsMutexLock(dataMutex_);
    memcpy(&(writeFrame_.data[0]), dataBuffer_ ,writeFrame_.can_dlc);
    epicsMutexUnlock(dataMutex_);
  }
  return writeBuffer_->addWriteCAN(&writeFrame_);
}

void ecmcCANOpenPDO::initAsyn() {

   ecmcAsynPortDriver *ecmcAsynPort = (ecmcAsynPortDriver *)getEcmcAsynPortDriver();
   if(!ecmcAsynPort) {
     printf("ERROR: ecmcAsynPort NULL.");
     throw std::runtime_error( "ERROR: ecmcAsynPort NULL." );
   }

  // Add resultdata "plugin.can.dev%d.<name>"
  std::string paramName = ECMC_PLUGIN_ASYN_PREFIX + std::string(".dev") + 
                          to_string(nodeId_) + ".pdo" /*+ to_string(objIndex_) */
                          + "." + std::string(name_);

  dataParam_ = ecmcAsynPort->addNewAvailParam(
                                          paramName.c_str(),     // name
                                          asynParamInt8Array,    // asyn type 
                                          dataBuffer_,           // pointer to data
                                          ODSize_,               // size of data
                                          ECMC_EC_U8,            // ecmc data type
                                          0);                    // die if fail

  if(!dataParam_) {
    printf("ERROR: Failed create asyn param for data.");
    throw std::runtime_error( "ERROR: Failed create asyn param for data: " + paramName);
  }
  
  // Allow different types depending on size
  if(ODSize_>1){
    dataParam_->addSupportedAsynType(asynParamInt16Array);
  }
  if(ODSize_>3){
    dataParam_->addSupportedAsynType(asynParamInt32Array);
    dataParam_->addSupportedAsynType(asynParamFloat32Array);
    dataParam_->addSupportedAsynType(asynParamInt32);
  }
  if(ODSize_>7){
    dataParam_->addSupportedAsynType(asynParamFloat64Array);
    dataParam_->addSupportedAsynType(asynParamFloat64);
  }

  dataParam_->setAllowWriteToEcmc(rw_ == DIR_WRITE);

  if(rw_ == DIR_WRITE) {
    dataParam_->setExeCmdFunctPtr(asynWritePDOValue,this);
  }

  dataParam_->refreshParam(1); // read once into asyn param lib
  ecmcAsynPort->callParamCallbacks(ECMC_ASYN_DEFAULT_LIST, ECMC_ASYN_DEFAULT_ADDR);  

  // Add resultdata "plugin.can.dev%d.error"
  paramName = ECMC_PLUGIN_ASYN_PREFIX + std::string(".dev") + 
              to_string(nodeId_) + ".sdo" /*+ to_string(objIndex_)*/ + std::string(".error");

  errorParam_ = ecmcAsynPort->addNewAvailParam(
                                          paramName.c_str(),     // name
                                          asynParamInt32,        // asyn type 
                                          (uint8_t*)&errorCode_, // pointer to data
                                          sizeof(errorCode_),    // size of data
                                          ECMC_EC_U32,           // ecmc data type
                                          0);                    // die if fail

  if(!errorParam_) {
    printf("ERROR: Failed create asyn param for data.");
    throw std::runtime_error( "ERROR: Failed create asyn param for data: " + paramName);
  }
  
  errorParam_->setAllowWriteToEcmc(false);  // need to callback here
  errorParam_->refreshParam(1); // read once into asyn param lib
  ecmcAsynPort->callParamCallbacks(ECMC_ASYN_DEFAULT_LIST, ECMC_ASYN_DEFAULT_ADDR);
}

// Avoid issues with std:to_string()
std::string ecmcCANOpenPDO::to_string(int value) {
  std::ostringstream os;
  os << value;
  return os.str();
}
