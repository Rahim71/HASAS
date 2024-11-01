/*

    Spectrum based direction server
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
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <Alloc.hh>

#include "Direction3.hh"


static bool bDebug;
static bool bDaemon;
clDirection3 *Direction3;


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
    if (bDebug) printf("direction3: Started\n");
    Direction3 = new clDirection3(3, 4);
    iRetVal = Direction3->Main();
    delete Direction3;
    if (bDebug) printf("direction3: Exit = %i\n", iRetVal);
    return iRetVal;
}


bool clDirection3::GetCfg ()
{
    Cfg.SetFileName(SD_CFGFILE);
    if (!Cfg.GetStr("LocalSocket", cpStreamSocket)) return false;
    Cfg.SetFileName(DIR3_CFGFILE);
    if (!Cfg.GetInt("ArrayType", &iArrayType)) return false;
    if (!Cfg.GetInt("SensorCount", &lSensorCount)) return false;
    if (!Cfg.GetFlt("SensorSpacing", &fSensorSpacing)) return false;
    if (!Cfg.GetInt("FilterSize", &lFilterSize))
        lFilterSize = DIR3_DEF_FILT_SIZE;
    /*if (!Cfg.GetInt("WindowSize", &lWindowSize))
        lWindowSize = DIR3_DEF_FFT_SIZE;*/
    if (!Cfg.GetInt("StartChannel", &lStartChannel))
        lStartChannel = 0l;
    if (!Cfg.GetInt("FilterType", &iFilterType))
        iFilterType = clRecDecimator::FILTER_TYPE_FFT;
    return true;
}


bool clDirection3::GetRq ()
{
    char cpMsgBuf[GLOBAL_HEADER_LEN];

    if (!SOpRequest.ReadSelect(DIR3_REQ_TIMEOUT)) return false;
    if (SOpRequest.ReadN(cpMsgBuf, GLOBAL_HEADER_LEN) != GLOBAL_HEADER_LEN)
        return false;
    Msg.GetRequest(cpMsgBuf, &sReq);
    return true;
}


bool clDirection3::ConnectStream ()
{
    int iSockH;
    stRawDataReq sDataReq;

    iSockH = SClient.Connect(cpStreamSocket);
    if (iSockH < 0) return false;
    SOpData.SetHandle(iSockH);
    if (!SOpData.ReadSelect(DIR3_RAW1ST_TIMEOUT)) return false;
    if (SOpData.ReadN(&sRawHdr, sizeof(sRawHdr)) < (int) sizeof(sRawHdr))
        return false;
    sDataReq.iChannel = -1;
    if (SOpData.WriteN(&sDataReq, sizeof(sDataReq)) < (int) sizeof(sDataReq))
        return false;
    return true;
}


bool clDirection3::InitDir ()
{
    if (bDebug) printf("direction3: Initializing spectrum algorithm\n");
    switch (iArrayType)
    {
        case DIR3_ARRAY_TYPE_DIPOLE:
            if (bDebug) 
                printf("direction3: Dipole array\n");
            SDDipole = new clSpectDirDipole2(fSensorSpacing, sReq.fSoundSpeed,
                sRawHdr.dSampleRate, lFilterSize, iFilterType, 
                sReq.lWindowSize, sReq.fIntegrationTime, bDebug);
            break;
        case DIR3_ARRAY_TYPE_TRIANGLE:
            if (bDebug)
                printf("direction3: Triangular array not supported yet\n");
            break;
        case DIR3_ARRAY_TYPE_LINE:
            if (bDebug)
                printf("direction3: Line array\n");
            SDLine = new clSpectDirLine2(lSensorCount, fSensorSpacing,
                sReq.fSoundSpeed, sRawHdr.dSampleRate, lFilterSize, iFilterType,
                sReq.lWindowSize, sReq.fIntegrationTime, bDebug);
            break;
        case DIR3_ARRAY_TYPE_PLANE:
            if (bDebug)
                printf("direction3: Planar array not supported yet\n");
            break;
        case DIR3_ARRAY_TYPE_CYLINDER:
            if (bDebug)
                printf("direction3: Cylindral array not supported yet\n");
            break;
        case DIR3_ARRAY_TYPE_SPHERE:
            if (bDebug)
                printf("direction3: Spherical array not supported yet\n");
            break;
        default:
            if (bDebug)
                printf("direction3: Unknown array type\n");
    }
    if (bDebug)
    {
        switch (iFilterType)
        {
            case clRecDecimator::FILTER_TYPE_FFT:
                puts("direction3: Using FFT filter");
                break;
            case clRecDecimator::FILTER_TYPE_FIR:
                puts("direction3: Using FIR filter");
                break;
            case clRecDecimator::FILTER_TYPE_IIR:
                puts("direction3: Using IIR filter");
                break;
        }
    }
    return true;
}


