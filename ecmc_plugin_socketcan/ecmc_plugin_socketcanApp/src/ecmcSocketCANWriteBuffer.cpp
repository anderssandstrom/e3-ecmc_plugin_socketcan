/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcSocketCANWriteBuffer.cpp
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*      Credits to  https://github.com/sgreg/dynamic-loading 
*
\*************************************************************************/

// Needed to get headers in ecmc right...
#define ECMC_IS_PLUGIN

#include <sstream>
#include "ecmcSocketCANWriteBuffer.h"
#include "epicsThread.h"

#define ECMC_PLUGIN_ASYN_PREFIX      "plugin.can"


// Start worker for socket read()
void f_worker_write(void *obj) {
  if(!obj) {
    printf("%s/%s:%d: Error: Worker write thread ecmcSocketCANWriteBuffer object NULL..\n",
            __FILE__, __FUNCTION__, __LINE__);
    return;
  }
  ecmcSocketCANWriteBuffer * canObj = (ecmcSocketCANWriteBuffer*)obj;
  canObj->doWriteWorker();
}

/** ecmc ecmcSocketCANWriteBuffer class
*/
ecmcSocketCANWriteBuffer::ecmcSocketCANWriteBuffer(int socketId, int cfgDbgMode) {
  memset(&txmsgBuffer1_,0,sizeof(struct can_frame)*ECMC_CAN_MAX_WRITE_CMDS);
  memset(&txmsgBuffer2_,0,sizeof(struct can_frame)*ECMC_CAN_MAX_WRITE_CMDS);
  bufferIdAddFrames_ = 1;  // start to add frames to buffer 1
  writeCmdCounter1_  = 0;
  writeCmdCounter2_  = 0;
  writeBusy_         = 0;
  socketId_          = socketId;
  cfgDbgMode_        = cfgDbgMode;
  destructs_         = 0;


  // Create worker thread for writing socket
  std::string threadname = "ecmc." ECMC_PLUGIN_ASYN_PREFIX".write";
  if(epicsThreadCreate(threadname.c_str(), 0, 32768, f_worker_write, this) == NULL) {
    throw std::runtime_error("Error: Failed create worker thread for write().");
  }

}

ecmcSocketCANWriteBuffer::~ecmcSocketCANWriteBuffer() {
  // kill worker
  destructs_ = 1;  // maybe need todo in other way..
  doWriteEvent_.signal();
}

// Write socket worker thread (switch between two buffers)
void ecmcSocketCANWriteBuffer::doWriteWorker() {
  int errorCode = 0;
  while(true) {    
    if(destructs_) {
      return;
    }

    doWriteEvent_.wait();
    if(destructs_) {
      return;
    }
    if(bufferIdAddFrames_ == 1) {
      // Write buffer 2 since addFrames to buffer 1
      for(int i=0; i<writeCmdCounter2_;i++) {
        errorCode = writeCAN(&txmsgBuffer2_[i]);
        if(errorCode) {
          lastWriteSumError_ = errorCode;
        }
      }
      writeCmdCounter2_ = 0;
    } else {
      // Write buffer 1 since addFrames to buffer 2
      for(int i=0; i<writeCmdCounter1_;i++) {
        errorCode = writeCAN(&txmsgBuffer1_[i]);
        if(errorCode) {
          lastWriteSumError_ = errorCode;
        }
      }
      writeCmdCounter1_ = 0;
    }
      
    writeBusy_ = 0;
  }
}

