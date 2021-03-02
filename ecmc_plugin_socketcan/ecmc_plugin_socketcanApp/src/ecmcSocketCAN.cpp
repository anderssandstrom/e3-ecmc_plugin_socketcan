/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcSocketCAN.cpp
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*      Credits to  https://github.com/sgreg/dynamic-loading 
*
\*************************************************************************/

// Needed to get headers in ecmc right...
#define ECMC_IS_PLUGIN

#define ECMC_PLUGIN_ASYN_PREFIX      "plugin.can"
#define ECMC_PLUGIN_ASYN_ENABLE      "enable"

#include <sstream>
#include "ecmcSocketCAN.h"
#include "ecmcPluginClient.h"
#include "ecmcAsynPortDriver.h"
#include "ecmcAsynPortDriverUtils.h"
#include "epicsThread.h"


// New data callback from ecmc
static int printMissingObjError = 1;

void f_worker_read(void *obj) {
  if(!obj) {
    printf("%s/%s:%d: Error: Worker read thread ecmcSocketCAN object NULL..\n",
            __FILE__, __FUNCTION__, __LINE__);
    return;
  }
  ecmcSocketCAN * canObj = (ecmcSocketCAN*)obj;
  canObj->doReadWorker();
}

/** ecmc ecmcSocketCAN class
 * This object can throw: 
 *    - bad_alloc
 *    - invalid_argument
 *    - runtime_error
*/
ecmcSocketCAN::ecmcSocketCAN(char* configStr,
                 char* portName) 
                  : asynPortDriver(portName,
                   1, /* maxAddr */
                   asynInt32Mask | asynFloat64Mask | asynFloat32ArrayMask |
                   asynFloat64ArrayMask | asynEnumMask | asynDrvUserMask |
                   asynOctetMask | asynInt8ArrayMask | asynInt16ArrayMask |
                   asynInt32ArrayMask | asynUInt32DigitalMask, /* Interface mask */
                   asynInt32Mask | asynFloat64Mask | asynFloat32ArrayMask |
                   asynFloat64ArrayMask | asynEnumMask | asynDrvUserMask |
                   asynOctetMask | asynInt8ArrayMask | asynInt16ArrayMask |
                   asynInt32ArrayMask | asynUInt32DigitalMask, /* Interrupt mask */
                   ASYN_CANBLOCK , /*NOT ASYN_MULTI_DEVICE*/
                   1, /* Autoconnect */
                   0, /* Default priority */
                   0) /* Default stack size */
                   {
  cfgCanIFStr_ = NULL;
  destructs_   = 0;
  socketId_    = -1;
  memset(&ifr_,0,sizeof(ifr_));
  memset(&rxmsg_,0,sizeof(struct can_frame));
  memset(&txmsg_,0,sizeof(struct can_frame));
  
  parseConfigStr(configStr); // Assigns all configs
  // Check valid nfft
  if(!cfgCanIFStr_ ) {
    throw std::out_of_range("CAN inteface must be defined (can0, vcan0...).");
  }

  // Create worker thread for reading socket
  std::string threadname = "ecmc." ECMC_PLUGIN_ASYN_PREFIX;
  if(epicsThreadCreate(threadname.c_str(), 0, 32768, f_worker_read, this) == NULL) {
    throw std::runtime_error("Error: Failed create worker thread.");
  }
  
  initCAN();
  initAsyn();
}

ecmcSocketCAN::~ecmcSocketCAN() {
  // kill worker
  destructs_ = 1;  // maybe need todo in other way..
  //doCalcEvent_.signal();
}

void ecmcSocketCAN::parseConfigStr(char *configStr) {

  // check config parameters
  if (configStr && configStr[0]) {    
    char *pOptions = strdup(configStr);
    char *pThisOption = pOptions;
    char *pNextOption = pOptions;
    
    while (pNextOption && pNextOption[0]) {
      pNextOption = strchr(pNextOption, ';');
      if (pNextOption) {
        *pNextOption = '\0'; /* Terminate */
        pNextOption++;       /* Jump to (possible) next */
      }
      
      // ECMC_PLUGIN_DBG_PRINT_OPTION_CMD (1/0)
      /*if (!strncmp(pThisOption, ECMC_PLUGIN_DBG_PRINT_OPTION_CMD, strlen(ECMC_PLUGIN_DBG_PRINT_OPTION_CMD))) {
        pThisOption += strlen(ECMC_PLUGIN_DBG_PRINT_OPTION_CMD);
        cfgDbgMode_ = atoi(pThisOption);
      } */
      
      // ECMC_PLUGIN_IF_OPTION_CMD (Source string)
      else if (!strncmp(pThisOption, ECMC_PLUGIN_IF_OPTION_CMD, strlen(ECMC_PLUGIN_IF_OPTION_CMD))) {
        pThisOption += strlen(ECMC_PLUGIN_IF_OPTION_CMD)destructs_;
        cfgCanIFStr_=strdup(pThisOption);
      }

      pThisOption = pNextOption;
    }    
    free(pOptions);
  }
  if(!cfgCanIFStr_) { 
    throw std::invalid_argument( "Data source not defined.");
  }
}

