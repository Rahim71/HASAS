/*

    Spectrum based direction finding for line array
    Copyright (C) 2000-2002 Jussi Laako

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

#include "SpectDirLine.hh"

#include <cmath>
#include <complex>


void clSpectDirLine::Calculate (int iScaling, stpSpectDirRN spRemoveNoise)
{
    long lSensorCntr;
    long lSpectCntr;
    GDT fMagnitude;
    GDT fPhaseDiff;
    GDT fFrequency;
    GDT *fpLeftRN;
    GDT *fpRightRN;
    GCDT *spLeftSpect;
    GCDT *spRightSpect;

    for (lSensorCntr = 0l; lSensorCntr < lSensorCount; lSensorCntr++)
    {
        DSP.Mul(vfpProcBuf[lSensorCntr], fpWinFunc, lFFTSize);
        DSP.FFTi(vspSpect[lSensorCntr], vfpProcBuf[lSensorCntr]);
        if (spRemoveNoise->iType != SDL_BNER_NONE)
        {
            DSP.Magnitude(vfpRNBuf[lSensorCntr], vspSpect[lSensorCntr],
                lSpectSize);
            switch (spRemoveNoise->iType)
            {
                case SDL_BNER_TPSW:
                    BNER.TPSW(vfpRNBuf[lSensorCntr], spRemoveNoise->fAlpha,
                        spRemoveNoise->lMeanLength, spRemoveNoise->lGapLength,
                        lSpectSize);
                    break;
                case SDL_BNER_OTA:
                    BNER.OTA(vfpRNBuf[lSensorCntr], spRemoveNoise->fAlpha,
                        spRemoveNoise->lMeanLength, lSpectSize);
                    break;
                case SDL_BNER_DIFF:
                    BNER.Diff(vfpRNBuf[lSensorCntr], spRemoveNoise->fAlpha,
                        lSpectSize);
                    break;
                case SDL_BNER_IDIFF:
                    BNER.InvDiff(vfpRNBuf[lSensorCntr], spRemoveNoise->fAlpha,
                        lSpectSize);
                    break;
            }
        }
    }
    for (lSensorCntr = 0l; lSensorCntr < (lSensorCount - 1); lSensorCntr++)
    {
        fpLeftRN = vfpRNBuf[lSensorCntr];
        fpRightRN = vfpRNBuf[lSensorCntr + 1];
        spLeftSpect = vspSpect[lSensorCntr];
        spRightSpect = vspSpect[lSensorCntr + 1];
        for (lSpectCntr = lMinBin; lSpectCntr < lMaxBin; lSpectCntr++)
        {
            complex<GDT> cplxLeft(spLeftSpect[lSpectCntr].R, 
                spLeftSpect[lSpectCntr].I);
            complex<GDT> cplxRight(spRightSpect[lSpectCntr].R, 
                spRightSpect[lSpectCntr].I);
            complex<GDT> cplxCorr = cplxLeft * conj(cplxRight);
            if (spRemoveNoise->iType == SDL_BNER_NONE)
                //fMagnitude = norm(cplxLeft) + norm(cplxRight);
                fMagnitude = norm(cplxCorr);
            else
                fMagnitude = fpLeftRN[lSpectCntr] + fpRightRN[lSpectCntr];
            switch (iScaling)
            {
                case SDL_SCALE_LIN:
                    break;
                case SDL_SCALE_LOG:
                default:
                    fMagnitude = 1.0 / (fabs(20.0 * log10(fMagnitude)) + 1.0);
                    break;
            }
            fPhaseDiff = arg(cplxCorr);
            fFrequency = lSpectCntr * fFreqRes;
            SetDirection(fFrequency, fMagnitude, fPhaseDiff);
        }
    }
}


clSpectDirLine::clSpectDirLine (long lSensors, GDT fSensorSpacing, 
    GDT fSoundSpeed, double dSampleRate, long lFiltSize, int iFilterType, 
    long lWindowSize, long lDirCount, GDT fIntTimeReq, bool bEnableDebug)
{
    long lSensorCntr;
    long lTwosPower;
    GDT fWindowTime;
    GDT *fpNullPtr = NULL;
    
    lSensorCount = lSensors;
    SetSensorSpacing(fSensorSpacing);
    SetSoundSpeed(fSoundSpeed);
    SetDirectionCount(lDirCount);
    bDebug = bEnableDebug;
    lDirectionCount = lDirCount;
    lFilterSize = lFiltSize;
    lFFTSize = lWindowSize;
    lTwosPower = (long)
        floor(log(dSampleRate / 2.0 / GetArrayFrequency()) / log(2.0));
    lDecimation = (long) pow(2.0, (double) lTwosPower);
    lSpectSize = lFFTSize / 2 + 1;
    fFreqRes =
        (GDT) dSampleRate / (GDT) lDecimation / (GDT) 2 / (GDT) lSpectSize;
    fWindowTime = (GDT) lFFTSize / ((GDT) dSampleRate / (GDT) lDecimation);
    if (fIntTimeReq <= 0)
    {
        fIntTime = fWindowTime;
        fOverlap = 0.0f;
    }
    else
    {
        fOverlap = 1.0f - fIntTimeReq / fWindowTime;
        lOldData = DSP.Round(lFFTSize * fOverlap);
        if (lOldData >= lFFTSize) lOldData = lFFTSize - 1l;
        lNewData = lFFTSize - lOldData;
        fIntTime = fWindowTime * ((GDT) lNewData / (GDT) lFFTSize);
    }
    lMaxBin = DSP.Round(GetArrayFrequency() / fFreqRes);
    WinFuncBuf.Size(lFFTSize * sizeof(GDT));
    fpWinFunc = WinFuncBuf;
    DSP.WinExactBlackman(fpWinFunc, lFFTSize);
    vProcBuf.resize(lSensorCount);
    vPrevBuf.resize(lSensorCount);
    vRNBuf.resize(lSensorCount);
    vSpectBuf.resize(lSensorCount);
    for (lSensorCntr = 0l; lSensorCntr < lSensorCount; lSensorCntr++)
    {
        vProcBuf[lSensorCntr].Size(lFFTSize * sizeof(GDT));
        vfpProcBuf.push_back(vProcBuf[lSensorCntr]);
        vPrevBuf[lSensorCntr].Size(lFFTSize * sizeof(GDT));
        vfpPrevBuf.push_back(vPrevBuf[lSensorCntr]);
        vRNBuf[lSensorCntr].Size(lSpectSize * sizeof(GDT));
        vfpRNBuf.push_back(vRNBuf[lSensorCntr]);
        vDecimator.push_back(new clRecDecimator);
        vDecimator[lSensorCntr]->Initialize(lDecimation, lFilterSize, 
            fpNullPtr, 0, iFilterType);
        vSpectBuf[lSensorCntr].Size(sizeof(GCDT) * lSpectSize);
        vspSpect.push_back(vSpectBuf[lSensorCntr]);
    }
    DSP.FFTInitialize(lFFTSize, true);
    if (bDebug)
    {
        printf("Filter size %li, DFT size %li\n", lFilterSize, lFFTSize);
        switch (iFilterType)
        {
            case clRecDecimator::FILTER_TYPE_FFT:
                puts("Using FFT decimation filter");
                break;
            case clRecDecimator::FILTER_TYPE_FIR:
                puts("Using FIR decimation filter");
                break;
            case clRecDecimator::FILTER_TYPE_IIR:
                puts("Using IIR decimation filter");
                break;
        }
        if (lOldData > 0l)
        {
            printf("Overlap %.1f%%, %li/%li\n", fOverlap * 100.0f, 
                lNewData, lOldData);
        }
        printf("Array frequency %.2f\n", GetArrayFrequency());
        printf("Decimation %li, Nyquist %.2f, resolution %.3f\n",
            lDecimation, ((GDT) dSampleRate / (GDT) lDecimation / (GDT) 2),
            fFreqRes);
    }
}


clSpectDirLine::~clSpectDirLine ()
{
    while (vDecimator.size() > 0)
    {
        delete vDecimator[vDecimator.size() - 1];
        vDecimator.pop_back();
    }
}


void clSpectDirLine::PutData (const GDT *fpInputData, long lSampleCount,
    long lStartCh, long lChCount)
{
    long lSensorCntr;
    long lChSampleCount;
    GDT *fpExtBuf;
    
    lChSampleCount = lSampleCount / lChCount;
    fpExtBuf = (GDT *) ExtBuf.Size(lChSampleCount * sizeof(GDT));
    for (lSensorCntr = 0l; lSensorCntr < lSensorCount; lSensorCntr++)
    {
        DSP.Extract(fpExtBuf, fpInputData, lStartCh + lSensorCntr, lChCount,
            lSampleCount);
        vDecimator[lSensorCntr]->Put(fpExtBuf, lChSampleCount);
    }
}


bool clSpectDirLine::GetResults (GDT *fpResults, GDT fLowFreqLimit,
    int iScaling, stpSpectDirRN spRemoveNoise)
{
    long lSensorCntr;

    for (lSensorCntr = 0l; lSensorCntr < lSensorCount; lSensorCntr++)
    {
        if (lOldData <= 0l)
        {
            if (!vDecimator[lSensorCntr]->Get(vfpProcBuf[lSensorCntr], 
                lFFTSize)) return false;
        }
        else
        {
            GDT *fpProc = vfpProcBuf[lSensorCntr];
            GDT *fpOld = vfpPrevBuf[lSensorCntr];
            
            if (vDecimator[lSensorCntr]->Get(&fpProc[lOldData], lNewData))
            {
                DSP.Copy(fpProc, &fpOld[lNewData], lOldData);
                DSP.Copy(fpOld, fpProc, lFFTSize);
            }
            else return false;
        }
    }
    lMinBin = DSP.Round(fLowFreqLimit / fFreqRes);
    if (lMinBin <= 0l) lMinBin = 1l;
    else if (lMinBin >= lMaxBin) lMinBin = lMaxBin - 1l;
    Calculate(iScaling, spRemoveNoise);
    GetDirections(fpResults);
    return true;
}


void clSpectDirLine::ResetResults ()
{
    ResetDirections();
}

