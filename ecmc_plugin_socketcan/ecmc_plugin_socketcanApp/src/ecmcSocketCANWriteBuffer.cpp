/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcSocketCANWriteBuffer.cpp
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
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
  memset(&buffer1_.frames,0,sizeof(struct can_frame)*ECMC_CAN_MAX_WRITE_CMDS);
  memset(&buffer2_.frames,0,sizeof(struct can_frame)*ECMC_CAN_MAX_WRITE_CMDS);
  bufferIdAddFrames_ = 1;  // start to add frames to buffer 1
  writeBusy_         = 0;
  socketId_          = socketId;
  cfgDbgMode_        = cfgDbgMode;
  destructs_         = 0;
  bufferSwitchMutex_ = epicsMutexCreate();
  lastWriteSumError_ = 0;
  
  writePauseTime_.tv_sec  = 0;
  writePauseTime_.tv_nsec = 2e6;  // 1ms
  buffer1_.frameCounter = 0;
  buffer2_.frameCounter = 0;

  bufferAdd_ = &buffer1_;
  bufferWrite_ = &buffer2_; 

  // Create worker thread for writing socket
  std::string threadname = "ecmc." ECMC_PLUGIN_ASYN_PREFIX".write";
  if(epicsThreadCreate(threadname.c_str(), 0, 32768, f_worker_write, this) == NULL) {
    throw std::runtime_error("Error: Failed create worker thread for write().");
  }
}

ecmcSocketCANWriteBuffer::~ecmcSocketCANWriteBuffer() {
  // kill worker
  destructs_ = 1;  // maybe need todo in other way..
}

// Write socket worker thread (switch between two buffers)
void ecmcSocketCANWriteBuffer::doWriteWorker() {
  while(true) {    
    if(destructs_) {
      return;
    }    

    nanosleep(&writePauseTime_,NULL);    

    if(writeBusy_) {      
      continue;
    }

    if(destructs_) {
      return;
    }

    writeBusy_ = 1;
    // Check if anything to write..
    if(bufferAdd_->frameCounter == 0) {
      writeBusy_ = 0;
      continue;
    }

    // Switch buffers and write!
    switchBuffer();
    
    writeBuffer();
    writeBusy_ = 0;
  }
}

int ecmcSocketCANWriteBuffer::addWriteCAN(can_frame *frame) {  
  // Cannot switch if busy..
  int errorCode = 0;
  epicsMutexLock(bufferSwitchMutex_);
  errorCode = addToBuffer(frame);
  epicsMutexUnlock(bufferSwitchMutex_);
  return errorCode;
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
  can_frame frame;
  frame.can_id  = canId;
  frame.can_dlc = len;     // data length
  frame.data[0] = data0;  // request read cmd
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  frame.data[5] = data5;
  frame.data[6] = data6;
  frame.data[7] = data7;
  return addWriteCAN(&frame);
}

int ecmcSocketCANWriteBuffer::getlastWritesError() {
  return lastWriteSumError_;
}

int ecmcSocketCANWriteBuffer::addToBuffer(can_frame *frame) {

  if(bufferAdd_->frameCounter >= ECMC_CAN_MAX_WRITE_CMDS) {
    return ECMC_CAN_ERROR_WRITE_FULL;
  }

  bufferAdd_->frames[bufferAdd_->frameCounter] = *frame;
  bufferAdd_->frameCounter++; 
  return 0;
}

//void ecmcSocketCANWriteBuffer::addToBuffer1(can_frame *frame) {
//  printf("addToBuffer1\n");
//  epicsMutexLock(bufferMutex1_);
//  buffer1_.frame[buffer1_.frameCounter] = *frame;
//  buffer1_.frameCounter++; 
//  epicsMutexUnlock(bufferMutex1_);
//}
//
//void ecmcSocketCANWriteBuffer::addToBuffer2(can_frame *frame) {
//  printf("addToBuffer2\n");
//  epicsMutexLock(bufferMutex2_);
//  buffer2_.frame[buffer2_.frameCounter] = *frame;
//  buffer2_.frameCounter++; 
//  epicsMutexUnlock(bufferMutex2_);
//}
//

int ecmcSocketCANWriteBuffer::writeBuffer() {

  int errorCode = 0;
  if(bufferWrite_->frameCounter==0) {
    return 0;
  }

  for(int i=0; i<bufferWrite_->frameCounter;i++) {
    errorCode = writeCAN(&bufferWrite_->frames[i]);
    if(errorCode) {
      lastWriteSumError_ = errorCode;
    }
  }
  bufferWrite_->frameCounter = 0;
  return lastWriteSumError_;
}

//void ecmcSocketCANWriteBuffer::writeBuffer1() {
//  //printf("writeBuffer1\n");
//  int errorCode = 0;
//  epicsMutexLock(bufferMutex1_);
//  if(buffer1_.frameCounter==0) {
//    return;
//  }
//  printf("writeBuffer1\n");
//  for(int i=0; i<buffer1_.frameCounter;i++) {
//    errorCode = writeCAN(&buffer1_.frame[i]);
//    if(errorCode) {
//      lastWriteSumError_ = errorCode;
//    }
//  }
//  buffer1_.frameCounter = 0;
//  epicsMutexUnlock(bufferMutex1_);  
//}
//
//void ecmcSocketCANWriteBuffer::writeBuffer2() {
//  
//  int errorCode = 0;
//  epicsMutexLock(bufferMutex2_);
//  if(buffer2_.frameCounter==0) {
//    return;
//  }
//  printf("writeBuffer2\n");
//
//  for(int i=0; i<buffer2_.frameCounter;i++) {
//    errorCode = writeCAN(&buffer2_.frame[i]);
//    if(errorCode) {
//      lastWriteSumError_ = errorCode;
//    }
//  }
//  buffer2_.frameCounter = 0;
//  epicsMutexUnlock(bufferMutex2_);
//}

int ecmcSocketCANWriteBuffer::switchBuffer() {

  // ensure safe buffer switch
  epicsMutexLock(bufferSwitchMutex_);
  canWriteBuffer *temp = bufferWrite_;
  bufferWrite_ = bufferAdd_;
  bufferAdd_   = temp;
  epicsMutexUnlock(bufferSwitchMutex_);
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
