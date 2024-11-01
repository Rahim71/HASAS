/*

    Server for beamformed audio
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
#include <sched.h>
#include <sys/time.h>
#include <sys/types.h>

#include "BeamAudio.hh"


static bool bDebug;
static bool bDaemon;
clBeamAudio *BeamAudio;


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
    #ifndef BSDSYS
    uid_t uidCurrent;
    struct sched_param sSchedParam;
    #endif

    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    signal(SIGSEGV, SigHandler);
    #ifndef BSDSYS
    uidCurrent = getuid();
    setuid(0);
    sSchedParam.sched_priority = BA_SCHED_PRIORITY;
    sched_setscheduler(0, SCHED_FIFO, &sSchedParam);
    setuid(uidCurrent);
    #endif
    bDebug = false;
    bDaemon = false;
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
    if (bDebug) printf("beamaudio: Started\n");
    BeamAudio = new clBeamAudio(3, 4);
    iRetVal = BeamAudio->Main();
    delete BeamAudio;
    if (bDebug) printf("beamaudio: Exit = %i\n", iRetVal);
    return iRetVal;
}


bool clBeamAudio::GetCfg ()
{
    CfgFile.SetFileName(SD_CFGFILE);
    if (!CfgFile.GetStr("LocalSocket", cpStreamSocket)) return false;
    CfgFile.SetFileName(BA_CFGFILE);
    if (!CfgFile.GetInt("StartCh", &lStartCh)) return false;
    if (!CfgFile.GetInt("ArrayType", &iArrayType)) return false;
    if (!CfgFile.GetInt("WindowSize", &lWindowSize))
        lWindowSize = BA_DEF_FRAGMENT_SIZE;
    if (!CfgFile.GetInt("SensorCount", &lSensorCount)) return false;
    if (!CfgFile.GetFlt("SensorSpacing", &fSensorSpacing)) return false;
    if (!CfgFile.GetInt("Shading", &iShadingType)) iShadingType = 0;
    return true;
}


bool clBeamAudio::GetRq ()
{
    char cpReqMsg[GLOBAL_HEADER_LEN];

    if (!SOpRequest.ReadSelect(BA_1STREQ_TIMEOUT)) return false;
    if (SOpRequest.ReadN(cpReqMsg, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    Msg.GetRequest(cpReqMsg, &sRequest);
    return true;
}


bool clBeamAudio::ConnectStream ()
{
    int iSockH;
    stRawDataReq sDataReq;
    
    iSockH = SClient.Connect(cpStreamSocket);
    if (iSockH < 0) return false;
    SOpRaw.SetHandle(iSockH);
    if (!SOpRaw.ReadSelect(BA_RAW1ST_TIMEOUT)) return false;
    if (SOpRaw.ReadN(&sRawHdr, sizeof(sRawHdr)) < (int) sizeof(sRawHdr))
        return false;
    sDataReq.iChannel = -1;
    if (SOpRaw.WriteN(&sDataReq, sizeof(sDataReq)) < (int) sizeof(sDataReq))
        return false;
    return true;
}


bool clBeamAudio::InitBeam ()
{
    switch (iArrayType)
    {
        case BA_ARRAY_TYPE_DIPOLE:
            if (bDebug)
                printf("beamaudio: Dipole array\n");
            FBeamDipole.SetDebug(bDebug);
            if (!FBeamDipole.Initialize(fSensorSpacing, lWindowSize, 
                (GDT) sRawHdr.dSampleRate)) return false;
            FBeamDipole.SetSoundSpeed(sRequest.fSoundSpeed);
            FBeamDipole.SetDirection(sRequest.fDirection, !sRequest.bHighFreq);
            break;
        case BA_ARRAY_TYPE_TRIANGLE:
            if (bDebug) 
                printf("beamaudio: Triangle array not supported yet\n");
            return false;
            break;
        case BA_ARRAY_TYPE_LINE:
            if (bDebug)
                printf("beamaudio: Line array\n");
            FBeamLine.SetDebug(bDebug);
            if (!FBeamLine.Initialize(lSensorCount, fSensorSpacing, 
                lWindowSize, (GDT) sRawHdr.dSampleRate)) return false;
            FBeamLine.SetSoundSpeed(sRequest.fSoundSpeed);
            FBeamLine.SetDirection(sRequest.fDirection, !sRequest.bHighFreq);
            break;
        case BA_ARRAY_TYPE_PLANE:
            if (bDebug) 
                printf("beamaudio: Plane array not supported yet\n");
            return false;
            break;
        case BA_ARRAY_TYPE_CYLINDER:
            if (bDebug)
                printf("beamaudio: Cylinder array not supported yet\n");
            return false;
            break;
        case BA_ARRAY_TYPE_SPHERE:
            if (bDebug)
                printf("beamaudio: Spherical array not supported yet\n");
            return false;
            break;
    }
    return true;
}


bool clBeamAudio::SendFirst ()
{
    char cpMsgBuf[GLOBAL_HEADER_LEN];
    stBeamAudioFirst sFirstMsg;

    sFirstMsg.lBufLength = lWindowSize / 2;
    sFirstMsg.iSampleRate = (int) (sRawHdr.dSampleRate + 0.5);
    Msg.SetFirst(cpMsgBuf, &sFirstMsg);
    if (SOpResult.WriteN(cpMsgBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    return true;
}


void clBeamAudio::SetDirection ()
{
    switch (iArrayType)
    {
        case BA_ARRAY_TYPE_DIPOLE:
            FBeamDipole.SetDirection(sRequest.fDirection, !sRequest.bHighFreq);
            break;
        case BA_ARRAY_TYPE_TRIANGLE:
            break;
        case BA_ARRAY_TYPE_LINE:
            FBeamLine.SetDirection(sRequest.fDirection, !sRequest.bHighFreq);
            break;
        case BA_ARRAY_TYPE_PLANE:
            break;
        case BA_ARRAY_TYPE_CYLINDER:
            break;
        case BA_ARRAY_TYPE_SPHERE:
            break;
    }
}


void clBeamAudio::ProcessLoop ()
{
    long lInDataCount = lWindowSize / 2 * sRawHdr.iChannels;
    long lInDataSize = lInDataCount * sizeof(GDT);
    long lOutDataCount = lWindowSize / 2;
    #ifdef __GNUG__
    GDT fpInData[lInDataCount];
    GDT fpResData[lOutDataCount];
    #else
    clDSPAlloc InData;
    clDSPAlloc ResData;
    GDT *fpInData = (GDT *) InData.Size(lInDataCount * sizeof(GDT));
    GDT *fpResData = (GDT *) ResData.Size(lOutDataCount * sizeof(GDT));
    #endif

    while (bRun)
    {
        if (SOpRequest.ReadSelect(0))
        {
            GetRq();
            SetDirection();
        }
        if (SOpRaw.ReadN(fpInData, lInDataSize) < lInDataSize) return;
        DSP.Zero(fpResData, lOutDataCount);
        switch (iArrayType)
        {
            case BA_ARRAY_TYPE_DIPOLE:
                FBeamDipole.Put(fpInData, lInDataCount, lStartCh, 
                    sRawHdr.iChannels);
                while (FBeamDipole.Get(fpResData, lOutDataCount))
                {
                    if (!SendResult(fpResData, lOutDataCount)) return;
                }
                break;
            case BA_ARRAY_TYPE_TRIANGLE:
                break;
            case BA_ARRAY_TYPE_LINE:
                FBeamLine.Put(fpInData, lInDataCount, lStartCh,
                    sRawHdr.iChannels);
                while (FBeamLine.Get(fpResData, lOutDataCount))
                {
                    if (!SendResult(fpResData, lOutDataCount)) return;
                }
                break;
            case BA_ARRAY_TYPE_PLANE:
                break;
            case BA_ARRAY_TYPE_CYLINDER:
                break;
            case BA_ARRAY_TYPE_SPHERE:
                break;
        }
    }
}


bool clBeamAudio::SendResult (GDT *fpResData, long lResCount)
{
    long lResultSize = GLOBAL_HEADER_LEN + lResCount * sizeof(GDT);
    #ifdef __GNUG__
    char cpMsgBuf[lResultSize];
    #else
    clAlloc MsgBuf;
    char *cpMsgBuf = (char *) MsgBuf.Size(lResultSize);
    #endif

    DSP.Mul(fpResData, fAttenuation, lResCount);
    gettimeofday(&sResHdr.sTimeStamp, NULL);
    sResHdr.lBufLength = lResCount;
    sResHdr.fPeakLevel = DSP.PeakLevel(fpResData, lResCount);
    sResHdr.fDirection = sRequest.fDirection;
    Msg.SetResult(cpMsgBuf, &sResHdr, fpResData);
    if (SOpResult.WriteN(cpMsgBuf, lResultSize) < lResultSize) return false;
    return true;
}


clBeamAudio::clBeamAudio (int iInHandle, int iOutHandle)
{
    bRun = true;
    fAttenuation = (GDT) pow(10.0, -1.0 / 20.0);
    SOpRequest.SetHandle(iInHandle);
    SOpResult.SetHandle(iOutHandle);
    SOpResult.DisableNagle();
}


clBeamAudio::~clBeamAudio ()
{
}


int clBeamAudio::Main ()
{
    if (!GetCfg())
    {
        if (bDebug) printf("beamaudio: Error reading configuration\n");
        return 1;
    }
    if (!GetRq())
    {
        if (bDebug) printf("beamaudio: Unable to receive request message\n");
        return 1;
    }
    if (!ConnectStream())
    {
        if (bDebug) printf("beamaudio: Unable to connect to streamdist\n");
        return 1;
    }
    if (!InitBeam())
    {
        if (bDebug) printf("beamaudio: Beamformer initialization failed\n");
        return 1;
    }
    if (!SendFirst())
    {
        if (bDebug) printf("beamaudio: Failed to send first message\n");
        return 1;
    }
    ProcessLoop();
    return 0;
}

