/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcCANOpenPDO.h
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*
\*************************************************************************/
#ifndef ECMC_CANOPEN_PDO_H_
#define ECMC_CANOPEN_PDO_H_

#include <stdexcept>
#include "ecmcDataItem.h"
#include "ecmcAsynPortDriver.h"
#include "ecmcSocketCANDefs.h"
#include "ecmcCANOpenPDO.h"
#include "inttypes.h"
#include <string>
#include "ecmcSocketCANWriteBuffer.h"
#include "epicsMutex.h"

#include <linux/can.h>
#include <linux/can/raw.h>

#define ECMC_CAN_ERROR_PDO_TIMEOUT 100

class ecmcCANOpenPDO {
 public:
  ecmcCANOpenPDO(ecmcSocketCANWriteBuffer* writeBuffer,
                 uint32_t cobId,
                 ecmc_can_direction rw,
                 uint32_t ODSize,
                 int readTimeoutMs,
                 int writeCycleMs,   //if <0 the write on demand..
                 int exeSampleTimeMs,
                 int dbgMode);
  ~ecmcCANOpenPDO();
  void execute();
  void newRxFrame(can_frame *frame);
  void setValue(uint64_t data);
  int  writeValue();
  
 private:
  int validateFrame(can_frame *frame);
  ecmcSocketCANWriteBuffer *writeBuffer_;
  uint32_t cobId_;   // with cobid
  int readTimeoutMs_;
  int writeCycleMs_;
  int exeSampleTimeMs_;
  ecmc_can_direction rw_;
  uint32_t ODSize_;
  int exeCounter_;
  int busy_;
  uint8_t *dataBuffer_;
  int errorCode_;
  void printBuffer();
  int dbgMode_;
  can_frame writeFrame_;
  epicsMutexId  dataMutex_;

};

#endif  /* ECMC_CANOPEN_PDO_H_ */
