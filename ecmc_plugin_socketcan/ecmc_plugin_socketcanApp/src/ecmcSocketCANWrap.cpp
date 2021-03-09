/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcFFTWrap.cpp
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
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
#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsExport.h>
#include <epicsEvent.h>

#include <iocsh.h>


#define ECMC_PLUGIN_MAX_PORTNAME_CHARS 64
#define ECMC_PLUGIN_PORTNAME_PREFIX "PLUGIN.CAN"

static ecmcSocketCAN*  can = NULL;
static char            portNameBuffer[ECMC_PLUGIN_MAX_PORTNAME_CHARS];

int createSocketCAN(char* configStr, int exeSampleTimeMs) {

  // create new ecmcFFT object

  // create asynport name for new object ()
  memset(portNameBuffer, 0, ECMC_PLUGIN_MAX_PORTNAME_CHARS);
  snprintf (portNameBuffer, ECMC_PLUGIN_MAX_PORTNAME_CHARS,
            ECMC_PLUGIN_PORTNAME_PREFIX);
  try {
    can = new ecmcSocketCAN(configStr, portNameBuffer, exeSampleTimeMs);
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
        can->connectExternal();
    }
    catch(std::exception& e) {
      printf("Exception: %s.\n",e.what());
      return ECMC_PLUGIN_SOCKETCAN_ERROR_CODE;
    }
  }
  else {
    return ECMC_PLUGIN_SOCKETCAN_ERROR_CODE;
  }
  return 0;
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

int getlastWritesError() {
  if(can){
    try {       
      return can->getlastWritesError();
    }
    catch(std::exception& e) {
      printf("Exception: %s.\n",e.what());
      return 1;
    }
  }
  return 1;
}

int execute() {
  if(can){
    can->execute();
  }
  return 0;
}

int addWriteSocketCAN( double canId,
                       double len,
                       double data0,
                       double data1,
                       double data2,
                       double data3,
                       double data4,
                       double data5,
                       double data6,
                       double data7) {
  if(can){
    try {       
      return can->addWriteCAN((uint32_t) canId,
                              (uint8_t) len,
                              (uint8_t) data0,
                              (uint8_t) data1,
                              (uint8_t) data2,
                              (uint8_t) data3,
                              (uint8_t) data4,
                              (uint8_t) data5,
                              (uint8_t) data6,
                              (uint8_t) data7);      
    }
    catch(std::exception& e) {
      printf("Exception: %s.\n",e.what());
      return ECMC_PLUGIN_SOCKETCAN_ERROR_CODE;
    }
  }
  return ECMC_PLUGIN_SOCKETCAN_ERROR_CODE;
}

void deleteSocketCAN() {
  if(can) {
    delete (can);
  }
}


/** 
 * EPICS iocsh shell command: ecmcCANOpenAddMaster
*/

void ecmcCANOpenAddMasterPrintHelp() {
  printf("\n");
  printf("       Use \"ecmcCANOpenAddMaster(<name>, <node id>\n");
  printf("          <name>      : Name of master device.\n");
  printf("          <node id>   : CANOpen node id of master.\n");
  printf("\n");
}

int ecmcCANOpenAddMaster(const char* name, int nodeId) {
  if(!name) {
    printf("Error: name.\n");
    ecmcCANOpenAddMasterPrintHelp();
    return asynError;
  }

  if(strcmp(name,"-h") == 0 || strcmp(name,"--help") == 0 ) {
    ecmcCANOpenAddMasterPrintHelp();
    return asynSuccess;
  }

  /* CODE HERE*/

  return asynSuccess;
}

static const iocshArg initArg0_0 =
{ "Name", iocshArgString };
static const iocshArg initArg1_0 =
{ "Node Id", iocshArgInt };

static const iocshArg *const initArgs_0[]  = { &initArg0_0, 
                                               &initArg1_0};

static const iocshFuncDef    initFuncDef_0 = { "ecmcCANOpenAddMaster", 2, initArgs_0 };
static void initCallFunc_0(const iocshArgBuf *args) {
  ecmcCANOpenAddMaster(args[0].sval, args[1].ival);
}

/** 
 * EPICS iocsh shell command: ecmcCANOpenAddDevice
*/

void ecmcCANOpenAddDevicePrintHelp() {
  printf("\n");
  printf("       Use \"ecmcCANOpenAddDevice(<name>, <node id>\n");
  printf("          <name>      : Name of device.\n");
  printf("          <node id>   : CANOpen node id of master.\n");
  printf("\n");
}

