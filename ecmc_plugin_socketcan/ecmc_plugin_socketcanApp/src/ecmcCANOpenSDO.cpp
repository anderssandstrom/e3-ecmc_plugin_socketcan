/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcCANOpenSDO.cpp
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*      Credits to  https://github.com/sgreg/dynamic-loading 
*
\*************************************************************************/

// Needed to get headers in ecmc right...
#define ECMC_IS_PLUGIN

#include <sstream>
#include "ecmcCANOpenSDO.h"

/** 
 * ecmc ecmcCANOpenSDO class
*/
ecmcCANOpenSDO::ecmcCANOpenSDO(ecmcSocketCANWriteBuffer* writeBuffer,
                               uint32_t SDOSlaveTXId,  // 0x580 + CobId
                               uint32_t SDOSlaveRXId,  // 0x600 + Cobid
                               ecmc_can_direction rw,
                               uint16_t ODIndex,
                               uint8_t ODSubIndex,
                               uint32_t ODSize,
                               int readSampleTimeMs, 
                               int exeSampleTimeMs) {

  writeBuffer_        = writeBuffer;
  SDOSlaveRXId_       = SDOSlaveRXId;
  SDOSlaveTXId_       = SDOSlaveTXId;
  ODIndex_            = ODIndex;
  ODSubIndex_         = ODSubIndex;
  ODSize_             = ODSize;
  // convert to ODIndex_ to indiviual bytes struct
  memcpy(&ODIndexBytes_, &ODIndex, 2);
  memcpy(&ODLengthBytes_, &ODSize_, 4);
  
  readSampleTimeMs_   = readSampleTimeMs;
  exeSampleTimeMs_    = exeSampleTimeMs;
  rw_                 = rw;
  exeCounter_         = 0;
  busy_               = 0;
  recivedBytes_       = 0;
  readStates_         = IDLE;
  useTg1Frame_        = 1;
  dataBuffer_         = new uint8_t(ODSize_);

  // Request data (send on slave RX)
  // w 0x603 [8] 0x40 0x40 0x26 0x00 0x00 0x00 0x00 0x00
  reqDataFrame_.can_id  = SDOSlaveRXId;
  reqDataFrame_.can_dlc = 8;     // data length
  reqDataFrame_.data[0] = 0x40;  // request read cmd
  reqDataFrame_.data[1] = ODIndexBytes_.byte0;
  reqDataFrame_.data[2] = ODIndexBytes_.byte1;
  reqDataFrame_.data[3] = ODSubIndex_;
  reqDataFrame_.data[4] = 0;
  reqDataFrame_.data[5] = 0;
  reqDataFrame_.data[6] = 0;
  reqDataFrame_.data[7] = 0;
  
  // Confirm Toggle 0
  // w 0x603 [8] 0x61 0x40 0x26 0x00 0x00 0x00 0x00 0x00
  confReqFrameTg0_.can_id  = SDOSlaveRXId;
  confReqFrameTg0_.can_dlc = 8;     // data length
  confReqFrameTg0_.data[0] = 0x61;  // confirm cmd toggle 0
  confReqFrameTg0_.data[1] = ODIndexBytes_.byte0;
  confReqFrameTg0_.data[2] = ODIndexBytes_.byte1;
  confReqFrameTg0_.data[3] = ODSubIndex_;
  confReqFrameTg0_.data[4] = 0;
  confReqFrameTg0_.data[5] = 0;
  confReqFrameTg0_.data[6] = 0;
  confReqFrameTg0_.data[7] = 0;
  
  // Confirm Toggle 1
  // w 0x603 [8] 0x71 0x40 0x26 0x00 0x00 0x00 0x00 0x00
  confReqFrameTg1_.can_id  = SDOSlaveRXId;
  confReqFrameTg1_.can_dlc = 8;     // data length
  confReqFrameTg1_.data[0] = 0x71;  // confirm cmd toggle 1
  confReqFrameTg1_.data[1] = ODIndexBytes_.byte0;
  confReqFrameTg1_.data[2] = ODIndexBytes_.byte1;
  confReqFrameTg1_.data[3] = ODSubIndex_;
  confReqFrameTg1_.data[4] = 0;
  confReqFrameTg1_.data[5] = 0;
  confReqFrameTg1_.data[6] = 0;
  confReqFrameTg1_.data[7] = 0;

  recConfRead_.can_id  = SDOSlaveTXId;
  recConfRead_.can_dlc = 8;     // data length
  recConfRead_.data[0] = 0x41;  // confirm cmd toggle 1
  recConfRead_.data[1] = ODIndexBytes_.byte0;
  recConfRead_.data[2] = ODIndexBytes_.byte1;
  recConfRead_.data[3] = ODSubIndex_;
  recConfRead_.data[4] = ODLengthBytes_.byte0;
  recConfRead_.data[5] = ODLengthBytes_.byte1;
  recConfRead_.data[6] = ODLengthBytes_.byte2;
  recConfRead_.data[7] = ODLengthBytes_.byte3 ;
}

