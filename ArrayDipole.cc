/*

    Class for dipole array operations
    Copyright (C) 1999-2002 Jussi Laako

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "ArrayDipole.hh"


clArrayDipole::clArrayDipole ()
{
    bInitialized = false;
}


clArrayDipole::~clArrayDipole ()
{
    if (bInitialized) Uninitialize();
}


bool clArrayDipole::Initialize (GDT fSensorSpacing, GDT fSndSpeed, 
    int iSRate, long lWinSize, GDT fLowFreqLimit, bool bEnableDebug)
{
    long lSensCntr;
    GDT *fpNullPtr = NULL;
    GDT fLowFreq;
    GDT fHighFreq;
    GDT fAudioLow;

    if (bEnableDebug) EnableDebug();
    if (bDebug) printf("Initializing dipole array\n");
    lWindowSize = lWinSize;
    iSampleRate = iSRate;
    if (bDebug) printf("Window size %li\n", lWindowSize);
    SetSoundSpeed(fSndSpeed);
    SetSampleRate(iSampleRate);
    fSpacing = fSensorSpacing;
    fDelay = fSpacing * fSecsPerMeter;
    if (bDebug) printf("Requested array frequency band %g - %g Hz\n",
        fLowFreqLimit, GetArrayFreq());
    if (GetMaxDelay() > lWindowSize)
        lBufferSize = GetMaxDelay() / lWindowSize * lWindowSize + lWindowSize;
    else
        lBufferSize = lWindowSize * 2;
    if (bDebug) printf("Buffer size %li (%li total)\n", lBufferSize,
        lBufferSize * 2);
    lBaseIdx = lBufferSize - lWindowSize;
    for (lSensCntr = 0; lSensCntr < 2; lSensCntr++)
    {
        if (!FilterBank[lSensCntr].Initialize(lWindowSize, fpNullPtr, true)) 
            return false;
        fLowFreq = fLowFreqLimit;
        fHighFreq = GetArrayFreq();
        FilterBank[lSensCntr].DesignNarrowBP(&fLowFreq, &fHighFreq, 
            iSampleRate);
        RawBuf[lSensCntr].Size(lBufferSize * sizeof(GDT));
        FiltBuf[lSensCntr].Size(lBufferSize * sizeof(GDT));
        HFBuf[lSensCntr].Size(lWindowSize * sizeof(GDT));
        DSP.Zero((GDT *) RawBuf[lSensCntr], lBufferSize);
        DSP.Zero((GDT *) FiltBuf[lSensCntr], lBufferSize);
        if (!FilterBank[2 + lSensCntr].Initialize(lWindowSize, fpNullPtr, 
            true)) return false;
        fAudioLow = fHighFreq;
        FilterBank[2 + lSensCntr].DesignHP(&fAudioLow, iSampleRate);
    }
    if (bDebug) printf("Filter size %li\n", lWindowSize * 2);
    if (bDebug) printf("Using frequency band %g - %g Hz for array\n",
        fLowFreq, fHighFreq);
    if (bDebug) printf("High frequency audio lower corner at %g Hz\n",
        fAudioLow);
    bInitialized = true;
    return true;
}


void clArrayDipole::Uninitialize ()
{
    long lSensCntr;

    bInitialized = false;
    for (lSensCntr = 0; lSensCntr < 2; lSensCntr++)
    {
        FilterBank[lSensCntr].Uninitialize();
        FilterBank[2 + lSensCntr].Uninitialize();
    }
}


GDT *clArrayDipole::GetRawPtr (long lSensor)
{
    GDT *fpSensPtr = RawBuf[lSensor];
    return (&fpSensPtr[lBaseIdx]);
}


GDT *clArrayDipole::GetFiltPtr (long lSensor)
{
    GDT *fpSensPtr = FiltBuf[lSensor];
    return (&fpSensPtr[lBaseIdx]);
}


GDT *clArrayDipole::GetHFPtr (long lSensor)
{
    GDT *fpSensPtr = HFBuf[lSensor];
    return (&fpSensPtr[lBaseIdx]);
}


long clArrayDipole::GetMaxDelay ()
{
    return ((long) (fDelay / fSampleSpacing + (GDT) 0.5));
}


GDT clArrayDipole::GetArrayFreq ()
{
    return ((GDT) 1.0 / fDelay / (GDT) 2.0);
}


GDT clArrayDipole::GetDelay (GDT fDirection)
{
    return (sin(fDirection) * fDelay);
}


GDT clArrayDipole::GetDelayTime (long lSensor, GDT fDirection)
{
    if (fDirection == (GDT) 0.0) return ((GDT) 0.0);
    else if (fDirection > (GDT) 0.0 && lSensor == 1)
    {
        return GetDelay(fDirection);
    }
    else if (fDirection < (GDT) 0.0 && lSensor == 0)
    {
        return fabs(GetDelay(fDirection));
    }
    return ((GDT) 0.0);
}


long clArrayDipole::GetDelaySamples (long lSensor, GDT fDirection)
{
    if (fDirection == (GDT) 0.0) return (0L);
    else if (fDirection > (GDT) 0.0 && lSensor == 1)
    {
        return ((long) 
            (GetDelay(fDirection) / fSampleSpacing + (GDT) 0.5));
    }
    else if (fDirection < (GDT) 0.0 && lSensor == 0)
    {
        return ((long) 
            (fabs(GetDelay(fDirection)) / fSampleSpacing + (GDT) 0.5));
    }
    return (0L);
}


void clArrayDipole::SetLowFreqLimit (GDT fLowFreqLimit)
{
    long lSensCntr;
    GDT fLowFreq;
    GDT fHighFreq;

    for (lSensCntr = 0; lSensCntr < 2; lSensCntr++)
    {
        fLowFreq = fLowFreqLimit;
        fHighFreq = GetArrayFreq();
        FilterBank[lSensCntr].DesignNarrowBP(&fLowFreq, &fHighFreq, 
            iSampleRate);
    }
    if (bDebug) printf("Using frequency band %g - %g Hz for array\n",
        fLowFreq, fHighFreq);
}


void clArrayDipole::AddData (const GDT *fpSource, long lStartIndex,
    long lChannels)
{
    long lBase;
    long lMoveIdx;
    long lDataCntr;
    GDT *fpRaw1 = RawBuf[0];
    GDT *fpRaw2 = RawBuf[1];
    GDT *fpDest1 = FiltBuf[0];
    GDT *fpDest2 = FiltBuf[1];
    GDT *fpHF1 = HFBuf[0];
    GDT *fpHF2 = HFBuf[1];

    lBase = lBufferSize - lWindowSize;
    lMoveIdx = lWindowSize;
    DSP.Copy(fpRaw1, &fpRaw1[lMoveIdx], lBase);
    DSP.Copy(fpRaw2, &fpRaw2[lMoveIdx], lBase);
    DSP.Copy(fpDest1, &fpDest1[lMoveIdx], lBase);
    DSP.Copy(fpDest2, &fpDest2[lMoveIdx], lBase);
    for (lDataCntr = 0; lDataCntr < lWindowSize; lDataCntr++)
    {
        fpRaw1[lBase + lDataCntr] = fpDest1[lBase + lDataCntr] = 
            fpHF1[lDataCntr] = 
            fpSource[lDataCntr * lChannels + lStartIndex];
        fpRaw2[lBase + lDataCntr] = fpDest2[lBase + lDataCntr] = 
            fpHF2[lDataCntr] = 
            fpSource[lDataCntr * lChannels + lStartIndex + 1];
    }
    /*FilterBank[0].ProcessBP(&fpDest1[lBase]);
    FilterBank[1].ProcessBP(&fpDest2[lBase]);*/
    FilterBank[0].Process(&fpDest1[lBase]);
    FilterBank[1].Process(&fpDest2[lBase]);
    FilterBank[2].Process(fpHF1);
    FilterBank[3].Process(fpHF2);
}