void ecmcSocketCAN::initCAN(){

	if((socketId_ = socket(PF_CAN, SOCK_RAW, CAN_RAW)) == -1) {
     throw std::runtime_error( "Error while opening socket.");		
		return -1;
	}

	strcpy(ifr.ifr_name, cfgCanIFStr_);
	ioctl(socketId, SIOCGIFINDEX, &ifr_);
	
	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr_.ifr_ifindex;

	printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

	if(bind(socketId, (struct sockaddr *)&addr, sizeof(addr)) == -1) {		
    throw std::runtime_error( "Error in socket bind.");
		return -2;
	}
}

// Read socket worker
void ecmcSocketCAN::doReadWorker() {

  while(true) {
    
    if(destructs_) {
      break;
    }

    // Wait for new CAN frame   
    read(socketId_, &rxmsg_, sizeof(rxmsg_));  
    
    printf("\n0x%02X", rxmsg_.can_id);
    printf(" [%d]", rxmsg_.can_dlc);
    for(int i=0; i<rxmsg_.can_dlc; i++ ) {
      printf(" 0x%02X", rxmsg_.data[i]);
    }

  }

}

// Test can write function
int ecmcSocketCAN::writeCAN() {
  struct can_frame frame;
	txmsg_.can_id  = 0x123;
	txmsg_.can_dlc = 2;
	txmsg_.data[0] = frame.data[0]+1;
	txmsg_.data[1] = frame.data[1]+1;
	int nbytes = write(socketId, &txmsg_, sizeof(struct can_frame));
	printf("\nWrote %d bytes\n", nbytes);
}

void ecmcSocketCAN::initAsyn() {

  // Add enable "plugin.fft%d.enable"
  /*std::string paramName =ECMC_PLUGIN_ASYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_ENABLE;
  
  if( createParam(0, paramName.c_str(), asynParamInt32, &asynEnableId_) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter enable");
  }
  setIntegerParam(asynEnableId_, cfgEnable_);

  // Add rawdata "plugin.fft%d.rawdata"
  paramName =ECMC_PLUGIN_ASYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_RAWDATA;

  if( createParam(0, paramName.c_str(), asynParamFloat64Array, &asynRawDataId_ ) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter rawdata");
  }
  doCallbacksFloat64Array(rawDataBuffer_, cfgNfft_, asynRawDataId_,0);

  // Add rawdata "plugin.fft%d.preprocdata"
  paramName =ECMC_PLUGIN_ASYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_PPDATA;

  if( createParam(0, paramName.c_str(), asynParamFloat64Array, &asynPPDataId_ ) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter preprocdata");
  }
  doCallbacksFloat64Array(prepProcDataBuffer_, cfgNfft_, asynPPDataId_,0);

n

  // Add fft amplitude "plugin.fft%d.fftamplitude"
  paramName = ECMC_PLUGIN_ASYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_FFT_AMP;

  if( createParam(0, paramName.c_str(), asynParamFloat64Array, &asynFFTAmpId_ ) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter fftamplitude");
  }
  doCallbacksFloat64Array(fftBufferResultAmp_, cfgNfft_/2+1, asynFFTAmpId_,0);

  // Add fft "plugin.fft%d.mode"
  paramName = ECMC_PLUGIN_ASYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_FFT_MODE;

  if( createParam(0, paramName.c_str(), asynParamInt32, &asynFFTModeId_ ) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter mode");
  }
  setIntegerParam(asynFFTModeId_, (epicsInt32)cfgMode_);

  // Add fft "plugin.fft%d.status"
  paramName = ECMC_PLUGIN_ASYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_FFT_STAT;

  if( createParam(0, paramName.c_str(), asynParamInt32, &asynFFTStatId_ ) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter status");
  }
  setIntegerParam(asynFFTStatId_, (epicsInt32)status_);

  // Add fft "plugin.fft%d.source"
  paramName = ECMC_PLUGIN_ASYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_FFT_SOURCE;

  if( createParam(0, paramName.c_str(), asynParamInt8Array, &asynSourceId_ ) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter source");
  }
  doCallbacksInt8Array(cfgCanIFStr_, strlen(cfgCanIFStr_), asynSourceId_,0);

  // Add fft "plugin.fft%d.trigg"
  paramName = ECMC_PLUGIN_ASYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_FFT_TRIGG;

  if( createParam(0, paramName.c_str(), asynParamInt32, &asynTriggId_ ) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter trigg");
  }
  setIntegerParam(asynTriggId_, (epicsInt32)triggOnce_);

  // Add fft "plugin.fft%d.fftxaxis"
  paramName = ECMC_PLUGIN_nSYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_FFT_X_FREQS;

  if( createParam(0, paramName.c_str(), asynParamFloat64Array, &asynFFTXAxisId_ ) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter xaxisfreqs");
  }
  doCallbacksFloat64Array(fftBufferXAxis_,cfgNfft_ / 2 + 1, asynFFTXAxisId_,0);

  // Add fft "plugin.fft%d.nfft"
  paramName = ECMC_PLUGIN_ASYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_NFFT;

  if( createParam(0, paramName.c_str(), asynParamInt32, &asynNfftId_ ) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter nfft");
  }
  setIntegerParam(asynNfftId_, (epicsInt32)cfgNfft_);

  // Add fft "plugin.fft%d.rate"
  paramName = ECMC_PLUGIN_ASYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_RATE;

  if( createParam(0, paramName.c_str(), asynParamFloat64, &asynSRateId_ ) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter rate");
  }
  setDoubleParam(asynSRateId_, cfgDataSampleRateHz_);

  // Add fft "plugin.fft%d.buffid"
  paramName = ECMC_PLUGIN_ASYN_PREFIX + to_string(objectId_) + 
             "." + ECMC_PLUGIN_ASYN_BUFF_ID;

  if( createParam(0, paramName.c_str(), asynParamInt32, &asynElementsInBuffer_ ) != asynSuccess ) {
    throw std::runtime_error("Failed create asyn parameter trigg");
  }
  setIntegerParam(asynElementsInBuffer_, (epicsInt32)elementsInBuffer_);

  // Update integers
  callParamCallbacks();*/
}

