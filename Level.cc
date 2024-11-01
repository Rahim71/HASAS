/*

    Level server
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


#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <float.h>
#include <unistd.h>
#include <sys/time.h>

#include "Level.hh"


static bool bDebug = false;
static bool bDaemon = false;
clLevel *Level;


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
    if (bDebug) printf("level: Started\n");
    Level = new clLevel(3, 4);
    iRetVal = Level->Main(argc, argv);
    delete Level;
    if (bDebug) printf("level: Exit = %i\n", iRetVal);
    return iRetVal;
}


bool clLevel::GetCfg ()
{
    CfgFile.SetFileName(SD_CFGFILE);
    if (!CfgFile.GetStr("LocalSocket", cpStreamSocket)) return false;
    CfgFile.SetFileName(LEVEL_CFGFILE);
    if (!CfgFile.GetInt("FilterSize", &lFilterSize))
        lFilterSize = LEVEL_DEF_FILTER_SIZE;
    return true;
}


bool clLevel::GetRequest ()
{
    char cpReqMsg[GLOBAL_HEADER_LEN];

    if (!SOpRequest.ReadSelect(LEVEL_1STREQ_TIMEOUT)) return false;
    if (SOpRequest.ReadN(cpReqMsg, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    Msg.GetRequest(cpReqMsg, &sRequest);
    return true;
}


bool clLevel::ConnectStream ()
{
    int iSockH;
    stRawDataReq sDataReq;

    iSockH = SClient.Connect(cpStreamSocket);
    if (iSockH < 0) return false;
    SOpRaw.SetHandle(iSockH);
    if (!SOpRaw.ReadSelect(LEVEL_RAW1ST_TIMEOUT)) return false;
    if (SOpRaw.ReadN(&sRawHdr, sizeof(sRawHdr)) < (int) sizeof(sRawHdr))
        return false;
    sDataReq.iChannel = sRequest.iChannel;
    if (SOpRaw.WriteN(&sDataReq, sizeof(sDataReq)) < (int) sizeof(sDataReq))
        return false;
    return true;
}


bool clLevel::Initialize ()
{
    float fLowCorner;
    float fHighCorner;
    GDT *fpNullPtr = NULL;
    
    lRawDataCount = lFilterSize / 2l;
    lRawBufSize = lRawDataCount * sizeof(GDT);
    RawBuf.Size(lRawBufSize);
    lSampleCount = (long) 
        (sRawHdr.dSampleRate * sRequest.fIntegrationTime + 0.5F);
    if (lSampleCount <= 0) 
        lSampleCount = 1l;
    DataBuf.Size(lSampleCount * sizeof(GDT));
    sResult.fIntegrationTime = 
        (float) lSampleCount / (float) sRawHdr.dSampleRate;
    if (bDebug) 
    {
        printf("level: Integration time: %f -> %f\n", 
            sRequest.fIntegrationTime, sResult.fIntegrationTime);
    }
    // Initialize filter
    Filter.Initialize(lFilterSize, fpNullPtr);
    if (sRequest.fLowFrequency < sRequest.fHighFrequency)
    {
        fLowCorner = sRequest.fLowFrequency;
        fHighCorner = sRequest.fHighFrequency;
        Filter.DesignBP(&fLowCorner, &fHighCorner, (GDT) sRawHdr.dSampleRate);
        if (bDebug)
        {
            printf("level: Lower corner %.2f -> %.2f\n", 
                sRequest.fLowFrequency, fLowCorner);
            printf("level: Higher corner %.2f -> %.2f\n",
                sRequest.fHighFrequency, fHighCorner);
        }
    }
    if (bDebug)
    {
        printf("level: Algorithm: ");
        switch (sRequest.iAlgorithm)
        {
            case MSG_LEVEL_ALG_PEAK:
                printf("Peak level\n");
                break;
            case MSG_LEVEL_ALG_RMS:
                printf("RMS\n");
                break;
            case MSG_LEVEL_ALG_MEAN:
                printf("Mean\n");
                break;
            case MSG_LEVEL_ALG_MEDIAN:
                printf("Median\n");
                break;
            case MSG_LEVEL_ALG_STDDEV:
                printf("Standard deviation\n");
                break;
            default:
                printf("Unknown\n");
        }
    }
    return true;
}


bool clLevel::ReceiveData ()
{
    GDT fTempPL;
    GDT *fpRaw;

    fpRaw = RawBuf;
    if (SOpRaw.ReadSelect(LEVEL_TIMEOUT))
    {
        if (SOpRaw.ReadN(fpRaw, lRawBufSize) < lRawBufSize)
            return false;
        fTempPL = DSP.PeakLevel(fpRaw, lRawDataCount);
        if (fTempPL > fResPeakLevel) fResPeakLevel = fTempPL;
        Filter.Put(fpRaw, lRawDataCount);
    }
    else return false;
    return true;
}


bool clLevel::Process ()
{
    GDT fStdDev;
    GDT fMean;
    GDT *fpData;

    fpData = DataBuf;
    gettimeofday(&sResult.sTimeStamp, NULL);
    switch (sRequest.iAlgorithm)
    {
        case MSG_LEVEL_ALG_PEAK:
            sResult.fResult = DSP.PeakLevel(fpData, lSampleCount);
            break;
        case MSG_LEVEL_ALG_RMS:
            sResult.fResult = 20.0 * log10(DSP.RMS(fpData, lSampleCount));
            break;
        case MSG_LEVEL_ALG_MEAN:
            DSP.Abs(fpData, lSampleCount);
            sResult.fResult = 20.0 * log10(DSP.Mean(fpData, lSampleCount));
            break;
        case MSG_LEVEL_ALG_MEDIAN:
            DSP.Abs(fpData, lSampleCount);
            sResult.fResult = 20.0 * log10(DSP.Median(fpData, lSampleCount));
            break;
        case MSG_LEVEL_ALG_STDDEV:
            DSP.Abs(fpData, lSampleCount);
            DSP.StdDev(&fStdDev, &fMean, fpData, lSampleCount);
            sResult.fResult = 20.0 * log10(fStdDev);
            break;
        default:
            sResult.fResult = 0;
    }
    return true;
}


bool clLevel::SendResult ()
{
    char cpMsgBuf[GLOBAL_HEADER_LEN];

    Msg.SetResult(cpMsgBuf, &sResult);
    if (SOpResult.WriteN(cpMsgBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    sResult.fPeakLevel = 0;
    return true;
}


clLevel::clLevel (int iInHandle, int iOutHandle)
{
    bRun = true;
    SOpRequest.SetHandle(iInHandle);
    SOpResult.SetHandle(iOutHandle);
}


clLevel::~clLevel ()
{
}


int clLevel::Main (int iArgC, char *cpaArgV[])
{
    GDT *fpData;

    if (!GetCfg())
    {
        if (bDebug) printf("level: Error reading configuration\n");
        return 1;
    }
    if (!GetRequest())
    {
        if (bDebug) printf("level: Unable to receive request message\n");
        return 1;
    }
    if (!ConnectStream())
    {
        if (bDebug) printf("level: Unable to connect to streamdist\n");
        return 1;
    }
    if (!Initialize())
    {
        if (bDebug) printf("level: Initialization failed\n");
        return 1;
    }
    fpData = DataBuf;
    while (bRun)
    {
        if (!ReceiveData()) 
        {
            if (bDebug) printf("level: ReceiveData() returned error\n");
            return 2;
        }
        while (Filter.Get(fpData, lSampleCount) && bRun)
        {
            if (!Process())
            {
                if (bDebug) printf("level: Process() returned error\n");
                return 2;
            }
            if (!SendResult())
            {
                if (bDebug) 
                {
                    printf(
                        "level: SendResult() returned error (disconnect?)\n");
                    bRun = false;
                }
            }
        }
    }
    return 0;
}

