/*

    Class representing single sensor for array operations
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
#include <math.h>
#include <float.h>

#include "ArraySensor.hh"


inline void clArraySensor::SinCosC (float fX,
    float *fpSinX, float *fpCosX)
{
    #ifdef _GNU_SOURCE
    sincosf(fX, fpSinX, fpCosX);
    #else
    *fpSinX = sin(fX);
    *fpCosX = cos(fX);
    #endif
    *fpSinX = -(*fpSinX);
}


inline void clArraySensor::SinCosC (double dX,
    double *dpSinX, double *dpCosX)
{
    #ifdef _GNU_SOURCE
    sincos(dX, dpSinX, dpCosX);
    #else
    *dpSinX = sin(dX);
    *dpCosX = cos(dX);
    #endif
    *dpSinX = -(*dpSinX);
}


clArraySensor::clArraySensor ()
{
    bDebug = false;
    fShadeCoeff = 1;
    fPI = acos(-1.0);
}


bool clArraySensor::Initialize (GDT fLDistance, GDT fRDistance, long lWinSize)
{
    GDT *fpNullPtr = NULL;

    fLeftDistance = fLDistance;
    fRightDistance = fRDistance;
    lWindowSize = lWinSize;
    lProcCount = lWindowSize / 2;
    if (bDebug) 
    {
        printf("Left distance %g, right distance %g\n",
            fLeftDistance, fRightDistance);
        printf("Window size %li\n", lWinSize);
    }
    dspvProcData.SetSize(lProcCount);
    if (!Filter.Initialize(lWindowSize, fpNullPtr, 
        (GDT) DSP_FILT_DEF_OVERLAP, (GDT) DSP_FILT_DEF_BETA, 
        FILTER2_SMOOTH_KAISER_BESSEL))
        return false;
    return true;
}


void clArraySensor::SetSampleRate (GDT fFs)
{
    fSampleRate = fFs;
    fSampleSpacing = (GDT) 1.0 / fSampleRate;
    fFreqResolution = fSampleRate / (GDT) 2 / 
        (GDT) (lWindowSize / 2 + 1);
    if (bDebug) 
    {
        printf("Samplerate %g, spacing %g ms, resolution %g Hz\n", 
            fSampleRate, fSampleSpacing * 1000.0, fFreqResolution);
    }
}


void clArraySensor::SetSoundSpeed (GDT fSndSpeed)
{
    fSoundSpeed = fSndSpeed;
    fSecsPerMeter = (GDT) 1.0 / fSndSpeed;
    fLeftTime = fLeftDistance * fSecsPerMeter;
    fRightTime = fRightDistance * fSecsPerMeter;
    if (bDebug) 
    {
        printf("Sound speed %g ms/m\n", fSecsPerMeter * 1000.0);
        printf("Left delay time %g ms, right delay time %g ms\n",
            fLeftTime * 1000.0, fRightTime * 1000.0);
    }
}


void clArraySensor::SetArrayFrequency (GDT fFa)
{
    fArrayFrequency = fFa;
    if (bDebug) printf("Array frequency %g Hz\n", fArrayFrequency);
}


void clArraySensor::SetShading (GDT fCoeff)
{
    fShadeCoeff = fCoeff;
    if (bDebug) printf("Shading coefficient %g\n", fShadeCoeff);
}


void clArraySensor::SetDirection (GDT fDir, bool bLowPass)
{
    long lMaxBin;
    long lBinCntr;
    GDT fDirPhase;
    GDT fArrayTime;
    GDT fFreqTime;
    GDT fFreqPhase;
    GDT fHighCoeff;
    clDSPVector<GCDT> dspvFiltCoeffs;

    if (bDebug) 
    {
        /*printf("Direction %g rad (%g deg)\n", 
            fDir, DSP.RadToDeg(fDir));
        if (bLowPass) puts("Lowpass filter enabled");
        else puts("Lowpass filter disabled");*/
    }
    dspvFiltCoeffs.SetSize(lWindowSize / 2 + 1);
    fDirPhase = sin(fDir) * fPI;
    fArrayTime = (fDir < (GDT) 0) ? fRightTime : fLeftTime;
    lMaxBin = (long) (fArrayFrequency / fFreqResolution + (GDT) 0.5);
    if (lMaxBin > dspvFiltCoeffs.Size())
        lMaxBin = dspvFiltCoeffs.Size();
    for (lBinCntr = 0; lBinCntr < lMaxBin; lBinCntr++)
    {
        fFreqTime = (GDT) 1 / 
            ((GDT) lBinCntr * fFreqResolution * (GDT) 2);
        fFreqPhase = fDirPhase * (fArrayTime / fFreqTime);
        SinCosC(fFreqPhase, &dspvFiltCoeffs[lBinCntr].I,
            &dspvFiltCoeffs[lBinCntr].R);
    }
    fHighCoeff = (bLowPass) ? 0 : 1;
    for (lBinCntr = lMaxBin; lBinCntr < dspvFiltCoeffs.Size(); lBinCntr++)
    {
        dspvFiltCoeffs[lBinCntr].R = fHighCoeff;
        dspvFiltCoeffs[lBinCntr].I = (GDT) 0;
    }
    Filter.SetCoeffs(dspvFiltCoeffs.Ptr(), true);
}


void clArraySensor::Put (const GDT *fpSrc, long lSrcCount)
{
    dspvInData.Put(fpSrc, lSrcCount);
    while (dspvInData.Get(dspvProcData.Ptr(), dspvProcData.Size()))
    {
        dspvProcData *= fShadeCoeff;
        Filter.Put(dspvProcData.Ptr(), dspvProcData.Size());
        while (Filter.Get(dspvProcData.Ptr(), dspvProcData.Size()))
        {
            dspvOutData.Put(dspvProcData.Ptr(), dspvProcData.Size());
        }
    }
}


bool clArraySensor::Get (GDT *fpDest, long lDestCount)
{
    return dspvOutData.Get(fpDest, lDestCount);
}
