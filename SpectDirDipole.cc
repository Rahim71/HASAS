/*

    Spectrum based direction finding for dipole array
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
//#include <cmath>
//#include <complex>

#include "SpectDirDipole.hh"

#include <cmath>
#include <complex>


void clSpectDirDipole::Calculate (int iScaling, stpSpectDirRN spRemoveNoise)
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

    for (lSensorCntr = 0l; lSensorCntr < 2l; lSensorCntr++)
    {
        DSP.Mul(fpProcBuf[lSensorCntr], fpWinFunc, lFFTSize);
        DSP.FFTi(spSpect[lSensorCntr], fpProcBuf[lSensorCntr]);
        if (spRemoveNoise->iType != SDD_BNER_NONE)
        {
            DSP.Magnitude(fpRNBuf[lSensorCntr], spSpect[lSensorCntr],
                lSpectSize);
            switch (spRemoveNoise->iType)
            {
                case SDD_BNER_TPSW:
                    BNER.TPSW(fpRNBuf[lSensorCntr], spRemoveNoise->fAlpha,
                        spRemoveNoise->lMeanLength, spRemoveNoise->lGapLength,
                        lSpectSize);
                    break;
                case SDD_BNER_OTA:
                    BNER.OTA(fpRNBuf[lSensorCntr], spRemoveNoise->fAlpha,
                        spRemoveNoise->lMeanLength, lSpectSize);
                    break;
                case SDD_BNER_DIFF:
                    BNER.Diff(fpRNBuf[lSensorCntr], spRemoveNoise->fAlpha,
                        lSpectSize);
                    break;
                case SDD_BNER_IDIFF:
                    BNER.InvDiff(fpRNBuf[lSensorCntr], spRemoveNoise->fAlpha,
                        lSpectSize);
                    break;
            }
        }
    }
    fpLeftRN = fpRNBuf[0];
    fpRightRN = fpRNBuf[1];
    spLeftSpect = spSpect[0];
    spRightSpect = spSpect[1];
    for (lSpectCntr = lMinBin; lSpectCntr < lMaxBin; lSpectCntr++)
    {
        complex<GDT> cplxLeft(spLeftSpect[lSpectCntr].R, 
            spLeftSpect[lSpectCntr].I);
        complex<GDT> cplxRight(spRightSpect[lSpectCntr].R, 
            spRightSpect[lSpectCntr].I);
        complex<GDT> cplxCorr = cplxLeft * conj(cplxRight);
        if (spRemoveNoise->iType == SDD_BNER_NONE)
            //fMagnitude = norm(cplxLeft) + norm(cplxRight);
            fMagnitude = norm(cplxCorr);
        else
            fMagnitude = fpLeftRN[lSpectCntr] + fpRightRN[lSpectCntr];
        switch (iScaling)
        {
            case SDD_SCALE_LIN:
                break;
            case SDD_SCALE_LOG:
            default:
                fMagnitude = 1.0 / (fabs(20.0 * log10(fMagnitude)) + 1.0);
                break;
        }
        fPhaseDiff = arg(cplxCorr);
        fFrequency = lSpectCntr * fFreqRes;
        SetDirection(fFrequency, fMagnitude, fPhaseDiff);
    }
}


clSpectDirDipole::clSpectDirDipole (GDT fSensorSpacing, GDT fSoundSpeed,
    double dSampleRate, long lFiltSize, int iFilterType, long lWindowSize, 
    long lDirCount, GDT fIntTimeReq, bool bEnableDebug)
{
    long lSensorCntr;
    long lTwosPower;
    GDT fWindowTime;
    GDT *fpNullPtr = NULL;
    
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
    for (lSensorCntr = 0l; lSensorCntr < 2; lSensorCntr++)
    {
        ProcBuf[lSensorCntr].Size(lFFTSize * sizeof(GDT));
        fpProcBuf[lSensorCntr] = ProcBuf[lSensorCntr];
        PrevBuf[lSensorCntr].Size(lFFTSize * sizeof(GDT));
        fpPrevBuf[lSensorCntr] = PrevBuf[lSensorCntr];
        RNBuf[lSensorCntr].Size(lSpectSize * sizeof(GDT));
        fpRNBuf[lSensorCntr] = RNBuf[lSensorCntr];
        Decimator[lSensorCntr].Initialize(lDecimation, lFilterSize, fpNullPtr,
            0, iFilterType);
        SpectBuf[lSensorCntr].Size(sizeof(GCDT) * lSpectSize);
        spSpect[lSensorCntr] = SpectBuf[lSensorCntr];
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


clSpectDirDipole::~clSpectDirDipole ()
{
}


void clSpectDirDipole::PutData (const GDT *fpInputData, long lSampleCount,
    long lStartCh, long lChCount)
{
    long lSensorCntr;
    long lChSampleCount;
    GDT *fpExtBuf;
    
    lChSampleCount = lSampleCount / lChCount;
    fpExtBuf = (GDT *) ExtBuf.Size(lChSampleCount * sizeof(GDT));
    for (lSensorCntr = 0l; lSensorCntr < 2l; lSensorCntr++)
    {
        DSP.Extract(fpExtBuf, fpInputData, lStartCh + lSensorCntr, lChCount,
            lSampleCount);
        Decimator[lSensorCntr].Put(fpExtBuf, lChSampleCount);
    }
}


bool clSpectDirDipole::GetResults (GDT *fpResults, GDT fLowFreqLimit,
    int iScaling, stpSpectDirRN spRemoveNoise)
{
    long lSensorCntr;

    for (lSensorCntr = 0l; lSensorCntr < 2l; lSensorCntr++)
    {
        if (lOldData <= 0l)
        {
            if (!Decimator[lSensorCntr].Get(fpProcBuf[lSensorCntr], 
                lFFTSize)) return false;
        }
        else
        {
            GDT *fpProc = fpProcBuf[lSensorCntr];
            GDT *fpOld = fpPrevBuf[lSensorCntr];
            
            if (Decimator[lSensorCntr].Get(&fpProc[lOldData], lNewData))
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


void clSpectDirDipole::ResetResults ()
{
    ResetDirections();
}

