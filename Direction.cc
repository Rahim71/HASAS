/*

    Direction calculation
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


#include <pthread.h>
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

#include "Direction.hh"


static bool bDebug = false;
static bool bDaemon = false;
static const char *cpaArrayTypes[] = { "Dipole", "Triangle", "Line",
    "Plane", "Cylinder", "Sphere" };
static clDirection *Dir;


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
    if (bDebug) printf("direction: Started\n");
    Dir = new clDirection(3, 4);
    iRetVal = Dir->Exec();
    if (bDebug) printf("direction: Exit = %i\n", iRetVal);
    return iRetVal;
}


void *ThrReceiveData (void *vpParam)
{
    return Dir->ReceiveData(vpParam);
}


void *ThrProcessData (void *vpParam)
{
    return Dir->ProcessData(vpParam);
}


void *ThrSendResults (void *vpParam)
{
    return Dir->SendResults(vpParam);
}


bool clDirection::GetRequest ()
{
    char cpMsgBuf[GLOBAL_HEADER_LEN];

    if (!SOpRequest->ReadSelect(DIR_REQ_TIMEOUT))
    {
        if (bDebug) printf("direction: clDirection::GetRequest() timeout\n");
        return false;
    }
    if (SOpRequest->ReadN(cpMsgBuf, GLOBAL_HEADER_LEN) != GLOBAL_HEADER_LEN)
    {
        if (bDebug) printf("direction: clSockOp::ReadN errno %i\n",
            SOpRequest->GetErrno());
        return false;
    }
    DirMsg.GetRequest(cpMsgBuf, &sDirRequest);
    return true;
}


bool clDirection::GetFirstMsg ()
{
    stRawDataReq sDataReq;

    if (!SOpData->ReadSelect(DIR_RAW1ST_TIMEOUT))
    {
        if (bDebug) printf("direction: clDirection::GetFirstMsg() timeout\n");
        return false;
    }
    if (SOpData->ReadN(&sDataFirst, sizeof(stRawDataFirst)) !=
        sizeof(stRawDataFirst))
    {
        if (bDebug) printf("direction: clSockOp::ReadN errno %i\n",
            SOpData->GetErrno());
        return false;
    }
    sDataReq.iChannel = -1;
    if (SOpData->WriteN(&sDataReq, sizeof(sDataReq)) !=
        sizeof(sDataReq))
    {
        if (bDebug) printf("direction: clSockOp::WriteN errno %i\n",
            SOpData->GetErrno());
    }
    return true;
}


bool clDirection::InitArray ()
{
    int iProcCntr;
    double dTemp;
    
    if (CfgDirection->GetInt("ArrayType", &iArrayType))
    {
        if (bDebug) printf("direction: Array type %s\n",
            cpaArrayTypes[iArrayType]);
    }
    else
    {
        if (!bDaemon) printf("direction: Array type not specified\n");
        return false;
    }
    if (iArrayType != DIR_ARRAY_TYPE_DIPOLE &&
        iArrayType != DIR_ARRAY_TYPE_TRIANGLE)
    {
        CfgDirection->GetInt("ShadingType", &iShadingType);
        /*if (bDebug) printf("direction: Using %s shading\n",
            cpaShadingTypes[iShadingType]);*/
        CfgDirection->GetInt("HorizontalSensors", &iHorizSensors);
        if (bDebug) printf("Horizontal sensor count %i\n", iHorizSensors);
        if (CfgDirection->GetFltArray("HorizontalSpacings",
            fpHorizSpacing) < (iHorizSensors - 1))
        {
            if (!bDaemon) printf("direction: Not enough sensor spacings\n");
            return false;
        }
        if (iArrayType != DIR_ARRAY_TYPE_LINE)
        {
            CfgDirection->GetInt("VerticalSensors", &iVertSensors);
            if (bDebug) printf("Vertical sensor count %i\n", iVertSensors);
            if (CfgDirection->GetFltArray("VerticalSpacings",
                fpVertSpacing) < (iVertSensors - 1))
            {
                if (!bDaemon) 
                    printf("direction: Not enough sensor spacings\n");
                return false;
            }
        }
    }
    else
    {
        if (!CfgDirection->GetFlt("SensorSpacing", &dTemp))
        {
            if (!bDaemon) printf("direction: Sensor spacing not specified\n");
            return false;
        }
        fSensorSpacing = (GDT) dTemp;
    }
    switch (iArrayType)
    {
        case DIR_ARRAY_TYPE_DIPOLE:
            iRawBufSize = lWindowSize * sizeof(GDT) * sDataFirst.iChannels;
            ArDipole.Initialize(fSensorSpacing, sDirRequest.fSoundSpeed,
                (int) (sDataFirst.dSampleRate + 0.5), lWindowSize, 
                sDirRequest.fLowFreqLimit, bDebug);
            if ((sDirRequest.iAlgorithm & MSG_DIR_ALG_BEAM) ==
                MSG_DIR_ALG_BEAM)
            {
                if (bDebug) printf("direction: Creating beamformers\n");
                for (iProcCntr = 0; iProcCntr < iProcessorCount; iProcCntr++)
                {
                    BeDipole[iProcCntr] = new clBeamDipole(&ArDipole, 
                        sDirRequest.fIntegrationTime, 
                        (int) (sDataFirst.dSampleRate + 0.5),
                        lWindowSize, bDebug);
                }
                sDirResHdr.fIntegrationTime = 
                    BeDipole[0]->GetIntegrationTime();
            }
            if ((sDirRequest.iAlgorithm & MSG_DIR_ALG_CORR) ==
                MSG_DIR_ALG_CORR)
            {
                if (bDebug) printf("direction: Creating correlators\n");
                for (iProcCntr = 0; iProcCntr < iProcessorCount; iProcCntr++)
                {
                    CoDipole[iProcCntr] = new clCorrDipole(&ArDipole,
                        sDirRequest.fIntegrationTime, 
                        (int) (sDataFirst.dSampleRate + 0.5),
                        lWindowSize, sDirRequest.bDisableFilter, bDebug);
                }
                sDirResHdr.fIntegrationTime = 
                    CoDipole[0]->GetIntegrationTime();
            }
            sDirResHdr.fHighFreqLimit = ArDipole.GetArrayFreq();
            sDirResHdr.b3DArray = false;
            break;
        case DIR_ARRAY_TYPE_TRIANGLE:
        case DIR_ARRAY_TYPE_LINE:
        case DIR_ARRAY_TYPE_PLANE:
        case DIR_ARRAY_TYPE_CYLINDER:
        case DIR_ARRAY_TYPE_SPHERE:
            if (!bDaemon) 
                printf("direction: This array type is not supported yet\n");
            return false;
            break;
        default:
            if (!bDaemon) printf("direction: Unknown array type!\n");
            return false;
    }
    sDirResHdr.lSectorCount = sDirRequest.lSectorCount;
    return true;
}


