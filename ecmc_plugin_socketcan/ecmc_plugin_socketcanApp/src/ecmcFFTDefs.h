/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcFFTDefs.h
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*      Credits to  https://github.com/sgreg/dynamic-loading 
*
\*************************************************************************/

#ifndef ECMC_FFT_DEFS_H_
#define ECMC_FFT_DEFS_H_

// Options
#define ECMC_PLUGIN_DBG_PRINT_OPTION_CMD   "DBG_PRINT="
#define ECMC_PLUGIN_SOURCE_OPTION_CMD      "SOURCE="
#define ECMC_PLUGIN_NFFT_OPTION_CMD        "NFFT="
//#define ECMC_PLUGIN_APPLY_SCALE_OPTION_CMD "APPLY_SCALE="
#define ECMC_PLUGIN_RM_DC_OPTION_CMD       "RM_DC="
#define ECMC_PLUGIN_ENABLE_OPTION_CMD      "ENABLE="
#define ECMC_PLUGIN_RATE_OPTION_CMD        "RATE="
#define ECMC_PLUGIN_RM_LIN_OPTION_CMD      "RM_LIN="
#define ECMC_PLUGIN_SCALE_OPTION_CMD       "SCALE="


// CONT, TRIGG
#define ECMC_PLUGIN_MODE_OPTION_CMD        "MODE="
#define ECMC_PLUGIN_MODE_CONT_OPTION       "CONT"
#define ECMC_PLUGIN_MODE_TRIGG_OPTION      "TRIGG"

typedef enum FFT_MODE{
  NO_MODE = 0,
  CONT    = 1,
  TRIGG   = 2,
} FFT_MODE;

typedef enum FFT_STATUS{
  NO_STAT = 0,
  IDLE    = 1,  // Doing nothing, waiting for trigg
  ACQ     = 2,  // Acquireing data
  CALC    = 3,  // Calc FFT
} FFT_STATUS;

/** Just one error code in "c" part of plugin 
(error handled with exceptions i c++ part) */
#define ECMC_PLUGIN_FFT_ERROR_CODE 1

// Default size (must be n²)
#define ECMC_PLUGIN_DEFAULT_NFFT 4096

#endif  /* ECMC_FFT_DEFS_H_ */
