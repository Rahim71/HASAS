/*

    Beamforming input server
    Copyright (C) 2002 Jussi Laako

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
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <vector>

#include <DynThreads.hh>

#include "BeamSrv.hh"


volatile bool bRun = true;
clDynThreads<clBeamSrvMaster> *BeamSrvMasterThreads;
BS_PROCINFOV_T vProcInfo;


void sig_handler_m (int signo)
{
    switch (signo)
    {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            bRun = false;
            break;
    }
}


void sig_handler_s (int signo)
{
    switch (signo)
    {
        case SIGHUP:
        case SIGTERM:
            //bRun = false;
            exit(0);
            break;
    }
}


int main (int argc, char *argv[])
{
    int iRetVal = 0;
    int iProcCntr;
    int iProcCount;
    int ipSockPair[2];
    pid_t pidFork;
    BS_PROCINFOV_T::iterator iterProcs;

    if (argc < 2)
    {
        fprintf(stderr, "process count not defined\n");
        return 1;
    }
    if (strcmp(argv[1], "--version") == 0)
    {
        fprintf(stderr, "%s v%i.%i.%i\n", argv[0], GLOBAL_VERSMAJ,
            GLOBAL_VERSMIN, GLOBAL_VERSPL);
        fprintf(stderr, "Copyright (C) 2002 Jussi Laako\n");
        return 0;
    }
    if (strcmp(argv[1], "--help") == 0)
    {
        fprintf(stderr, "%s [--version|--help] <process count>\n\n", argv[0]);
        fprintf(stderr, "--version      display version information\n");
        fprintf(stderr, "--help         display this help\n");
        return 0;
    }
    iProcCount = atoi(argv[1]);
    for (iProcCntr = 0; iProcCntr < iProcCount; iProcCntr++)
    {
        if (socketpair(AF_LOCAL, SOCK_STREAM, 0, ipSockPair) < 0)
        {
            perror("socketpair()");
            return 2;
        }
        pidFork = fork();
        if (pidFork == 0)
        {
            signal(SIGHUP, sig_handler_s);
            signal(SIGTERM, sig_handler_s);
            signal(SIGPIPE, SIG_IGN);
    
            close(ipSockPair[1]);

            clBeamSrvSlave *BeamSrvSlave;

            BeamSrvSlave = new clBeamSrvSlave(iProcCntr + 1, ipSockPair[0]);
            iRetVal = BeamSrvSlave->Main(&argc, &argv);
            delete BeamSrvSlave;
            return iRetVal;
        }
        else
        {
            close(ipSockPair[0]);

            stBeamProcInfo sProcInfo;
            
            sProcInfo.pidProc = pidFork;
            sProcInfo.iSockH = ipSockPair[1];
            vProcInfo.push_back(sProcInfo);
        }
    }

    signal(SIGINT, sig_handler_m);
    signal(SIGHUP, sig_handler_m);
    signal(SIGTERM, sig_handler_m);
    signal(SIGPIPE, SIG_IGN);

    clBeamSrvMaster *BeamSrvMaster;

    BeamSrvMaster = new clBeamSrvMaster;
    BeamSrvMasterThreads = 
        new clDynThreads<clBeamSrvMaster>(*BeamSrvMaster);
    iRetVal = BeamSrvMaster->Main(&argc, &argv);
    delete BeamSrvMaster;
    
    iterProcs = vProcInfo.begin();
    while (iterProcs != vProcInfo.end())
    {
        kill((*iterProcs).pidProc, SIGHUP);
        close((*iterProcs).iSockH);
        waitpid((*iterProcs).pidProc, NULL, 0);
        iterProcs++;
    }    
    
    return iRetVal;
}


// --- MASTER


bool clBeamSrvMaster::ReadConfig ()
{
    if (!Cfg.GetInt("Type", &sNodeParams.iType))
    {
        Log.Add('!', "Parameter \"Type\" is missing");
        return false;
    }
    if (!Cfg.GetInt("Sensors", &sNodeParams.iSensors))
    {
        Log.Add('!', "Parameter \"Sensors\" is missing");
        return false;
    }
    if (!Cfg.GetFlt("Spacing", &sNodeParams.fSpacing))
    {
        Log.Add('!', "Parameter \"Spacing\" is missing");
        return false;
    }
    if (!Cfg.GetFlt("SoundSpeed", &sNodeParams.fSoundSpeed))
    {
        Log.Add('!', "Parameter \"SoundSpeed\" is missing");
        return false;
    }
    if (!Cfg.GetInt("Decimate", &iDecimate))
    {
        Log.Add('!', "Parameter \"Decimate\" is missing");
        return false;
    }
    if (!Cfg.GetInt("WindowSize", &sNodeParams.iWindowSize))
    {
        Log.Add('!', "Parameter \"WindowSize\" is missing");
        return false;
    }
    if (!Cfg.GetInt("BlockSize", &sNodeParams.iBlockSize))
    {
        Log.Add('!', "Parameter \"BlockSize\" is missing");
        return false;
    }
    if (!Cfg.GetInt("Beams", &sNodeParams.iBeamCount))
    {
        Log.Add('!', "Parameter \"Beams\" is missing");
        return false;
    }

    return true;
}


bool clBeamSrvMaster::InitFilterBank ()
{
    int iTwosExp;
    int iFilterCntr;
    float fArrayFreq;

    if (!Cfg.GetInt("FilterType", &iFilterType))
    {
        Log.Add('!', "Parameter \"FilterType\" is missing");
        Abort();
    }

    fArrayFreq = sNodeParams.fSoundSpeed / sNodeParams.fSpacing / 2.0f;
    if (iDecimate)
    {
        iTwosExp = (int) 
            floor(log((float) sInHdr.dSampleRate / 2.0f / fArrayFreq) / 
            log(2.0));
        iDecFact = (int) pow(2.0, iTwosExp);
    }
    else
    {
        iDecFact = 1;
    }
    sprintf(cpLogBuf, "Array frequency %g Hz, decimation factor %i", 
        fArrayFreq, iDecFact);
    Log.Add(' ', cpLogBuf);

    if (iDecFact > 1)
    {
        fprintf(stderr, "beamsrv(master): Initializing filters...\n");
        FilterBank = new clRecDecimator[sNodeParams.iSensors];
        for (iFilterCntr = 0; iFilterCntr < sNodeParams.iSensors; iFilterCntr++)
        {
            if (!FilterBank[iFilterCntr].Initialize(iDecFact, 
                -sNodeParams.iWindowSize, (GDT *) NULL, (GDT) 0, iFilterType))
                return false;
        }
        fprintf(stderr, "beamsrv(master): Filter initialization complete\n");
    }

    sNodeParams.fSampleRate = (float) sInHdr.dSampleRate / (float) iDecFact;

    return true;
}


bool clBeamSrvMaster::InitProcessing ()
{
    if ((int) vProcInfo.size() < sNodeParams.iBeamCount)
    {
        Log.Add('!', "Insufficient number of processes available");
        return false;
    }
    
    if ((sInHdr.iChannels - iChOffset) < sNodeParams.iSensors)
    {
        Log.Add('!', "Misconfiguration (not enough channels)");
        return false;
    }

    return SendNodeParams();
}


void clBeamSrvMaster::ProcessLoop ()
{
    bool bInData;
    int iNodeCntr;
    int iInDataCount;
    int iNodeDataCount;
    int iNodeResCount;
    int iNodeResSize;
    clAlloc InData;
    clAlloc NodeData;
    clAlloc NodeRes;
    clAlloc LOutData;
    BS_PROCINFOV_T::iterator iterProcs;

    iInDataCount = sNodeParams.iBlockSize * sInHdr.iChannels;
    iNodeDataCount = sNodeParams.iBlockSize * sNodeParams.iSensors;
    iNodeResCount = sNodeParams.iBlockSize;
    iNodeResSize = iNodeResCount * sizeof(GDT);
    iOutDataCount = sNodeParams.iBlockSize * sNodeParams.iBeamCount;
    InData.Size(iInDataCount * sizeof(GDT));
    NodeData.Size(iNodeDataCount * sizeof(GDT));
    NodeRes.Size(iNodeResCount * sizeof(GDT));
    LOutData.Size(iOutDataCount * sizeof(GDT));
    OutData.Size(iOutDataCount * sizeof(GDT));

    fprintf(stderr, "beamsrv(master): Entering process loop...\n");
    while (bRun)
    {
        SemIn.Wait();
        MtxIn.Wait();
        bInData = InBuf.Get((GDT *) InData, iInDataCount);
        MtxIn.Release();
        if (!bInData) continue;

        if (!bRun) break;

        if (iDecFact <= 1)
        {
            CompactData(NodeData, InData, 
                sNodeParams.iSensors, sInHdr.iChannels,
                iChOffset, sNodeParams.iBlockSize);
        }
        else
        {
            if (!FilterData(NodeData, InData,
                sNodeParams.iSensors, sInHdr.iChannels,
                iChOffset, sNodeParams.iBlockSize))
                continue;
        }

        SendInData(NodeData, iNodeDataCount);

        iterProcs = vProcInfo.begin();
        iNodeCntr = 0;
        while (iterProcs != vProcInfo.end())
        {
#           ifndef LINUXSYS
            if (recv((*iterProcs).iSockH, NodeRes, iNodeResSize, 
                MSG_WAITALL) <= 0)
#           else
            if (recv((*iterProcs).iSockH, NodeRes, iNodeResSize, 
                MSG_WAITALL|MSG_NOSIGNAL) <= 0)
#           endif
            {
                sprintf(cpLogBuf, "recv() from process %u failed",
                    (*iterProcs).pidProc);
                Log.Add('!', cpLogBuf, errno);
            }
            DSP.Pack((GDT *) LOutData, (GDT *) NodeRes, iNodeCntr, 
                sNodeParams.iBeamCount, iNodeResCount);
            iterProcs++;
            iNodeCntr++;
        }

        MtxOut.Wait();
        OutData = LOutData;
        iBlockCntr++;
        CndOut.NotifyAll();
        MtxOut.Release();
    }
}


bool clBeamSrvMaster::SendNodeParams ()
{
    BS_PROCINFOV_T::iterator iterProcs;

    iterProcs = vProcInfo.begin();
    fprintf(stderr, "beamsrv(master): Sending node parameters...\n");
    while (iterProcs != vProcInfo.end())
    {
#       ifndef LINUXSYS
        if (send((*iterProcs).iSockH, &sNodeParams, sizeof(sNodeParams), 0) <
            (int) sizeof(sNodeParams))
#       else
        if (send((*iterProcs).iSockH, &sNodeParams, sizeof(sNodeParams), 
            MSG_NOSIGNAL) < (int) sizeof(sNodeParams))
#       endif
        {
            sprintf(cpLogBuf, "send() to process %u failed",
                (*iterProcs).pidProc);
            Log.Add('!', cpLogBuf, errno);
            return false;
        }
        iterProcs++;
    }
    fprintf(stderr, "beamsrv(master): Node parameters sent\n");
    return true;
}


bool clBeamSrvMaster::WaitReady ()
{
    int iProcess;
    BS_PROCINFOV_T::iterator iterProcs;

    iterProcs = vProcInfo.begin();
    fprintf(stderr, 
        "beamsrv(master): Waiting for ready messages... (may take a while)\n");
    while (iterProcs != vProcInfo.end())
    {
#       ifndef LINUXSYS
        if (recv((*iterProcs).iSockH, &iProcess, sizeof(int), MSG_WAITALL) <= 0)
#       else
        if (recv((*iterProcs).iSockH, &iProcess, sizeof(int), 
            MSG_WAITALL|MSG_NOSIGNAL) <= 0)
#       endif
        {
            sprintf(cpLogBuf, "recv() from process %u failed",
                (*iterProcs).pidProc);
            Log.Add('!', cpLogBuf, errno);
            return false;
        }
        fprintf(stderr, "beamsrv(master): slave%i (%u) ready\n",
            iProcess, (*iterProcs).pidProc);
        iterProcs++;
    }
    fprintf(stderr, "beamsrv(master): All nodes are ready\n");
    return true;
}


bool clBeamSrvMaster::SendInData (const GDT *fpData, int iDataCount)
{
    int iDataSize;
    BS_PROCINFOV_T::iterator iterProcs;

    iDataSize = iDataCount * sizeof(GDT);
    iterProcs = vProcInfo.begin();
    while (iterProcs != vProcInfo.end())
    {
#       ifndef LINUXSYS
        if (send((*iterProcs).iSockH, fpData, iDataSize, 0) < iDataSize)
#       else
        if (send((*iterProcs).iSockH, fpData, iDataSize, MSG_NOSIGNAL) < 
            iDataSize)
#       endif
        {
            sprintf(cpLogBuf, "send() to process %u failed",
                (*iterProcs).pidProc);
            Log.Add('!', cpLogBuf, errno);
            return false;
        }
        iterProcs++;
    }
    return true;
}


void clBeamSrvMaster::CompactData (GDT *fpDest, const GDT *fpSrc,
    long lDestChs, long lSrcChs, long lOffset, long lCount)
{
    long lSampleCntr;
    long lChCntr;
    long lSrcIdx;
    long lDestIdx;
    
    for (lSampleCntr = 0; lSampleCntr < lCount; lSampleCntr++)
    {
        lSrcIdx = lSampleCntr * lSrcChs + lOffset;
        lDestIdx = lSampleCntr * lDestChs;
        for (lChCntr = 0; lChCntr < lDestChs; lChCntr++)
        {
            fpDest[lDestIdx + lChCntr] = fpSrc[lSrcIdx + lChCntr];
        }
    }
}


bool clBeamSrvMaster::FilterData (GDT *fpDest, const GDT *fpSrc,
    long lDestChs, long lSrcChs, long lOffset, long lCount)
{
    long lOutData = 0;
    long lChCntr;

    FiltWork.Size(lCount * sizeof(GDT));

#   ifdef _OPENMP
#   pragma omp parallel
#   pragma omp for
#   endif
    for (lChCntr = 0; lChCntr < lDestChs; lChCntr++)
    {
        DSP.Extract((GDT *) FiltWork, fpSrc, lOffset + lChCntr, lSrcChs, 
            lCount * lSrcChs);
        /*DSP.Extract((GDT *) FiltWork, fpSrc, lOffset, lSrcChs, 
            lCount * lSrcChs);*/
        FilterBank[lChCntr].Put((GDT *) FiltWork, lCount);
        if (FilterBank[lChCntr].Get((GDT *) FiltWork, lCount))
        {
            DSP.Pack(fpDest, (GDT *) FiltWork, lChCntr, lDestChs, lCount);
#           ifdef _OPENMP
#           pragma omp atomic
#           endif
            lOutData++;
        }
    }
    return ((lOutData) ? true : false);
}