void clArrayDipole::GetRawData (GDT **fpaDest, GDT fDirection)
{
    long lDelay1;
    long lDelay2;
    GDT *fpDest1 = fpaDest[0];
    GDT *fpDest2 = fpaDest[1];
    GDT *fpSrc1 = RawBuf[0];
    GDT *fpSrc2 = RawBuf[1];

    lDelay1 = GetDelaySamples(0, fDirection);
    lDelay2 = GetDelaySamples(1, fDirection);
    DSP.Copy(fpDest1, &fpSrc1[lBaseIdx - lDelay1], lWindowSize);
    DSP.Copy(fpDest2, &fpSrc2[lBaseIdx - lDelay2], lWindowSize);
}


GDT *clArrayDipole::GetRawDataPtr (long lSensor, GDT fDirection)
{
    long lDelayCount;
    GDT *fpSource;

    fpSource = RawBuf[lSensor];
    lDelayCount = GetDelaySamples(lSensor, fDirection);
    return (&fpSource[lBaseIdx - lDelayCount]);
}


void clArrayDipole::GetFilteredData (GDT **fpaDest, GDT fDirection)
{
    long lDelay1;
    long lDelay2;
    GDT *fpDest1 = fpaDest[0];
    GDT *fpDest2 = fpaDest[1];
    GDT *fpSrc1 = FiltBuf[0];
    GDT *fpSrc2 = FiltBuf[1];

    lDelay1 = GetDelaySamples(0, fDirection);
    lDelay2 = GetDelaySamples(1, fDirection);
    DSP.Copy(fpDest1, &fpSrc1[lBaseIdx - lDelay1], lWindowSize);
    DSP.Copy(fpDest2, &fpSrc2[lBaseIdx - lDelay2], lWindowSize);
}


GDT *clArrayDipole::GetFilteredDataPtr (long lSensor, GDT fDirection)
{
    long lDelayCount;
    GDT *fpSource;

    fpSource = FiltBuf[lSensor];
    lDelayCount = GetDelaySamples(lSensor, fDirection);
    return (&fpSource[lBaseIdx - lDelayCount]);
}


void clArrayDipole::GetAudioData (GDT *fpDest, GDT fDirection, bool bFullBand)
{
    long lDelay1;
    long lDelay2;
    long lDataCntr;
    GDT fScaler;
    GDT *fpSrc1 = FiltBuf[0];
    GDT *fpSrc2 = FiltBuf[1];
    GDT *fpSrc3;

    lDelay1 = GetDelaySamples(0, fDirection);
    lDelay2 = GetDelaySamples(1, fDirection);
    if (bFullBand)
    {
        fScaler = (GDT) 1.0 / (GDT) 3.0;
        if (fDirection >= 0.0)
            fpSrc3 = HFBuf[0];
        else
            fpSrc3 = HFBuf[1];
        for (lDataCntr = 0; lDataCntr < lWindowSize; lDataCntr++)
        {
            fpDest[lDataCntr] =
                (fpSrc1[lBaseIdx - lDelay1 + lDataCntr] +
                fpSrc2[lBaseIdx - lDelay2 + lDataCntr] +
                fpSrc3[lDataCntr]) * fScaler;
        }
    }
    else
    {
        DSP.Mix(fpDest, &fpSrc1[lBaseIdx - lDelay1], 
            &fpSrc2[lBaseIdx - lDelay2], lWindowSize);
    }
}

