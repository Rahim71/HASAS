/*

    Background noise estimation and removal
    Copyright (C) 2000-2001 Jussi Laako

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


#include <math.h>
#include <float.h>
#include <stdio.h>

#ifdef USE_GSL
#include <gsl/gsl_sort_float.h>
#include <gsl/gsl_sort_double.h>
#endif

#include "RemoveNoise.hh"


inline void clRemoveNoise::GetPosSize (long *lpStartIdx, long *lpMeanCount,
    long lCenter, long lMeanLength, long lWinLength)
{
    long lHalfMeanLen;
    long lEndIdx;

    lHalfMeanLen = lMeanLength / 2L;
    *lpStartIdx = lCenter - lHalfMeanLen;
    if (*lpStartIdx < 0L) *lpStartIdx = 0L;
    lEndIdx = lCenter + lHalfMeanLen;
    if (lEndIdx >= lWinLength) lEndIdx = lWinLength - 1L;
    *lpMeanCount = lEndIdx - *lpStartIdx + 1L;
}


inline void clRemoveNoise::GetPosSize1 (long *lpStartIdx, long *lpMeanCount,
    long lCenter, long lMeanLength, long lGapLength, long lWinLength)
{
    long lHalfMeanLen;
    long lHalfGapLen;
    long lEndIdx;

    lHalfMeanLen = lMeanLength / 2L;
    lHalfGapLen = (lGapLength + 1L) / 2L;
    *lpStartIdx = lCenter - lHalfGapLen - lHalfMeanLen;
    if (*lpStartIdx < 0L) *lpStartIdx = 0L;
    lEndIdx = lCenter - lHalfGapLen;
    if (lEndIdx < 0L) lEndIdx = 0L;
    *lpMeanCount = lEndIdx - *lpStartIdx + 1L;
}


inline void clRemoveNoise::GetPosSize2 (long *lpStartIdx, long *lpMeanCount,
    long lCenter, long lMeanLength, long lGapLength, long lWinLength)
{
    long lHalfMeanLen;
    long lHalfGapLen;
    long lEndIdx;

    lHalfMeanLen = lMeanLength / 2L;
    lHalfGapLen = (lGapLength + 1L) / 2L;
    *lpStartIdx = lCenter + lHalfGapLen;
    if (*lpStartIdx >= lWinLength) *lpStartIdx = lWinLength - 1L;
    lEndIdx = lCenter + lHalfGapLen + lHalfMeanLen;
    if (lEndIdx >= lWinLength) lEndIdx = lWinLength - 1L;
    *lpMeanCount = lEndIdx - *lpStartIdx + 1L;
}


inline long clRemoveNoise::GetAlphaMedian (const float *fpSrc, float fAlpha,
    long lLength)
{
    long lAlphaMedian;
    float fMedian;

    fMedian = ((lLength % 2L) != 0L) ?
        fpSrc[(lLength - 1L) / 2L] :
        (0.5F * (fpSrc[lLength / 2L - 1L] + fpSrc[lLength / 2L]));
    for (lAlphaMedian = lLength / 2L - 1L; 
        lAlphaMedian < lLength; 
        lAlphaMedian++)
    {
        if (fpSrc[lAlphaMedian] > (fAlpha * fMedian)) return lAlphaMedian;
    }
    return lLength;
}


inline long clRemoveNoise::GetAlphaMedian (const double *dpSrc, double dAlpha,
    long lLength)
{
    long lAlphaMedian;
    double dMedian;

    dMedian = ((lLength % 2L) != 0L) ?
        dpSrc[(lLength - 1L) / 2L] :
        (0.5 * (dpSrc[lLength / 2L - 1L] + dpSrc[lLength / 2L]));
    for (lAlphaMedian = lLength / 2L - 1L;
        lAlphaMedian < lLength;
        lAlphaMedian++)
    {
        if (dpSrc[lAlphaMedian] > (dAlpha * dMedian)) return lAlphaMedian;
    }
    return lLength;
}


clRemoveNoise::clRemoveNoise ()
{
    lPrevSize = 0L;
}


clRemoveNoise::~clRemoveNoise ()
{
}


void clRemoveNoise::TPSW (float *fpVect, float fAlpha, long lMeanLength,
    long lGapLength, long lLength)
{
    long lLoopCntr;
    long lStartIdx;
    long lMeanCount;
    float fTempMean;
    #ifdef __GNUG__
    float fpClippedMean[lLength];
    #else
    clDSPAlloc ClippedMean;
    float *fpClippedMean = (float *) 
        ClippedMean.Size(lLength * sizeof(float));
    #endif

    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize1(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength, 
            lGapLength, lLength);
        fTempMean = Mean(&fpVect[lStartIdx], lMeanCount);
        GetPosSize2(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength,
            lGapLength, lLength);
        fTempMean += Mean(&fpVect[lStartIdx], lMeanCount);
        fTempMean *= 0.5f;
        fpClippedMean[lLoopCntr] = 
            (fpVect[lLoopCntr] > (fAlpha * fTempMean)) ?
            fTempMean : fpVect[lLoopCntr];
    }
    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize1(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength, 
            lGapLength, lLength);
        fTempMean = Mean(&fpClippedMean[lStartIdx], lMeanCount);
        GetPosSize2(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength,
            lGapLength, lLength);
        fTempMean += Mean(&fpClippedMean[lStartIdx], lMeanCount);
        fTempMean *= 0.5f;
        fpVect[lLoopCntr] = (fTempMean != 0.0f) ?
            (fpVect[lLoopCntr] - fTempMean) / fTempMean : 0.0f;
    }
}


void clRemoveNoise::TPSW (double *dpVect, double dAlpha, long lMeanLength,
    long lGapLength, long lLength)
{
    long lLoopCntr;
    long lStartIdx;
    long lMeanCount;
    double dTempMean;
    #ifdef __GNUG__
    double dpClippedMean[lLength];
    #else
    clDSPAlloc ClippedMean;
    double *dpClippedMean = (double *) 
        ClippedMean.Size(lLength * sizeof(double));
    #endif

    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize1(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength, 
            lGapLength, lLength);
        dTempMean = Mean(&dpVect[lStartIdx], lMeanCount);
        GetPosSize2(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength,
            lGapLength, lLength);
        dTempMean += Mean(&dpVect[lStartIdx], lMeanCount);
        dTempMean *= 0.5;
        dpClippedMean[lLoopCntr] =
            (dpVect[lLoopCntr] > (dAlpha * dTempMean)) ?
            dTempMean : dpVect[lLoopCntr];
    }
    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize1(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength, 
            lGapLength, lLength);
        dTempMean = Mean(&dpClippedMean[lStartIdx], lMeanCount);
        GetPosSize2(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength,
            lGapLength, lLength);
        dTempMean += Mean(&dpClippedMean[lStartIdx], lMeanCount);
        dTempMean *= 0.5;
        dpVect[lLoopCntr] = (dTempMean != 0.0) ?
            (dpVect[lLoopCntr] - dTempMean) / dTempMean : 0.0;
    }
}


void clRemoveNoise::TPSW (float *fpDest, const float *fpSrc, float fAlpha, 
    long lMeanLength, long lGapLength, long lLength)
{
    long lLoopCntr;
    long lStartIdx;
    long lMeanCount;
    float fTempMean;
    #ifdef __GNUG__
    float fpClippedMean[lLength];
    #else
    clDSPAlloc ClippedMean;
    float *fpClippedMean = (float *)
        ClippedMean.Size(lLength * sizeof(float));
    #endif

    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize1(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength, 
            lGapLength, lLength);
        fTempMean = Mean(&fpSrc[lStartIdx], lMeanCount);
        GetPosSize2(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength,
            lGapLength, lLength);
        fTempMean += Mean(&fpSrc[lStartIdx], lMeanCount);
        fTempMean *= 0.5f;
        fpClippedMean[lLoopCntr] =
            (fpSrc[lLoopCntr] > (fAlpha * fTempMean)) ?
            fTempMean : fpSrc[lLoopCntr];
    }
    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize1(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength, 
            lGapLength, lLength);
        fTempMean = Mean(&fpClippedMean[lStartIdx], lMeanCount);
        GetPosSize2(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength,
            lGapLength, lLength);
        fTempMean += Mean(&fpClippedMean[lStartIdx], lMeanCount);
        fTempMean *= 0.5f;
        fpDest[lLoopCntr] = (fTempMean != 0.0f) ?
            (fpSrc[lLoopCntr] - fTempMean) / fTempMean : 0.0f;
    }
}


void clRemoveNoise::TPSW (double *dpDest, const double *dpSrc, double dAlpha,
    long lMeanLength, long lGapLength, long lLength)
{
    long lLoopCntr;
    long lStartIdx;
    long lMeanCount;
    double dTempMean;
    #ifdef __GNUG__
    double dpClippedMean[lLength];
    #else
    clDSPAlloc ClippedMean;
    double *dpClippedMean = (double *)
        ClippedMean.Size(lLength * sizeof(double));
    #endif

    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize1(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength, 
            lGapLength, lLength);
        dTempMean = Mean(&dpSrc[lStartIdx], lMeanCount);
        GetPosSize2(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength,
            lGapLength, lLength);
        dTempMean += Mean(&dpSrc[lStartIdx], lMeanCount);
        dTempMean *= 0.5;
        dpClippedMean[lLoopCntr] =
            (dpSrc[lLoopCntr] > (dAlpha * dTempMean)) ?
            dTempMean : dpSrc[lLoopCntr];
    }
    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize1(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength, 
            lGapLength, lLength);
        dTempMean = Mean(&dpClippedMean[lStartIdx], lMeanCount);
        GetPosSize2(&lStartIdx, &lMeanCount, lLoopCntr, lMeanLength,
            lGapLength, lLength);
        dTempMean += Mean(&dpClippedMean[lStartIdx], lMeanCount);
        dTempMean *= 0.5;
        dpDest[lLoopCntr] = (dTempMean != 0.0) ?
            (dpSrc[lLoopCntr] - dTempMean) / dTempMean : 0.0;
    }
}


void clRemoveNoise::OTA (float *fpVect, float fAlpha, long lMeanLength,
    long lLength)
{
    long lLoopCntr;
    long lStartIdx;
    long lMedianCount;
    long lMeanCount;
    float fTempMean;
    #ifdef __GNUG__
    float fpTempWin[lMeanLength];
    float fpNoiseMean[lLength];
    #else
    clDSPAlloc TempWin;
    clDSPAlloc NoiseMean;
    float *fpTempWin = (float *) 
        TempWin.Size(lMeanLength * sizeof(float));
    float *fpNoiseMean = (float *)
        NoiseMean.Size(lLength * sizeof(float));
    #endif

    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize(&lStartIdx, &lMedianCount, lLoopCntr, lMeanLength,
            lLength);
        Copy(fpTempWin, &fpVect[lStartIdx], lMedianCount);
        #ifndef USE_GSL
        Sort(fpTempWin, lMedianCount);
        #else
        gsl_sort_float(fpTempWin, 1, lMedianCount);
        #endif
        lMeanCount = GetAlphaMedian(fpTempWin, fAlpha, lMedianCount);
        fpNoiseMean[lLoopCntr] = Mean(fpTempWin, lMeanCount);
    }
    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        fTempMean = fpNoiseMean[lLoopCntr];
        fpVect[lLoopCntr] = (fTempMean != 0.0f) ?
            (fpVect[lLoopCntr] - fTempMean) / fTempMean : 0.0f;
    }
}


void clRemoveNoise::OTA (double *dpVect, double dAlpha, long lMeanLength,
    long lLength)
{
    long lLoopCntr;
    long lStartIdx;
    long lMedianCount;
    long lMeanCount;
    double dTempMean;
    #ifdef __GNUG__
    double dpTempWin[lMeanLength];
    double dpNoiseMean[lLength];
    #else
    clDSPAlloc TempWin;
    clDSPAlloc NoiseMean;
    double *dpTempWin = (double *)
        TempWin.Size(lMeanLength * sizeof(double));
    double *dpNoiseMean = (double *)
        TempWin.Size(lLength * sizeof(double));
    #endif

    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize(&lStartIdx, &lMedianCount, lLoopCntr, lMeanLength,
            lLength);
        Copy(dpTempWin, &dpVect[lStartIdx], lMedianCount);
        #ifndef USE_GSL
        Sort(dpTempWin, lMedianCount);
        #else
        gsl_sort(dpTempWin, 1, lMedianCount);
        #endif
        lMeanCount = GetAlphaMedian(dpTempWin, dAlpha, lMedianCount);
        dpNoiseMean[lLoopCntr] = Mean(dpTempWin, lMeanCount);
    }
    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        dTempMean = dpNoiseMean[lLoopCntr];
        dpVect[lLoopCntr] = (dTempMean != 0.0) ?
            (dpVect[lLoopCntr] - dTempMean) / dTempMean : 0.0;
    }
}


void clRemoveNoise::OTA (float *fpDest, const float *fpSrc, float fAlpha, 
    long lMeanLength, long lLength)
{
    long lLoopCntr;
    long lStartIdx;
    long lMedianCount;
    long lMeanCount;
    float fTempMean;
    #ifdef __GNUG__
    float fpTempWin[lMeanLength];
    #else
    clDSPAlloc TempWin;
    float *fpTempWin = (float *)
        TempWin.Size(lMeanLength * sizeof(float));
    #endif

    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize(&lStartIdx, &lMedianCount, lLoopCntr, lMeanLength,
            lLength);
        Copy(fpTempWin, &fpSrc[lStartIdx], lMedianCount);
        #ifndef USE_GSL
        Sort(fpTempWin, lMedianCount);
        #else
        gsl_sort_float(fpTempWin, 1, lMedianCount);
        #endif
        lMeanCount = GetAlphaMedian(fpTempWin, fAlpha, lMedianCount);
        fTempMean = Mean(fpTempWin, lMeanCount);
        fpDest[lLoopCntr] = (fTempMean != 0.0f) ?
            (fpSrc[lLoopCntr] - fTempMean) / fTempMean : 0.0f;
    }
}


void clRemoveNoise::OTA (double *dpDest, const double *dpSrc, double dAlpha,
    long lMeanLength, long lLength)
{
    long lLoopCntr;
    long lStartIdx;
    long lMedianCount;
    long lMeanCount;
    double dTempMean;
    #ifdef __GNUG__
    double dpTempWin[lMeanLength];
    #else
    clDSPAlloc TempWin;
    double *dpTempWin = (double *)
        TempWin.Size(lMeanLength * sizeof(double));
    #endif

    for (lLoopCntr = 0; lLoopCntr < lLength; lLoopCntr++)
    {
        GetPosSize(&lStartIdx, &lMedianCount, lLoopCntr, lMeanLength,
            lLength);
        Copy(dpTempWin, &dpSrc[lStartIdx], lMedianCount);
        #ifndef USE_GSL
        Sort(dpTempWin, lMedianCount);
        #else
        gsl_sort(dpTempWin, 1, lMedianCount);
        #endif
        lMeanCount = GetAlphaMedian(dpTempWin, dAlpha, lMedianCount);
        dTempMean = Mean(dpTempWin, lMeanCount);
        dpDest[lLoopCntr] = (dTempMean != 0.0) ?
            (dpSrc[lLoopCntr] - dTempMean) / dTempMean : 0.0;
    }
}


void clRemoveNoise::Diff (float *fpVect, float fWeight, long lLength)
{
    float *fpPrev;
    #ifdef __GNUG__
    float fpWork[lLength];
    #else
    clDSPAlloc Work;
    float *fpWork = (float *) Work.Size(lLength * sizeof(float));
    #endif

    if (lLength != lPrevSize)
    {
        PrevBuf.Size(lLength * sizeof(float));
        Zero((float *) PrevBuf, lLength);
        lPrevSize = lLength;
    }
    fpPrev = PrevBuf;

    Sub(fpWork, fpVect, fpPrev, lLength);
    Abs(fpWork, lLength);
    
    Mul(fpVect, fWeight, lLength);
    Mul(fpPrev, 1.0f - fWeight, lLength);
    Add(fpPrev, fpVect, lLength);
    
    Copy(fpVect, fpWork, lLength);
}


void clRemoveNoise::Diff (double *dpVect, double dWeight, long lLength)
{
    double *dpPrev;
    #ifdef __GNUG__
    double dpWork[lLength];
    #else
    clDSPAlloc Work;
    double *dpWork = (double *) Work.Size(lLength * sizeof(double));
    #endif

    if (lLength != lPrevSize)
    {
        PrevBuf.Size(lLength * sizeof(double));
        Zero((double *) PrevBuf, lLength);
        lPrevSize = lLength;
    }
    dpPrev = PrevBuf;

    Sub(dpWork, dpVect, dpPrev, lLength);
    Abs(dpWork, lLength);

    Mul(dpVect, dWeight, lLength);
    Mul(dpPrev, 1.0 - dWeight, lLength);
    Add(dpPrev, dpVect, lLength);

    Copy(dpVect, dpWork, lLength);
}


void clRemoveNoise::Diff (float *fpDest, const float *fpSrc, float fWeight,
    long lLength)
{
    float *fpPrev;
    #ifdef __GNUG__
    float fpAvgWork[lLength];
    #else
    clDSPAlloc AvgWork;
    float *fpAvgWork = (float *) AvgWork.Size(lLength * sizeof(float));
    #endif

    if (lLength != lPrevSize)
    {
        PrevBuf.Size(lLength * sizeof(float));
        Zero((float *) PrevBuf, lLength);
        lPrevSize = lLength;
    }
    fpPrev = PrevBuf;

    Sub(fpDest, fpSrc, fpPrev, lLength);
    Abs(fpDest, lLength);

    Copy(fpAvgWork, fpSrc, lLength);
    Mul(fpAvgWork, fWeight, lLength);
    Mul(fpPrev, 1.0f - fWeight, lLength);
    Add(fpPrev, fpAvgWork, lLength);
}


void clRemoveNoise::Diff (double *dpDest, const double *dpSrc, double dWeight,
    long lLength)
{
    double *dpPrev;
    #ifdef __GNUG__
    double dpAvgWork[lLength];
    #else
    clDSPAlloc AvgWork;
    double *dpAvgWork = (double *) AvgWork.Size(lLength * sizeof(double));
    #endif

    if (lLength != lPrevSize)
    {
        PrevBuf.Size(lLength * sizeof(double));
        Zero((double *) PrevBuf, lLength);
        lPrevSize = lLength;
    }
    dpPrev = PrevBuf;

    Sub(dpDest, dpSrc, dpPrev, lLength);
    Abs(dpDest, lLength);

    Copy(dpAvgWork, dpSrc, lLength);
    Mul(dpAvgWork, dWeight, lLength);
    Mul(dpPrev, 1.0 - dWeight, lLength);
    Add(dpPrev, dpAvgWork, lLength);
}


void clRemoveNoise::InvDiff (float *fpVect, float fWeight, long lLength)
{
    float fMin;
    float fMax;
    float fAdj;
    float *fpPrev;
    #ifdef __GNUG__
    float fpWork[lLength];
    #else
    clDSPAlloc Work;
    float *fpWork = (float *) Work.Size(lLength * sizeof(float));
    #endif

    if (lLength != lPrevSize)
    {
        PrevBuf.Size(lLength * sizeof(float));
        Zero((float *) PrevBuf, lLength);
        lPrevSize = lLength;
    }
    fpPrev = PrevBuf;

    Sub(fpWork, fpVect, fpPrev, lLength);
    Abs(fpWork, lLength);
    MinMax(&fMin, &fMax, fpWork, lLength);
    fAdj = fMin + fMax;
    Negate(fpWork, lLength);
    Add(fpWork, fAdj, lLength);

    Mul(fpVect, fWeight, lLength);
    Mul(fpPrev, 1.0f - fWeight, lLength);
    Add(fpPrev, fpVect, lLength);
    
    Copy(fpVect, fpWork, lLength);
}


void clRemoveNoise::InvDiff (double *dpVect, double dWeight, long lLength)
{
    double dMin;
    double dMax;
    double dAdj;
    double *dpPrev;
    #ifdef __GNUG__
    double dpWork[lLength];
    #else
    clDSPAlloc Work;
    double *dpWork = (double *) Work.Size(lLength * sizeof(double));
    #endif

    if (lLength != lPrevSize)
    {
        PrevBuf.Size(lLength * sizeof(double));
        Zero((double *) PrevBuf, lLength);
        lPrevSize = lLength;
    }
    dpPrev = PrevBuf;

    Sub(dpWork, dpVect, dpPrev, lLength);
    Abs(dpWork, lLength);
    MinMax(&dMin, &dMax, dpWork, lLength);
    dAdj = dMin + dMax;
    Negate(dpWork, lLength);
    Add(dpWork, dAdj, lLength);

    Mul(dpVect, dWeight, lLength);
    Mul(dpPrev, 1.0 - dWeight, lLength);
    Add(dpPrev, dpVect, lLength);

    Copy(dpVect, dpWork, lLength);
}


void clRemoveNoise::InvDiff (float *fpDest, const float *fpSrc, 
    float fWeight, long lLength)
{
    float fMin;
    float fMax;
    float fAdj;
    float *fpPrev;
    #ifdef __GNUG__
    float fpAvgWork[lLength];
    #else
    clDSPAlloc AvgWork;
    float *fpAvgWork = (float *) AvgWork.Size(lLength * sizeof(float));
    #endif

    if (lLength != lPrevSize)
    {
        PrevBuf.Size(lLength * sizeof(float));
        Zero((float *) PrevBuf, lLength);
        lPrevSize = lLength;
    }
    fpPrev = PrevBuf;

    Sub(fpDest, fpSrc, fpPrev, lLength);
    Abs(fpDest, lLength);
    MinMax(&fMin, &fMax, fpDest, lLength);
    fAdj = fMin + fMax;
    Negate(fpDest, lLength);
    Add(fpDest, fAdj, lLength);

    Copy(fpAvgWork, fpSrc, lLength);
    Mul(fpAvgWork, fWeight, lLength);
    Mul(fpPrev, 1.0f - fWeight, lLength);
    Add(fpPrev, fpAvgWork, lLength);
}


void clRemoveNoise::InvDiff (double *dpDest, const double *dpSrc,
    double dWeight, long lLength)
{
    double dMin;
    double dMax;
    double dAdj;
    double *dpPrev;
    #ifdef __GNUG__
    double dpAvgWork[lLength];
    #else
    clDSPAlloc AvgWork;
    double *dpAvgWork = (double *) AvgWork.Size(lLength * sizeof(double));
    #endif

    if (lLength != lPrevSize)
    {
        PrevBuf.Size(lLength * sizeof(double));
        Zero((double *) PrevBuf, lLength);
        lPrevSize = lLength;
    }
    dpPrev = PrevBuf;

    Sub(dpDest, dpSrc, dpPrev, lLength);
    Abs(dpDest, lLength);
    MinMax(&dMin, &dMax, dpDest, lLength);
    dAdj = dMin + dMax;
    Negate(dpDest, lLength);
    Add(dpDest, dAdj, lLength);

    Copy(dpAvgWork, dpSrc, lLength);
    Mul(dpAvgWork, dWeight, lLength);
    Mul(dpPrev, 1.0 - dWeight, lLength);
    Add(dpPrev, dpAvgWork, lLength);
}

