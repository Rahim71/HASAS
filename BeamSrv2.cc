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
#include <signal.h>
#include <limits.h>
#include <math.h>
#include <float.h>

#include <mpi.h>

#include <DynThreads.hh>

#include "BeamSrv2.hh"


volatile bool bRun = true;
clDynThreads<clBeamSrv2Master> *BeamSrvMasterThreads;


void sig_handler (int signo)
{
    switch (signo)
    {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            //clBeamSrv2Master::Abort();
            bRun = false;
            break;
    }
}


int main (int argc, char *argv[])
{
    int iRetVal = 0;
    int iRank;

    if (argc > 1)
    {
        if (strcmp(argv[1], "--version") == 0)
        {
            fprintf(stderr, "%s v%i.%i.%i\n", argv[0], GLOBAL_VERSMAJ,
                GLOBAL_VERSMIN, GLOBAL_VERSPL);
            fprintf(stderr, "Copyright (C) 2002 Jussi Laako\n");
            return 0;
        }
        if (strcmp(argv[1], "--help") == 0)
        {
            fprintf(stderr, "%s [--version|--help]\n\n", argv[0]);
            fprintf(stderr, "--version      display version information\n");
            fprintf(stderr, "--help         display this help\n");
            return 0;
        }
    }

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    {
        fprintf(stderr, "MPI_Init() failed!\n");
        return 1;
    }
    if (MPI_Comm_rank(MPI_COMM_WORLD, &iRank) == MPI_SUCCESS)
    {
        if (!iRank)
        {
            signal(SIGINT, sig_handler);
            signal(SIGHUP, sig_handler);
            signal(SIGTERM, sig_handler);

            clBeamSrv2Master *BeamSrvMaster;

            BeamSrvMaster = new clBeamSrv2Master;
            BeamSrvMasterThreads = 
                new clDynThreads<clBeamSrv2Master>(*BeamSrvMaster);
            iRetVal = BeamSrvMaster->Main(&argc, &argv);
            delete BeamSrvMaster;
        }
        else
        {
            clBeamSrv2Slave *BeamSrvSlave;

            BeamSrvSlave = new clBeamSrv2Slave(iRank);
            iRetVal = BeamSrvSlave->Main(&argc, &argv);
            delete BeamSrvSlave;
        }
    }
    else fprintf(stderr, "MPI_Comm_rank() failed!\n");
    MPI_Finalize();
    
    return iRetVal;
}


// --- SHARED


bool BeamCommNodeParams (stBeamNodeParams &sNodeParams)
{
    if (MPI_Bcast(&sNodeParams.iType, 1, MPI_INT, 0, MPI_COMM_WORLD) !=
        MPI_SUCCESS) return false;
    if (MPI_Bcast(&sNodeParams.iSensors, 1, MPI_INT, 0, MPI_COMM_WORLD) !=
        MPI_SUCCESS) return false;
    if (MPI_Bcast(&sNodeParams.fSpacing, 1, MPI_FLOAT, 0, MPI_COMM_WORLD) !=
        MPI_SUCCESS) return false;
    if (MPI_Bcast(&sNodeParams.fSoundSpeed, 1, MPI_FLOAT, 0, MPI_COMM_WORLD) !=
        MPI_SUCCESS) return false;
    if (MPI_Bcast(&sNodeParams.iBeamCount, 1, MPI_INT, 0, MPI_COMM_WORLD) !=
        MPI_SUCCESS) return false;
    if (MPI_Bcast(&sNodeParams.iWindowSize, 1, MPI_INT, 0, MPI_COMM_WORLD) !=
        MPI_SUCCESS) return false;
    if (MPI_Bcast(&sNodeParams.iBlockSize, 1, MPI_INT, 0, MPI_COMM_WORLD) !=
        MPI_SUCCESS) return false;
    if (MPI_Bcast(&sNodeParams.fSampleRate, 1, MPI_FLOAT, 0, MPI_COMM_WORLD) !=
        MPI_SUCCESS) return false;
    return true;
}


bool BeamCommInData (GDT *fpData, int iDataCount)
{
    if (typeid(GDT) == typeid(float))
    {
        if (MPI_Bcast(fpData, iDataCount, MPI_FLOAT, 0, MPI_COMM_WORLD) !=
            MPI_SUCCESS) return false;
    }
    else if (typeid(GDT) == typeid(double))
    {
        if (MPI_Bcast(fpData, iDataCount, MPI_DOUBLE, 0, MPI_COMM_WORLD) !=
            MPI_SUCCESS) return false;
    }
    return true;
}


