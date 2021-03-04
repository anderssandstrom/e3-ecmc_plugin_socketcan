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
                               uint32_t cobIdTx,  // 0x580 + CobId
                               uint32_t cobIdRx,  // 0x600 + Cobid
                               ecmc_can_direction rw,
                               uint16_t ODIndex,
                               uint8_t ODSubIndex,
                               uint32_t ODSize,
                               int readSampleTimeMs, 
                               int exeSampleTimeMs,
                               int dbgMode) {

  writeBuffer_        = writeBuffer;
  cobIdRx_            = cobIdRx;
  cobIdTx_            = cobIdTx;
  ODIndex_            = ODIndex;
  ODSubIndex_         = ODSubIndex;
  ODSize_             = ODSize;
  dbgMode_            = dbgMode;
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
  writeStates_        = IDLE;
  useTg1Frame_        = 1;
  dataBuffer_         = new uint8_t(ODSize_);

  // Request data (send on slave RX)
  // w 0x603 [8] 0x40 0x40 0x26 0x00 0x00 0x00 0x00 0x00
  readReqTransferFrame_.can_id  = cobIdRx;
  readReqTransferFrame_.can_dlc = 8;     // data length
  readReqTransferFrame_.data[0] = 0x40;  // request read cmd
  readReqTransferFrame_.data[1] = ODIndexBytes_.byte0;
  readReqTransferFrame_.data[2] = ODIndexBytes_.byte1;
  readReqTransferFrame_.data[3] = ODSubIndex_;
  readReqTransferFrame_.data[4] = 0;
  readReqTransferFrame_.data[5] = 0;
  readReqTransferFrame_.data[6] = 0;
  readReqTransferFrame_.data[7] = 0;
  
  // Confirm Toggle 0
  // w 0x603 [8] 0x61 0x40 0x26 0x00 0x00 0x00 0x00 0x00
  readConfReqFrameTg0_.can_id  = cobIdRx;
  readConfReqFrameTg0_.can_dlc = 8;     // data length
  readConfReqFrameTg0_.data[0] = 0x61;  // confirm cmd toggle 0
  readConfReqFrameTg0_.data[1] = ODIndexBytes_.byte0;
  readConfReqFrameTg0_.data[2] = ODIndexBytes_.byte1;
  readConfReqFrameTg0_.data[3] = ODSubIndex_;
  readConfReqFrameTg0_.data[4] = 0;
  readConfReqFrameTg0_.data[5] = 0;
  readConfReqFrameTg0_.data[6] = 0;
  readConfReqFrameTg0_.data[7] = 0;
  
  // Confirm Toggle 1
  // w 0x603 [8] 0x71 0x40 0x26 0x00 0x00 0x00 0x00 0x00
  readConfReqFrameTg1_.can_id  = cobIdRx;
  readConfReqFrameTg1_.can_dlc = 8;     // data length
  readConfReqFrameTg1_.data[0] = 0x71;  // confirm cmd toggle 1
  readConfReqFrameTg1_.data[1] = ODIndexBytes_.byte0;
  readConfReqFrameTg1_.data[2] = ODIndexBytes_.byte1;
  readConfReqFrameTg1_.data[3] = ODSubIndex_;
  readConfReqFrameTg1_.data[4] = 0;
  readConfReqFrameTg1_.data[5] = 0;
  readConfReqFrameTg1_.data[6] = 0;
  readConfReqFrameTg1_.data[7] = 0;

  // Slave read confirm frame
  readSlaveConfFrame_.can_id  = cobIdTx;
  readSlaveConfFrame_.can_dlc = 8;     // data length
  readSlaveConfFrame_.data[0] = 0x41;  // confirm cmd toggle 1
  readSlaveConfFrame_.data[1] = ODIndexBytes_.byte0;
  readSlaveConfFrame_.data[2] = ODIndexBytes_.byte1;
  readSlaveConfFrame_.data[3] = ODSubIndex_;
  readSlaveConfFrame_.data[4] = ODLengthBytes_.byte0;
  readSlaveConfFrame_.data[5] = ODLengthBytes_.byte1;
  readSlaveConfFrame_.data[6] = ODLengthBytes_.byte2;
  readSlaveConfFrame_.data[7] = ODLengthBytes_.byte3;

  // Request write to slave
  writeReqTransferFrame_.can_id  = cobIdTx;
  writeReqTransferFrame_.can_dlc = 8;     // data length
  writeReqTransferFrame_.data[0] = 0x21;  // Write cmd
  writeReqTransferFrame_.data[1] = ODIndexBytes_.byte0;
  writeReqTransferFrame_.data[2] = ODIndexBytes_.byte1;
  writeReqTransferFrame_.data[3] = ODSubIndex_;
  writeReqTransferFrame_.data[4] = ODLengthBytes_.byte0;
  writeReqTransferFrame_.data[5] = ODLengthBytes_.byte1;
  writeReqTransferFrame_.data[6] = ODLengthBytes_.byte2;
  writeReqTransferFrame_.data[7] = ODLengthBytes_.byte3;

  // Slave write confirm frame
  writeSlaveConfFrame_.can_id  = cobIdTx;
  writeSlaveConfFrame_.can_dlc = 8;     // data length
  writeSlaveConfFrame_.data[0] = 0x60;  // confirm frame for write
  writeSlaveConfFrame_.data[1] = ODIndexBytes_.byte0;
  writeSlaveConfFrame_.data[2] = ODIndexBytes_.byte1;
  writeSlaveConfFrame_.data[3] = ODSubIndex_;
  writeSlaveConfFrame_.data[4] = 0;
  writeSlaveConfFrame_.data[5] = 0;
  writeSlaveConfFrame_.data[6] = 0;
  writeSlaveConfFrame_.data[7] = 0;
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
     
      readStates_ = READ_REQ_TRANSFER;
      if(dbgMode_) {
        printf("STATE = READ_REQ_TRANSFER\n");
      }
      // IMPORTANT!! LOCKLOCK!!!! LOCK all slave trafic while 0x583 and 0x603 for any other trafic while processing
      //initiate
      recivedBytes_ = 0;
      readStates_ = READ_WAIT_FOR_CONF;
      writeBuffer_->addWriteCAN(&readReqTransferFrame_);

      if(dbgMode_) {
        printf("STATE = READ_WAIT_FOR_CONF\n");
      }
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
     case READ_WAIT_FOR_CONF:
        // Compare to the conf frame.. might not always be correct
        if ( !frameEqual(&readSlaveConfFrame_,frame)) {
          return;
        }
        readStates_ = READ_WAIT_FOR_DATA; //Next frame should be data!
        if(dbgMode_) {
          printf("STATE = READ_WAIT_FOR_DATA\n");
        }
        writeBuffer_->addWriteCAN(&readConfReqFrameTg0_);  // Send tg0 frame and wait for data, also size must match to go ahead
        useTg1Frame_ = 1;
        break;

      case READ_WAIT_FOR_DATA:
        if(frame->can_id != cobIdTx_) {
          return; // not correct frame
        }
        //Add data to buffer
        if(frame->can_dlc-1 + recivedBytes_ <= ODSize_) {
          memcpy(dataBuffer_ + recivedBytes_, &(frame->data[1]),frame->can_dlc-1);
          recivedBytes_ += frame->can_dlc-1;
        }
        if(recivedBytes_ < ODSize_) {  // Ask for more data but must toggle so alternat the prepared frames
          if(useTg1Frame_) {
            writeBuffer_->addWriteCAN(&readConfReqFrameTg1_);
            useTg1Frame_ = 0;
          } else {
            writeBuffer_->addWriteCAN(&readConfReqFrameTg0_);
            useTg1Frame_ = 1;
          }
        }
        if(dbgMode_) {
          printf("recived bytes = %d\n",recivedBytes_);
        }
        
        if (recivedBytes_ == ODSize_) {
          readStates_ =IDLE;
          busy_ = 0;
          if(dbgMode_) {
            printf("All data transfered for SDO.\n");
            printBuffer();
          }
        }

        break;
      default:
        return;
        break;
    }
  }

  // Write
  else {
    switch(writeStates_) {
     case WRITE_WAIT_FOR_CONF:
        // Compare to the conf frame.. might not always be correct
        if ( !frameEqual(&writeSlaveConfFrame_,frame)) {
          return;
        }

        readStates_ = WRITE_DATA; //Next frame should be data!
        if(dbgMode_) {
          printf("STATE = READ_WAIT_FOR_DATA\n");
        }
        writeBuffer_->addWriteCAN(&readConfReqFrameTg0_);  // Send first data frame
        
        useTg1Frame_ = 1;
        break;

      case READ_WAIT_FOR_DATA:
        if(frame->can_id != cobIdTx_) {
          return; // not correct frame
        }
        //Add data to buffer
        if(frame->can_dlc-1 + recivedBytes_ <= ODSize_) {
          memcpy(dataBuffer_ + recivedBytes_, &(frame->data[1]),frame->can_dlc-1);
          recivedBytes_ += frame->can_dlc-1;
        }
        if(recivedBytes_ < ODSize_) {  // Ask for more data but must toggle so alternat the prepared frames
          if(useTg1Frame_) {
            writeBuffer_->addWriteCAN(&readConfReqFrameTg1_);
            useTg1Frame_ = 0;
          } else {
            writeBuffer_->addWriteCAN(&readConfReqFrameTg0_);
            useTg1Frame_ = 1;
          }
        }
        if(dbgMode_) {
          printf("recived bytes = %d\n",recivedBytes_);
        }
        
        if (recivedBytes_ == ODSize_) {
          readStates_ =IDLE;
          busy_ = 0;
          if(dbgMode_) {
            printf("All data transfered for SDO.\n");
            printBuffer();
          }
        }

        break;
      default:
        return;
        break;
    }
  }

}