clBeamSrvMaster::clBeamSrvMaster ()
{
    Log.Open(BS_LOGFILE);
    Log.Add('*', "Starting");
    iBlockCntr = 0;
    FilterBank = NULL;
}


clBeamSrvMaster::~clBeamSrvMaster ()
{
    if (FilterBank != NULL)
        delete [] FilterBank;
    Log.Add('*', "Stopping");
}


int clBeamSrvMaster::Main (int *ipArgC, char ***cpppArgV)
{
    int iServerThreadH;
    int iInDataThreadH;
    int iPort;
    int iSockH;
    char cpServSock[_POSIX_PATH_MAX + 1];
    stRawDataReq sReq;
    clSockClie SClie;

    Cfg.SetFileName(BS_CFGFILE);

    if (!Cfg.GetInt("Port", &iPort))
    {
        Log.Add('!', "Parameter \"Port\" is missing");
        Abort();
    }
    if (!SServ.Bind((unsigned short) iPort))
    {
        Log.Add('!', "clSockServ::Bind() failed", SServ.GetErrno());
        Abort();
    }

    if (!Cfg.GetStr("StreamSource", cpServSock))
    {
        Log.Add('!', "Parameter \"StreamSource\" is missing");
        Abort();
    }
    iSockH = SClie.Connect(cpServSock);
    if (iSockH < 0)
    {
        Log.Add('!', "clSockClie::Connect() failed", SClie.GetErrno());
        Abort();
    }
    SOpIn.SetHandle(iSockH);
    sReq.iChannel = -1;
    if (SOpIn.ReadN(&sInHdr, sizeof(sInHdr)) <= 0)
    {
        Log.Add('!', "clSockOp::Readn() failed", SOpIn.GetErrno());
        Abort();
    }
    if (SOpIn.WriteN(&sReq, sizeof(sReq)) <= 0)
    {
        Log.Add('!', "clSockOp::WriteN() failed", SOpIn.GetErrno());
        Abort();
    }
    sprintf(cpLogBuf, "Receiving data for %i channels at fs %g",
        sInHdr.iChannels, sInHdr.dSampleRate);
    Log.Add(' ', cpLogBuf);

    if (!Cfg.GetInt("ChOffset", &iChOffset))
    {
        Log.Add('!', "Parameter \"ChOffset\" is missing");
        Abort();
    }

    if (!ReadConfig())
    {
        Log.Add('!', "Reading of configuration file failed");
        Abort();
    }

    if (!InitFilterBank())
    {
        Log.Add('!', "Filter bank initialization failed");
        Abort();
    }

    if (!InitProcessing())
    {
        Log.Add('!', "Initialization of processing system failed");
        Abort();
    }

    if (!WaitReady())
    {
        Log.Add('!', "All nodes didn't confirm ready state");
        Abort();
    }

    iServerThreadH = BeamSrvMasterThreads->Create(
        &clBeamSrvMaster::ServerThread, NULL, false);
    iInDataThreadH = BeamSrvMasterThreads->Create(
        &clBeamSrvMaster::InDataThread, NULL, false);

    ProcessLoop();
    
    Abort();
    BeamSrvMasterThreads->Wait(iInDataThreadH);
    BeamSrvMasterThreads->Wait(iServerThreadH);
    
    return 0;
}


