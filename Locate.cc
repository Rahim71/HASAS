/*

    Algorithm for locating sound sources in array field
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


#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <signal.h>
#include <unistd.h>

#ifdef USE_TRILLIUM
#include <tstdio.h>
#endif

#include "Locate.hh"


static bool bDebug = false;
static int iNodeCount;
static int iRank;
clMPIProc MPIProc;
clLocate *Locate;
clSubLocate *SubLocate;


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
    MPIProc.Initialize(&argc, &argv);
    MPIProc.GetNodeCount(&iNodeCount);
    MPIProc.GetRank(&iRank);
    if (argc >= 2)
    {
        if (strcmp(argv[1], "--debug") == 0)
        {
            bDebug = true;
        }
        if (strcmp(argv[1], "--version") == 0)
        {
            if (iRank == 0)
            {
                printf("%s v%i.%i.%i\n", argv[0], GLOBAL_VERSMAJ,
                    GLOBAL_VERSMIN, GLOBAL_VERSPL);
                printf("Copyright (C) 2000-2001 Jussi Laako\n");
            }
            MPIProc.Finalize();
            return 0;
        }
    }
    if (iRank == 0)
    {
        Locate = new clLocate;
        iRetVal = Locate->Main(argc, argv);
        delete Locate;
    }
    else
    {
        SubLocate = new clSubLocate;
        iRetVal = SubLocate->Main(argc, argv);
        delete SubLocate;
    }
    MPIProc.Finalize();
    return iRetVal;
}


/* - MASTER - */


void MasterSigHandler (int iSigNum)
{
    switch (iSigNum)
    {
        case SIGINT:
            fprintf(stderr, "Received SIGINT, terminating...\n");
            Locate->Stop();
            break;
        default:
            fprintf(stderr, "Received unknown signal, terminating...\n");
            Locate->Stop();
    }
}


void *WrapProcessThread (void *vpParam)
{
    return Locate->ProcessThread(vpParam);
}


void *WrapServeClientThread (void *vpParam)
{
    return Locate->ServeClientThread(vpParam);
}


bool clLocate::Initialize ()
{
    int iReadRes;
    int iHostPort;
    long lPosX;
    long lPosY;
    float fAzimuthDeg;
    float fAzimuth;
    char cpHostName[LOCATE_HOSTNAME_MAXLEN];
    FILE *fileSensors;
    
    Cfg.SetFileName(LOCATE_CFGFILE);
    if (!Cfg.GetInt("Width", &lWidth)) return false;
    if (!Cfg.GetInt("Height", &lHeight)) return false;
    if (!Cfg.GetFlt("Weight", &fWeight)) return false;
    if (!Cfg.GetInt("Port", &iPort)) return false;

    if (!Cfg.GetInt("WindowSize", &sDirCfg.lWindowSize))
        sDirCfg.lWindowSize = LOCATE_DEF_WINDOWSIZE;
    if (!Cfg.GetFlt("SoundSpeed", &sDirCfg.fSoundSpeed))
        sDirCfg.fSoundSpeed = LOCATE_DEF_SOUNDSPEED;
    if (!Cfg.GetFlt("LowFrequency", &sDirCfg.fLowFrequency))
        sDirCfg.fLowFrequency = LOCATE_DEF_LOWFREQ;
    if (!Cfg.GetFlt("IntegrationTime", &sDirCfg.fIntegrationTime))
        sDirCfg.fIntegrationTime = LOCATE_DEF_INTTIME;
    if (!Cfg.GetInt("Scaling", &sDirCfg.iScaling))
        sDirCfg.iScaling = LOCATE_DEF_SCALING;
    if (!Cfg.GetFlt("ScalingExp", &sDirCfg.fScalingExp))
        sDirCfg.fScalingExp = LOCATE_DEF_SCALINGEXP;
    if (!Cfg.GetInt("RemoveNoise", &sDirCfg.iRemoveNoise))
        sDirCfg.iRemoveNoise = LOCATE_DEF_REMOVENOISE;
    if (!Cfg.GetFlt("Alpha", &sDirCfg.fAlpha))
        sDirCfg.fAlpha = LOCATE_DEF_ALPHA;
    if (!Cfg.GetInt("MeanLength", &sDirCfg.lMeanLength))
        sDirCfg.lMeanLength = LOCATE_DEF_MEANLENGTH;
    if (!Cfg.GetInt("GapLength", &sDirCfg.lGapLength))
        sDirCfg.lGapLength = LOCATE_DEF_GAPLENGTH;

    fileSensors = fopen(LOCATE_SENSOR_LIST, "rt");
    if (fileSensors == NULL) return false;
    do
    {
        iReadRes = fscanf(fileSensors, "%s %i %li %li %f", 
            cpHostName, &iHostPort,
            &lPosX, &lPosY, &fAzimuthDeg);
        fAzimuth = DSP.DegToRad(fAzimuthDeg);
        if (iReadRes == EOF) break;
        if (iSensorCount < (iNodeCount - 1))
        {
            SendParams(iSensorCount, cpHostName, iHostPort, lPosX, lPosY, 
                fAzimuth);
            iSensorCount++;
        }
        else fprintf(stderr, "Ran out of nodes!\n");
    } while (!feof(fileSensors));
    fclose(fileSensors);
    fpLocMatrix = (GDT *) LocMatrix.Size(lWidth * lHeight * sizeof(GDT));
    iMsgSize = GLOBAL_HEADER_LEN + lWidth * lHeight * sizeof(GDT);
    ResMsg.Size(iMsgSize);
    return true;
}


