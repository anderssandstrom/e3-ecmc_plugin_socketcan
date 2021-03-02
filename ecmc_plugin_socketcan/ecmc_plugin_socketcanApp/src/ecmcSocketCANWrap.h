/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcSocketCANWrap.h
*
*  Created on: Mar 02, 2021
*      Author: anderssandstrom
*
\*************************************************************************/
#ifndef ECMC_SOCKETCAN_WRAP_H_
#define ECMC_SOCKETCAN_WRAP_H_
#include "ecmcSocketCANDefs.h"

# ifdef __cplusplus
extern "C" {
# endif  // ifdef __cplusplus

/** \brief Create new SocketCAN object
 *
 *  The configuration string needs to define tha can interface by:\n
 *  "IF=<data source>;"\n
 *  Example:\n
 *  "IF=can0";\n
 *  \param[in] configStr Configuration string.\n
 *
 *  \return 0 if success or otherwise an error code.\n
 */
int         createSocketCAN(char *configStr);

/** \brief Delete SocketCAN object\n
 *
 * Should be called when destructs.\n
 */

void deleteSocketCAN();

# ifdef __cplusplus
}
# endif  // ifdef __cplusplus

#endif  /* ECMC_SOCKETCAN_WRAP_H_ */