void clDirection3::ProcessLoop ()
{
    int iInSize;
    int iResSize = 0;
    int iMsgSize;
    long lTotalSamples;
    char *cpMsgBuf;
    GDT *fpProcData;
    stSpectDir2RN sRemoveNoise;
    clAlloc ProcBuf;
    clAlloc MsgBuf;

    lTotalSamples = abs(lFilterSize) / 2 * sRawHdr.iChannels;
    iInSize = lTotalSamples * sizeof(GDT);
    switch (iArrayType)
    {
        case DIR3_ARRAY_TYPE_DIPOLE:
            iResSize = SDDipole->GetResultCount() * sizeof(GDT);
            break;
        case DIR3_ARRAY_TYPE_TRIANGLE:
            break;
        case DIR3_ARRAY_TYPE_LINE:
            iResSize = SDLine->GetResultCount() * sizeof(GDT);
            break;
        case DIR3_ARRAY_TYPE_PLANE:
        case DIR3_ARRAY_TYPE_CYLINDER:
        case DIR3_ARRAY_TYPE_SPHERE:
            break;
    }
    iMsgSize = GLOBAL_HEADER_LEN + iResSize * 2;
    sRemoveNoise.iType = sReq.iRemoveNoise;
    sRemoveNoise.fAlpha = sReq.fAlpha;
    sRemoveNoise.lMeanLength = sReq.lMeanLength;
    sRemoveNoise.lGapLength = sReq.lGapLength;
    fpProcData = (GDT *) ProcBuf.Size(lTotalSamples * sizeof(GDT));
    cpMsgBuf = (char *) MsgBuf.Size(iMsgSize);
    SOpData.SetRecvBufSize(iInSize * 3);
    SOpResult.SetSendBufSize(iMsgSize * 3);
    if (bDebug) printf("direction3: Buffer size %i/%i bytes\n", 
        iInSize * 3, iMsgSize * 3);
    if (bDebug) printf("direction3: Entering processing loop\n");
    while (bRun)
    {
        if (SOpData.ReadN(fpProcData, iInSize) != iInSize) return;
        gettimeofday(&sResHdr.sTimeStamp, NULL);
        sResHdr.fPeakLevel = DSP.PeakLevel(fpProcData, lTotalSamples);
        switch (iArrayType)
        {
            case DIR3_ARRAY_TYPE_DIPOLE:
                SDDipole->PutData(fpProcData, lTotalSamples, lStartChannel,
                    sRawHdr.iChannels);
                while (SDDipole->GetResults(NULL, NULL, sReq.fLowFreqLimit,
                    sReq.iScaling, &sRemoveNoise) && bRun)
                {
                    sResHdr.lMinBin = SDDipole->GetMinBin();
                    sResHdr.lMaxBin = SDDipole->GetMaxBin();
                    sResHdr.fFreqResolution = SDDipole->GetFreqResolution();
                    sResHdr.lResultCount = SDDipole->GetResultCount();
                    sResHdr.fIntegrationTime = SDDipole->GetIntegrationTime();
                    Msg.SetResult(cpMsgBuf, &sResHdr, SDDipole->GetLevels(),
                        SDDipole->GetDirections());
                    if (SOpResult.WriteN(cpMsgBuf, iMsgSize) != iMsgSize) 
                        return;
                }
                break;
            case DIR3_ARRAY_TYPE_TRIANGLE:
                break;
            case DIR3_ARRAY_TYPE_LINE:
                SDLine->PutData(fpProcData, lTotalSamples, lStartChannel,
                    sRawHdr.iChannels);
                while (SDLine->GetResults(NULL, NULL, sReq.fLowFreqLimit,
                    sReq.iScaling, &sRemoveNoise) && bRun)
                {
                    sResHdr.lMinBin = SDLine->GetMinBin();
                    sResHdr.lMaxBin = SDLine->GetMaxBin();
                    sResHdr.fFreqResolution = SDLine->GetFreqResolution();
                    sResHdr.lResultCount = SDLine->GetResultCount();
                    sResHdr.fIntegrationTime = SDLine->GetIntegrationTime();
                    Msg.SetResult(cpMsgBuf, &sResHdr, SDLine->GetLevels(),
                        SDLine->GetDirections());
                    if (SOpResult.WriteN(cpMsgBuf, iMsgSize) != iMsgSize)
                        return;
                }
                break;
            case DIR3_ARRAY_TYPE_PLANE:
            case DIR3_ARRAY_TYPE_CYLINDER:
            case DIR3_ARRAY_TYPE_SPHERE:
                break;
        }
    }
}


clDirection3::clDirection3 (int iInHandle, int iOutHandle)
{
    bRun = true;
    SOpRequest.SetHandle(iInHandle);
    SOpResult.SetHandle(iOutHandle);
}


clDirection3::~clDirection3 ()
{
}


int clDirection3::Main ()
{
    if (!GetCfg())
    {
        if (bDebug) printf("direction3: Error reading configuration\n");
        return 1;
    }
    if (!GetRq())
    {
        if (bDebug) printf("direction3: Unable to receive request message\n");
        return 1;
    }
    if (!ConnectStream())
    {
        if (bDebug) printf("direction3: Unable to connect to streamdist\n");
        return 1;
    }
    if (!InitDir())
    {
        if (bDebug) 
            printf("direction3: Direction finding initialization failed\n");
        return 1;
    }
    ProcessLoop();
    return 0;
}

