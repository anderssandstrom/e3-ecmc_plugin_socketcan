/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcFFTWrap.h
*
*  Created on: Mar 22, 2020
*      Author: anderssandstrom
*
\*************************************************************************/
#ifndef ECMC_FFT_WRAP_H_
#define ECMC_FFT_WRAP_H_
#include "ecmcFFTDefs.h"

# ifdef __cplusplus
extern "C" {
# endif  // ifdef __cplusplus

/** \brief Create new FFT object
 *
 *  The plugin supports creation of multiple FFT objects\n
 *  (if loaded several times).\n
 *  The different fft are adressed by fftindex (in other functions below).\n
 *  The first loaded fft get index 0 and then increases for each load.\n
 *  This function call will create the custom asynparameters dedicated for this plugin.\
 *  The configuration string needs to define a data source by:\n
 *  "SOURCE=<data source>;"\n
 *  Example:\n
 *  "SOURCE=ec0.s1.AI_1";\n
 *  \param[in] configStr Configuration string.\n
 *
 *  \return 0 if success or otherwise an error code.\n
 */
int         createFFT(char *configStr);

/** \brief Enable/disable FFT object
 *
 *  Enable/disable FFT object. If disabled no data will be acquired\n
 *  and no calculations will be made.\n
 *  \param[in] fftIndex Index of fft (first loaded fft have index 0 then increases)\n
 *  \param[in] enable enable/disable (1/0).\n
 *
 *  \return 0 if success or otherwise an error code.\n
 */
int         enableFFT(int fftIndex, int enable);

/** \brief Clear FFT object\n
 *
 *  Clears buffers. After this command the acquistion can start from scratch.\n
 *  \param[in] fftIndex Index of fft (first loaded fft have index 0 then increases)\n
 *
 *  \return 0 if success or otherwise an error code.\n
 */
int         clearFFT(int fftIndex);

/** \brief Set mode of FFT object
 *
 *  The FFT object can measure in two differnt modes:\n
 *    CONT(1) : Continious measurement (Acq data, calc, then Acq data ..)\n
 *    TRIGG(2): Measurements are triggered from plc or over asyn and is only done once (untill next trigger)\n
 *  \param[in] fftIndex Index of fft (first loaded fft have index 0 then increases)\n
 *  \param[in] mode Mode CONT(1) or TRIGG(2)\n
 *
 *  \return 0 if success or otherwise an error code.\n
 */
int         modeFFT(int fftIndex, FFT_MODE mode);

/** \brief Trigger FFT object\n
 *
 *  If in triggered mode a new measurment cycle is initiated (fft will be cleared first).\n
 *  \param[in] fftIndex Index of fft (first loaded fft have index 0 then increases)\n
 *
 *  \return 0 if success or otherwise an error code.\n
 */
int         triggFFT(int fftIndex);

/** \brief Get status of FFT object
 *
 *  The FFT object can be in different states:\n
 *    NO_STAT(0): Invalid state (something is most likely wrong)\n
 *    IDLE(1)   : Waiting for trigger in triggered mode\n
 *    ACQ(2)    : Acquiring data (filling data buffer)\n
 *    CALC(3)   : Calculating FFT results\n
 *  \param[in] fftIndex Index of fft (first loaded fft have index 0 then increases)\n
 *
 *  \return Status of fft (if index is out of range NO_STAT will be returned).\n
 */
FFT_STATUS  statFFT(int fftIndex);

/** \brief Link data to _all_ fft objects
 *
 *  This tells the FFT lib to connect to ecmc to find it's data source.\n
 *  This function should be called just before entering realtime since then all\n
 *  data sources in ecmc will be definded (plc sources are compiled just before runtime\n
 *  so are only fist accesible now).\n
 *  \return 0 if success or otherwise an error code.\n
 */
int  linkDataToFFTs();

/** \brief Deletes all created fft objects\n
 *
 * Should be called when destructs.\n
 */

void deleteAllFFTs();

# ifdef __cplusplus
}
# endif  // ifdef __cplusplus

#endif  /* ECMC_FFT_WRAP_H_ */
