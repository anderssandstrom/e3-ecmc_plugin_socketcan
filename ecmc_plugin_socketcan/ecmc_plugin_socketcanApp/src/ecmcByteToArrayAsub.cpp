// aSub, EPICS related headers
#include <aSubRecord.h>
#include <registryFunction.h>
#include <epicsExport.h>
// std::cout
#include <iostream>
// split double into fractional and integer
#include <math.h>
#include <string.h>

// declare init function
static long ecmcByteToArrayInit(struct aSubRecord *rec);
epicsRegisterFunction(ecmcByteToArrayInit);

// declare worker function
static long ecmcByteToArray(struct aSubRecord *rec);
epicsRegisterFunction(ecmcByteToArray);

// init (INAM)
static long ecmcByteToArrayInit(struct aSubRecord *rec){
  std::cout << "ecmcByteToArrayInit aSubRecord: "<< rec->name << std::endl;
  printf("ecmcByteToArray aSubRecord: %d\n",rec->noa);  
  return 0;
}

static long ecmcByteToArray(struct aSubRecord *rec){
   printf("EXECUTING!!!!!\n");

  // input A must be size of output array
  epicsUInt8 byteCount=(epicsUInt8)rec->nova;
  
  // Max 18 byte in a row
  if(byteCount <=0 || byteCount>18){
    printf("WARNING: Only 18 first bytes will be transferred to output.\n");    
  }

  epicsUInt8	*vala;
  vala = (epicsUInt8 *)rec->vala;
      
  if(byteCount >= 1 && rec->noa ==1) {
    vala[0]=*(epicsUInt8 *)rec->a;
  }

  if(byteCount >= 2 && rec->nob ==1) {
    vala[1]=*(epicsUInt8 *)rec->b;
  }

  if(byteCount >= 3 && rec->noc ==1) {
    vala[2]=*(epicsUInt8 *)rec->c;
  }

  if(byteCount >= 4 && rec->nod ==1) {
    vala[3]=*(epicsUInt8 *)rec->d;
  }

  if(byteCount >= 5 && rec->noe ==1) {
    vala[4]=*(epicsUInt8 *)rec->e;
  }

  if(byteCount >= 6 && rec->nof ==1) {
    vala[5]=*(epicsUInt8 *)rec->f;
  }

  if(byteCount >= 7 && rec->nog ==1) {
    vala[6]=*(epicsUInt8 *)rec->g;
  }

  if(byteCount >= 8 && rec->noh ==1) {
    vala[7]=*(epicsUInt8 *)rec->h;
  }

  if(byteCount >= 9 && rec->noi ==1) {
    vala[8]=*(epicsUInt8 *)rec->i;
  }

  if(byteCount >= 10 && rec->noj ==1) {
    vala[9]=*(epicsUInt8 *)rec->j;
  }

  if(byteCount >= 11 && rec->nok ==1) {
    vala[10]=*(epicsUInt8 *)rec->k;
  }

  if(byteCount >= 12 && rec->nol ==1) {
    vala[11]=*(epicsUInt8 *)rec->l;
  }

  if(byteCount >= 13 && rec->nom ==1) {
    vala[12]=*(epicsUInt8 *)rec->m;
  }

  if(byteCount >= 13 && rec->non ==1) {
    vala[13]=*(epicsUInt8 *)rec->n;
  }

  if(byteCount >= 14 && rec->noo ==1) {
    vala[14]=*(epicsUInt8 *)rec->o;
  }

  if(byteCount >= 15 && rec->nop ==1) {
    vala[15]=*(epicsUInt8 *)rec->p;
  }

  if(byteCount >= 16 && rec->noq ==1) {
    vala[16]=*(epicsUInt8 *)rec->q;
  }

  if(byteCount >= 17 && rec->nor ==1) {
    vala[17]=*(epicsUInt8 *)rec->r;
  }

  if(byteCount >= 18 && rec->nos ==1) {
    vala[18]=*(epicsUInt8 *)rec->s;
  }

   return 0;
}

/*
--------------------------------------------------------------------------------
EPICS database example
record(aSub,   "$(P)CAN${CH_ID}-BasicConfigPackArray_") {
  field(INAM,  "ecmcByteToArrayInit")
  field(SNAM,  "ecmcByteToArray")
  field(FTA,   "UCHAR")
  field(NOA,   "1")
  field(INPA,  "$(P)CAN${CH_ID}-BasicConfigB0_.VAL")    # Byte 0
  field(FTB,   "UCHAR")
  field(NOB,   "1")
  field(INPB,  "$(P)CAN${CH_ID}-VrefPwrCmdCalcB1_.VAL") # Byte 1
  field(FTC,   "UCHAR")
  field(NOC,   "1")
  field(INPC,  "$(P)CAN${CH_ID}-VrefPwrCmdCalcB2_.VAL") # Byte 2
  field(FTD,   "UCHAR")
  field(NOD,   "1")
  field(INPD,  "$(P)CAN${CH_ID}-VdcCtrlCmdCalcB3_.VAL") # Byte 3
  field(FTE,   "UCHAR")
  field(NOE,   "1")
  field(INPE,  "$(P)CAN${CH_ID}-VdcCtrlCmdCalcB4_.VAL") # Byte 4
  field(FTF,   "UCHAR")
  field(NOF,   "1")
  field(INPF,  "0")                                     # Byte 5
  field(FTG,   "UCHAR")
  field(NOG,   "1")
  field(INPG,  "0")                                     # Byte 6
  field(FTVA,  "UCHAR")
  field(OUTA,  "$(P)CAN${CH_ID}-SDO02-BasicConfig")
  field(NOVA,  "7")                                     # 7 bytes (0..6 corresponds to input A..G)
  field(FLNK,  "$(P)CAN${CH_ID}-SDO02-BasicConfig.PROC") # Send the data
}
--------------------------------------------------------------------------------
*/
