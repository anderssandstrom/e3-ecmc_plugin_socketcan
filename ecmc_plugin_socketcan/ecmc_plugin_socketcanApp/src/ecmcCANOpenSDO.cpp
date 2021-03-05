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

#define ECMC_SDO_TRANSFER_MAX_BYTES 7

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
  writtenBytes_       = 0; 
  readStates_         = READ_IDLE;
  writeStates_        = WRITE_IDLE;
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
  writeSlaveConfCmdFrame_.can_id  = cobIdRx;
  writeSlaveConfCmdFrame_.can_dlc = 8;     // data length
  writeSlaveConfCmdFrame_.data[0] = 0x60;  // confirm frame for write
  writeSlaveConfCmdFrame_.data[1] = ODIndexBytes_.byte0;
  writeSlaveConfCmdFrame_.data[2] = ODIndexBytes_.byte1;
  writeSlaveConfCmdFrame_.data[3] = ODSubIndex_;
  writeSlaveConfCmdFrame_.data[4] = 0;
  writeSlaveConfCmdFrame_.data[5] = 0;
  writeSlaveConfCmdFrame_.data[6] = 0;
  writeSlaveConfCmdFrame_.data[7] = 0;

  // Data frame base
  writeDataFrame_.can_id  = cobIdTx;
  writeDataFrame_.can_dlc = 8;     // data length
  writeDataFrame_.data[0] = 0;     // need to toggle here
  writeDataFrame_.data[1] = 0;
  writeDataFrame_.data[2] = 0;
  writeDataFrame_.data[3] = 0;
  writeDataFrame_.data[4] = 0;
  writeDataFrame_.data[5] = 0;
  writeDataFrame_.data[6] = 0;
  writeDataFrame_.data[7] = 0;

  // Slave write confirm frame TG0
  writeConfReqFrameTg0_.can_id  = cobIdRx;
  writeConfReqFrameTg0_.can_dlc = 8;     // data length
  writeConfReqFrameTg0_.data[0] = 0x20;  // Toggle 0
  writeConfReqFrameTg0_.data[1] = 0;
  writeConfReqFrameTg0_.data[2] = 0;
  writeConfReqFrameTg0_.data[3] = 0;
  writeConfReqFrameTg0_.data[4] = 0;
  writeConfReqFrameTg0_.data[5] = 0;
  writeConfReqFrameTg0_.data[6] = 0;
  writeConfReqFrameTg0_.data[7] = 0;

  // Slave write confirm frame TG1
  writeConfReqFrameTg1_.can_id  = cobIdRx;
  writeConfReqFrameTg1_.can_dlc = 8;     // data length
  writeConfReqFrameTg1_.data[0] = 0x30;  // Toggle 1
  writeConfReqFrameTg1_.data[1] = 0;
  writeConfReqFrameTg1_.data[2] = 0;
  writeConfReqFrameTg1_.data[3] = 0;
  writeConfReqFrameTg1_.data[4] = 0;
  writeConfReqFrameTg1_.data[5] = 0;
  writeConfReqFrameTg1_.data[6] = 0;
  writeConfReqFrameTg1_.data[7] = 0;  
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
  int errorCode = 0;
  if(!busy_) {
    // Not waiting for any data..
    return;
  }

  // Esnure that frame is from slave
  if(frame->can_id != cobIdTx_) {
    return; // not correct frame NEED MORE CHECKS HERE!! Ensure correct and not error frame
  }

  if(rw_ == DIR_READ) {
    errorCode = readDataStateMachine(frame);
  }  
  else { // Write
    errorCode = writeDataStateMachine(frame);
  }
}

