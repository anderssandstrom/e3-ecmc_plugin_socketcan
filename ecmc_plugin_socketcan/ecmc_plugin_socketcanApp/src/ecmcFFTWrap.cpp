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
#include "ecmcFFTWrap.h"
#include "ecmcFFT.h"
#include "ecmcFFTDefs.h"

#define ECMC_PLUGIN_MAX_PORTNAME_CHARS 64
#define ECMC_PLUGIN_PORTNAME_PREFIX "PLUGIN.FFT"

static std::vector<ecmcFFT*>  ffts;
static int                    fftObjCounter = 0;
static char                   portNameBuffer[ECMC_PLUGIN_MAX_PORTNAME_CHARS];

int createFFT(char* configStr) {

  // create new ecmcFFT object
  ecmcFFT* fft = NULL;

  // create asynport name for new object ()
  memset(portNameBuffer, 0, ECMC_PLUGIN_MAX_PORTNAME_CHARS);
  snprintf (portNameBuffer, ECMC_PLUGIN_MAX_PORTNAME_CHARS,
            ECMC_PLUGIN_PORTNAME_PREFIX "%d", fftObjCounter);
  try {
    fft = new ecmcFFT(fftObjCounter, configStr, portNameBuffer);
  }
  catch(std::exception& e) {
    if(fft) {
      delete fft;
    }
    printf("Exception: %s. Plugin will unload.\n",e.what());
    return ECMC_PLUGIN_FFT_ERROR_CODE;
  }
  
  ffts.push_back(fft);
  fftObjCounter++;

  return 0;
}

void deleteAllFFTs() {
  for(std::vector<ecmcFFT*>::iterator pfft = ffts.begin(); pfft != ffts.end(); ++pfft) {
    if(*pfft) {
      delete (*pfft);
    }
  }
}

int  linkDataToFFTs() {
  for(std::vector<ecmcFFT*>::iterator pfft = ffts.begin(); pfft != ffts.end(); ++pfft) {
    if(*pfft) {
      try {
        (*pfft)->connectToDataSource();
      }
      catch(std::exception& e) {
        printf("Exception: %s. Plugin will unload.\n",e.what());
        return ECMC_PLUGIN_FFT_ERROR_CODE;
      }
    }
  }
  return 0;
}

int enableFFT(int fftIndex, int enable) {
  try {
    ffts.at(fftIndex)->setEnable(enable);
  }
  catch(std::exception& e) {
    printf("Exception: %s. FFT index out of range.\n",e.what());
    return ECMC_PLUGIN_FFT_ERROR_CODE;
  }
  return 0;
}

int clearFFT(int fftIndex) {
  try {
    ffts.at(fftIndex)->clearBuffers();
  }
  catch(std::exception& e) {
    printf("Exception: %s. FFT index out of range.\n",e.what());
    return ECMC_PLUGIN_FFT_ERROR_CODE;
  }  
  return 0;
}

int triggFFT(int fftIndex) {
  try {
    ffts.at(fftIndex)->triggFFT();
  }
  catch(std::exception& e) {
    printf("Exception: %s. FFT index out of range.\n",e.what());
    return ECMC_PLUGIN_FFT_ERROR_CODE;
  }  
  return 0;
}

int modeFFT(int fftIndex, FFT_MODE mode) {
  try {
    ffts.at(fftIndex)->setModeFFT(mode);
  }
  catch(std::exception& e) {
    printf("Exception: %s. FFT index out of range.\n",e.what());
    return ECMC_PLUGIN_FFT_ERROR_CODE;
  }  
  return 0;
}

FFT_STATUS  statFFT(int fftIndex) {
  try {
    return ffts.at(fftIndex)->getStatusFFT();
  }
  catch(std::exception& e) {
    printf("Exception: %s. FFT index out of range.\n",e.what());
    return NO_STAT;
  }  
  return NO_STAT;
}