// Test can write function (simple if for plc func)
int ecmcSocketCANWriteBuffer::addWriteCAN( uint32_t canId,
                                           uint8_t len,
                                           uint8_t data0,
                                           uint8_t data1,
                                           uint8_t data2,
                                           uint8_t data3,
                                           uint8_t data4,
                                           uint8_t data5,
                                           uint8_t data6,
                                           uint8_t data7) {
  
  // Cannot switch if busy..  
  if(writeBusy_) {
    if(bufferIdAddFrames_ == 1 && writeCmdCounter1_ >= ECMC_CAN_MAX_WRITE_CMDS){
      return ECMC_CAN_ERROR_WRITE_FULL;
    }
    if(bufferIdAddFrames_ == 2 && writeCmdCounter2_ >= ECMC_CAN_MAX_WRITE_CMDS){
      return ECMC_CAN_ERROR_WRITE_FULL;
    }
  } else {  // switch buffer if full
    if(bufferIdAddFrames_ == 1 && writeCmdCounter1_ >= ECMC_CAN_MAX_WRITE_CMDS){
      triggWrites();  // will also switch buffer id
    } 
    if(bufferIdAddFrames_ == 2 && writeCmdCounter2_ >= ECMC_CAN_MAX_WRITE_CMDS){
      triggWrites();  // will also switch buffer id
    } 
  }  

  if(bufferIdAddFrames_ == 1){
    txmsgBuffer1_[writeCmdCounter1_].can_id  = canId;
    txmsgBuffer1_[writeCmdCounter1_].can_dlc = len;
    txmsgBuffer1_[writeCmdCounter1_].data[0] = data0;
    txmsgBuffer1_[writeCmdCounter1_].data[1] = data1;
    txmsgBuffer1_[writeCmdCounter1_].data[2] = data2;
    txmsgBuffer1_[writeCmdCounter1_].data[3] = data3;
    txmsgBuffer1_[writeCmdCounter1_].data[4] = data4;
    txmsgBuffer1_[writeCmdCounter1_].data[5] = data5;
    txmsgBuffer1_[writeCmdCounter1_].data[6] = data6;
    txmsgBuffer1_[writeCmdCounter1_].data[7] = data7;
    writeCmdCounter1_++;
  }
  else {
    txmsgBuffer2_[writeCmdCounter2_].can_id  = canId;
    txmsgBuffer2_[writeCmdCounter2_].can_dlc = len;
    txmsgBuffer2_[writeCmdCounter2_].data[0] = data0;
    txmsgBuffer2_[writeCmdCounter2_].data[1] = data1;
    txmsgBuffer2_[writeCmdCounter2_].data[2] = data2;
    txmsgBuffer2_[writeCmdCounter2_].data[3] = data3;
    txmsgBuffer2_[writeCmdCounter2_].data[4] = data4;
    txmsgBuffer2_[writeCmdCounter2_].data[5] = data5;
    txmsgBuffer2_[writeCmdCounter2_].data[6] = data6;
    txmsgBuffer2_[writeCmdCounter2_].data[7] = data7;
    writeCmdCounter2_++;
  }
  return 0;
}

int ecmcSocketCANWriteBuffer::getlastWritesError() {
  return lastWriteSumError_;
}  

// Trigger all writes
int ecmcSocketCANWriteBuffer::triggWrites() {
  
  if(writeBusy_) {
    return ECMC_CAN_ERROR_WRITE_BUSY;
  }

  if(bufferIdAddFrames_ == 1) {
    bufferIdAddFrames_ = 2;
  } else {
    bufferIdAddFrames_ = 1;
  }
  writeBusy_ = 1;
  lastWriteSumError_ = 0;
  doWriteEvent_.signal(); // let worker start
  return 0;
}

// Write to socket
int ecmcSocketCANWriteBuffer::writeCAN(can_frame *frame){

  if(!frame) {
    return ECMC_CAN_ERROR_WRITE_NO_DATA;
  }

  // Maybe need to add the size to write here.. if struct is not full, hmm?!
  int nbytes = write(socketId_, frame, sizeof(struct can_frame));
  if (nbytes!= sizeof(struct can_frame)) {
    return ECMC_CAN_ERROR_WRITE_INCOMPLETE;
  }

  if(cfgDbgMode_) {	  
    // Simulate candump printout
    printf("w 0x%03X", frame->can_id);
    printf(" [%d]", frame->can_dlc);
    for(int i=0; i<frame->can_dlc; i++ ) {
      printf(" 0x%02X", frame->data[i]);
    }
    printf("\n");
  }
  return 0;
}

// Avoid issues with std:to_string()
std::string ecmcSocketCANWriteBuffer::to_string(int value) {
  std::ostringstream os;
  os << value;
  return os.str();
}
