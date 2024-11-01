/*

    LOFAR/DEMON calculation server
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
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <sys/time.h>

#include "LofarDemon.hh"
#include "DC-Block.h"


static bool bDebug;
static bool bDaemonProc;
clLofarDemon *LofarDemon;

#define LOFAR_NWINS 12
const static char *cpaWindowFuncs[] = {
    "Rectangle",
    "Bartlett",
    "Blackman",
    "Blackman-Harris",
    "Cosine tapered",
    "Exponential",
    "Flat-top",
    "Hamming",
    "Hanning",
    "Kaiser",
    "Kaiser-Bessel",
    "Tukey" };


void SigHandler (int iSigNum)
{
    switch (iSigNum)
    {
        case SIGSEGV:
            if (bDebug) 
            {
                printf("Oops, received SIGSEGV, crash...\n");
                abort();
            }
            else exit(-1);
            break;
    }
}


int main (int argc, char *argv[])
{
    int iRetVal;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    signal(SIGSEGV, SigHandler);
    bDebug = false;
    bDaemonProc = false;
    if (argc >= 2)
    {
        if (strcmp(argv[1], "--debug") == 0)
        {
            bDebug = true;
        }
        if (strcmp(argv[1], "-D") == 0)
        {
            bDaemonProc = true;
        }
    }
    if (bDebug) printf("lofardemon: Started\n");
    LofarDemon = new clLofarDemon(3, 4);
    iRetVal = LofarDemon->Exec();
    delete LofarDemon;
    if (bDebug) printf("lofardemon: Exit = %i\n", iRetVal);
    return iRetVal;
}


bool clLofarDemon::GetRequestMsg ()
{
    char cpMsgBuf[GLOBAL_HEADER_LEN];

    if (!SOpRequest.ReadSelect(LOFAR_REQ_TIMEOUT))
        return false;
    if (SOpRequest.ReadN(cpMsgBuf, GLOBAL_HEADER_LEN) != GLOBAL_HEADER_LEN)
        return false;
    LofarMsg.GetRequest(cpMsgBuf, &sLofarRq);
    return true;
}


bool clLofarDemon::GetDataHdr ()
{
    int iTempHandle;
    char cpSocketName[_POSIX_PATH_MAX + 1];
    stRawDataReq sDataReq;

    Cfg.SetFileName(SD_CFGFILE);
    if (!Cfg.GetStr("LocalSocket", cpSocketName))
        return false;
    iTempHandle = SockClie.Connect(cpSocketName);
    if (iTempHandle < 0)
        return false;
    SOpData.SetHandle(iTempHandle);
    if (!SOpData.ReadSelect(LOFAR_RAW1ST_TIMEOUT))
        return false;
    if (SOpData.ReadN(&sDataHdr, sizeof(sDataHdr)) != sizeof(sDataHdr))
        return false;
    sDataReq.iChannel = sLofarRq.iChannel;
    if (SOpData.WriteN(&sDataReq, sizeof(sDataReq)) != sizeof(sDataReq))
        return false;
    return true;
}


bool clLofarDemon::Initialize ()
{
    Cfg.SetFileName(LOFAR_CFGFILE);
    if (!InitFilter()) return false;
    if (!InitFFT()) return false;
    return true;
}


bool clLofarDemon::InitFilter ()
{
    int iTempInt;
    int iBandNo;
    long lTwosPower;
    float fNyquist;
    float fBandWidth;
    GDT fLowCorner;
    GDT fHighCorner;
    GDT fNormBandCenter;
    GDT fDemonLowCorner;
    GDT fDemonHighCorner;
    GDT *fpNullPtr = NULL;

    // Initialize filter
    if (Cfg.GetInt("FilterSize", &iTempInt)) 
        lFilterSize = iTempInt;
    else
        lFilterSize = LOFAR_DEF_FILTER_SIZE;
    if (!Cfg.GetInt("FilterType", &iFilterType))
        iFilterType = clRecDecimator::FILTER_TYPE_FFT;
    if (Cfg.GetInt("DEMONDecimation", &iTempInt))
        lDemonDecimation = iTempInt;
    else
        lDemonDecimation = 2;
    if (!Cfg.GetInt("DEMONDCNotch", &iDCBlock))
        iDCBlock = LOFAR_DEF_DC_BLOCK;
    fNyquist = (float) sDataHdr.dSampleRate / 2.0f;
    fBandWidth = sLofarRq.fHighFreq - sLofarRq.fLowFreq;
    lTwosPower = (long) (log(fNyquist / fBandWidth) / log(2.0) + 0.5);
    lDecimation = (long) pow(2.0, lTwosPower);
    fBandWidth = fNyquist / lDecimation;
    iBandNo = (int) (sLofarRq.fHighFreq / fBandWidth + 0.5f);
    if ((fBandWidth * iBandNo) < (sLofarRq.fLowFreq + fBandWidth / 2.0f))
        iBandNo++;
    fHighCorner = fBandWidth * iBandNo;
    fLowCorner = fHighCorner - fBandWidth;
    fDemonLowCorner = 0;
    fDemonHighCorner = fBandWidth / lDemonDecimation;
    fNormBandCenter = (fLowCorner + fBandWidth / 2) / fNyquist;
    if (!sLofarRq.bDemon)
        bReverseOrder = ((iBandNo - 1) % 2 == 0) ? false : true;
    else
        bReverseOrder = false;
    sLofarResHdr.fLowFreq = fLowCorner;
    sLofarResHdr.fHighFreq = fHighCorner;
    sLofarResHdr.fDemonBand = fDemonHighCorner;
    if (bDebug)
    {
        printf("lofardemon: Samplerate: %g  decimation: %li\n",
            sDataHdr.dSampleRate, lDecimation);
        printf("lofardemon: Lower corner: %.2f  higher corner: %.2f\n",
            fLowCorner, fHighCorner);
        switch (iFilterType)
        {
            case clRecDecimator::FILTER_TYPE_FFT:
                puts("lofardemon: Using FFT decimator");
                break;
            case clRecDecimator::FILTER_TYPE_FIR:
                puts("lofardemon: Using FIR decimator");
                break;
            case clRecDecimator::FILTER_TYPE_IIR:
                puts("lofardemon: Using IIR decimator");
                break;
        }
        if (!bReverseOrder)
            printf("lofardemon: Normal order\n");
        else
            printf("lofardemon: Reverse order\n");
    }
    if (!sLofarRq.bDemon)
    {
        if (lDecimation >= 2l)
        {
            Decimator.Initialize(lDecimation, lFilterSize, fpNullPtr,
                fNormBandCenter, iFilterType);
            if (bDebug)
            {
                printf("lofardemon: Filter %.2f - %.2f\n", 
                    fLowCorner, fHighCorner);
            }
        }
    }
    else
    {
        if (lDecimation >= 2l)
        {
            DemonDecimator.Initialize(lDecimation, lFilterSize, fpNullPtr,
                fNormBandCenter, iFilterType);
        }
        DCBlockIIR.Initialize(fpDCBlock, lDCBlockCount);
        Decimator.Initialize(lDemonDecimation, lFilterSize, fpNullPtr,
            0.0f, iFilterType);
        if (bDebug)
        {
            printf("lofardemon: DEMON BP filter %.2f - %.2f\n",
                fLowCorner, fHighCorner);
            printf("lofardemon: DEMON LP filter %.2f - %.2f\n",
                fDemonLowCorner, fDemonHighCorner);
            if (iDCBlock)
                puts("lofardemon: IIR DC notch");
        }
    }
    return true;
}


bool clLofarDemon::InitFFT ()
{
    float fPercMult;
    
    fpDataBuf = (GDT *) DataBuf.Size(sLofarRq.lWinLength * sizeof(GDT));
    lWinSize = sLofarRq.lWinLength;
    lSpectSize = sLofarRq.lWinLength / 2L + 1L;
    sLofarResHdr.lSpectLength = lSpectSize;
    fpWinFunc = (GDT *) WinFunc.Size(sLofarRq.lWinLength * sizeof(GDT));
    if (sLofarRq.iWindow < LOFAR_NWINS && bDebug)
        printf("lofardemon: %s window\n", cpaWindowFuncs[sLofarRq.iWindow]);
    iNormIdx = 0;
    if (!Cfg.GetInt("NormHistorySize", &iNormHistCount))
        iNormHistCount = 1;
    if (sLofarRq.iClip == MSG_LOFAR_CLIP_SLIDING && bDebug)
        printf("lofardemon: Normalization sliding average count %i\n",
            iNormHistCount);
    switch (sLofarRq.iWindow)
    {
        case MSG_LOFAR_WIN_BARTLETT:
            DSP.WinBartlett(fpWinFunc, sLofarRq.lWinLength);
            break;
        case MSG_LOFAR_WIN_BLACKMAN:
            DSP.WinExactBlackman(fpWinFunc, sLofarRq.lWinLength);
            break;
        case MSG_LOFAR_WIN_BM_HARRIS:
            DSP.WinBlackmanHarris(fpWinFunc, sLofarRq.lWinLength);
            break;
        case MSG_LOFAR_WIN_COS_TAPER:
            DSP.WinCosTapered(fpWinFunc, sLofarRq.lWinLength);
            break;
        case MSG_LOFAR_WIN_EXP:
            DSP.WinExp(fpWinFunc, (GDT) 1.0, sLofarRq.lWinLength);
            break;
        case MSG_LOFAR_WIN_FLATTOP:
            DSP.WinFlatTop(fpWinFunc, sLofarRq.lWinLength);
            break;
        case MSG_LOFAR_WIN_HAMMING:
            DSP.WinHamming(fpWinFunc, sLofarRq.lWinLength);
            break;
        case MSG_LOFAR_WIN_HANNING:
            DSP.WinHanning(fpWinFunc, sLofarRq.lWinLength);
            break;
        case MSG_LOFAR_WIN_KAISER:
            DSP.WinKaiser(fpWinFunc, sLofarRq.fWinParameter, 
                sLofarRq.lWinLength);
            break;
        case MSG_LOFAR_WIN_KAI_BES:
            DSP.WinKaiserBessel(fpWinFunc, sLofarRq.fWinParameter, 
                sLofarRq.lWinLength);
            break;
        case MSG_LOFAR_WIN_TUKEY:
            DSP.WinTukey(fpWinFunc, sLofarRq.lWinLength);
            break;
        case MSG_LOFAR_WIN_RECT:
        default:
            DSP.Set(fpWinFunc, (GDT) 1.0, sLofarRq.lWinLength);
            break;
    }
    switch (sLofarRq.iType)
    {
        case MSG_LOFAR_TYPE_AUTOCORR:
        case MSG_LOFAR_TYPE_FFT:
        case MSG_LOFAR_TYPE_CEPSTRUM:
        default:
            DSP.FFTInitialize(sLofarRq.lWinLength, true);
    }
    if (bDebug) 
        printf("lofardemon: %li-point FFT initialized\n", sLofarRq.lWinLength);
    fpSpectRes = (GDT *) SpectRes.Size(lSpectSize * sizeof(GDT));
    fpLofarRes = (GDT *) LofarRes.Size(lSpectSize * sizeof(GDT));
    if (fpLofarRes == NULL) return false;
    fPercMult = sLofarRq.iOverlap / 100.0f;
    lOldDataSize = (long) (sLofarRq.lWinLength * fPercMult + 0.5f);
    lNewDataSize = sLofarRq.lWinLength - lOldDataSize;
    if (lNewDataSize <= 0)
    {
        lNewDataSize = 1;
        lOldDataSize = sLofarRq.lWinLength - lNewDataSize;
    }
    else if (lNewDataSize > sLofarRq.lWinLength)
    {
        lNewDataSize = sLofarRq.lWinLength;
        lOldDataSize = 0;
    }
    HistoryBuf.Size(lSpectSize * sizeof(GDT) * sLofarRq.lMeanLength);
    fpNormHist = (GDT *) 
        NormHist.Size(lSpectSize * sizeof(GDT) * iNormHistCount);
    DSP.Zero(fpNormHist, lSpectSize * iNormHistCount);
    return true;
}


int clLofarDemon::MainLoop ()
{
    long lBlockSize = (long) (abs(lFilterSize) * (1.0 - DSP_FILT_DEF_OVERLAP));
    int iRawSize = lBlockSize * sizeof(GDT);
    GDT fTempLevel;
    #ifdef __GNUG__
    GDT fpFilterData[lBlockSize];
    GDT fpNewData[lNewDataSize];
    GDT fpOldData[lOldDataSize];
    #else
    clDSPAlloc FilterData;
    clDSPAlloc NewData;
    clDSPAlloc OldData;
    GDT *fpFilterData = (GDT *) FilterData.Size(iRawSize);
    GDT *fpNewData = (GDT *) NewData.Size(lNewDataSize * sizeof(GDT));
    GDT *fpOldData = (GDT *) OldData.Size(lOldDataSize * sizeof(GDT));
    #endif
    
    iRawSize = lBlockSize * sizeof(GDT);
    DSP.Zero(fpOldData, lOldDataSize);
    if (SOpData.GetRecvBufSize() < (iRawSize * 2))
    {
        if (!SOpData.SetRecvBufSize(iRawSize * 2) && bDebug)
        {
            printf("lofardemon: Failed to set receive buffer size\n");
        }
    }
    if (bDebug) printf("lofardemon: Entering the main loop\n");
    while (bRun)
    {
        if (SOpData.ReadN(fpFilterData, iRawSize) == iRawSize)
        {
            //sLofarResHdr.fPeakLevel = DSP.PeakLevel(fpFilterData, lFilterSize);
            PutData(fpFilterData, lBlockSize);
            while (PullData(fpNewData, lNewDataSize) && bRun)
            {
                DSP.Copy(fpDataBuf, fpOldData, lOldDataSize);
                DSP.Copy(&fpDataBuf[lOldDataSize], fpNewData, lNewDataSize);
                DSP.Copy(fpOldData, &fpDataBuf[lNewDataSize], lOldDataSize);
                fTempLevel = DSP.PeakLevel(fpDataBuf, sLofarRq.lWinLength);
                if (fTempLevel > sLofarResHdr.fPeakLevel)
                    sLofarResHdr.fPeakLevel = fTempLevel;
                gettimeofday(&sLofarResHdr.sTimeStamp, NULL);
                CalcSpect();
                sLofarResHdr.fLineTime = 1.0f /
                    (((float) sDataHdr.dSampleRate / (float) lDecimation) /
                    (float) lNewDataSize) * sLofarRq.lAvgCount;
                if (sLofarRq.bDemon)
                    sLofarResHdr.fLineTime *= lDemonDecimation;
                if (lAvgCntr >= sLofarRq.lAvgCount)
                {
                    bRun = SendResults();
                    lAvgCntr = 0;
                    sLofarResHdr.fPeakLevel = -FLT_MAX;
                    DSP.Zero(fpLofarRes, lSpectSize);
                }
            }
        }
        else
        {
            if (bDebug) printf("lofardemon: Not enough raw data\n");
            bRun = false;
        }
    }
    if (bDebug) printf("lofardemon: Exit from the main loop\n");
    return 0;
}


void clLofarDemon::CalcSpect ()
{
    long lSpectCntr;
    GDT fScale;
    GDT fSpectMin;
    GDT fSpectMax;
    GDT fSpectMean;
    GDT fSpectMedian;
    GDT fAvgScale;
    GDT fTemp;
    #ifdef __GNUG__
    GDT fpTempSpect[lWinSize];
    GCDT spSpect[lSpectSize];
    #else
    clDSPAlloc TempSpect;
    clDSPAlloc Spect;
    GDT *fpTempSpect = 
        (GDT *) TempSpect.Size(lWinSize * sizeof(GDT));
    GCDT *spSpect = (GCDT *) Spect.Size(lSpectSize * sizeof(GCDT));
    #endif

    DSP.Mul(fpDataBuf, fpWinFunc, sLofarRq.lWinLength);
    switch (sLofarRq.iType)
    {
        case MSG_LOFAR_TYPE_AUTOCORR:
            DSP.FFTi(spSpect, fpDataBuf);
            DSP.Mul(spSpect, spSpect, lSpectSize);
            DSP.IFFTo(fpTempSpect, spSpect);
            break;
        case MSG_LOFAR_TYPE_CEPSTRUM:
            DSP.FFTi(spSpect, fpDataBuf);
            for (lSpectCntr = 0; lSpectCntr < lSpectSize; lSpectCntr++)
            {
                #ifndef HAVE_GLIBC
                spSpect[lSpectCntr].R = log10(sqrt(
                    spSpect[lSpectCntr].R * spSpect[lSpectCntr].R +
                    spSpect[lSpectCntr].I * spSpect[lSpectCntr].I));
                #else
                spSpect[lSpectCntr].R = log10(hypot(
                    spSpect[lSpectCntr].R, spSpect[lSpectCntr].I));
                #endif
                spSpect[lSpectCntr].I = 0;
            }
            DSP.IFFTo(fpTempSpect, spSpect);
            fpTempSpect[0] = 0;
            fpTempSpect[1] = 0;
            fScale = (GDT) 2 / (GDT) sLofarRq.lWinLength;
            DSP.Mul(fpTempSpect, fScale, lSpectSize);
            break;
        case MSG_LOFAR_TYPE_FFT:
        default:
            DSP.FFTi(spSpect, fpDataBuf);
            if (sLofarRq.bLinear)
            {
                DSP.Magnitude(fpTempSpect, spSpect, lSpectSize);
            }
            else
            {
                DSP.Power(fpTempSpect, spSpect, lSpectSize);
            }
    }
    if (bReverseOrder)
    {
        DSP.Reverse(fpSpectRes, fpTempSpect, lSpectSize);
    }
    else
    {
        DSP.Copy(fpSpectRes, fpTempSpect, lSpectSize);
    }
    if (sLofarRq.bDemon)
    {
        fpSpectRes[0] = 0;
        //fpSpectRes[1] = 0;
    }
    else if (sLofarResHdr.fLowFreq == 0.0f)
    {
        fpSpectRes[0] = 0;
    }
    DSP.Scale01(fpSpectRes, lSpectSize);
    DSP.Add(fpLofarRes, fpSpectRes, lSpectSize);
    lAvgCntr++;
    if (lAvgCntr >= sLofarRq.lAvgCount)
    {
        fAvgScale = (GDT) 1.0 / sLofarRq.lAvgCount;
        DSP.Mul(fpLofarRes, fAvgScale, lSpectSize);
        switch (sLofarRq.iRemoveNoise)
        {
            case MSG_LOFAR_BNER_TPSW:
                RemoveNoise.TPSW(fpLofarRes, sLofarRq.fAlpha, 
                    sLofarRq.lMeanLength, sLofarRq.lGapLength, lSpectSize);
                break;
            case MSG_LOFAR_BNER_OTA:
                RemoveNoise.OTA(fpLofarRes, sLofarRq.fAlpha, 
                    sLofarRq.lMeanLength, lSpectSize);
                break;
            case MSG_LOFAR_BNER_DIFF:
                RemoveNoise.Diff(fpLofarRes, sLofarRq.fAlpha, lSpectSize);
                break;
            case MSG_LOFAR_BNER_IDIFF:
                RemoveNoise.InvDiff(fpLofarRes, sLofarRq.fAlpha, lSpectSize);
                break;
            case MSG_LOFAR_BNER_STDDEV:
                SpectStdDev(fpLofarRes);
                break;
            case MSG_LOFAR_BNER_NONE:
            default:
                // None
                break;
        }
        switch (sLofarRq.iClip)
        {
            case MSG_LOFAR_CLIP_LOW:
                DSP.ClipZero(fpLofarRes, lSpectSize);
                DSP.Scale01(fpLofarRes, lSpectSize);
                break;
            case MSG_LOFAR_CLIP_BOTH:
                DSP.Clip(fpLofarRes, (GDT) 0.0, (GDT) 1.0, lSpectSize);
                DSP.Scale01(fpLofarRes, lSpectSize);
                break;
            case MSG_LOFAR_CLIP_MEAN:
                fSpectMean = DSP.Mean(fpLofarRes, lSpectSize);
                DSP.Clip(fpLofarRes, fSpectMean, lSpectSize);
                DSP.Scale01(fpLofarRes, lSpectSize);
                break;
            case MSG_LOFAR_CLIP_MEDIAN:
                fSpectMedian = DSP.Median(fpLofarRes, lSpectSize);
                DSP.Clip(fpLofarRes, fSpectMedian, lSpectSize);
                DSP.Scale01(fpLofarRes, lSpectSize);
                break;
            case MSG_LOFAR_CLIP_10DB:
                fSpectMean = DSP.Mean(fpLofarRes, lSpectSize);
                fSpectMean *= (GDT) pow(10.0, 0.5);
                DSP.Clip(fpLofarRes, (GDT) 0.0, fSpectMean, lSpectSize);
                DSP.Scale01(fpLofarRes, lSpectSize);
                break;
            case MSG_LOFAR_CLIP_20DB:
                fSpectMean = DSP.Mean(fpLofarRes, lSpectSize);
                fSpectMean *= (GDT) 10.0;
                DSP.Clip(fpLofarRes, (GDT) 0.0, fSpectMean, lSpectSize);
                DSP.Scale01(fpLofarRes, lSpectSize);
                break;
            case MSG_LOFAR_CLIP_50P:
                DSP.MinMax(&fSpectMin, &fSpectMax, fpLofarRes, lSpectSize);
                DSP.Clip(fpLofarRes, (GDT) 0.0, fSpectMax * 0.5f, lSpectSize);
                DSP.Scale01(fpLofarRes, lSpectSize);
                break;
            case MSG_LOFAR_CLIP_75P:
                DSP.MinMax(&fSpectMin, &fSpectMax, fpLofarRes, lSpectSize);
                DSP.Clip(fpLofarRes, (GDT) 0.0, fSpectMax * 0.75f, lSpectSize);
                DSP.Scale01(fpLofarRes, lSpectSize);
                break;
            case MSG_LOFAR_CLIP_OFFSET2:
                DSP.Clip(fpLofarRes, (GDT) 3.33333, lSpectSize);
            case MSG_LOFAR_CLIP_OFFSET:
                DSP.MinMax(&fSpectMin, &fSpectMax, fpLofarRes, lSpectSize);
                fTemp = (fSpectMax == (GDT) 0.0) ? FLT_EPSILON : fSpectMax;
                DSP.MulAdd(fpLofarRes, 
                    (GDT) 1.0 / fTemp * (GDT) 0.8, 
                    (GDT) 0.2,
                    lSpectSize);
                DSP.ClipZero(fpLofarRes, lSpectSize);
                break;
            case MSG_LOFAR_CLIP_OFFSET3:
                DSP.Copy(fpTempSpect, fpLofarRes, lSpectSize);
                DSP.Sort(fpTempSpect, lSpectSize);
                fTemp = fpTempSpect[(long) (lSpectSize * 0.99)];
                DSP.Clip(fpLofarRes, fTemp, lSpectSize);
                DSP.MinMax(&fSpectMin, &fSpectMax, fpLofarRes, lSpectSize);
                fTemp = (fSpectMax == (GDT) 0.0) ? FLT_EPSILON : fSpectMax;
                DSP.MulAdd(fpLofarRes, 
                    (GDT) 1.0 / fTemp * (GDT) 0.8, 
                    (GDT) 0.2,
                    lSpectSize);
                DSP.ClipZero(fpLofarRes, lSpectSize);
                break;
            case MSG_LOFAR_CLIP_SLIDING:
                DSP.Copy(&fpNormHist[iNormIdx * lSpectSize], fpLofarRes,
                    lSpectSize);
                DSP.Zero(fpTempSpect, lSpectSize);
                for (lSpectCntr = 0; 
                    lSpectCntr < iNormHistCount;
                    lSpectCntr++)
                {
                    DSP.Add(fpTempSpect, &fpNormHist[lSpectCntr * lSpectSize],
                        lSpectSize);
                }
                DSP.Mul(fpTempSpect, (GDT) 1.0 / iNormHistCount,
                    lSpectSize);
                DSP.MinMax(&fSpectMin, &fSpectMax, fpTempSpect, lSpectSize);
                fSpectMax = (fSpectMax == (GDT) 0.0) ? FLT_EPSILON : fSpectMax;
                fScale = (GDT) 1.0 / fSpectMax * (GDT) 0.8;
                DSP.MulAdd(fpLofarRes, fScale, (GDT) 0.2, lSpectSize);
                iNormIdx++;
                if (iNormIdx >= iNormHistCount)
                    iNormIdx = 0;
                break;
            case MSG_LOFAR_CLIP_NONE:
            default:
                DSP.Scale01(fpLofarRes, lSpectSize);
                break;
        }
    }
}


bool clLofarDemon::SendResults ()
{
    int iMsgSize = GLOBAL_HEADER_LEN + sizeof(GDT) * lSpectSize;
    #ifdef __GNUG__
    char cpMsgBuf[iMsgSize];
    #else
    char *cpMsgBuf;
    clDSPAlloc MsgBuf;
    cpMsgBuf = (char *) MsgBuf.Size(iMsgSize);
    #endif

    LofarMsg.SetResult(cpMsgBuf, &sLofarResHdr, fpLofarRes);
    if (SOpResult.WriteSelect(LOFAR_TIMEOUT))
    {
        if (SOpResult.WriteN(cpMsgBuf, iMsgSize) != iMsgSize)
        {
            if (bDebug) 
                printf("lofardemon: Write error, client disconnected?\n");
            return false;
        }
    }
    else if (SOpResult.GetErrno() != 0)
    {
        if (bDebug) printf("lofardemon: Client disconnected\n");
        return false;
    }
    return true;
}


void clLofarDemon::PutData (GDT *fpProcBuf, long lProcLen)
{
    if (!sLofarRq.bDemon)
    {
        if (lDecimation >= 2l)  // normal case
            Decimator.Put(fpProcBuf, lProcLen);
        else  // special case without decimation
            ReBuffer.Put(fpProcBuf, lProcLen);
    }
    else
    {
        if (lDecimation >= 2l)  // normal case
        {
            DemonDecimator.Put(fpProcBuf, lProcLen);
            while (DemonDecimator.Get(fpProcBuf, lProcLen))
            {
                //DSP.Square(fpProcBuf, lProcLen);
                DemonProc(fpProcBuf, lProcLen);
                if (iDCBlock) DCBlockIIR.Process(fpProcBuf, lProcLen);
                Decimator.Put(fpProcBuf, lProcLen);
            }
        }
        else  // special case without decimation
        {
            DemonProc(fpProcBuf, lProcLen);
            if (iDCBlock) DCBlockIIR.Process(fpProcBuf, lProcLen);
            Decimator.Put(fpProcBuf, lProcLen);
        }
    }
}


bool clLofarDemon::PullData (GDT *fpDestBuf, long lDestLen)
{
    bool bGetRes;

    if (lDecimation >= 2l || sLofarRq.bDemon)  // normal case
        bGetRes = Decimator.Get(fpDestBuf, lDestLen);
    else  // special case without decimation
        bGetRes = ReBuffer.Get(fpDestBuf, lDestLen);

    return bGetRes;
}


void clLofarDemon::DemonProc (GDT *fpProcBuf, long lProcLen)
{
    long lProcCntr;

    for (lProcCntr = 0L; lProcCntr < lProcLen; lProcCntr++)
    {
        fpProcBuf[lProcCntr] = fpProcBuf[lProcCntr] * fpProcBuf[lProcCntr];
    }
}


void clLofarDemon::SpectStdDev (GDT *fpSpect)
{
    long lBinCntr;
    long lHistLen = sLofarRq.lMeanLength;
    GDT fStdDev;
    GDT fMean;
    GDT *fpHistory = HistoryBuf;
    #ifdef __GNUG__
    GDT fpBinHist[lHistLen];
    #else
    clDSPAlloc BinHist;
    GDT *fpBinHist = (GDT *) BinHist.Size(lHistLen * sizeof(GDT));
    #endif

    if (lHistCntr >= lHistLen) lHistCntr = 0L;
    DSP.Copy(&fpHistory[lSpectSize * lHistCntr], fpSpect, lSpectSize);
    for (lBinCntr = 0L; lBinCntr < lSpectSize; lBinCntr++)
    {
        for (lHistCntr = 0L; lHistCntr < lHistLen; lHistCntr++)
        {
            fpBinHist[lHistCntr] = 
                fpHistory[lSpectSize * lHistCntr + lBinCntr];
        }
        DSP.StdDev(&fStdDev, &fMean, fpBinHist, lHistLen);
        fpSpect[lBinCntr] = fStdDev;
    }
}


clLofarDemon::clLofarDemon (int iInHandle, int iOutHandle)
{
    lAvgCntr = 0;
    lHistCntr = 0;
    fpWinFunc = NULL;
    fpDataBuf = NULL;
    fpSpectRes = NULL;
    fpLofarRes = NULL;
    bRun = true;
    SOpRequest.SetHandle(iInHandle);
    SOpResult.SetHandle(iOutHandle);
    sLofarResHdr.fPeakLevel = -FLT_MAX;
}


clLofarDemon::~clLofarDemon ()
{
}


int clLofarDemon::Exec ()
{
    if (!GetRequestMsg())
    {
        if (bDebug) printf("lofardemon: GetRequestMsg() timed out\n");
        return 1;
    }
    if (!GetDataHdr())
    {
        if (bDebug) 
            printf("lofardemon: Unable to get header from streamdist\n");
        return 1;
    }
    if (!Initialize())
    {
        if (bDebug)
            printf("lofardemon: Initialization failed\n");
        return 1;
    }
    return MainLoop();
}