void clBeamSrvMaster::Abort ()
{
    bRun = false;
}


void *clBeamSrvMaster::InDataThread (void *vpParam)
{
    int iInDataCount;
    clAlloc InData;

    BeamSrvMasterThreads->SetSched(BeamSrvMasterThreads->Self(),
        SCHED_FIFO, BS_INTHREAD_PRIORITY);

    iInDataCount = sNodeParams.iBlockSize * sInHdr.iChannels;
    InData.Size(iInDataCount * sizeof(GDT));
    while (bRun)
    {
        if (SOpIn.ReadN(InData, InData.GetSize()) <= 0)
        {
            Abort();
            break;
        }

        MtxIn.Wait();
        InBuf.Put((GDT *) InData, iInDataCount);
        MtxIn.Release();
        SemIn.Post();
    }
    
    return NULL;
}


void *clBeamSrvMaster::ServerThread (void *vpParam)
{
    int iSockH;

    while (bRun)
    {
        iSockH = SServ.WaitForConnect(BS_ACCEPT_TO);
        if (iSockH >= 0)
        {
            BeamSrvMasterThreads->Create(&clBeamSrvMaster::ServeClientThread,
                (void *) iSockH, true);
        }
    }

    return NULL;
}


void *clBeamSrvMaster::ServeClientThread (void *vpParam)
{
    int iSockH = (int) vpParam;
    int iMsgSize;
    int iLBlockCntr;
    stSoundStart sOutHdr;
    clAlloc OutMsg;
    clAlloc LOutData;
    clSockOp SOp;
    clSoundMsg Msg;
    
    BeamSrvMasterThreads->SetSched(BeamSrvMasterThreads->Self(),
        SCHED_FIFO, BS_OUTTHREAD_PRIORITY);

    Log.Add('+', "Client connected");
    SOp.SetHandle(iSockH);
    iMsgSize = iOutDataCount * sizeof(GDT);
    OutMsg.Size(GLOBAL_HEADER_LEN + iMsgSize);
    LOutData.Size(iOutDataCount * sizeof(GDT));

    sOutHdr.iChannels = sNodeParams.iBeamCount;
    sOutHdr.dSampleRate = sNodeParams.fSampleRate;
    sOutHdr.iFragmentSize = iOutDataCount;
    sOutHdr.iCompress = MSG_SOUND_COMPRESS_NONE;
    Msg.SetStart(OutMsg, &sOutHdr);
    if (SOp.WriteN(OutMsg, GLOBAL_HEADER_LEN) <= 0)
    {
        Log.Add('-', "Error sending header to client, disconnecting");
        return NULL;
    }

    if (!SOp.DisableNagle())
    {
        Log.Add('#', "Failed to disable TCP Nagle algorithm", SOp.GetErrno());
    }
    if (!SOp.SetTypeOfService(IPTOS_LOWDELAY))
    {
        Log.Add('#', "Unable to set type-of-service flag", SOp.GetErrno());
    }

    MtxOut.Wait();
    iLBlockCntr = iBlockCntr;
    MtxOut.Release();

    while (bRun)
    {
        MtxOut.Wait();
        CndOut.Wait(MtxOut.GetPtr());
        LOutData = OutData;
        iLBlockCntr++;
        if (iLBlockCntr != iBlockCntr)
        {
            printf("beamsrv: %i blocks lost\n",
                iBlockCntr - iLBlockCntr);
            iLBlockCntr = iBlockCntr;
        }
        MtxOut.Release();
        
        Msg.SetData(OutMsg, (GDT *) LOutData, iOutDataCount);
        if (SOp.WriteN(OutMsg, iMsgSize) <= 0)
            break;
    }

    Log.Add('-', "Client disconnected");
    return NULL;
}