int ecmcCANOpenSDO::frameEqual(can_frame *frame1,can_frame *frame2) {
  if(frame1->can_id  == frame2->can_id  &&
     frame1->can_dlc == frame2->can_dlc &&
     frame1->data[0] == frame2->data[0] &&
     frame1->data[1] == frame2->data[1] &&
     frame1->data[2] == frame2->data[2] &&
     frame1->data[3] == frame2->data[3] &&
     frame1->data[4] == frame2->data[4] &&
     frame1->data[5] == frame2->data[5] &&
     frame1->data[6] == frame2->data[6] &&
     frame1->data[7] == frame2->data[7]) {
    return 1;
  }
  return 0;
  //return memcmp(frame1,frame2, sizeof(can_frame)) == 0; Why not working, union??!?
}

void ecmcCANOpenSDO::printBuffer() {
  if(!dataBuffer_) {
    return;
  }

  for(uint32_t i = 0; i < ODSize_; i = i + 2) {
    uint16_t test;
    memcpy(&test,&dataBuffer_[i],2);
    printf("data[%02d]: %u\n",i/2,test);
  }
}

void ecmcCANOpenSDO::setValue(uint8_t *data, size_t bytes) {
  int bytesToCopy = bytes;
  if(ODSize_ < bytes) {
   bytesToCopy = ODSize_;
  }
  if(bytesToCopy == 0) {
    return;
  }
  memcpy(dataBuffer_, &data, ODSize_);
}

int ecmcCANOpenSDO::writeValue() {
  // Busy right now!
  if(busy_ || writeStates_ != IDLE ) {
    return ECMC_CAN_ERROR_SDO_WRITE_BUSY;
  }
  busy_ = 1;

  writeStates_ = WRITE_REQ_TRANSFER;
  if(dbgMode_) {
    printf("STATE = WRITE_REQ_TRANSFER\n");
  }

  writeBuffer_->addWriteCAN(&writeReqTransferFrame_);

  writeStates_ = WRITE_WAIT_FOR_CONF;
  if(dbgMode_) {
    printf("STATE = WRITE_WAIT_FOR_CONF\n");
  }
  // State machine is now in rx frame()
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
