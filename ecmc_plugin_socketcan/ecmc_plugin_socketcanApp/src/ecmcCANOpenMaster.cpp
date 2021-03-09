/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcCANOpenMaster.cpp
*
*  Created on: Mar 09, 2021
*      Author: anderssandstrom
*
\*************************************************************************/

// Needed to get headers in ecmc right...
#define ECMC_IS_PLUGIN

#include <sstream>
#include "ecmcCANOpenMaster.h"

/** 
 * ecmc ecmcCANOpenMaster class
*/
ecmcCANOpenMaster::ecmcCANOpenMaster(ecmcSocketCANWriteBuffer* writeBuffer,
                                     uint32_t nodeId,
                                     int exeSampleTimeMs,
                                     const char* name,
                                     int dbgMode): 
                   ecmcCANOpenDevice(writeBuffer,
                                     nodeId,  // 0x580 + CobId
                                     exeSampleTimeMs,
                                     name,
                                     dbgMode) {
  lssPdo_   = NULL;
  syncPdo_  = NULL;
  heartPdo_ = NULL;
  int errorCode = 0;
  
  // lssPdo_ = new ecmcCANOpenPDO( writeBuffer_, 0x7E5,DIR_WRITE,0,0,1000,exeSampleTimeMs_,"lss", cfgDbgMode_);
  errorCode = addPDO(0x7E5,       // uint32_t cobId,
                     DIR_WRITE,   // ecmc_can_direction rw,                     
                     0,           // uint32_t ODSize,
                     0,           // int readTimeoutMs,
                     1000,        // int writeCycleMs, if < 0 then write on demand.
                     "lss");      // const char* name);
  if(errorCode) {
    throw std::runtime_error( "LSS PDO NULL.");
  }
  lssPdo_ = pdos_[pdoCounter_-1];
  
  // Test sync signal
  // can0  0x80   [0]
  // syncPdo_ = new ecmcCANOpenPDO( writeBuffer_, 0x80,DIR_WRITE,0,0,1000,exeSampleTimeMs_,"sync", cfgDbgMode_);
  errorCode = addPDO(0x80,       // uint32_t cobId,
                     DIR_WRITE,   // ecmc_can_direction rw,                     
                     0,           // uint32_t ODSize,
                     0,           // int readTimeoutMs,
                     1000,        // int writeCycleMs, if < 0 then write on demand.
                     "sync");      // const char* name);

  if(errorCode) {
    throw std::runtime_error( "Sync PDO NULL.");
  }
  syncPdo_ = pdos_[pdoCounter_-1];

  // Test heartbeat signal
  // can0  0x701   [1]  05
  //can_add_write(1793,1,5,0,0,0,0,0,0,0);
  //heartPdo_ = new ecmcCANOpenPDO( writeBuffer_, 0x701,DIR_WRITE,1,0,1000,exeSampleTimeMs_,"heartbeat",cfgDbgMode_);
  //heartPdo_->setValue(5);
  errorCode = addPDO(0x700+nodeId_,       // uint32_t cobId,
                     DIR_WRITE,   // ecmc_can_direction rw,                     
                     1,           // uint32_t ODSize,
                     0,           // int readTimeoutMs,
                     1000,        // int writeCycleMs, if < 0 then write on demand.
                     "heart");    // const char* name);
  if(errorCode) {
    throw std::runtime_error( "Heart PDO NULL.");
  }
  heartPdo_ = pdos_[pdoCounter_-1];
  heartPdo_->setValue(5);
}

ecmcCANOpenMaster::~ecmcCANOpenMaster() {
}