ecmcCANOpenSDO::~ecmcCANOpenSDO() {
  delete[] dataBuffer_;
}

void ecmcCANOpenSDO::execute() {

  exeCounter_++;
  if(exeCounter_* exeSampleTimeMs_ >= readSampleTimeMs_ && ! busy_) {

    exeCounter_ =0;
    if(rw_ == DIR_READ) {
      busy_ = 1;
      // IMPORTANT!! LOCKLOCK!!!! LOCK all slave trafic while 0x583 and 0x603 for any other trafic while processing
      //initiate
      recivedBytes_ = 0;
      readStates_ = WAIT_FOR_REQ_CONF;
      printf("readStates_ = WAIT_FOR_REQ_CONF!!!\n");

      writeBuffer_->addWriteCAN(&reqDataFrame_);
      writeBuffer_->triggWrites();
    }
  }
}

// new rx frame recived!
void ecmcCANOpenSDO::newRxFrame(can_frame *frame) {
  // Wait for:
  // # r 0x583 [8] 0x41 0x40 0x26 0x00 0x38 0x00 0x00 0x00
  if(!busy_) {
    // Not waiting for any data
    return;
  }
  if(rw_ == DIR_READ) {
    switch(readStates_) {
     case WAIT_FOR_REQ_CONF:
        // Compare to the conf frame.. might not always be correct
        if ( !frameEqual(&recConfRead_,frame)) {
          printf("frame not equal\n");
          // Not "my frame", wait for new
          return;
        }
        readStates_ = WAIT_FOR_DATA; //Next frame should be data!
        printf("readStates_ = WAIT_FOR_DATA!!!\n");
        writeBuffer_->addWriteCAN(&confReqFrameTg0_);  // Send tg0 frame and wait for data, also size must match to go ahead
        writeBuffer_->triggWrites();

        useTg1Frame_ = 1;
        break;

      case WAIT_FOR_DATA:
        if(frame->can_id != SDOSlaveTXId_) {
          return; // not correct frame
        }
        //Add data to buffer
        if(frame->can_dlc + recivedBytes_ <= ODSize_) {
          memcpy(dataBuffer_ + recivedBytes_, &(frame->data[0]),frame->can_dlc);
          recivedBytes_ += frame->can_dlc;
        }
        if(recivedBytes_ < ODSize_) {  // Ask for more data but must toggle so alternat the prepared frames
          if(useTg1Frame_) {
            writeBuffer_->addWriteCAN(&confReqFrameTg1_);
            useTg1Frame_ = 0;
          } else {
            writeBuffer_->addWriteCAN(&confReqFrameTg0_);
            useTg1Frame_ = 1;
          }
          writeBuffer_->triggWrites();
        }
        printf("recivedBytes = %d!!!\n",recivedBytes_);
        
        if (recivedBytes_ == ODSize_) {
          readStates_ =IDLE;
          busy_ = 0;
          printf("All data transfered");
        }

        break;
      default:
        return;
        break;
    }
  }
}

int ecmcCANOpenSDO::frameEqual(can_frame *frame1,can_frame *frame2) {
  return memcmp(frame1,frame2, sizeof(can_frame)) == 0;
}

//# w 0x603 [8] 0x40 0x40 0x26 0x00 0x00 0x00 0x00 0x00
//# w 0x603 [8] 0x61 0x40 0x26 0x00 0x00 0x00 0x00 0x00
//# w 0x603 [8] 0x71 0x40 0x26 0x00 0x00 0x00 0x00 0x00
//# w 0x603 [8] 0x61 0x40 0x26 0x00 0x00 0x00 0x00 0x00
//# w 0x603 [8] 0x71 0x40 0x26 0x00 0x00 0x00 0x00 0x00
//# w 0x603 [8] 0x61 0x40 0x26 0x00 0x00 0x00 0x00 0x00
//# w 0x603 [8] 0x71 0x40 0x26 0x00 0x00 0x00 0x00 0x00
//# w 0x603 [8] 0x61 0x40 0x26 0x00 0x00 0x00 0x00 0x00
//# 
//# 0x38 bytes to recive!!
//# r 0x583 [8] 0x41 0x40 0x26 0x00 0x38 0x00 0x00 0x00
//# Data below
//# r 0x583 [8] 0x00 0x18 0x00 0x24 0x00 0x0E 0x00 0x0A
//# r 0x583 [8] 0x10 0x00 0x00 0x00 0xBF 0x00 0x00 0x00
//# r 0x583 [8] 0x00 0xC2 0x01 0x00 0x00 0x00 0x00 0x35
//# r 0x583 [8] 0x10 0x1C 0x84 0x02 0x46 0x1A 0x3C 0x49
//# r 0x583 [8] 0x00 0xC2 0x01 0x00 0x00 0x00 0x00 0x00
//# r 0x583 [8] 0x10 0x00 0xC8 0x48 0x51 0x2F 0x00 0x00
//# r 0x583 [8] 0x00 0x5C 0x2D 0x81 0x14 0x67 0x0D 0xA6
//