void clDirection::Stop ()
{
    MtxClassData.Wait();
    bRun = false;
    MtxClassData.Release();
}


inline void clDirection::Scale (GDT *fpValue)
{
    GDT fLogMult;

    switch (sDirRequest.iScaling)
    {
        case MSG_DIR_SCAL_LOG:
            fLogMult = (GDT) (1.0 / log10(2.0));
            *fpValue = (GDT) log10(*fpValue + 1.0) * fLogMult;
            break;
        case MSG_DIR_SCAL_EXP:
            *fpValue = (GDT) pow(*fpValue, sDirRequest.fScalingExp);
            break;
        case MSG_DIR_SCAL_SIN:
            // asin(1) = M_PI_2
            *fpValue = (GDT) sin(*fpValue * asin(1));
            break;
        case MSG_DIR_SCAL_LIN:
        default:
            // Nothing
            break;
    }
}


void clDirection::CalcDipoleBeams (int iThreadIdx, long lStartBeam, 
    long lStopBeam, bool bMultiply)
{
    int iDebug = 0;
    long lBeamCntr;
    GDT fBeamStep;
    GDT fBeamDir;
    GDT fTempRes;
    GDT *fpDirResData = DirResData;

    fBeamStep = (sDirRequest.fRightDir - sDirRequest.fLeftDir) / 
        (GDT) sDirRequest.lSectorCount;
    for (lBeamCntr = lStartBeam; lBeamCntr < lStopBeam; lBeamCntr++)
    {
        fBeamDir = fBeamStep * (GDT) lBeamCntr + sDirRequest.fLeftDir;
        fTempRes = BeDipole[iThreadIdx]->Process(fBeamDir);
        Scale(&fTempRes);
        MtxClassData.Wait();
        if (!bMultiply)
            fpDirResData[lBeamCntr] = fTempRes;
        else
            fpDirResData[lBeamCntr] *= fTempRes;
        MtxClassData.Release();
        iDebug++;
    }
    BeDipole[iThreadIdx]->SetHistory();
}


