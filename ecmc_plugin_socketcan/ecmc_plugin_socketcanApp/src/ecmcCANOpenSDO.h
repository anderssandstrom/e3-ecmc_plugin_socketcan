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
                 int exeSampleTimeMs);
  ~ecmcCANOpenSDO();
  void execute();
  void newRxFrame(can_frame *frame);

 private:
  int frameEqual(can_frame *frame1,can_frame *frame2);

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
  can_frame reqDataFrame_;
  can_frame confReqFrameTg0_;
  can_frame confReqFrameTg1_;
  can_frame recConfRead_;

  int busy_;
  uint8_t *dataBuffer_;
  uint32_t recivedBytes_;
  int useTg1Frame_;
  ecmc_read_states readStates_;
  void printBuffer();
};

#endif  /* ECMC_CANOPEN_SDO_H_ */
