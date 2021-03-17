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
static long ecmcWriteArrayUint8ElementsInit(struct aSubRecord *rec);
epicsRegisterFunction(ecmcWriteArrayUint8ElementsInit);

// declare worker function
static long ecmcWriteArrayUint8Elements(struct aSubRecord *rec);
epicsRegisterFunction(ecmcWriteArrayUint8Elements);

// init (INAM)
static long ecmcWriteArrayUint8ElementsInit(struct aSubRecord *rec){
  std::cout << "ecmcWriteArrayUint8ElementsInit aSubRecord: "<< rec->name << std::endl;
  printf("ecmcWriteArrayUint8ElementsInit aSubRecord: %d\n",rec->noa);
  return 0;
}

////noa elements in A
////Note that the subroutine code must always handle the value fields (A-U, VALA-VALU) as arrays, even if they contain only a single element.
//    /*memcpy(pwfData, (double *)precord->a, precord->noa *
//sizeof(double));
////data processing on pwfData: std, max, min, fft,...
//    ...
//
//    /* put the calculated results into ai records*/
//    *(double *)precord->vala = ave;*/
//
//// work (SNAM)
// NEVA output filed size
static long ecmcWriteArrayUint8Elements(struct aSubRecord *rec){
   printf("EXECUTING!!!!!\n");

  // input A must be size of output array
  if(rec->noa !=1){
    printf("Input A not scalar\n");
    return 0;
  }
  
  epicsUInt8 sizeOfBuffer=*(epicsUInt8*)rec->a;
  
  // Max 20 byte in a row
  if(sizeOfBuffer <=0 || sizeOfBuffer>18){
    printf("Input A out of range\n");
    return 0;
  }
  

  if(sizeOfBuffer != rec->nova){
    printf("Size missmatch\n");
    return 0;
  }

      
  printf("EXECUTING!!!!!\n");
  epicsUInt8 *buffer;
  buffer=new epicsUInt8[rec->noa];
  memset(buffer,0,sizeOfBuffer);
  
  if(sizeOfBuffer >= 1 && rec->nob ==1) {
    buffer[1]=*(epicsUInt8 *)rec->b;
  }

  if(sizeOfBuffer >= 2 && rec->noc ==1) {
    buffer[2]=*(epicsUInt8 *)rec->c;
  }

  if(sizeOfBuffer >= 3 && rec->nod ==1) {
    buffer[3]=*(epicsUInt8 *)rec->d;
  }

  if(sizeOfBuffer >= 4 && rec->noe ==1) {
    buffer[4]=*(epicsUInt8 *)rec->e;
  }

  if(sizeOfBuffer >= 5 && rec->nof ==1) {
    buffer[5]=*(epicsUInt8 *)rec->f;
  }

  if(sizeOfBuffer >= 6 && rec->nog ==1) {
    buffer[6]=*(epicsUInt8 *)rec->g;
  }

  if(sizeOfBuffer >= 7 && rec->noh ==1) {
    buffer[7]=*(epicsUInt8 *)rec->h;
  }

  if(sizeOfBuffer >= 8 && rec->noi ==1) {
    buffer[8]=*(epicsUInt8 *)rec->i;
  }

  if(sizeOfBuffer >= 9 && rec->noj ==1) {
    buffer[9]=*(epicsUInt8 *)rec->j;
  }

  if(sizeOfBuffer >= 10 && rec->nok ==1) {
    buffer[10]=*(epicsUInt8 *)rec->k;
  }

  if(sizeOfBuffer >= 11 && rec->nol ==1) {
    buffer[11]=*(epicsUInt8 *)rec->l;
  }

  if(sizeOfBuffer >= 12 && rec->nom ==1) {
    buffer[12]=*(epicsUInt8 *)rec->m;
  }

  if(sizeOfBuffer >= 13 && rec->non ==1) {
    buffer[13]=*(epicsUInt8 *)rec->n;
  }

  if(sizeOfBuffer >= 14 && rec->noo ==1) {
    buffer[14]=*(epicsUInt8 *)rec->o;
  }

  if(sizeOfBuffer >= 15 && rec->nop ==1) {
    buffer[15]=*(epicsUInt8 *)rec->p;
  }

  if(sizeOfBuffer >= 16 && rec->noq ==1) {
    buffer[16]=*(epicsUInt8 *)rec->q;
  }

  if(sizeOfBuffer >= 17 && rec->nor ==1) {
    buffer[17]=*(epicsUInt8 *)rec->r;
  }

  if(sizeOfBuffer >= 18 && rec->nos ==1) {
    buffer[18]=*(epicsUInt8 *)rec->s;
  }

//  if(sizeOfBuffer >= 19 && rec->not ==1) {
//    buffer[19]=*(epicsUInt8 *)rec->t;
//  }
//
//  if(sizeOfBuffer >= 20 && rec->nou ==1) {
//    buffer[20]=*(epicsUInt8 *)rec->u;
//  }

  // better to use vala direct instead of buffer... hmm
  epicsUInt8	*vala;
  vala = (epicsUInt8 *)rec->vala;
  for(int i = 0; i<sizeOfBuffer; i++) {
    vala[i] = buffer[i];        
  }

   printf("EXECUTED!!!!!\n");

   return 0;

}

/*
--------------------------------------------------------------------------------
EPICS database example
record(aSub,  "${SYS}:ECATtimestamp") {
  field(DESC,  "ECAT timestamp")
  field(INAM,  "ECATtimestampInit")
  field(SNAM,  "ECATtimestamp")
  field(FTA,   "DOUBLE")
  field(NOA,   1)
  field(TSE,  -2)
}
--------------------------------------------------------------------------------
*/