void clDirection::CalcDipoleCorrs (int iThreadIdx, long lStartSect,
    long lStopSect, bool bMultiply)
{
    long lSectCntr;
    GDT fSectStep;
    GDT fSectDir;
    GDT fTempRes;
    GDT *fpDirResData = DirResData;

    fSectStep = (sDirRequest.fRightDir - sDirRequest.fLeftDir) /
        (GDT) sDirRequest.lSectorCount;
    for (lSectCntr = lStartSect; lSectCntr < lStopSect; lSectCntr++)
    {
        fSectDir = fSectStep * (GDT) lSectCntr + sDirRequest.fLeftDir;
        fTempRes = CoDipole[iThreadIdx]->Process(fSectDir);
        fTempRes = (fTempRes + (GDT) 1.0) * (GDT) 0.5;
        Scale(&fTempRes);
        MtxClassData.Wait();
        if (!bMultiply)
            fpDirResData[lSectCntr] = fTempRes;
        else
            fpDirResData[lSectCntr] *= fTempRes;
        MtxClassData.Release();
    }
    CoDipole[iThreadIdx]->SetHistory();
}


void clDirection::StartTiming ()
{
    struct timeval sTimingReal;

    ctTiming = clock();
    gettimeofday(&sTimingReal, NULL);
    dTimingReal = (double) sTimingReal.tv_sec + (double) sTimingReal.tv_usec / 
        1000000.0;
}


void clDirection::StopTiming ()
{
    clock_t ctDiff;
    double dDiffTime;
    double dRealEnd;
    double dRealDiff;
    struct timeval sTimingReal;

    ctDiff = clock() - ctTiming;
    gettimeofday(&sTimingReal, NULL);
    dDiffTime = (double) ctDiff / (double) CLOCKS_PER_SEC;
    dRealEnd = (double) sTimingReal.tv_sec + (double) sTimingReal.tv_usec / 
        1000000.0;
    dRealDiff = dRealEnd - dTimingReal;
    if (dDiffTime > sDirResHdr.fIntegrationTime)
    {
        if (bDebug) 
            printf("direction: Underpowered CPU for this task (%f / %f)\n",
                sDirResHdr.fIntegrationTime, dDiffTime);
    }
    else if (dRealDiff > sDirResHdr.fIntegrationTime)
    {
        if (bDebug) 
            printf("direction: CPU load too high for this task (%f / %f)\n",
                sDirResHdr.fIntegrationTime, dRealDiff);
    }
}


clDirection::clDirection (int iInHandle, int iOutHandle)
{
    int iProcCntr;

    bRun = true;
    bConnected = true;
    iProcessorCount = 1;
    iStartChannel = 0;
    iResultsRefCount = 0;
    lWindowSize = DIR_DEF_WIN_SIZE;
    SOpRequest = new clSockOp(iInHandle);
    SOpResult = new clSockOp(iOutHandle);
    SOpData = NULL;
    CfgStreamDist = new clCfgFile(SD_CFGFILE);
    CfgDirection = new clCfgFile(DIR_CFGFILE);
    for (iProcCntr = 0; iProcCntr < DIR_MAX_CPUS; iProcCntr++)
    {
        BeDipole[iProcCntr] = NULL;
        CoDipole[iProcCntr] = NULL;
    }
}


clDirection::~clDirection ()
{
    int iProcCntr;

    for (iProcCntr = 0; iProcCntr < iProcessorCount; iProcCntr++)
    {
        if (BeDipole[iProcCntr] != NULL) delete BeDipole[iProcCntr];
        if (CoDipole[iProcCntr] != NULL) delete CoDipole[iProcCntr];
    }
    delete CfgStreamDist;
    delete CfgDirection;
    delete SOpRequest;
    if (SOpData != NULL) delete SOpData;
    delete SOpResult;
}