// --- SLAVE


bool clBeamSrvSlave::RecvParams ()
{
#   ifndef LINUXSYS
    if (recv(iSockH, &sNodeParams, sizeof(sNodeParams), 
        MSG_WAITALL) < (int) sizeof(sNodeParams))
#   else
    if (recv(iSockH, &sNodeParams, sizeof(sNodeParams), 
        MSG_WAITALL|MSG_NOSIGNAL) < (int) sizeof(sNodeParams))
#   endif
        return false;
    return true;
}


bool clBeamSrvSlave::RecvInData (GDT *fpData, int iDataCount)
{
    int iDataSize;
    
    iDataSize = iDataCount * sizeof(GDT);
#   ifndef LINUXSYS
    if (recv(iSockH, fpData, iDataSize, MSG_WAITALL) < iDataSize)
#   else
    if (recv(iSockH, fpData, iDataSize, MSG_WAITALL|MSG_NOSIGNAL) < iDataSize)
#   endif
        return false;
    return true;
}


bool clBeamSrvSlave::SendReady ()
{
#   ifndef LINUXSYS
    if (send(iSockH, &iProcess, sizeof(int), 0) < (int) sizeof(int))
#   else
    if (send(iSockH, &iProcess, sizeof(int), MSG_NOSIGNAL) < (int) sizeof(int))
#   endif
        return false;
    return true;
}