// --- MASTER


bool clBeamSrv2Master::ReadConfig ()
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


bool clBeamSrv2Master::InitFilterBank ()
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
        FilterBank = new clRecDecimator[sNodeParams.iSensors];
        for (iFilterCntr = 0; iFilterCntr < sNodeParams.iSensors; iFilterCntr++)
        {
            if (!FilterBank[iFilterCntr].Initialize(iDecFact, 
                -sNodeParams.iWindowSize, (GDT *) NULL, (GDT) 0, iFilterType))
                return false;
        }
    }

    sNodeParams.fSampleRate = (float) sInHdr.dSampleRate / (float) iDecFact;

    return true;
}


bool clBeamSrv2Master::InitProcessing ()
{
    int iNodeCount;

    MPI_Comm_size(MPI_COMM_WORLD, &iNodeCount);
    if (iNodeCount <= sNodeParams.iBeamCount)
    {
        Log.Add('!', "Insufficient number of nodes available");
        return false;
    }
    
    if ((sInHdr.iChannels - iChOffset) < sNodeParams.iSensors)
    {
        Log.Add('!', "Misconfiguration (not enough channels)");
        return false;
    }

    return BeamCommNodeParams(sNodeParams);
}


bool clBeamSrv2Master::WaitReady ()
{
    int iNodeCntr;
    int iNode;
    
    for (iNodeCntr = 0; iNodeCntr < sNodeParams.iBeamCount; iNodeCntr++)
    {
        if (MPI_Recv(&iNode, 1, MPI_INT, iNodeCntr + 1, BS_TAG_READY,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS)
        {
            sprintf(cpLogBuf, "Communication error with rank %i",
                iNodeCntr + 1);
            Log.Add('!', cpLogBuf);
            return false;
        }
    }
    return true;
}


void clBeamSrv2Master::ProcessLoop ()
{
    bool bInData;
    int iNodeCntr;
    int iInDataCount;
    int iNodeDataCount;
    int iNodeResCount;
    clAlloc InData;
    clAlloc NodeData;
    clAlloc NodeRes;
    clAlloc LOutData;
    clDSPOp DSP;

    iInDataCount = sNodeParams.iBlockSize * sInHdr.iChannels;
    iNodeDataCount = sNodeParams.iBlockSize * sNodeParams.iSensors;
    iNodeResCount = sNodeParams.iBlockSize;
    iOutDataCount = sNodeParams.iBlockSize * sNodeParams.iBeamCount;
    InData.Size(iInDataCount * sizeof(GDT));
    NodeData.Size(iNodeDataCount * sizeof(GDT));
    NodeRes.Size(iNodeResCount * sizeof(GDT));
    LOutData.Size(iOutDataCount * sizeof(GDT));
    OutData.Size(iOutDataCount * sizeof(GDT));
    while (bRun)
    {
        SemIn.Wait();
        MtxIn.Wait();
        bInData = InBuf.Get((GDT *) InData, iInDataCount);
        MtxIn.Release();
        if (!bInData) continue;

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

        BeamCommInData(NodeData, iNodeDataCount);

        for (iNodeCntr = 0; iNodeCntr < sNodeParams.iBeamCount; iNodeCntr++)
        {
            if (typeid(GDT) == typeid(float))
            {
                if (MPI_Recv(NodeRes, iNodeResCount, MPI_FLOAT, iNodeCntr + 1,
                    BS_TAG_RES, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != 
                    MPI_SUCCESS)
                {
                    sprintf(cpLogBuf, "Communication error with rank %i",
                        iNodeCntr + 1);
                    Log.Add('!', cpLogBuf);
                }
            }
            else if (typeid(GDT) == typeid(double))
            {
                if (MPI_Recv(NodeRes, iNodeResCount, MPI_DOUBLE, iNodeCntr + 1,
                    BS_TAG_RES, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != 
                    MPI_SUCCESS)
                {
                    sprintf(cpLogBuf, "Communication error with rank %i",
                        iNodeCntr + 1);
                    Log.Add('!', cpLogBuf);
                }
            }
            DSP.Pack((GDT *) LOutData, (GDT *) NodeRes, iNodeCntr, 
                sNodeParams.iBeamCount, iNodeResCount);
        }

        MtxOut.Wait();
        OutData = LOutData;
        CndOut.NotifyAll();
        MtxOut.Release();
    }
}


void clBeamSrv2Master::CompactData (GDT *fpDest, const GDT *fpSrc,
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


bool clBeamSrv2Master::FilterData (GDT *fpDest, const GDT *fpSrc,
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


clBeamSrv2Master::clBeamSrv2Master ()
{
    Log.Open(BS_LOGFILE);
    Log.Add('*', "Starting");
    FilterBank = NULL;
}


clBeamSrv2Master::~clBeamSrv2Master ()
{
    if (FilterBank != NULL)
        delete [] FilterBank;
    Log.Add('*', "Stopping");
}


int clBeamSrv2Master::Main (int *ipArgC, char ***cpppArgV)
{
    int iPort;
    int iSockH;
    int iServerThreadH;
    int iInDataThreadH;
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
        &clBeamSrv2Master::ServerThread, NULL, false);
    iInDataThreadH = BeamSrvMasterThreads->Create(
        &clBeamSrv2Master::InDataThread, NULL, false);

    ProcessLoop();
    
    Abort();
    BeamSrvMasterThreads->Wait(iInDataThreadH);
    BeamSrvMasterThreads->Wait(iServerThreadH);

    return 0;
}


void clBeamSrv2Master::Abort (int iErrorCode)
{
    MPI_Abort(MPI_COMM_WORLD, iErrorCode);
}


void *clBeamSrv2Master::InDataThread (void *vpParam)
{
    int iInDataCount;
    clAlloc InData;

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


void *clBeamSrv2Master::ServerThread (void *vpParam)
{
    int iSockH;

    while (bRun)
    {
        iSockH = SServ.WaitForConnect(BS_ACCEPT_TO);
        if (iSockH >= 0)
        {
            BeamSrvMasterThreads->Create(&clBeamSrv2Master::ServeClientThread,
                (void *) iSockH, true);
        }
    }

    return NULL;
}


void *clBeamSrv2Master::ServeClientThread (void *vpParam)
{
    int iSockH = (int) vpParam;
    int iMsgSize;
    stSoundStart sOutHdr;
    clAlloc OutMsg;
    clAlloc LOutData;
    clSockOp SOp;
    clSoundMsg Msg;
    
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

    while (bRun)
    {
        MtxOut.Wait();
        CndOut.Wait(MtxOut.GetPtr());
        LOutData = OutData;
        MtxOut.Release();
        
        Msg.SetData(OutMsg, (GDT *) LOutData, iOutDataCount);
        if (SOp.WriteN(OutMsg, iMsgSize) <= 0)
            break;
    }

    Log.Add('-', "Client disconnected");
    return NULL;
}


// --- SLAVE


bool clBeamSrv2Slave::SendReady ()
{
    if (MPI_Send(&iRank, 1, MPI_INT, 0, BS_TAG_READY, MPI_COMM_WORLD) !=
        MPI_SUCCESS)
        return false;
    return true;
}


bool clBeamSrv2Slave::SendRes (GDT *fpData, int iDataCount)
{
    if (typeid(GDT) == typeid(float))
    {
        if (MPI_Send(fpData, iDataCount, MPI_FLOAT, 0, BS_TAG_RES,
            MPI_COMM_WORLD) != MPI_SUCCESS)
            return false;
    }
    else if (typeid(GDT) == typeid(double))
    {
        if (MPI_Send(fpData, iDataCount, MPI_DOUBLE, 0, BS_TAG_RES,
            MPI_COMM_WORLD) != MPI_SUCCESS)
            return false;
    }
    return true;
}


clBeamSrv2Slave::clBeamSrv2Slave (int iRankP)
{
    iRank = iRankP;
}


clBeamSrv2Slave::~clBeamSrv2Slave ()
{
}


int clBeamSrv2Slave::Main (int *ipArgC, char ***cpppArgV)
{
    int iNodeCount;
    int iInDataCount;
    int iOutDataCount;
    float fDir;
    clAlloc InData;
    clAlloc OutData;
    clDSPOp DSP;

    MPI_Comm_size(MPI_COMM_WORLD, &iNodeCount);

    if (!BeamCommNodeParams(sNodeParams))
        return 1;

    fDir = acos(-1.0) / (sNodeParams.iBeamCount - 1) * (iRank - 1) - asin(1.0);
    iInDataCount = sNodeParams.iSensors * sNodeParams.iBlockSize;
    iOutDataCount = sNodeParams.iBlockSize;
    InData.Size(iInDataCount * sizeof(GDT));
    OutData.Size(iOutDataCount * sizeof(GDT));

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

    if (!SendReady())
        return 3;

    while (bRun)
    {
        if (!BeamCommInData(InData, iInDataCount))
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