int ecmcCANOpenSDO::readDataStateMachine(can_frame *frame) {
  int bytesToRead = 0;
  switch(readStates_) {
    case READ_WAIT_FOR_CONF:
      // Compare to the conf frame.. might not always be correct  NEED MORE CHECKS HERE!!
      if (!frameEqual(&readSlaveConfFrame_,frame)) {
        return 0;
      }
      readStates_ = READ_WAIT_FOR_DATA; //Next frame should be data!
      if(dbgMode_) {
        printf("STATE = READ_WAIT_FOR_DATA\n");
      }
      writeBuffer_->addWriteCAN(&readConfReqFrameTg0_);  // Send tg0 frame and wait for data, also size must match to go ahead
      useTg1Frame_ = 1;
      break;
    case READ_WAIT_FOR_DATA:
      //Add data to buffer
      bytesToRead = frame->can_dlc-1;
      if( bytesToRead > ECMC_SDO_TRANSFER_MAX_BYTES) {
        bytesToRead = ECMC_SDO_TRANSFER_MAX_BYTES;
      }
      
      if(bytesToRead + recivedBytes_ <= ODSize_) {
        memcpy(dataBuffer_ + recivedBytes_, &(frame->data[1]),bytesToRead);
        recivedBytes_ += bytesToRead;
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
      
      if (recivedBytes_ >= ODSize_) {
        readStates_ =READ_IDLE;
        busy_ = 0;
        useTg1Frame_ = 0;
        if(dbgMode_) {
          printf("STATE = READ_IDLE\n");
          printf("All data read from slave SDO.\n");
          printBuffer();            
        }
        return 0;
      }
      break;
    default:
      return 0;
      break;
  }
  return 0;
}

int ecmcCANOpenSDO::writeDataStateMachine(can_frame *frame) {
  int bytes = 0;
  switch(writeStates_) {
    case WRITE_WAIT_FOR_CONF:
      
      writtenBytes_ = 0;
      useTg1Frame_ = 0;
      // Compare to the conf frame.. might not always be correct MORE TESTS NEEDED HERE!!!!
      if ( !frameEqual(&writeSlaveConfCmdFrame_,frame)) {
        return 0;
      }

      writeNextDataToSlave(useTg1Frame_);
      writeStates_ = WRITE_DATA; //Next frame should be data!
      if(dbgMode_) {
        printf("STATE = WRITE_DATA\n");
      }

      break;

    case WRITE_DATA:

      // Wait for writeConfReqFrameTgX_ from from slave (toggle X = 1 or 0)
      if(!writeWaitForDataConfFrame(useTg1Frame_, frame)) {
          return 0;
      }

      // next frame use the other toggle option
      useTg1Frame_ = !useTg1Frame_;
      bytes = writeNextDataToSlave(useTg1Frame_);

      if(writtenBytes_ >= ODSize_) {
        writeStates_ = WRITE_IDLE;
        useTg1Frame_ = 0;
        busy_ = 0;
        if(dbgMode_) {
          printf("STATE = WRITE_IDLE\n");
          printf("All data written to slave SDO.\n");
          printBuffer();          
        }
        return 0;
      }
      break;
    default:
      return 0;
      break;
  }
  return 0;
}

// Return Number of bytes written if 0 then we are done!
int ecmcCANOpenSDO::writeNextDataToSlave(int useToggle) {

  // How many bytes should we write
  int bytesToWrite = ODSize_-writtenBytes_;
  if(bytesToWrite>ECMC_SDO_TRANSFER_MAX_BYTES) {
    bytesToWrite = ECMC_SDO_TRANSFER_MAX_BYTES;          
  }
  if (bytesToWrite<=0) {
    return 0;
  }
  //Copy data to frame (start at element 1 since toggle is at data 0)
  memcpy(&(writeDataFrame_.data[1]),dataBuffer_ + writtenBytes_,bytesToWrite);
  writeDataFrame_.can_dlc = bytesToWrite + 1; // need to include the toggle byte
  // add toggle byte (start with toggle 0)
  if(useToggle) {
    writeDataFrame_.data[0] = 0x30; // toggle 1
  } else {
    writeDataFrame_.data[0] = 0x20; // toggle 0
  }

  writeBuffer_->addWriteCAN(&writeDataFrame_);  // Send first data frame
  writtenBytes_ += bytesToWrite;
  return bytesToWrite;
}

int ecmcCANOpenSDO::writeWaitForDataConfFrame(int useToggle, can_frame *frame) { 
  // Wait for writeConfReqFrameTg0_ from from slave (toggle 1 or 0)
  if (useToggle) {
    if (frameEqual(&writeConfReqFrameTg1_,frame)) {          
      return 1;
    }
  }
  else {
    if (frameEqual(&writeConfReqFrameTg0_,frame)) {          
      return 1;
    }
  }
  return 0;
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
  if(busy_ || writeStates_ != WRITE_IDLE ) {
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
