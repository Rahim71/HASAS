/*

    3D audio engine
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


#define COMPILE
#include "Audio3D.hh"


void clAudio3D::CopyToLocal(GDT *fpDest, const float *fpSrc)
{
    long lCopyCntr;

    for (lCopyCntr = 0l; lCopyCntr < HRTF_LENGTH; lCopyCntr++)
    {
        fpDest[lCopyCntr] = fpSrc[lCopyCntr];
    }
    for (lCopyCntr = HRTF_LENGTH; lCopyCntr < lFFTSize; lCopyCntr++)
    {
        fpDest[lCopyCntr] = (GDT) 0;
    }
}


void clAudio3D::HeadingElev0 (int iHeading)
{
    int iAH = abs(iHeading);

    if (iAH >= 0 && iAH <= 2)
    {
        CopyToLocal(fpLeftFilt, fpH0e000aL);
        CopyToLocal(fpRightFilt, fpH0e000aR);
    }
    else if (iAH >= 3 && iAH <= 7)
    {
        CopyToLocal(fpLeftFilt, fpH0e005aL);
        CopyToLocal(fpRightFilt, fpH0e005aR);
    }
    else if (iAH >= 8 && iAH <= 12)
    {
        CopyToLocal(fpLeftFilt, fpH0e010aL);
        CopyToLocal(fpRightFilt, fpH0e010aR);
    }
    else if (iAH >= 13 && iAH <= 17)
    {
        CopyToLocal(fpLeftFilt, fpH0e015aL);
        CopyToLocal(fpRightFilt, fpH0e015aR);
    }
    else if (iAH >= 18 && iAH <= 22)
    {
        CopyToLocal(fpLeftFilt, fpH0e020aL);
        CopyToLocal(fpRightFilt, fpH0e020aR);
    }
    else if (iAH >= 23 && iAH <= 27)
    {
        CopyToLocal(fpLeftFilt, fpH0e025aL);
        CopyToLocal(fpRightFilt, fpH0e025aR);
    }
    else if (iAH >= 28 && iAH <= 32)
    {
        CopyToLocal(fpLeftFilt, fpH0e030aL);
        CopyToLocal(fpRightFilt, fpH0e030aR);
    }
    else if (iAH >= 33 && iAH <= 37)
    {
        CopyToLocal(fpLeftFilt, fpH0e035aL);
        CopyToLocal(fpRightFilt, fpH0e035aR);
    }
    else if (iAH >= 38 && iAH <= 42)
    {
        CopyToLocal(fpLeftFilt, fpH0e040aL);
        CopyToLocal(fpRightFilt, fpH0e040aR);
    }
    else if (iAH >= 43 && iAH <= 47)
    {
        CopyToLocal(fpLeftFilt, fpH0e045aL);
        CopyToLocal(fpRightFilt, fpH0e045aR);
    }
    else if (iAH >= 48 && iAH <= 52)
    {
        CopyToLocal(fpLeftFilt, fpH0e050aL);
        CopyToLocal(fpRightFilt, fpH0e050aR);
    }
    else if (iAH >= 53 && iAH <= 57)
    {
        CopyToLocal(fpLeftFilt, fpH0e055aL);
        CopyToLocal(fpRightFilt, fpH0e055aR);
    }
    else if (iAH >= 58 && iAH <= 62)
    {
        CopyToLocal(fpLeftFilt, fpH0e060aL);
        CopyToLocal(fpRightFilt, fpH0e060aR);
    }
    else if (iAH >= 63 && iAH <= 67)
    {
        CopyToLocal(fpLeftFilt, fpH0e065aL);
        CopyToLocal(fpRightFilt, fpH0e065aR);
    }
    else if (iAH >= 68 && iAH <= 72)
    {
        CopyToLocal(fpLeftFilt, fpH0e070aL);
        CopyToLocal(fpRightFilt, fpH0e070aR);
    }
    else if (iAH >= 73 && iAH <= 77)
    {
        CopyToLocal(fpLeftFilt, fpH0e075aL);
        CopyToLocal(fpRightFilt, fpH0e075aR);
    }
    else if (iAH >= 78 && iAH <= 82)
    {
        CopyToLocal(fpLeftFilt, fpH0e080aL);
        CopyToLocal(fpRightFilt, fpH0e080aR);
    }
    else if (iAH >= 83 && iAH <= 87)
    {
        CopyToLocal(fpLeftFilt, fpH0e085aL);
        CopyToLocal(fpRightFilt, fpH0e085aR);
    }
    else if (iAH >= 88 && iAH <= 92)
    {
        CopyToLocal(fpLeftFilt, fpH0e090aL);
        CopyToLocal(fpRightFilt, fpH0e090aR);
    }
    else if (iAH >= 93 && iAH <= 97)
    {
        CopyToLocal(fpLeftFilt, fpH0e095aL);
        CopyToLocal(fpRightFilt, fpH0e095aR);
    }
    else if (iAH >= 98 && iAH <= 102)
    {
        CopyToLocal(fpLeftFilt, fpH0e100aL);
        CopyToLocal(fpRightFilt, fpH0e100aR);
    }
    else if (iAH >= 103 && iAH <= 107)
    {
        CopyToLocal(fpLeftFilt, fpH0e105aL);
        CopyToLocal(fpRightFilt, fpH0e105aR);
    }
    else if (iAH >= 108 && iAH <= 112)
    {
        CopyToLocal(fpLeftFilt, fpH0e110aL);
        CopyToLocal(fpRightFilt, fpH0e110aR);
    }
    else if (iAH >= 113 && iAH <= 117)
    {
        CopyToLocal(fpLeftFilt, fpH0e115aL);
        CopyToLocal(fpRightFilt, fpH0e115aR);
    }
    else if (iAH >= 118 && iAH <= 122)
    {
        CopyToLocal(fpLeftFilt, fpH0e120aL);
        CopyToLocal(fpRightFilt, fpH0e120aR);
    }
    else if (iAH >= 123 && iAH <= 127)
    {
        CopyToLocal(fpLeftFilt, fpH0e125aL);
        CopyToLocal(fpRightFilt, fpH0e125aR);
    }
    else if (iAH >= 128 && iAH <= 132)
    {
        CopyToLocal(fpLeftFilt, fpH0e130aL);
        CopyToLocal(fpRightFilt, fpH0e130aR);
    }
    else if (iAH >= 133 && iAH <= 137)
    {
        CopyToLocal(fpLeftFilt, fpH0e135aL);
        CopyToLocal(fpRightFilt, fpH0e135aR);
    }
    else if (iAH >= 138 && iAH <= 142)
    {
        CopyToLocal(fpLeftFilt, fpH0e140aL);
        CopyToLocal(fpRightFilt, fpH0e140aR);
    }
    else if (iAH >= 143 && iAH <= 147)
    {
        CopyToLocal(fpLeftFilt, fpH0e145aL);
        CopyToLocal(fpRightFilt, fpH0e145aR);
    }
    else if (iAH >= 148 && iAH <= 152)
    {
        CopyToLocal(fpLeftFilt, fpH0e150aL);
        CopyToLocal(fpRightFilt, fpH0e150aR);
    }
    else if (iAH >= 153 && iAH <= 157)
    {
        CopyToLocal(fpLeftFilt, fpH0e155aL);
        CopyToLocal(fpRightFilt, fpH0e155aR);
    }
    else if (iAH >= 158 && iAH <= 162)
    {
        CopyToLocal(fpLeftFilt, fpH0e160aL);
        CopyToLocal(fpRightFilt, fpH0e160aR);
    }
    else if (iAH >= 163 && iAH <= 167)
    {
        CopyToLocal(fpLeftFilt, fpH0e165aL);
        CopyToLocal(fpRightFilt, fpH0e165aR);
    }
    else if (iAH >= 168 && iAH <= 172)
    {
        CopyToLocal(fpLeftFilt, fpH0e170aL);
        CopyToLocal(fpRightFilt, fpH0e170aR);
    }
    else if (iAH >= 173 && iAH <= 177)
    {
        CopyToLocal(fpLeftFilt, fpH0e175aL);
        CopyToLocal(fpRightFilt, fpH0e175aR);
    }
    else
    {
        CopyToLocal(fpLeftFilt, fpH0e180aL);
        CopyToLocal(fpRightFilt, fpH0e180aR);
    }
    if (iHeading >= 0)
        Prepare(false);
    else
        Prepare(true);
}


void clAudio3D::HeadingElev90 ()
{
    CopyToLocal(fpLeftFilt, fpH90e000aL);
    CopyToLocal(fpRightFilt, fpH90e000aR);
    Prepare(false);
}


void clAudio3D::Prepare (bool bSwapped)
{
    GDT fLeftMin;
    GDT fLeftMax;
    GDT fRightMin;
    GDT fRightMax;
    #ifdef __GNUG__
    GDT fpLeftMagn[lSpectSize];
    GDT fpRightMagn[lSpectSize];
    #else
    clDSPAlloc LeftMagn;
    clDSPAlloc RightMagn;
    GDT *fpLeftMagn = (GDT *) LeftMagn.Size(lSpectSize * sizeof(GDT));
    GDT *fpRightMagn = (GDT *) RightMagn.Size(lSpectSize * sizeof(GDT));
    #endif

    if (!bSwapped)
    {
        DSP.FFTi(spLeftFilt, fpLeftFilt);
        DSP.FFTi(spRightFilt, fpRightFilt);
    }
    else
    {
        DSP.FFTi(spRightFilt, fpLeftFilt);
        DSP.FFTi(spLeftFilt, fpRightFilt);
    }
    DSP.Magnitude(fpLeftMagn, spLeftFilt, lSpectSize);
    DSP.Magnitude(fpRightMagn, spRightFilt, lSpectSize);
    DSP.MinMax(&fLeftMin, &fLeftMax, fpLeftMagn, lSpectSize);
    DSP.MinMax(&fRightMin, &fRightMax, fpRightMagn, lSpectSize);
    if (fLeftMax >= fRightMax)
        fAmp = (GDT) 1 / fLeftMax;
    else
        fAmp = (GDT) 1 / fRightMax;
}


void clAudio3D::ProcessDistance (GDT *fpLeft, GDT *fpRight, long lDataCount)
{
    long lDataCntr;
    GDT fWeight2 = (GDT) 1 - fDistance;

    for (lDataCntr = 0l; lDataCntr < lDataCount; lDataCntr++)
    {
        fpLeft[lDataCntr] = 
            fpLeft[lDataCntr] * fWeight2 + fpRight[lDataCntr] * fDistance;
        fpRight[lDataCntr] = 
            fpRight[lDataCntr] * fWeight2 + fpLeft[lDataCntr] * fDistance;
    }
}


clAudio3D::clAudio3D ()
{
    bInitialized = false;
}


clAudio3D::~clAudio3D ()
{
    if (bInitialized) Uninitialize();
}


void clAudio3D::Initialize (long lSize)
{
    GDT *fpNullPtr = NULL;

    if (bInitialized) Uninitialize();
    lWindowSize = lSize;
    lFFTSize = lWindowSize * 2;
    lSpectSize = lFFTSize / 2 + 1;
    DSP.FFTInitialize(lFFTSize, true);
    fpLeftFilt = (GDT *) LeftFilt.Size(lFFTSize * sizeof(GDT));
    fpRightFilt = (GDT *) RightFilt.Size(lFFTSize * sizeof(GDT));
    spLeftFilt = (GCDT *) CLeftFilt.Size(lSpectSize * sizeof(GCDT));
    spRightFilt = (GCDT *) CRightFilt.Size(lSpectSize * sizeof(GCDT));
    LeftData.Size(lWindowSize * sizeof(GDT));
    RightData.Size(lWindowSize * sizeof(GDT));
    CopyToLocal(fpLeftFilt, fpH0e000aL);
    CopyToLocal(fpRightFilt, fpH0e000aR);
    Prepare(false);
    FilterLeft.Initialize(lWindowSize, fpNullPtr, AUD3D_USE_WINDOW);
    FilterRight.Initialize(lWindowSize, fpNullPtr, AUD3D_USE_WINDOW);
    bInitialized = true;
}


void clAudio3D::Uninitialize ()
{
    FilterLeft.Uninitialize();
    FilterRight.Uninitialize();
    DSP.FFTUninitialize();
    bInitialized = false;
}


void clAudio3D::SetAngles (int iHeading, int iPitch, GDT fDist)
{
    if (iPitch <= -35) HeadingElevN40(iHeading);
    else if (iPitch > -35 && iPitch <= -25) HeadingElevN30(iHeading);
    else if (iPitch > -25 && iPitch <= -15) HeadingElevN20(iHeading);
    else if (iPitch > -15 && iPitch <= -5) HeadingElevN10(iHeading);
    else if (iPitch > -5 && iPitch < 5) HeadingElev0(iHeading);
    else if (iPitch >= 5 && iPitch < 15) HeadingElev10(iHeading);
    else if (iPitch >= 15 && iPitch < 25) HeadingElev20(iHeading);
    else if (iPitch >= 25 && iPitch < 35) HeadingElev30(iHeading);
    else if (iPitch >= 35 && iPitch < 45) HeadingElev40(iHeading);
    else if (iPitch >= 45 && iPitch < 55) HeadingElev50(iHeading);
    else if (iPitch >= 55 && iPitch < 65) HeadingElev60(iHeading);
    else if (iPitch >= 65 && iPitch < 75) HeadingElev70(iHeading);
    else if (iPitch >= 75 && iPitch < 85) HeadingElev80(iHeading);
    else HeadingElev90();
    fDistance = fDist;
}


void clAudio3D::Process (GDT *fpData)
{
    long lDestCntr;
    long lSrcCntr;
    GDT *fpLeftData = LeftData;
    GDT *fpRightData = RightData;

    lSrcCntr = 0l;
    for (lDestCntr = 0l; lDestCntr < lWindowSize; lDestCntr++)
    {
        fpLeftData[lDestCntr] = fpData[lSrcCntr++];
        fpRightData[lDestCntr] = fpData[lSrcCntr++];
    }
    FilterLeft.Process(fpLeftData, spLeftFilt);
    FilterRight.Process(fpRightData, spRightFilt);
    ProcessDistance(fpLeftData, fpRightData, lWindowSize);
    DSP.Mul(fpLeftData, fAmp, lWindowSize);
    DSP.Mul(fpRightData, fAmp, lWindowSize);
    lDestCntr = 0l;
    for (lSrcCntr = 0l; lSrcCntr < lWindowSize; lSrcCntr++)
    {
        fpData[lDestCntr++] = fpLeftData[lSrcCntr];
        fpData[lDestCntr++] = fpRightData[lSrcCntr];
    }
}


void clAudio3D::Process (GDT *fpLeftData, GDT *fpRightData)
{
    FilterLeft.Process(fpLeftData, spLeftFilt);
    FilterRight.Process(fpRightData, spRightFilt);
    ProcessDistance(fpLeftData, fpRightData, lWindowSize);
    DSP.Mul(fpLeftData, fAmp, lWindowSize);
    DSP.Mul(fpRightData, fAmp, lWindowSize);
}