void clLocate::SendParams (int iSensor, 
    const char *cpHostName, int iHostPort,
    long lPosX, long lPosY, float fAzimuth)
{
    int iDestRank = iSensor + 1;

    MPICParam.Send(iDestRank, (char *) cpHostName, LOCATE_HOSTNAME_MAXLEN);
    MPICParam.Send(iDestRank, &iHostPort, 1);

    MPICParam.Send(iDestRank, &lWidth, 1);
    MPICParam.Send(iDestRank, &lHeight, 1);

    MPICParam.Send(iDestRank, &lPosX, 1);
    MPICParam.Send(iDestRank, &lPosY, 1);
    MPICParam.Send(iDestRank, &fAzimuth, 1);

    MPICParam.Send(iDestRank, (void *) &sDirCfg, sizeof(stDirCfg));
}


clLocate::clLocate ()
{
    bRun = true;
    iSensorCount = 0;
    Log.Open(LOCATE_LOGFILE);
    MPICCtrl.SetTag(LOCATE_CONTROL_TAG);
    MPICParam.SetTag(LOCATE_PARAM_TAG);
    MPICNormal.SetTag(LOCATE_NORMAL_TAG);
    Log.Add('*', "Starting");
}


clLocate::~clLocate ()
{
    Log.Add('*', "Stopping");
}


int clLocate::Main (int iArgC, char **cpArgV)
{
    int iSockH;
    pthread_t ptidProcess;
    pthread_t ptidServeClient;
    clSockServ SServer;

    signal(SIGINT, MasterSigHandler);
    if (bDebug) printf("clLocate::Main -> Initialize\n");
    if (!Initialize ()) return 1;
    if (!SServer.Bind(NULL, iPort)) return 2;
    pthread_create(&ptidProcess, NULL, WrapProcessThread, NULL);
    if (bDebug) printf("clLocate::Main ... waiting connections ...\n");
    while (bRun)
    {
        iSockH = SServer.WaitForConnect(LOCATE_TIMEOUT);
        if (iSockH >= 0)
        {
            pthread_create(&ptidServeClient, NULL, WrapServeClientThread, 
                (void *) iSockH);
            pthread_detach(ptidServeClient);
        }
    }
    pthread_join(ptidProcess, NULL);
    if (bDebug) printf("clLocate::Main END\n");
    return 0;
}


void clLocate::Stop ()
{
    int iNodeCntr;
    int iCtrlCmd = LOCATE_CTRL_STOP;
    
    bRun = false;
    for (iNodeCntr = 1; iNodeCntr <= iNodeCount; iNodeCntr++)
        MPICCtrl.Send(iNodeCntr, &iCtrlCmd, 1);
}


