/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcSocketCAN.h
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*
\*************************************************************************/
#ifndef ECMC_SOCKETCAN_H_
#define ECMC_SOCKETCAN_H_

#include <stdexcept>
#include "ecmcDataItem.h"
#include "ecmcAsynPortDriver.h"
#include "ecmcSocketCANDefs.h"
#include "ecmcSocketCANWriteBuffer.h"
#include "ecmcCANOpenSDO.h"
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
#define ECMC_CAN_ERROR_WRITE_BUFFER_NULL 14

class ecmcSocketCAN : public asynPortDriver {
 public:

  /** ecmc ecmcSocketCAN class
   * This object can throw: 
   *    - bad_alloc
   *    - invalid_argument
   *    - runtime_error
   *    - out_of_range
  */
  ecmcSocketCAN(char* configStr,
                char* portName,
                int exeSampelTimeMs);
  ~ecmcSocketCAN();

  void doReadWorker();
  void doWriteWorker();
  void doConnectWorker();

  virtual asynStatus    writeInt32(asynUser *pasynUser, epicsInt32 value);
  virtual asynStatus    readInt32(asynUser *pasynUser, epicsInt32 *value);
  virtual asynStatus    readInt8Array(asynUser *pasynUser, epicsInt8 *value, 
                                      size_t nElements, size_t *nIn);
  virtual asynStatus    readFloat64(asynUser *pasynUser, epicsFloat64 *value);
  void                  connectExternal();  
  int                   getConnected();
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
  int                   getlastWritesError();
  void                  execute();  // ecmc rt loop

 private:
  void                  parseConfigStr(char *configStr);
  void                  initAsyn();
  static std::string    to_string(int value);
  void                  connectPrivate();
  int                   writeCAN(can_frame *frame);
  char*                 cfgCanIFStr_;   // Config: can interface can0, vcan0..
  int                   cfgDbgMode_;
  int                   cfgAutoConnect_;
  int                   destructs_;
  int                   connected_;
  epicsEvent            doConnectEvent_;
  epicsEvent            doWriteEvent_;
  struct can_frame      rxmsg_;
  struct ifreq          ifr_;
  int                   socketId_;
  struct sockaddr_can   addr_;
  struct can_frame      txmsgBuffer_[ECMC_CAN_MAX_WRITE_CMDS];
  int                   writeCmdCounter_;
  int                   writeBusy_;
  int                   lastWriteSumError_;
  int                   exeSampleTimeMs_;
  
  ecmcSocketCANWriteBuffer *writeBuffer_;
  ecmcCANOpenSDO *testSdo_;
};

#endif  /* ECMC_SOCKETCAN_H_ */
