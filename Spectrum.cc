/*

    Spectrum server
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
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <sys/time.h>

#include <Alloc.hh>

#include "Spectrum.hh"


static bool bDebug;
static bool bDaemon;
clSpectrum *Spectrum;

#define SPECT_NWIN 13
static const char *cpaWindowFuncs[] = {
    "Rectangle",
    "Bartlett",
    "Blackman",
    "Blackman-Harris",
    "Cosine tapered",
    "Exponential",
    "Flat-top",
    "Generic cosine",
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

    bDebug = false;
    bDaemon = false;
    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    signal(SIGSEGV, SigHandler);
    if (argc >= 2)
    {
        if (strcmp(argv[1], "--debug") == 0)
        {
            bDebug = true;
        }
        if (strcmp(argv[1], "-D") == 0)
        {
            bDaemon = true;
        }
    }
    if (bDebug) printf("spectrum: Started\n");
    Spectrum = new clSpectrum(3, 4);
    iRetVal = Spectrum->Main();
    delete Spectrum;
    if (bDebug) printf("spectrum: Exit = %i\n", iRetVal);
    return iRetVal;
}


bool clSpectrum::GetCfg ()
{
    Cfg.SetFileName(SD_CFGFILE);
    if (!Cfg.GetStr("LocalSocket", cpStreamSocket)) return false;
    Cfg.SetFileName(SPECT_CFGFILE);
    if (!Cfg.GetInt("FilterSize", &lFilterSize))
        lFilterSize = SPECT_DEF_FILTSIZE;
    if (!Cfg.GetInt("FilterType", &iFilterType))
        iFilterType = clRecDecimator::FILTER_TYPE_FFT;
    return true;
}


bool clSpectrum::GetRq ()
{
    char cpMsgBuf[GLOBAL_HEADER_LEN];

    if (!SOpRequest.ReadSelect(SPECT_REQ_TIMEOUT)) return false;
    if (SOpRequest.ReadN(cpMsgBuf, GLOBAL_HEADER_LEN) != GLOBAL_HEADER_LEN)
        return false;
    Msg.GetRequest(cpMsgBuf, &sReq);
    return true;
}


bool clSpectrum::ConnectStream ()
{
    int iSockH;
    stRawDataReq sStreamReq;

    iSockH = SClient.Connect(cpStreamSocket);
    if (iSockH < 0) return false;
    SOpData.SetHandle(iSockH);
    if (!SOpData.ReadSelect(SPECT_RAW1ST_TIMEOUT)) return false;
    if (SOpData.ReadN(&sRawHdr, sizeof(sRawHdr)) < (int) sizeof(sRawHdr))
        return false;
    sStreamReq.iChannel = sReq.iChannel;
    if (SOpData.WriteN(&sStreamReq, sizeof(sStreamReq)) < 
        (int) sizeof(sStreamReq)) return false;
    return true;
}


bool clSpectrum::Init ()
{
    GDT *fpNullPtr = NULL;

    lInDataCount = abs(lFilterSize) / 2;
    iInDataSize = lInDataCount * sizeof(GDT);
    InData.Size(iInDataSize);
    if (bDebug)
    {
        printf("spectrum: Filter size %li\n", lFilterSize);
        switch (iFilterType)
        {
            case clRecDecimator::FILTER_TYPE_FFT:
                puts("spectrum: Using FFT decimation filter");
                break;
            case clRecDecimator::FILTER_TYPE_FIR:
                puts("spectrum: Using FIR decimation filter");
                break;
            case clRecDecimator::FILTER_TYPE_IIR:
                puts("spectrum: Using IIR decimation filter");
                break;
        }
    }
    CreateFilter();
    switch (sReq.iType)
    {
        case MSG_SPECT_TYPE_FFT:
            if (bDebug) printf("spectrum: STFT\n");
            lSpectPoints = sReq.lLength;
            lSpectLen = lSpectPoints / 2 + 1;
            FFT.FFTInitialize(lSpectPoints, true);
            break;
        case MSG_SPECT_TYPE_MRFFT:
            printf("spectrum: Multiresolution-STFT not implemented yet\n");
            break;
        case MSG_SPECT_TYPE_GABOR:
            printf("spectrum: Gabor transform not implemented yet\n");
            break;
        case MSG_SPECT_TYPE_WVD:
            if (bDebug) printf("spectrum: Wigner-Ville Distribution\n");
            lSpectPoints = sReq.lLength;
            lSpectLen = lSpectPoints / 2 + 1;
            FFT.FFTInitialize(lSpectPoints, true);
            WVDDec.Initialize(2, -128, (GDT *) NULL, (GDT) 0.25, 
                clRecDecimator::FILTER_TYPE_FIR);
            sReq.iOverlap = 50;
            break;
        case MSG_SPECT_TYPE_HANKEL:
            if (bDebug) printf("spectrum: Hankel transform\n");
            lSpectPoints = sReq.lLength / 2 + 1;
            lSpectLen = lSpectPoints;
            Hankel.Initialize(sReq.lLength, fpNullPtr);
            break;
        case MSG_SPECT_TYPE_AUTOCORR:
            if (bDebug) printf("spectrum: Autocorrelation\n");
            lSpectPoints = sReq.lLength;
            lSpectLen = lSpectPoints / 2;
            FFT.FFTInitialize(lSpectPoints, true);
            break;
        case MSG_SPECT_TYPE_CEPSTRUM:
            if (bDebug) printf("spectrum: Cepstrum\n");
            lSpectPoints = sReq.lLength;
            lSpectLen = lSpectPoints / 2;
            FFT.FFTInitialize(lSpectPoints, true);
            break;
        default:
            printf("spectrum: unknown transform requested\n");
    }
    lResSize = GLOBAL_HEADER_LEN + lSpectLen * sizeof(GDT);
    SpectIn.Size(lSpectPoints * sizeof(GDT));
    SpectOut.Size(lSpectLen * sizeof(GDT));
    ResMsg.Size(lResSize);
    lOldDataCount = (long) (lSpectPoints * (sReq.iOverlap / 100.0) + 0.5);
    lNewDataCount = lSpectPoints - lOldDataCount;
    if (lNewDataCount < 1)
    {
        lNewDataCount = 1;
        lOldDataCount = lSpectPoints - lNewDataCount;
    }
    if (bDebug)
    {
        printf("spectrum: %li samples old, %li samples new data\n", 
            lOldDataCount, lNewDataCount);
    }
    Window.Size(lSpectPoints * sizeof(GDT));
    switch (sReq.iWindow)
    {
        case MSG_SPECT_WND_RECT:
            DSP.Set((GDT *) Window, (GDT) 1, lSpectPoints);
            break;
        case MSG_SPECT_WND_BARTLETT:
            DSP.WinBartlett((GDT *) Window, lSpectPoints);
            break;
        case MSG_SPECT_WND_BLACKMAN:
            DSP.WinExactBlackman((GDT *) Window, lSpectPoints);
            break;
        case MSG_SPECT_WND_BM_HAR:
            DSP.WinBlackmanHarris((GDT *) Window, lSpectPoints);
            break;
        case MSG_SPECT_WND_COS_TAPER:
            DSP.WinCosTapered((GDT *) Window, lSpectPoints);
            break;
        case MSG_SPECT_WND_EXP:
            DSP.WinExp((GDT *) Window, (GDT) sReq.fWinParam, lSpectPoints);
            break;
        case MSG_SPECT_WND_FLATTOP:
            DSP.WinFlatTop((GDT *) Window, lSpectPoints);
            break;
        case MSG_SPECT_WND_GEN_COS:
            DSP.Set((GDT *) Window, (GDT) 1, lSpectPoints);
            break;
        case MSG_SPECT_WND_HAMMING:
            DSP.WinHamming((GDT *) Window, lSpectPoints);
            break;
        case MSG_SPECT_WND_HANNING:
            DSP.WinHanning((GDT *) Window, lSpectPoints);
            break;
        case MSG_SPECT_WND_KAISER:
            DSP.WinKaiser((GDT *) Window, (GDT) sReq.fWinParam, lSpectPoints);
            break;
        case MSG_SPECT_WND_KAI_BES:
            DSP.WinKaiserBessel((GDT *) Window, (GDT) sReq.fWinParam,
                lSpectPoints);
            break;
        case MSG_SPECT_WND_TUKEY:
            DSP.WinTukey((GDT *) Window, lSpectPoints);
            break;
        default:
            printf("spectrum: unknown window function requested\n");
    }
    if (sReq.iWindow < SPECT_NWIN && bDebug)
        printf("spectrum: %s window\n", cpaWindowFuncs[sReq.iWindow]);
    return true;
}


void clSpectrum::CreateFilter ()
{
    int iBandNo;
    long lTwosPower;
    float fNyquist;
    float fBandWidth;
    GDT fBandCenter;
    GDT *fpNullPtr = NULL;

    fNyquist = (float) sRawHdr.dSampleRate / 2.0f;
    if (sReq.iLowFreq == sReq.iHighFreq)
    {
        sReq.iLowFreq = 0;
        sReq.iHighFreq = (int) fNyquist;
    }
    fBandWidth = sReq.iHighFreq - sReq.iLowFreq;
    lTwosPower = (long) (log(fNyquist / fBandWidth) / log(2.0) + 0.5);
    lDecimation = (long) pow(2.0, lTwosPower);
    fBandWidth = fNyquist / lDecimation;
    iBandNo = (int) (sReq.iHighFreq / fBandWidth + 0.5f);
    if ((fBandWidth * iBandNo) < (sReq.iLowFreq + fBandWidth / 2.0f))
        iBandNo++;
    fHighCorner = fBandWidth * iBandNo;
    fLowCorner = fHighCorner - fBandWidth;
    fBandCenter = (fLowCorner + fBandWidth / 2.0f) / fNyquist;
    bReverseOrder = ((iBandNo - 1) % 2 == 0) ? false : true;
    if (bDebug) 
    {
        printf("spectrum: lower corner %.2f higher corner %.2f\n",
            fLowCorner, fHighCorner);
        printf("spectrum: decimation %li, %s order\n",
            lDecimation,
            (bReverseOrder) ? "reverse" : "normal");
        printf("spectrum: normalized center %f\n", fBandCenter);
    }
    Decimator.Initialize(lDecimation, lFilterSize, fpNullPtr, fBandCenter, 
        iFilterType);
}


void clSpectrum::ProcessLoop ()
{
    long lSpectCntr;
    long lSpectMax;
    GDT fScale;
    GDT fLogScale;
    GDT fPeakLevel;
    GDT *fpWindow = Window;
    GDT *fpSpectIn = SpectIn;
    GDT *fpSpectOut = SpectOut;
    GDT *fpOldData;
    GDT *fpNewData;
    GDT *fpRevBuf;
    GDT *fpTemp;
    GCDT *spCplxSpect;
    clDSPAlloc OldData;
    clDSPAlloc NewData;
    clDSPAlloc RevBuf;
    clDSPAlloc Temp;
    clDSPAlloc CplxSpect;

    fpOldData = (GDT *) OldData.Size(lOldDataCount * sizeof(GDT));
    DSP.Zero(fpOldData, lOldDataCount);
    fpNewData = (GDT *) NewData.Size(lNewDataCount * sizeof(GDT));
    fpRevBuf = (GDT *) RevBuf.Size(lSpectLen * sizeof(GDT));
    fpTemp = (GDT *) Temp.Size(lSpectPoints * sizeof(GDT));
    DSP.Zero(fpTemp, lSpectPoints);
    spCplxSpect = (GCDT *) CplxSpect.Size(lSpectPoints * sizeof(GCDT));
    DSP.Zero(spCplxSpect, lSpectPoints);
    fLogScale = 1.0f / sReq.fDynRange;
    while (bRun)
    {
        if (!GetData())
        {
            Stop();
            break;
        }
        while (PullData(fpNewData, lNewDataCount) && bRun)
        {
            DSP.Copy(fpSpectIn, fpOldData, lOldDataCount);
            DSP.Copy(&fpSpectIn[lOldDataCount], fpNewData, lNewDataCount);
            DSP.Copy(fpOldData, &fpSpectIn[lNewDataCount], lOldDataCount);
            fPeakLevel = DSP.PeakLevel(fpSpectIn, lSpectPoints);
            switch (sReq.iType)
            {
                case MSG_SPECT_TYPE_FFT:
                    DSP.Mul(fpSpectIn, fpWindow, lSpectPoints);
                    FFT.FFTi(spCplxSpect, fpSpectIn);
                    DSP.Magnitude(fpSpectOut, spCplxSpect, lSpectLen);
                    break;
                case MSG_SPECT_TYPE_MRFFT:
                    break;
                case MSG_SPECT_TYPE_GABOR:
                    break;
                case MSG_SPECT_TYPE_WVD:
                    /*WVDDec.Put(fpSpectIn, lSpectPoints);
                    WVDDec.Get(fpTemp, lSpectPoints / 2);
                    DSP.Zero(&fpTemp[lSpectPoints / 2], lSpectPoints / 2);
                    DSP.Reverse(fpWindow, fpTemp, lSpectPoints);
                    DSP.Mul(fpSpectIn, fpWindow, lSpectPoints);*/
                    FFT.FFTi(spCplxSpect, fpSpectIn);
                    DSP.MulC(spCplxSpect, spCplxSpect, 
                        lSpectPoints / 2 + 1);
                    DSP.Magnitude(fpSpectOut, spCplxSpect, lSpectLen);
                    DSP.Mul(fpSpectOut, (GDT) 2, lSpectLen);
                    break;
                case MSG_SPECT_TYPE_HANKEL:
                    DSP.Mul(fpSpectIn, fpWindow, lSpectPoints);
                    Hankel.Process0(fpSpectOut, fpSpectIn);  // 0th order
                    //Hankel.Process1(fpSpectOut, fpSpectIn);  // 1st order
                    break;
                case MSG_SPECT_TYPE_AUTOCORR:
                    DSP.Mul(fpSpectIn, fpWindow, lSpectPoints);
                    FFT.FFTi(spCplxSpect, fpSpectIn);
                    DSP.MulC(spCplxSpect, spCplxSpect, 
                        lSpectPoints / 2 + 1);
                    FFT.IFFTo(fpTemp, spCplxSpect);
                    DSP.Copy(fpSpectOut, fpTemp, lSpectLen);
                    break;
                case MSG_SPECT_TYPE_CEPSTRUM:
                    DSP.Mul(fpSpectIn, fpWindow, lSpectPoints);
                    FFT.FFTi(spCplxSpect, fpSpectIn);
                    lSpectMax = lSpectPoints / 2 + 1;
                    for (lSpectCntr = 0; 
                        lSpectCntr < lSpectMax;
                        lSpectCntr++)
                    {
                        #ifndef HAVE_GLIBC
                        spCplxSpect[lSpectCntr].R = log10(sqrt(
                            spCplxSpect[lSpectCntr].R * 
                            spCplxSpect[lSpectCntr].R +
                            spCplxSpect[lSpectCntr].I *
                            spCplxSpect[lSpectCntr].I));
                        #else
                        spCplxSpect[lSpectCntr].R = log10(hypot(
                            spCplxSpect[lSpectCntr].R, 
                            spCplxSpect[lSpectCntr].I));
                        #endif
                        spCplxSpect[lSpectCntr].I = 0;
                    }
                    FFT.IFFTo(fpTemp, spCplxSpect);
                    fpTemp[0] = 0;
                    fpTemp[1] = 0;
                    fScale = (GDT) 2 / (GDT) lSpectPoints;
                    DSP.Mul(fpSpectOut, fpTemp, fScale, lSpectLen);
                    break;
            }
            if (!sReq.bLinear)
            {
                if (!sReq.bNormalize)
                {
                    for (lSpectCntr = 0; 
                        lSpectCntr < lSpectLen; 
                        lSpectCntr++)
                    {
                        fpSpectOut[lSpectCntr] = (GDT)
                            (20.0 * log10(fpSpectOut[lSpectCntr]) +
                            sReq.fDynRange) * fLogScale;
                    }
                }
                else
                {
                    for (lSpectCntr = 0;
                        lSpectCntr < lSpectLen;
                        lSpectCntr++)
                    {
                        fpSpectOut[lSpectCntr] = (GDT)
                            (20.0 * log10(fpSpectOut[lSpectCntr]));
                    }
                }
            }
            if (bReverseOrder)
            {
                DSP.Copy(fpRevBuf, fpSpectOut, lSpectLen);
                DSP.Reverse(fpSpectOut, fpRevBuf, lSpectLen);
            }
            if (sReq.bNormalize)
            {
                DSP.Scale01(fpSpectOut, lSpectLen);
            }
            if (sReq.fSlope != 0.0f)
            {
                double dSlopeMul = pow(10.0, sReq.fSlope / 20.0);
                double dNyquist = (double) sRawHdr.dSampleRate / 
                    (double) lDecimation / 2.0;
                double dFreqRes = dNyquist / (double) lSpectLen;
                for (long lFreqBinCntr = 1; 
                    lFreqBinCntr < lSpectLen; 
                    lFreqBinCntr++)
                {
                    double dBinFreq = dFreqRes * lFreqBinCntr;
                    fpSpectOut[lFreqBinCntr] *= (GDT)
                        (pow(dSlopeMul, log(dBinFreq) / log(2.0)));
                }
            }
            if (sReq.fGain != 0.0f)
            {
                GDT fGainMul = (GDT) pow(10.0, sReq.fGain / 20.0);
                DSP.Mul(fpSpectOut, fGainMul, lSpectLen);
            }
            switch (sReq.iRemoveNoise)
            {
                case MSG_SPECT_BNER_NONE:
                    break;
                case MSG_SPECT_BNER_TPSW:
                    RN.TPSW(fpSpectOut, sReq.fAlpha, sReq.lMeanLength,
                        sReq.lGapLength, lSpectLen);
                    break;
                case MSG_SPECT_BNER_OTA:
                    RN.OTA(fpSpectOut, sReq.fAlpha, sReq.lMeanLength,
                        lSpectLen);
                    break;
                case MSG_SPECT_BNER_DIFF:
                    RN.Diff(fpSpectOut, sReq.fAlpha, lSpectLen);
                    break;
                case MSG_SPECT_BNER_IDIFF:
                    RN.InvDiff(fpSpectOut, sReq.fAlpha, lSpectLen);
                    break;
                default:
                    // None
                    break;
            }
            gettimeofday(&sResHdr.sTimeStamp, NULL);
            sResHdr.iChannel = sReq.iChannel;
            sResHdr.lLength = lSpectLen;
            sResHdr.iLowFreq = (int) (fLowCorner + 0.5f);
            sResHdr.iHighFreq = (int) (fHighCorner + 0.5f);
            sResHdr.iSampleRate = (int) (sRawHdr.dSampleRate + 0.5);
            sResHdr.bLinear = sReq.bLinear;
            sResHdr.fPeakLevel = fPeakLevel;
            if (lDecimation > 0)
            {
                sResHdr.fLineTime = 
                    (float) lNewDataCount / 
                    ((float) sRawHdr.dSampleRate / (float) lDecimation);
            }
            else
            {
                sResHdr.fLineTime = (float) lNewDataCount /
                    (float) sRawHdr.dSampleRate;
            }
            Msg.SetResult(ResMsg, &sResHdr, fpSpectOut);
            if (SOpResult.WriteN(ResMsg, lResSize) < lResSize)
            {
                if (bDebug)
                    printf("spectrum: clSockOp::WriteN() error, client disconnected?\n");
                Stop();
                return;
            }
        }
    }
}