void *clLocate::ProcessThread (void *vpParam)
{
    int iNodeCntr;
    GDT *fpSubRes;
    stLocateRes sResHdr;
    sigset_t sigsetThis;
    clAlloc SubRes;
    clLocateSystem LocateSystem;

    sigemptyset(&sigsetThis);
    sigaddset(&sigsetThis, SIGFPE);
    sigaddset(&sigsetThis, SIGINT);
    pthread_sigmask(SIG_BLOCK, &sigsetThis, NULL);
    fpSubRes = (GDT *) SubRes.Size(lWidth * lHeight * sizeof(GDT));
    LocateSystem.Initialize(lWidth, lHeight, fWeight);
    while (bRun)
    {
        iNodeCntr = 1;
        do
        {
            if (bDebug) printf("clLocate::ProcessThread ... receiving ...\n");
            if (MPICNormal.Recv(MPI_ANY_SOURCE, fpSubRes, lWidth * lHeight))
            {
                if (bDebug) printf("clLocate::ProcessThread ... adding ...\n");
                LocateSystem.Add(fpSubRes);
                iNodeCntr++;
            }
        } while (iNodeCntr < iNodeCount);
        if (bDebug) printf("clLocate::ProcessThread ... finalizing ...\n");
        LocateSystem.Process();
        LocateSystem.GetResults(fpLocMatrix);
        sResHdr.lPointCount = lWidth * lHeight;
        MtxResMsg.Wait();
        Msg.SetResult(ResMsg, &sResHdr, fpLocMatrix);
        CndResMsg.NotifyAll();
        MtxResMsg.Release();
    }
    return NULL;
}


