/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcPluginExample.cpp
*
*  Created on: Mar 21, 2020
*      Author: anderssandstrom
*
\*************************************************************************/

// Needed to get headers in ecmc right...
#define ECMC_IS_PLUGIN
#define ECMC_EXAMPLE_PLUGIN_VERSION 2

#ifdef __cplusplus
extern "C" {
#endif  // ifdef __cplusplus

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ecmcPluginDefs.h"
#include "ecmcPluginClient.h"
#include "ecmcSocketCANWrap.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

static int    lastEcmcError   = 0;
static char*  lastConfStr     = NULL;
static int    alreadyLoaded   = 0;


/*static int    socketId = -1;
struct can_frame frame;*/

/** Optional. 
 *  Will be called once after successfull load into ecmc.
 *  Return value other than 0 will be considered error.
 *  configStr can be used for configuration parameters.
 **/
int canConstruct(char *configStr)
{
  if(alreadyLoaded) {    
    return 1;
  }

  alreadyLoaded = 1;
  // create SocketCAN object and register data callback
  lastConfStr = strdup(configStr);
  return createSocketCAN(configStr,getEcmcSampleTimeMS());

/*	int nbytes;
	struct sockaddr_can addr;

	struct ifreq ifr;

	//const char *ifname = "vcan0";
  const char *ifname = "can0";

	if((socketId = socket(PF_CAN, SOCK_RAW, CAN_RAW)) == -1) {
		perror("Error while opening socket");
		return -1;
	}

	strcpy(ifr.ifr_name, ifname);
	ioctl(socketId, SIOCGIFINDEX, &ifr);
	
	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

	if(bind(socketId, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		perror("Error in socket bind");
		return -2;
	}

	frame.can_id  = 0x123;
	frame.can_dlc = 2;
	frame.data[0] = 0x11;
	frame.data[1] = 0x22;
deleteSocketCANbytes);
	*/

  //return 0;
}

/** Optional function.
 *  Will be called once at unload.
 **/
void canDestruct(void)
{  
  if(lastConfStr){
    free(lastConfStr);
  }
  deleteSocketCAN();
}

/** Optional function.
 *  Will be called each realtime cycle if definded
 *  ecmcError: Error code of ecmc. Makes it posible for 
 *  this plugin to react on ecmc errors
 *  Return value other than 0 will be considered to be an error code in ecmc.
 **/
int canRealtime(int ecmcError)
{
  
	/*frame.can_id  = 0x123;
	frame.can_dlc = 2;
	frame.data[0] = frame.data[0]+1;
	frame.data[1] = frame.data[1]+1;

	int nbytes = write(socketId, &frame, sizeof(struct can_frame));
	printf("\nWrote %d bytes\n", nbytes);

  // NOTE read is BLOCKING so need to start separate thread
  struct can_frame rxmsg;      
  read(socketId, &rxmsg, sizeof(rxmsg));  
  printf("\n0x%02X", rxmsg.can_id);
  printf(" [%d]", rxmsg.can_dlc);
  for(int i=0; i<rxmsg.can_dlc; i++ ) {
    printf(" 0x%02X", rxmsg.data[i]);
  }
*/
  lastEcmcError = ecmcError;
  return execute();
}

/** Link to data source here since all sources should be availabe at this stage
 *  (for example ecmc PLC variables are defined only at enter of realtime)
 **/
int canEnterRT(){
  return 0;
}

/** Optional function.
 *  Will be called once just before leaving realtime mode
 *  Return value other than 0 will be considered error.
 **/
int canExitRT(void){
  return 0;
}
// Plc function for connect to can
double can_connect() {
  return (double)connectSocketCAN();
}

// Plc function for connected to connected
double can_connected() {
  return (double)getSocketCANConnectd();
}

// trigger all writes added to buffer
//double can_trigg_writes() {
//  return (double)triggWrites();
//}

// trigger all writes added to buffer
double can_last_writes_error() {
  return (double)getlastWritesError();
}

// Add frrame to output buffer
double can_add_write(double canId,
                     double len,
                     double data0,
                     double data1,
                     double data2,
                     double data3,
                     double data4,
                     double data5,
                     double data6,
                     double data7) {
  return (double)addWriteSocketCAN(canId,
                                   len,
                                   data0,
                                   data1,
                                   data2,
                                   data3,
                                   data4,
                                   data5,
                                   data6,
                                   data7);
}

// Register data for plugin so ecmc know what to use
struct ecmcPluginData pluginDataDef = {
  // Allways use ECMC_PLUG_VERSION_MAGIC
  .ifVersion = ECMC_PLUG_VERSION_MAGIC, 
  // Name 
  .name = "ecmcPlugin_socketcan",
  // Description
  .desc = "SocketCAN plugin for use with ecmc.",
  // Option description
  .optionDesc = "\n    "ECMC_PLUGIN_DBG_PRINT_OPTION_CMD"<1/0>    : Enables/disables printouts from plugin, default = disabled (=0).\n"
                "    "ECMC_PLUGIN_IF_OPTION_CMD"<if name>         : Sets can interface (example: can0, vcan0..).\n"
                "    "ECMC_PLUGIN_CONNECT_OPTION_CMD"<1/0>        : Auto connect to if at startup, default = autoconnect (=1).\n"
                ,
  // Plugin version
  .version = ECMC_EXAMPLE_PLUGIN_VERSION,
  // Optional construct func, called once at load. NULL if not definded.
  .constructFnc = canConstruct,
  // Optional destruct func, called once at unload. NULL if not definded.
  .destructFnc = canDestruct,
  // Optional func that will be called each rt cycle. NULL if not definded.
  .realtimeFnc = canRealtime,
  // Optional func that will be called once just before enter realtime mode
  .realtimeEnterFnc = canEnterRT,
  // Optional func that will be called once just before exit realtime mode
  .realtimeExitFnc = canExitRT,
  // PLC funcs
  .funcs[0] =
      { /*----can_connect----*/
        // Function name (this is the name you use in ecmc plc-code)
        .funcName = "can_connect",
        // Function description
        .funcDesc = "double can_connect() : Connect to can interface (from config str).",
        /**
        * 7 different prototypes allowed (only doubles since reg in plc).
        * Only funcArg${argCount} func shall be assigned the rest set to NULL.
        **/
        .funcArg0 = can_connect,
        .funcArg1 = NULL,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,
        .funcGenericObj = NULL,
      },
  .funcs[1] =
      { /*----can_connected----*/
        // Function name (this is the name you use in ecmc plc-code)
        .funcName = "can_connected",
        // Function description
        .funcDesc = "double can_connected() : Connected to can interface.",
        /**
        * 7 different prototypes allowed (only doubles since reg in plc).
        * Only funcArg${argCount} func shall be assigned the rest set to NULL.
        **/
        .funcArg0 = can_connected,
        .funcArg1 = NULL,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,
        .funcGenericObj = NULL,
      },
  .funcs[2] =
      { /*----can_connected----*/
        // Function name (this is the name you use in ecmc plc-code)
        .funcName = "can_add_write",
        // Function description
        .funcDesc = "double can_add_write(canId,len,data0..data7) : Add write frame to can interface output buffer.",
        /**
        * 7 different prototypes allowed (only doubles since reg in plc).
        * Only funcArg${argCount} func shall be assigned the rest set to NULL.
        **/
        .funcArg0 = NULL,
        .funcArg1 = NULL,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = can_add_write,
        .funcGenericObj = NULL,
      },
  .funcs[3] =
      { /*----can_last_writes_error----*/
        // Function name (this is the name you use in ecmc plc-code)
        .funcName = "can_last_writes_error",
        // Function description
        .funcDesc = "double can_last_writes_error() : get error from last writes.",
        /**
        * 7 different prototypes allowed (only doubles since reg in plc).
        * Only funcArg${argCount} func shall be assigned the rest set to NULL.
        **/
        .funcArg0 = can_last_writes_error,
        .funcArg1 = NULL,
        .funcArg2 = NULL,
        .funcArg3 = NULL,
        .funcArg4 = NULL,
        .funcArg5 = NULL,
        .funcArg6 = NULL,
        .funcArg7 = NULL,
        .funcArg8 = NULL,
        .funcArg9 = NULL,
        .funcArg10 = NULL,
        .funcGenericObj = NULL,
      },      
  .funcs[4] = {0},  // last element set all to zero..
  // PLC consts
  .consts[0] = {0}, // last element set all to zero..
};

ecmc_plugin_register(pluginDataDef);

# ifdef __cplusplus
}
# endif  // ifdef __cplusplus