bool clBeamSrvSlave::SendRes (const GDT *fpData, int iDataCount)
{
    int iDataSize;

    iDataSize = iDataCount * sizeof(GDT);
#   ifndef LINUXSYS
    if (send(iSockH, fpData, iDataSize, 0) < iDataSize)
#   else
    if (send(iSockH, fpData, iDataSize, MSG_NOSIGNAL) < iDataSize)
#   endif
        return false;
    return true;
}


clBeamSrvSlave::clBeamSrvSlave (int iProcessP, int iSockHP)
{
    iProcess = iProcessP;
    iSockH = iSockHP;
}


clBeamSrvSlave::~clBeamSrvSlave ()
{
    close(iSockH);
}


int clBeamSrvSlave::Main (int *ipArgC, char ***cpppArgV)
{
    int iInDataCount;
    int iOutDataCount;
    float fDir;
    clAlloc InData;
    clAlloc OutData;
    clDSPOp DSP;

    fprintf(stderr, "beamsrv(slave%i): Receiving node parameters...\n",
        iProcess);
    if (!RecvParams())
        return 1;

    fDir = acos(-1.0) / (sNodeParams.iBeamCount - 1) * (iProcess - 1) - 
        asin(1.0);
    fprintf(stderr, "beamsrv(slave%i): Beam direction %.1f\n",
        iProcess, 180.0 / acos(-1.0) * fDir);
    iInDataCount = sNodeParams.iSensors * sNodeParams.iBlockSize;
    iOutDataCount = sNodeParams.iBlockSize;
    InData.Size(iInDataCount * sizeof(GDT));
    OutData.Size(iOutDataCount * sizeof(GDT));

    fprintf(stderr, 
        "beamsrv(slave%i): Node parameters received, initializing beamformer...\n", 
        iProcess);

    switch (sNodeParams.iType)
    {
        case BS_ARRAY_TYPE_DIPOLE:
            if (!FBDipole.Initialize(sNodeParams.fSpacing,
                sNodeParams.iWindowSize, sNodeParams.fSampleRate))
                return 2;
            FBDipole.SetSoundSpeed(sNodeParams.fSoundSpeed);
            FBDipole.SetDirection(fDir, true);
            break;
        case BS_ARRAY_TYPE_TRIANGLE:
            break;
        case BS_ARRAY_TYPE_LINE:
            if (!FBLine.Initialize(sNodeParams.iSensors,
                sNodeParams.fSpacing, sNodeParams.iWindowSize,
                sNodeParams.fSampleRate))
                return 2;
            FBLine.SetSoundSpeed(sNodeParams.fSoundSpeed);
            FBLine.SetDirection(fDir, true);
            break;
        case BS_ARRAY_TYPE_PLANE:
            break;
        case BS_ARRAY_TYPE_CYLINDER:
            break;
        case BS_ARRAY_TYPE_SPHERE:
            break;
    }

    fprintf(stderr, 
        "beamsrv(slave%i): Beamformer initialized, entering process loop...\n", 
        iProcess);
    if (!SendReady())
        return 3;

    while (bRun)
    {
        if (!RecvInData(InData, iInDataCount))
            break;

        switch (sNodeParams.iType)
        {
            case BS_ARRAY_TYPE_DIPOLE:
                FBDipole.Put(InData, iInDataCount, 0, sNodeParams.iSensors);
                if (FBDipole.Get(OutData, iOutDataCount))
                {
                    if (!SendRes(OutData, iOutDataCount))
                        return 4;
                }
                else
                {
                    DSP.Zero((GDT *) OutData, iOutDataCount);
                    if (!SendRes(OutData, iOutDataCount))
                        return 4;
                }
                break;
            case BS_ARRAY_TYPE_TRIANGLE:
                break;
            case BS_ARRAY_TYPE_LINE:
                FBLine.Put(InData, iInDataCount, 0, sNodeParams.iSensors);
                if (FBLine.Get(OutData, iOutDataCount))
                {
                    if (!SendRes(OutData, iOutDataCount))
                        return 4;
                }
                else
                {
                    DSP.Zero((GDT *) OutData, iOutDataCount);
                    if (!SendRes(OutData, iOutDataCount))
                        return 4;
                }
                break;
            case BS_ARRAY_TYPE_PLANE:
                break;
            case BS_ARRAY_TYPE_CYLINDER:
                break;
            case BS_ARRAY_TYPE_SPHERE:
                break;
        }
    }

    return 0;
}