void *clLocate::ServeClientThread (void *vpParam)
{
    bool bConnected = true;
    char cpHdrBuf[GLOBAL_HEADER_LEN];
    stLocateHdr sHdr;
    sigset_t sigsetThis;
    clAlloc LocalMsg;
    clSockOp SOp((int) vpParam);
    clLocateMsg Msg;

    sigemptyset(&sigsetThis);
    sigaddset(&sigsetThis, SIGFPE);
    sigaddset(&sigsetThis, SIGINT);
    sigaddset(&sigsetThis, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &sigsetThis, NULL);
    LocalMsg.Size(iMsgSize);
    sHdr.iWidth = lWidth;
    sHdr.iHeight = lHeight;
    Msg.SetHeader(cpHdrBuf, &sHdr);
    if (SOp.WriteN(cpHdrBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        bConnected = false;
    while (bConnected && bRun)
    {
        MtxResMsg.Wait();
        CndResMsg.Wait(MtxResMsg.GetPtr());
        memcpy(LocalMsg, ResMsg, iMsgSize);
        MtxResMsg.Release();
        if (SOp.WriteN(LocalMsg, iMsgSize) < iMsgSize)
            bConnected = false;
    }
    return NULL;
}


/* - SLAVE - */


bool clSubLocate::RecvParams ()
{
    if (!MPICParam.ProbeAny(0)) return false;
    if (MPICParam.GetSenderTag() != LOCATE_PARAM_TAG) return false;

    MPICParam.Recv(0, cpHostName, LOCATE_HOSTNAME_MAXLEN);
    MPICParam.Recv(0, &iHostPort, 1);

    MPICParam.Recv(0, &lWidth, 1);
    MPICParam.Recv(0, &lHeight, 1);

    MPICParam.Recv(0, &lPosX, 1);
    MPICParam.Recv(0, &lPosY, 1);
    MPICParam.Recv(0, &fAzimuth, 1);

    MPICParam.Recv(0, (void *) &sDirCfg, sizeof(stDirCfg));
    return true;
}


bool clSubLocate::Initialize ()
{
    long lSpectSize = sDirCfg.lWindowSize / 2 + 1;

    if (!LocSens.Initialize(lWidth, lHeight, lPosX, lPosY, fAzimuth, false)) 
        return false;
    iDirMsgSize = GLOBAL_HEADER_LEN + lSpectSize * sizeof(GDT) * 2;
    DirMsg.Size(iDirMsgSize);
    fpLevRes = (GDT *) LevRes.Size(sDirCfg.lWindowSize * sizeof(GDT));
    fpDirRes = (GDT *) DirRes.Size(sDirCfg.lWindowSize * sizeof(GDT));
    return true;
}


bool clSubLocate::ConnectDir ()
{
    int iSockH;
    char cpReqMsg[GLOBAL_HEADER_LEN];
    stDirReq2 sReq;
    clSockClie SClient;

    sReq.lWindowSize = sDirCfg.lWindowSize;
    sReq.fSoundSpeed = sDirCfg.fSoundSpeed;
    sReq.fLowFreqLimit = sDirCfg.fLowFrequency;
    sReq.fIntegrationTime = sDirCfg.fIntegrationTime;
    sReq.iScaling = sDirCfg.iScaling;
    sReq.fScalingExp = sDirCfg.fScalingExp;
    sReq.iRemoveNoise = sDirCfg.iRemoveNoise;
    sReq.fAlpha = sDirCfg.fAlpha;
    sReq.lMeanLength = sDirCfg.lMeanLength;
    sReq.lGapLength = sDirCfg.lGapLength;
    iSockH = SClient.Connect(cpHostName, NULL, iHostPort);
    if (iSockH < 0) return false;
    SOp.SetHandle(iSockH);
    strcpy(cpReqMsg, LOCATE_DIR_PROC);
    if (SOp.WriteN(cpReqMsg, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    Msg.SetRequest(cpReqMsg, &sReq);
    if (SOp.WriteN(cpReqMsg, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    return true;
}


clSubLocate::clSubLocate ()
{
    MPICCtrl.SetTag(LOCATE_CONTROL_TAG);
    MPICParam.SetTag(LOCATE_PARAM_TAG);
    MPICNormal.SetTag(LOCATE_NORMAL_TAG);
}


clSubLocate::~clSubLocate ()
{
}


int clSubLocate::Main (int iArgC, char **cpArgV)
{
    int iCtrlCmd;
    stDirRes2 sHdr;

#   ifdef USE_TRILLIUM
    if (bDebug) tprintf("clSubLocate::Main -> RecvParams\n");
#   endif
    if (!RecvParams()) return 1;
#   ifdef USE_TRILLIUM
    if (bDebug) tprintf("clSubLocate::Main -> Initialize\n");
#   endif
    if (!Initialize()) return 2;
#   ifdef USE_TRILLIUM
    if (bDebug) tprintf("clSubLocate::Main -> ConnectDir\n");
#   endif
    if (!ConnectDir()) return 3;
    SOp.SetRecvBufSize(iDirMsgSize * 2);
    while (!MPICCtrl.ProbeNB(0))
    {
#       ifdef USE_TRILLIUM
        if (bDebug) tprintf("clSubLocate::Main ... receiving ...\n");
#       endif
        if (SOp.ReadN(DirMsg, iDirMsgSize) < iDirMsgSize) return 4;
        Msg.GetResult(DirMsg, &sHdr, fpLevRes, fpDirRes);
#       ifdef USE_TRILLIUM
        if (bDebug) tprintf("clSubLocate::Main ... working ...\n");
#       endif
        LocSens.SetDirectionValues(fpLevRes, fpDirRes, sHdr.lResultCount,
            sHdr.lMinBin, sHdr.lMaxBin, sHdr.fFreqResolution);
#       ifdef USE_TRILLIUM
        if (bDebug) tprintf("clSubLocate::Main ... sending ...\n");
#       endif
        if (!MPICNormal.Send(0, LocSens.GetResultMatrix(), lWidth * lHeight))
            return 5;
    }
    MPICCtrl.Recv(0, &iCtrlCmd, 1);
#   ifdef USE_TRILLIUM
    if (bDebug) tprintf("clSubLocate::Main -> OK, I'm out of here (%i)\n", 
        iCtrlCmd);
#   endif
    /*iCtrlCmd = LOCATE_CTRL_ACK;
    MPICCtrl.Send(0, &iCtrlCmd, 1);*/
    return 0;
}

