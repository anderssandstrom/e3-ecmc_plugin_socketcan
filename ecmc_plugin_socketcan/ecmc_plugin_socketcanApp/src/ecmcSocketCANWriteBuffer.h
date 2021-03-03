/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcSocketCANWriteBuffer.h
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*
\*************************************************************************/
#ifndef ECMC_SOCKETCAN_BUFFER_WRITE_H_
#define ECMC_SOCKETCAN_BUFFER_WRITE_H_

#include "ecmcAsynPortDriver.h"

#include <stdexcept>
#include "inttypes.h"
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#define ECMC_CAN_MAX_WRITE_CMDS 128
#define ECMC_CAN_ERROR_WRITE_FULL 10
#define ECMC_CAN_ERROR_WRITE_BUSY 11
#define ECMC_CAN_ERROR_WRITE_NO_DATA 12
#define ECMC_CAN_ERROR_WRITE_INCOMPLETE 13

class ecmcSocketCANWriteBuffer {
 public:

  /** ecmc ecmcSocketCANWriteBuffer class
   * This object can throw: 
   *    - bad_alloc
   *    - invalid_argument
   *    - runtime_error
   *    - out_of_range
  */
  ecmcSocketCANWriteBuffer(int socketId, int dbgMode);
  ~ecmcSocketCANWriteBuffer();

  
  void doWriteWorker();
  
  int                   addWriteCAN(uint32_t canId,
                                    uint8_t len,
                                    uint8_t data0,
                                    uint8_t data1,
                                    uint8_t data2,
                                    uint8_t data3,
                                    uint8_t data4,
                                    uint8_t data5,
                                    uint8_t data6,
                                    uint8_t data7);
  int                   triggWrites();
  int                   getlastWritesError();

 private:
  static std::string    to_string(int value);
  int                   writeCAN(can_frame *frame);
  int                   destructs_;
  int                   cfgDbgMode_;
  epicsEvent            doWriteEvent_;
  int                   socketId_;
  struct can_frame      txmsgBuffer1_[ECMC_CAN_MAX_WRITE_CMDS];
  struct can_frame      txmsgBuffer2_[ECMC_CAN_MAX_WRITE_CMDS];
  int                   writeCmdCounter1_;
  int                   writeCmdCounter2_;
  int                   writeBusy_;
  int                   lastWriteSumError_;
  int                   bufferIdAddFrames_;
};

#endif  /* ECMC_SOCKETCAN_BUFFER_WRITE_H_ */
