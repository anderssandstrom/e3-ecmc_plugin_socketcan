/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcCANOpenMaster.h
*
*  Created on: Mar 08, 2021
*      Author: anderssandstrom
*
\*************************************************************************/
#ifndef ECMC_CANOPEN_MASTER_H_
#define ECMC_CANOPEN_MASTER_H_

#include <stdexcept>
#include "ecmcDataItem.h"
#include "ecmcAsynPortDriver.h"
#include "ecmcSocketCANDefs.h"
#include "ecmcCANOpenPDO.h"
#include "ecmcCANOpenSDO.h"
#include "ecmcCANOpenDevice.h"
#include "inttypes.h"
#include <string>
#include "ecmcSocketCANWriteBuffer.h"
#include "epicsMutex.h"

#include <linux/can.h>
#include <linux/can/raw.h>

class ecmcCANOpenMaster : public ecmcCANOpenDevice {
 public:
  ecmcCANOpenMaster(ecmcSocketCANWriteBuffer* writeBuffer,
                    uint32_t nodeId,
                    int exeSampleTimeMs,
                    int lssSampleTimeMs,
                    int syncSampleTimeMs,
                    int heartSampleTimeMs,
                    const char* name,
                    int dbgMode);
  ~ecmcCANOpenMaster();

  private:
    ecmcCANOpenPDO *lssPdo_;
    ecmcCANOpenPDO *syncPdo_;
    ecmcCANOpenPDO *heartPdo_;
    int lssSampleTimeMs_;
    int syncSampleTimeMs_;
    int heartSampleTimeMs_;
};

#endif  /* ECMC_CANOPEN_MASTER_H_ */