int clDirection::Exec ()
{
    int iDataH;
    int iProcCntr;
    int iTempWinSize;
    char cpIDSockName[_POSIX_PATH_MAX + 1];

    if (CfgDirection->GetInt("ProcessorCount", &iProcessorCount))
    {
        if (bDebug) printf("direction: Using %i CPUs\n", iProcessorCount);
    }
    if (CfgDirection->GetInt("StartChannel", &iStartChannel))
    {
        if (bDebug) printf("direction: Data starts at channel %i\n",
            iStartChannel);
    }
    if (bDebug) printf("direction: Get request message\n");
    if (!GetRequest()) return 1;
    if (CfgStreamDist->GetStr("LocalSocket", cpIDSockName))
    {
        iDataH = SClient.Connect(cpIDSockName);
        if (iDataH >= 0)
        {
            SOpData = new clSockOp(iDataH);
            if (bDebug) printf("direction: Get first data message\n");
            if (!GetFirstMsg()) return 1;
            if (CfgDirection->GetInt("WindowSize", &iTempWinSize))
                lWindowSize = iTempWinSize;
            if (bDebug) printf("direction: Initializing array\n");
            if (!InitArray()) return 1;
            if (bDebug)
            {
                printf("direction: Background noise estimation and removal: ");
                switch (sDirRequest.iRemoveNoise)
                {
                    case MSG_DIR_BNER_TPSW:
                        printf("Two-Pass Split-Window\n");
                        break;
                    case MSG_DIR_BNER_OTA:
                        printf("Order-Truncate-Average\n");
                        break;
                    default:
                        printf("None\n");
                        break;
                }
            }
            RawData.Size(iRawBufSize);
            DirResData = (sDirRequest.lSectorCount * sizeof(GDT));
            if (bDebug) printf("direction: Starting threads\n");
            pthread_create(&tidReceiveData, NULL, ThrReceiveData, NULL);
            MtxClassData.Wait();
            for (iProcCntr = 0; iProcCntr < iProcessorCount; iProcCntr++)
            {
                pthread_create(&tidpProcessData[iProcCntr], NULL,
                    ThrProcessData, NULL);
            }
            MtxClassData.Release();
            pthread_create(&tidSendResults, NULL, ThrSendResults, NULL);
            if (bDebug) printf("direction: Running...\n");
            pthread_join(tidSendResults, NULL);
            for (iProcCntr = 0; iProcCntr < iProcessorCount; iProcCntr++)
            {
                pthread_join(tidpProcessData[iProcCntr], NULL);
            }
            pthread_join(tidReceiveData, NULL);
            if (bDebug) printf("direction: Threads ended\n");
        }
        else
        {
            if (!bDaemon) printf("clSockClie::Connect() failed\n");
            return 1;
        }
    }
    else
    {
        if (!bDaemon) printf("clCfgFile::GetStr() failed\n");
        return 1;
    }
    return 0;
}


void *clDirection::ReceiveData (void *vpParam)
{
    bool bLocalRun = true;
    int iDataCount = iRawBufSize;
    sigset_t sigsetThis;

    sigemptyset(&sigsetThis);
    sigaddset(&sigsetThis, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &sigsetThis, NULL);
    if (bDebug) printf("direction: ReceiveData thread running\n");
    while (bLocalRun)
    {
        if (SOpData->ReadSelect(DIR_TIMEOUT))
        {
            if (SOpData->ReadN(RawData, iDataCount) != iDataCount)
            {
                if (bDebug) printf("direction: SockOp::ReadN errno %i (%s~%i)\n",
                    SOpData->GetErrno(), __FILE__, __LINE__);
                Stop();
                pthread_exit(NULL);
            }
            switch (iArrayType)
            {
                case DIR_ARRAY_TYPE_DIPOLE:
                    MtxDataReady.Wait();
                    ArDipole.AddData(RawData, iStartChannel, 
                        sDataFirst.iChannels);
                    CndDataReady.NotifyAll();
                    MtxDataReady.Release();
                    break;
                case DIR_ARRAY_TYPE_TRIANGLE:
                case DIR_ARRAY_TYPE_LINE:
                case DIR_ARRAY_TYPE_PLANE:
                case DIR_ARRAY_TYPE_CYLINDER:
                case DIR_ARRAY_TYPE_SPHERE:
                    break;
            }
        }
        MtxClassData.Wait();
        bLocalRun = bRun;
        MtxClassData.Release();
    }
    if (bDebug) printf("direction: ReceiveData thread ending\n");
    return NULL;
}