int ecmcCANOpenAddDevice(const char* name, int nodeId) {
  if(!name) {
    printf("Error: name.\n");
    ecmcCANOpenAddDevicePrintHelp();
    return asynError;
  }

  if(strcmp(name,"-h") == 0 || strcmp(name,"--help") == 0 ) {
    ecmcCANOpenAddDevicePrintHelp();
    return asynSuccess;
  }

  /* CODE HERE*/

  return asynSuccess;
}

static const iocshArg initArg0_1 =
{ "Name", iocshArgString };
static const iocshArg initArg1_1 =
{ "Node Id", iocshArgInt };

static const iocshArg *const initArgs_1[]  = { &initArg0_1, 
                                               &initArg1_1};

static const iocshFuncDef    initFuncDef_1 = { "ecmcCANOpenAddDevice", 2, initArgs_1 };
static void initCallFunc_1(const iocshArgBuf *args) {
  ecmcCANOpenAddDevice(args[0].sval, args[1].ival);
}

/** 
 * EPICS iocsh shell command: ecmcCANOpenAddSDO
*/

void ecmcCANOpenAddSDOPrintHelp() {
  printf("\n");
  printf("       Use \"ecmcCANOpenAddSDO(<name>, <node id>\n");
  printf("          <name>      : Name of master device.\n");
  printf("          <node id>   : CANOpen node id of master.\n");
  printf("\n");
}

int ecmcCANOpenAddSDO(const char* name, int nodeId) {
  if(!name) {
    printf("Error: name.\n");
    ecmcCANOpenAddSDOPrintHelp();
    return asynError;
  }

  if(strcmp(name,"-h") == 0 || strcmp(name,"--help") == 0 ) {
    ecmcCANOpenAddSDOPrintHelp();
    return asynSuccess;
  }

  /* CODE HERE*/

  return asynSuccess;
}

static const iocshArg initArg0_2 =
{ "Name", iocshArgString };
static const iocshArg initArg1_2 =
{ "Node Id", iocshArgInt };

static const iocshArg *const initArgs_2[]  = { &initArg0_2, 
                                               &initArg1_2};

static const iocshFuncDef    initFuncDef_2 = { "ecmcCANOpenAddSDO", 2, initArgs_2 };
static void initCallFunc_2(const iocshArgBuf *args) {
  ecmcCANOpenAddSDO(args[0].sval, args[1].ival);
}

/** 
 * EPICS iocsh shell command: ecmcCANOpenAddPDO
*/

void ecmcCANOpenAddPDOPrintHelp() {
  printf("\n");
  printf("       Use \"ecmcCANOpenAddPDO(<name>, <node id>\n");
  printf("          <name>      : Name of master device.\n");
  printf("          <node id>   : CANOpen node id of master.\n");
  printf("\n");
}

int ecmcCANOpenAddPDO(const char* name, int nodeId) {
  if(!name) {
    printf("Error: name.\n");
    ecmcCANOpenAddPDOPrintHelp();
    return asynError;
  }

  if(strcmp(name,"-h") == 0 || strcmp(name,"--help") == 0 ) {
    ecmcCANOpenAddPDOPrintHelp();
    return asynSuccess;
  }

  /* CODE HERE*/

  return asynSuccess;
}

static const iocshArg initArg0_3 =
{ "Name", iocshArgString };
static const iocshArg initArg1_3 =
{ "Node Id", iocshArgInt };

static const iocshArg *const initArgs_3[]  = { &initArg0_3,
                                               &initArg1_3};

static const iocshFuncDef    initFuncDef_3 = { "ecmcCANOpenAddPDO", 2, initArgs_3 };
static void initCallFunc_3(const iocshArgBuf *args) {
  ecmcCANOpenAddPDO(args[0].sval, args[1].ival);
}

/** 
 * Register all functions
*/
void ecmcCANPluginDriverRegister(void) {
  iocshRegister(&initFuncDef_0,    initCallFunc_0);   // ecmcCANOpenAddMaster
  iocshRegister(&initFuncDef_1,    initCallFunc_1);   // ecmcCANOpenAddDevice
  iocshRegister(&initFuncDef_2,    initCallFunc_2);   // ecmcCANOpenAddSDO
  iocshRegister(&initFuncDef_3,    initCallFunc_3);   // ecmcCANOpenAddPDO
}

epicsExportRegistrar(ecmcCANPluginDriverRegister);
