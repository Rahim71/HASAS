/*

    Correlator class for dipole array
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

#include "CorrDipole.hh"


clCorrDipole::clCorrDipole (clArrayDipole *ArrayPtr, GDT fIntegrationTime, 
    int iSampleRate, long lWindowSize, bool bDisableFilter, bool bEnableDebug)
{
    long lSensCntr;
    
    bDebug = bEnableDebug;
    bFilter = !bDisableFilter;
    Array = ArrayPtr;
    lWinSize = lWindowSize;
    lSampleCount = (long) ((GDT) iSampleRate * fIntegrationTime + (GDT) 0.5);
    if ((lSampleCount % lWinSize) > (lWinSize / 2))
        lSampleCount += lSampleCount % lWinSize;
    else
        lSampleCount -= lSampleCount % lWinSize;
    fIntTime = (GDT) lSampleCount / (GDT) iSampleRate;
    if (bDebug) printf("Integration time %f -> %f\n", fIntegrationTime,
        fIntTime);
    lBaseIdx = Array->GetMaxDelay();
    if (bDebug) printf("Integration buffers %li/%li samples (%li/%li bytes)\n",
        lSampleCount, lBaseIdx + lSampleCount,
        lSampleCount * (long) sizeof(GDT), 
        (lBaseIdx + lSampleCount) * (long) sizeof(GDT));
    for (lSensCntr = 0; lSensCntr < 2; lSensCntr++)
    {
        Data[lSensCntr].Size((lBaseIdx + lSampleCount) * sizeof(GDT));
        DSPBank[lSensCntr].Zero((GDT *) Data[lSensCntr], 
            lBaseIdx + lSampleCount);
    }
    if (bDebug && !bFilter) printf("Array phi/2 filtering disabled\n");
}


clCorrDipole::~clCorrDipole ()
{
}


bool clCorrDipole::AddData ()
{
    long lReBufRes;
    long lSensCntr;
    GDT *fpSensPtr;
    
    for (lSensCntr = 0; lSensCntr < 2; lSensCntr++)
    {
        fpSensPtr = Data[lSensCntr];
        if (bFilter)
        {
            lReBufRes = DSPBank[lSensCntr].ReBuffer(&fpSensPtr[lBaseIdx], 
                Array->GetFiltPtr(lSensCntr), lSampleCount, lWinSize);
        }
        else
        {
            lReBufRes = DSPBank[lSensCntr].ReBuffer(&fpSensPtr[lBaseIdx], 
                Array->GetRawPtr(lSensCntr), lSampleCount, lWinSize);
        }
    }
    if (lReBufRes >= 1) return true;
    return false;
}


GDT clCorrDipole::Process (GDT fDirection)
{
    long lDelay1;
    long lDelay2;
    GDT fCrossCorrelation;
    GDT *fpDataPtr1;
    GDT *fpDataPtr2;

    lDelay1 = Array->GetDelaySamples(0, fDirection);
    lDelay2 = Array->GetDelaySamples(1, fDirection);
    fpDataPtr1 = Data[0];
    fpDataPtr2 = Data[1];
    fCrossCorrelation = DSP.CrossCorr(&fpDataPtr1[lBaseIdx - lDelay1],
        &fpDataPtr2[lBaseIdx - lDelay2], lSampleCount);
    return fCrossCorrelation;
}


void clCorrDipole::SetHistory ()
{
    long lSensCntr;
    GDT *fpDataPtr;

    for (lSensCntr = 0; lSensCntr < 2; lSensCntr++)
    {
        fpDataPtr = Data[lSensCntr];
        DSPBank[lSensCntr].Copy(fpDataPtr, &fpDataPtr[lSampleCount],
            lBaseIdx);
    }    
}


GDT clCorrDipole::GetPeakLevel ()
{
    long lSensCntr;
    GDT fBufMin;
    GDT fBufMax;
    GDT fRealMax;
    GDT *fpDataPtr;

    fRealMax = (GDT) 0.0;
    for (lSensCntr = 0; lSensCntr < 2; lSensCntr++)
    {
        fpDataPtr = Data[lSensCntr];
        DSPBank[lSensCntr].MinMax(&fBufMin, &fBufMax, &fpDataPtr[lBaseIdx], 
            lSampleCount);
        fBufMin = (GDT) fabs(fBufMin);
        if (fBufMin > fRealMax) fRealMax = fBufMin;
        else if (fBufMax > fRealMax) fRealMax = fBufMax;
    }
    if (fRealMax > (GDT) 0.0)
        return ((GDT) (20.0 * log10(fRealMax)));
    return (-DIR_DB_SCALE);
}

