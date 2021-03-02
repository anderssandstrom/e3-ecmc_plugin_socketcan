/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcFFTWrap.cpp
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*      Credits to  https://github.com/sgreg/dynamic-loading 
*
\*************************************************************************/

// Needed to get headers in ecmc right...
#define ECMC_IS_PLUGIN

#include <vector>
#include <stdexcept>
#include <string>
#include "ecmcSocketCANWrap.h"
#include "ecmcSocketCAN.h"
#include "ecmcSocketCANDefs.h"

#define ECMC_PLUGIN_MAX_PORTNAME_CHARS 64
#define ECMC_PLUGIN_PORTNAME_PREFIX "PLUGIN.CAN"

static ecmcSocketCAN*  can = NULL;
static char            portNameBuffer[ECMC_PLUGIN_MAX_PORTNAME_CHARS];

int createSocketCAN(char* configStr) {

  // create new ecmcFFT object

  // create asynport name for new object ()
  memset(portNameBuffer, 0, ECMC_PLUGIN_MAX_PORTNAME_CHARS);
  snprintf (portNameBuffer, ECMC_PLUGIN_MAX_PORTNAME_CHARS,
            ECMC_PLUGIN_PORTNAME_PREFIX);
  try {
    can = new ecmcSocketCAN(configStr, portNameBuffer);
  }
  catch(std::exception& e) {
    if(can) {
      delete can;
    }
    printf("Exception: %s. Plugin will unload.\n",e.what());
    return ECMC_PLUGIN_SOCKETCAN_ERROR_CODE;
  }
  
  return 0;
}

int connectSocketCAN() {
  if(can){
      try {       
        can->connect();
    }
    catch(std::exception& e) {
      printf("Exception: %s.\n",e.what());
      return ECMC_PLUGIN_SOCKETCAN_ERROR_CODE;
    }
  }
  else {
    return ECMC_PLUGIN_SOCKETCAN_ERROR_CODE;
  }
}

int getSocketCANConnectd() {
  if(can){
    try {       
      return can->getConnected();      
    }
    catch(std::exception& e) {
      printf("Exception: %s.\n",e.what());
      return 0;
    }
  }
  return 0;
}

void deleteSocketCAN() {
  if(can) {
    delete (can);
  }
}
