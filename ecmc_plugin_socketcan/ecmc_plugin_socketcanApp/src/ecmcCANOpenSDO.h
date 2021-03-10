/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcCANOpenSDO.h
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*
\*************************************************************************/
#ifndef ECMC_CANOPEN_SDO_H_
#define ECMC_CANOPEN_SDO_H_

#include <stdexcept>
#include "ecmcDataItem.h"
#include "ecmcAsynPortDriver.h"
#include "ecmcSocketCANDefs.h"
#include "ecmcCANOpenSDO.h"
#include "inttypes.h"
#include <string>
#include "ecmcSocketCANWriteBuffer.h"

#include <linux/can.h>
#include <linux/can/raw.h>

#define ECMC_CAN_ERROR_SDO_WRITE_BUSY 110

#define ECMC_CAN_ERROR_SDO_TIMEOUT 111


class ecmcCANOpenSDO {
 public:
  ecmcCANOpenSDO(ecmcSocketCANWriteBuffer* writeBuffer,
                 uint32_t cobIdTx,    // 0x580 + CobId
                 uint32_t cobIdRx,    // 0x600 + Cobid
                 ecmc_can_direction rw,
                 uint16_t ODIndex,    // Object dictionary index
                 uint8_t ODSubIndex, // Object dictionary subindex
                 uint32_t ODSize,
                 int readSampleTimeMs,
                 int exeSampleTimeMs,
                 const char *name,
                 std::atomic_flag *ptrSdo1Lock,
                 int dbgMode);                
  ~ecmcCANOpenSDO();
  void execute();
  void newRxFrame(can_frame *frame);
  void setValue(uint8_t *data, size_t bytes);
  int writeValue();

 private:
  int frameEqual(can_frame *frame1,can_frame *frame2);
  int readDataStateMachine(can_frame *frame);
  int writeDataStateMachine(can_frame *frame);
  int writeNextDataToSlave(int useToggle);
  int writeWaitForDataConfFrame(int useToggle, can_frame *frame);
  int tryLock();
  int tryUnlock();
  ecmcSocketCANWriteBuffer *writeBuffer_;
  uint32_t cobIdRx_;   // with cobid
  uint32_t cobIdTx_;   // with cobid
  int readSampleTimeMs_;
  int exeSampleTimeMs_;
  ecmc_can_direction rw_;
  uint16_t ODIndex_;
  uint8_t ODSubIndex_;
  uint32_t ODSize_;
  ODLegthBytes ODLengthBytes_;
  ODIndexBytes ODIndexBytes_;
  int exeCounter_;
  can_frame readReqTransferFrame_;
  can_frame readConfReqFrameTg0_;
  can_frame readConfReqFrameTg1_;
  can_frame readSlaveConfFrame_;

  can_frame writeReqTransferFrame_;
  can_frame writeSlaveConfCmdFrame_;
  can_frame writeDataFrame_;
  can_frame writeConfReqFrameTg0_;
  can_frame writeConfReqFrameTg1_;
  
  int dbgMode_;
  int errorCode_;
  uint8_t *dataBuffer_;
  uint8_t *tempReadBuffer_;
  uint32_t recivedBytes_;
  int useTg1Frame_;
  ecmc_read_states readStates_;
  ecmc_write_states writeStates_;
  void printBuffer();
  uint32_t writtenBytes_;
  char *name_;
  epicsMutexId  dataMutex_;
  epicsMutexId  getLockMutex_;
  int busyCounter_;
  //std::atomic_flag *ptrSdo1Busy_;
  //std::atomic_flag busy_;
  std::atomic_flag *ptrSdo1Lock_;
  bool busy_;
};

#endif  /* ECMC_CANOPEN_SDO_H_ */