// Avoid issues with std:to_string()
std::string ecmcSocketCAN::to_string(int value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

asynStatus ecmcSocketCAN::writeInt32(asynUser *pasynUser, epicsInt32 value) {
  int function = pasynUser->reason;
  /*if( function == asynEnableId_ ) {
    cfgEnable_ = value;
    return asynSuccess;
  } else if( function == asynFFTModeId_){
    cfgMode_ = (FFT_MODE)value;// Called from low prio worker thread. Makes the hard work
void ecmcSocketCAN::doCalcWorker() {

  while(true) {
    doCalcEvent_.wait();
    if(destructs_) {
      break;
    }
    // Pre-process    
    removeDCOffset();  // Remove dc on rawdata
    removeLin();       // Remove fitted line
    // Process
    calcFFT();         // FFT cacluation
    // Post-process    
    scaleFFT();        // Scale FFT
    calcFFTAmp();      // Calculate amplitude from complex
    calcFFTXAxis();    // Calculate x axis

    doCallbacksFloat64Array(rawDataBuffer_,     cfgNfft_,     asynRawDataId_, 0);
    doCallbacksFloat64Array(prepProcDataBuffer_, cfgNfft_,    asynPPDataId_,  0);
    doCallbacksFloat64Array(fftBufferResultAmp_,cfgNfft_/2+1, asynFFTAmpId_,  0);
    doCallbacksFloat64Array(fftBufferXAxis_,    cfgNfft_/2+1, asynFFTXAxisId_,0);
    callParamCallbacks();    
    if(cfgDbgMode_){
      printComplexArray(fftBufferResult_,
                        cfgNfft_,
                        objectId_);
      printEcDataArray((uint8_t*)rawDataBuffer_,
                       cfgNfft_*sizeof(double),
                       ECMC_EC_F64,
                       objectId_);    
    }
    
    clearBuffers();
    triggOnce_ = 0;    // Wait for next trigger if in trigg mode
    setIntegerParam(asynTriggId_,triggOnce_);
    fftWaitingForCalc_ = 0;
  } 
}
    return asynSuccess;
  }
  return asynError;*/
  return asynSuccess;
}

asynStatus ecmcSocketCAN::readInt32(asynUser *pasynUser, epicsInt32 *value) {
  int function = pasynUser->reason;
  /*if( function == asynEnableId_ ) {
    *value = cfgEnable_;
    return asynSuccess;
  } else if( function == asynFFTModeId_ ){
    *value = cfgMode_;
    return asynSuccess;
  } else if( function == asynTriggId_ ){
    *value = triggOnce_;
    return asynSuccess;
  }else if( function == asynFFTStatId_ ){
    *value = (epicsInt32)status_;
    return asynSuccess;
  }else if( function == asynNfftId_ ){
    *value = (epicsInt32)cfgNfft_;
    return asynSuccess;
  }else if( function == asynElementsInBuffer_){
    *value = (epicsInt32)elementsInBuffer_;
    return asynSuccess;
  }
  return asynError;*/
  return asynSuccess;
}



asynStatus ecmcSocketCAN::readInt8Array(asynUser *pasynUser, epicsInt8 *value, 
                                   size_t nElements, size_t *nIn) {
  int function = pasynUser->reason;
  /*if( function == asynSourceId_ ) {
    unsigned int ncopy = strlen(cfgCanIFStr_);
    if(nElements < ncopy) {
      ncopy = nElements;
    } 
    memcpy (value, cfgCanIFStr_, ncopy);
    *nIn = ncopy;
    return asynSuccess;
  }

  *nIn = 0;
  return asynError;*/
  return asynSuccess;
}

asynStatus  ecmcSocketCAN::readFloat64(asynUser *pasynUser, epicsFloat64 *value) {
  int function = pasynUser->reason;
/*  if( function == asynSRateId_ ) {
    *value = cfgDataSampleRateHz_;
    return asynSuccess;
  }

  return asynError;*/
  return asynSuccess;
}
