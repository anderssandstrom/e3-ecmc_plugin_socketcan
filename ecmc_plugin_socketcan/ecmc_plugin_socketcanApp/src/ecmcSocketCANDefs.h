/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcSocketCANDefs.h
*
*  Created on: March 02, 2021
*      Author: anderssandstrom
*      Credits to  https://github.com/sgreg/dynamic-loading 
*
\*************************************************************************/

#ifndef ECMC_SOCKETCAN_DEFS_H_
#define ECMC_SOCKETCAN_DEFS_H_

// Options
#define ECMC_PLUGIN_DBG_PRINT_OPTION_CMD   "DBG_PRINT="
#define ECMC_PLUGIN_IF_OPTION_CMD          "IF="
#define ECMC_PLUGIN_CONNECT_OPTION_CMD     "CONNECT="

/** Just one error code in "c" part of plugin 
(error handled with exceptions i c++ part) */
#define ECMC_PLUGIN_SOCKETCAN_ERROR_CODE 1

#endif  /* ECMC_SOCKETCAN_DEFS_H_ */
