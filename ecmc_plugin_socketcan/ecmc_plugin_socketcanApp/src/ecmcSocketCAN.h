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
#ifndef ECMC_FFT_H_
#define ECMC_FFT_H_

#include <stdexcept>
#include "ecmcDataItem.h"
#include "ecmcAsynPortDriver.h"
#include "ecmcSocketCANDefs.h"
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
            char* portName);
  ~ecmcSocketCAN();
  void doReadWorker();

  virtual asynStatus    writeInt32(asynUser *pasynUser, epicsInt32 value);
  virtual asynStatus    readInt32(asynUser *pasynUser, epicsInt32 *value);
  virtual asynStatus    readInt8Array(asynUser *pasynUser, epicsInt8 *value, 
                                      size_t nElements, size_t *nIn);
  virtual asynStatus    readFloat64(asynUser *pasynUser, epicsFloat64 *value);

  void                  connect();
  int                   getConnected();
  int                   writeCAN();   // Add args later
 private:
  void                  parseConfigStr(char *configStr);
  void                  initAsyn();
  static std::string    to_string(int value);
  char*                 cfgCanIFStr_;   // Config: can interface can0, vcan0..
  int                   cfgDbgMode_;
  int                   cfgAutoConnect_;
  int                   destructs_;
  int                   connected_;
  struct can_frame      rxmsg_;
  struct can_frame      txmsg_;
  struct ifreq          ifr_;
  int                   socketId_;
  struct sockaddr_can   addr_;
};

#endif  /* ECMC_FFT_H_ */