void *clDirection::ProcessData (void *vpParam)
{
    bool bLocalRun = true;
    int iProcCntr;
    int iThreadIdx = 0;
    int iAddDataRes = 0;
    int iResultsReady = 0;
    long lStartSect;
    long lStopSect;
    pthread_t tidThis;
    sigset_t sigsetThis;

    tidThis = pthread_self();
    sigemptyset(&sigsetThis);
    sigaddset(&sigsetThis, SIGFPE);
    pthread_sigmask(SIG_BLOCK, &sigsetThis, NULL);
    MtxClassData.Wait();
    for (iProcCntr = 0; iProcCntr < iProcessorCount; iProcCntr++)
    {
        if (tidpProcessData[iProcCntr] == tidThis)
        {
            iThreadIdx = iProcCntr;
            break;
        }
    }
    MtxClassData.Release();
    lStartSect = sDirRequest.lSectorCount / iProcessorCount * iThreadIdx;
    lStopSect = sDirRequest.lSectorCount / iProcessorCount * (iThreadIdx + 1);
    if (bDebug) 
        printf("direction: ProcessData[%i] thread running (%li - %li)\n",
            iThreadIdx, lStartSect, lStopSect);
    while (bLocalRun)
    {
        MtxDataReady.Wait();
        if (CndDataReady.Wait(MtxDataReady.GetPtr()))
        {
            switch (iArrayType)
            {
                case DIR_ARRAY_TYPE_DIPOLE:
                    if ((sDirRequest.iAlgorithm & MSG_DIR_ALG_BEAM) ==
                        MSG_DIR_ALG_BEAM)
                    {
                        if (BeDipole[iThreadIdx]->AddData())
                            iAddDataRes |= MSG_DIR_ALG_BEAM;
                    }
                    if ((sDirRequest.iAlgorithm & MSG_DIR_ALG_CORR) ==
                        MSG_DIR_ALG_CORR)
                    {
                        if (CoDipole[iThreadIdx]->AddData())
                            iAddDataRes |= MSG_DIR_ALG_CORR;
                    }
                    MtxDataReady.Release();
                    StartTiming();
                    if ((iAddDataRes & MSG_DIR_ALG_BEAM) == MSG_DIR_ALG_BEAM)
                    {
                        CalcDipoleBeams(iThreadIdx, lStartSect, lStopSect,
                            false);
                        iResultsReady |= MSG_DIR_ALG_BEAM;
                    }
                    if ((iAddDataRes & MSG_DIR_ALG_CORR) == MSG_DIR_ALG_CORR)
                    {
                        if ((iAddDataRes & MSG_DIR_ALG_BEAM) ==
                            MSG_DIR_ALG_BEAM)
                        {
                            CalcDipoleCorrs(iThreadIdx, lStartSect, lStopSect,
                                true);
                        }
                        else
                        {
                            CalcDipoleCorrs(iThreadIdx, lStartSect, lStopSect,
                                false);
                        }
                        iResultsReady |= MSG_DIR_ALG_CORR;
                    }
                    StopTiming();
                    if (iResultsReady == sDirRequest.iAlgorithm)
                    {
                        if (iThreadIdx == 0)
                        {
                            if ((iResultsReady & MSG_DIR_ALG_BEAM) ==
                                MSG_DIR_ALG_BEAM)
                            {
                                sDirResHdr.fPeakLevel =
                                    BeDipole[iThreadIdx]->GetPeakLevel();
                            }
                            else if ((iResultsReady & MSG_DIR_ALG_CORR) == 
                                MSG_DIR_ALG_CORR)
                            {
                                sDirResHdr.fPeakLevel =
                                    CoDipole[iThreadIdx]->GetPeakLevel();
                            }
                        }
                        MtxResultsReady.Wait();
                        iResultsRefCount++;
                        CndResultsReady.Notify();
                        MtxResultsReady.Release();
                        iAddDataRes = 0;
                        iResultsReady = 0;
                    }
                    break;
                case DIR_ARRAY_TYPE_TRIANGLE:
                case DIR_ARRAY_TYPE_LINE:
                case DIR_ARRAY_TYPE_PLANE:
                case DIR_ARRAY_TYPE_CYLINDER:
                case DIR_ARRAY_TYPE_SPHERE:
                    break;
            }
        }
        else
        {
            MtxDataReady.Release();
        }
        MtxClassData.Wait();
        bLocalRun = bRun;
        MtxClassData.Release();
    }
    if (bDebug) printf("direction: ProcessData[%i] thread ending\n",
        iThreadIdx);
    return NULL;
}