bool clSpectrum::GetData ()
{
    GDT *fpInData = InData;

    if (SOpData.ReadN(InData, iInDataSize) < iInDataSize)
    {
        if (bDebug)
            printf("spectrum: clSockOp::ReadN() error, server disconnected?\n");
        return false;
    }
    if (lDecimation < 2)
        ReBuffer.Put(fpInData, lInDataCount);
    else
        Decimator.Put(fpInData, lInDataCount);
    return true;
}


bool clSpectrum::PullData (GDT *fpDestBuf, long lDestCount)
{
    if (lDecimation < 2)
        return ReBuffer.Get(fpDestBuf, lDestCount);
    else
        return Decimator.Get(fpDestBuf, lDestCount);
}


clSpectrum::clSpectrum (int iInHandle, int iOutHandle)
{
    bRun = true;
    SOpRequest.SetHandle(iInHandle);
    SOpResult.SetHandle(iOutHandle);
}


clSpectrum::~clSpectrum ()
{
}


int clSpectrum::Main ()
{
    if (!GetCfg())
    {
        if (bDebug) printf("spectrum: Error reading configuration\n");
        return 1;
    }
    if (!GetRq())
    {
        if (bDebug) printf("spectrum: Unable to receive request message\n");
        return 1;
    }
    if (!ConnectStream())
    {
        if (bDebug) printf("spectrum: Unable to connect to streamdist\n");
        return 1;
    }
    if (!Init())
    {
        if (bDebug)
            printf("spectrum: Direction finding initialization failed\n");
        return 1;
    }
    ProcessLoop();
    return 0;
}
