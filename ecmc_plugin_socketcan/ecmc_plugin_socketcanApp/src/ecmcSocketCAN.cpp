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


// Start worker for socket read()
void f_worker_read(void *obj) {
  if(!obj) {
    printf("%s/%s:%d: Error: Worker read thread ecmcSocketCAN object NULL..\n",
            __FILE__, __FUNCTION__, __LINE__);
    return;
  }
  ecmcSocketCAN * canObj = (ecmcSocketCAN*)obj;
  canObj->doReadWorker();
}

// Start worker for socket connect()
void f_worker_connect(void *obj) {
  if(!obj) {
    printf("%s/%s:%d: Error: Worker connect thread ecmcSocketCAN object NULL..\n",
            __FILE__, __FUNCTION__, __LINE__);
    return;
  }
  ecmcSocketCAN * canObj = (ecmcSocketCAN*)obj;
  canObj->doConnectWorker();
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
  // Init
  cfgCanIFStr_ = NULL;
  cfgDbgMode_  = 0;
  cfgAutoConnect_ = 1;
  destructs_   = 0;
  socketId_    = -1;
  connected_   = 0;
  memset(&ifr_,0,sizeof(struct ifreq));
  memset(&rxmsg_,0,sizeof(struct can_frame));
  memset(&txmsg_,0,sizeof(struct can_frame));
  memset(&addr_,0,sizeof(struct sockaddr_can));
  
  parseConfigStr(configStr); // Assigns all configs
  // Check valid nfft
  if(!cfgCanIFStr_ ) {
    throw std::out_of_range("CAN inteface must be defined (can0, vcan0...).");
  }

  // Create worker thread for reading socket
  std::string threadname = "ecmc." ECMC_PLUGIN_ASYN_PREFIX".read";
  if(epicsThreadCreate(threadname.c_str(), 0, 32768, f_worker_read, this) == NULL) {
    throw std::runtime_error("Error: Failed create worker thread for read().");
  }

  // Create worker thread for connecting socket
  threadname = "ecmc." ECMC_PLUGIN_ASYN_PREFIX".connect";
  if(epicsThreadCreate(threadname.c_str(), 0, 32768, f_worker_connect, this) == NULL) {
    throw std::runtime_error("Error: Failed create worker thread for connect().");
  }

  if(cfgAutoConnect_) {
    connectPrivate();
  }

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
      if (!strncmp(pThisOption, ECMC_PLUGIN_DBG_PRINT_OPTION_CMD, strlen(ECMC_PLUGIN_DBG_PRINT_OPTION_CMD))) {
        pThisOption += strlen(ECMC_PLUGIN_DBG_PRINT_OPTION_CMD);
        cfgDbgMode_ = atoi(pThisOption);
      }
      
      // ECMC_PLUGIN_CONNECT_OPTION_CMD (1/0)
      if (!strncmp(pThisOption, ECMC_PLUGIN_CONNECT_OPTION_CMD, strlen(ECMC_PLUGIN_CONNECT_OPTION_CMD))) {
        pThisOption += strlen(ECMC_PLUGIN_DBG_PRINT_OPTION_CMD);
        cfgAutoConnect_ = atoi(pThisOption);
      }

      // ECMC_PLUGIN_IF_OPTION_CMD (Source string)
      else if (!strncmp(pThisOption, ECMC_PLUGIN_IF_OPTION_CMD, strlen(ECMC_PLUGIN_IF_OPTION_CMD))) {
        pThisOption += strlen(ECMC_PLUGIN_IF_OPTION_CMD);
        cfgCanIFStr_=strdup(pThisOption);
      }

      pThisOption = pNextOption;
    }    
    free(pOptions);
  }
  if(!cfgCanIFStr_) { 
    throw std::invalid_argument( "CAN interface not defined.");
  }
}

// For connect commands over asyn or plc. let worker connect
void ecmcSocketCAN::connectExternal() {
  if(!connected_) {
    doConnectEvent_.signal(); // let worker start
  }
}

void ecmcSocketCAN::connectPrivate() {

	if((socketId_ = socket(PF_CAN, SOCK_RAW, CAN_RAW)) == -1) {
    throw std::runtime_error( "Error while opening socket.");		
		return;
	}

	strcpy(ifr_.ifr_name, cfgCanIFStr_);
	ioctl(socketId_, SIOCGIFINDEX, &ifr_);
	
	addr_.can_family  = AF_CAN;
	addr_.can_ifindex = ifr_.ifr_ifindex;

	printf("%s at index %d\n", cfgCanIFStr_, ifr_.ifr_ifindex);

	if(bind(socketId_, (struct sockaddr *)&addr_, sizeof(addr_)) == -1) {		
    throw std::runtime_error( "Error in socket bind.");
    return;
	}
  connected_ = 1;
}

int ecmcSocketCAN::getConnected() {
  return connected_;
}

// Read socket worker
void ecmcSocketCAN::doReadWorker() {

  while(true) {
    
    if(destructs_) {
      break;
    }

    // Wait for new CAN frame 

    // TODO MUST CHECK RETRUN VALUE OF READ!!!!!  
    read(socketId_, &rxmsg_, sizeof(rxmsg_));

    if(cfgDbgMode_) {
      // Simulate candump printout
      printf("r 0x%03X", rxmsg_.can_id);
      printf(" [%d]", rxmsg_.can_dlc);
      for(int i=0; i<rxmsg_.can_dlc; i++ ) {
        printf(" 0x%02X", rxmsg_.data[i]);
      }
      printf("\n");
    }
  }
}

// Read socket worker
void ecmcSocketCAN::doConnectWorker() {

  while(true) {
    
    if(destructs_) {
      break;
    }
    doConnectEvent_.wait();
    connectPrivate();
  }
}

// Test can write function (simple if for plc func)
void ecmcSocketCAN::writeCAN( uint32_t canId,
                             uint8_t len,
                             uint8_t data0,
                             uint8_t data1,
                             uint8_t data2,
                             uint8_t data3,
                             uint8_t data4,
                             uint8_t data5,
                             uint8_t data6,
                             uint8_t data7) {
	txmsg_.can_id  = canId;
	txmsg_.can_dlc = len;
	txmsg_.data[0] = data0;
	txmsg_.data[1] = data1;
  txmsg_.data[2] = data2;
  txmsg_.data[3] = data3;
  txmsg_.data[4] = data4;
  txmsg_.data[5] = data5;
  txmsg_.data[6] = data6;
  txmsg_.data[7] = data7;

  // Maybe need to add the size to write here.. if struct is not full, hmm?!
	int nbytes = write(socketId_, &txmsg_, sizeof(struct can_frame));
  if (nbytes!= sizeof(struct can_frame)) {
    throw std::runtime_error( "Error in write.");
  }

  if(cfgDbgMode_) {	  
    // Simulate candump printout
    printf("w 0x%03X", txmsg_.can_id);
    printf(" [%d]", txmsg_.can_dlc);
    for(int i=0; i<txmsg_.can_dlc; i++ ) {
      printf(" 0x%02X", txmsg_.data[i]);
    }
    printf("\n");
  }
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
  /*if( function == asynSRateId_ ) {
    *value = cfgDataSampleRateHz_;
    return asynSuccess;
  }

  return asynError;*/
  return asynSuccess;
}