void *clDirection::SendResults (void *vpParam)
{
    bool bLocalRun = true;
    int iMsgSize;
    GDT fResMin;
    GDT fResMax;
    GDT *fpDirResData = DirResData;
    sigset_t sigsetThis;
    clAlloc MsgBuf;

    sigemptyset(&sigsetThis);
    sigaddset(&sigsetThis, SIGPIPE);
    sigaddset(&sigsetThis, SIGFPE);
    pthread_sigmask(SIG_BLOCK, &sigsetThis, NULL);
    iMsgSize = GLOBAL_HEADER_LEN + sDirRequest.lSectorCount * sizeof(GDT);
    MsgBuf.Size(iMsgSize);
    if (bDebug) printf("direction: SendResults thread running\n");
    while (bLocalRun)
    {
        MtxResultsReady.Wait();
        if (CndResultsReady.Wait(MtxResultsReady.GetPtr()))
        {
            if (iResultsRefCount == iProcessorCount)
            {
                MtxClassData.Wait();
                switch (sDirRequest.iRemoveNoise)
                {
                    case MSG_DIR_BNER_TPSW:
                        NoiseEstRem.TPSW(fpDirResData, 
                            (GDT) sDirRequest.fAlpha, sDirRequest.lMeanLength,
                            sDirRequest.lGapLength, sDirRequest.lSectorCount);
                        break;
                    case MSG_DIR_BNER_OTA:
                        NoiseEstRem.OTA(fpDirResData,
                            (GDT) sDirRequest.fAlpha, sDirRequest.lMeanLength,
                            sDirRequest.lSectorCount);
                        break;
                    default:
                        // None
                        break;
                }
                if (sDirRequest.bNormalize)
                {
                    DSP.MinMax(&fResMin, &fResMax, fpDirResData,
                        sDirRequest.lSectorCount);
                    DSP.Sub(fpDirResData, fResMin, sDirRequest.lSectorCount);
                    DSP.Mul(fpDirResData, (GDT) 1.0 / (fResMax - fResMin),
                        sDirRequest.lSectorCount);
                }
                gettimeofday(&sDirResHdr.sTimeStamp, NULL);
                DirMsg.SetResult(MsgBuf, &sDirResHdr, fpDirResData);
                MtxClassData.Release();
                iResultsRefCount = 0;
                MtxResultsReady.Release();
                if (SOpResult->WriteSelect(DIR_TIMEOUT))
                {
                    if (SOpResult->WriteN(MsgBuf, iMsgSize) < iMsgSize)
                    {
                        if (bDebug) 
                            printf("direction: clSockOp::WriteN errno %i\n",
                                SOpResult->GetErrno());
                        Stop();
                        pthread_exit(NULL);
                    }
                }
                else
                {
                    if (bDebug) 
                        printf("direction: clSockOp::WriteSelect() timeout\n");
                }
            }
            else
            {
                MtxResultsReady.Release();
            }
        }
        else
        {
            MtxResultsReady.Release();
        }
        MtxClassData.Wait();
        bLocalRun = bRun;
        MtxClassData.Release();
    }
    if (bDebug) printf("direction: SendResults thread ending\n");
    return NULL;
}

