/*

    ComediServer
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


#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ComediSrv.hh"


static bool bDaemon = false;
clComediSrv ComediSrv;


int main (int argc, char *argv[])
{
    return ComediSrv.Main(argc, argv);
}


void SigHandler (int iSigNum)
{
    switch (iSigNum)
    {
        case SIGINT:
            ComediSrv.Log.Add('#', "Received SIGINT, terminating...");
            ComediSrv.Stop();
            break;
        case SIGHUP:
            ComediSrv.Log.Add('#', "Received SIGHUP, terminating...");
            ComediSrv.Stop();
            break;
        case SIGTERM:
            ComediSrv.Log.Add('#', "Received SIGTERM, terminating...");
            ComediSrv.Stop();
            break;
        default:
            ComediSrv.Log.Add('!', "Received unknown signal, terminating...");
            ComediSrv.Stop();
    }
}


void *WrapAudioInThread (void *vpParam)
{
    return ComediSrv.AudioInThread(vpParam);
}


void *WrapServeClientThread (void *vpParam)
{
    return ComediSrv.ServeClientThread(vpParam);
}


#ifdef USE_FLAC

FLAC__StreamEncoderWriteStatus WrapFLACWrite (
    const FLAC__StreamEncoder *spFLACEnc, const FLAC__byte cpBuffer[], 
    unsigned uiBytes, unsigned uiSamples, unsigned uiCurrFrame,
    void *vpDataPtr)
{
    clComediSrv *ComediSrvInst = (clComediSrv *) vpDataPtr;
    
    return ComediSrvInst->FLACWrite(spFLACEnc, cpBuffer, uiBytes, uiSamples,
        uiCurrFrame);
}


void WrapFLACMetaData (
    const FLAC__StreamEncoder *spFLACEnc, 
    const FLAC__StreamMetadata *spFLACMetaData,
    void *vpDataPtr)
{
    clComediSrv *ComediSrvInst = (clComediSrv *) vpDataPtr;

    ComediSrvInst->FLACMetaData(spFLACEnc, spFLACMetaData);
}

#endif


bool clComediSrv::GetAudioCfg (char *cpDevice, int *ipChannels,
    double *dpSampleRate, int *ipBits, double *dpRange, int *ipFragSize)
{
    if (!Cfg.GetStr("Device", cpDevice))
    {
        Log.Add('!', "\"Device\" not found from configuration file");
        return false;
    }
    if (!Cfg.GetInt("Channels", ipChannels))
    {
        Log.Add('!', "\"Channels\" not found from configuration file");
        return false;
    }
    if (!Cfg.GetFlt("SampleRate", dpSampleRate))
    {
        Log.Add('!', "\"SampleRate\" not found from configuration file");
        return false;
    }
    if (!Cfg.GetInt("Bits", ipBits))
    {
        Log.Add('!', "\"Bits\" not found from configuration file");
        return false;
    }
    if (!Cfg.GetFlt("Range", dpRange))
    {
        Log.Add('!', "\"Range\" not found from configuration file");
        return false;
    }
    if (!Cfg.GetInt("FragSize", ipFragSize))
    {
        Log.Add('!', "\"FragSize\" not found from configuration file");
        return false;
    }
    return true;
}


bool clComediSrv::InitCompress (int iChannels, int iSampleRate, int iBits,
    int iFragSize, int iCompress)
{
    if (iCompress == MSG_SOUND_COMPRESS_FLAC)
    {
        #ifdef USE_FLAC
        int iMaxLPCOrder;
        int iMinRiceOrder;
        int iMaxRiceOrder;
        FLAC__StreamEncoderState iEncState;

        if (!Cfg.GetInt("FLACMaxLPCOrder", &iMaxLPCOrder))
            iMaxLPCOrder = 0;
        if (iMaxLPCOrder > (int) FLAC__MAX_LPC_ORDER)
            iMaxLPCOrder = FLAC__MAX_LPC_ORDER;
        if (!Cfg.GetInt("FLACMinRiceOrder", &iMinRiceOrder))
            iMinRiceOrder = 0;
        if (!Cfg.GetInt("FLACMaxRiceOrder", &iMaxRiceOrder))
            iMaxRiceOrder = 0;
        if (iMaxRiceOrder > (int) FLAC__MAX_RICE_PARTITION_ORDER)
            iMaxRiceOrder = FLAC__MAX_RICE_PARTITION_ORDER;
        FLACFrame.Size(iFragSize * sizeof(FLAC__int32));

        spFLACEnc = FLAC__stream_encoder_new();
        if (spFLACEnc == NULL)
        {
            Log.Add('!', "FLAC constructor failed");
            return false;
        }
        FLAC__stream_encoder_set_streamable_subset(spFLACEnc,
            1);
        FLAC__stream_encoder_set_do_mid_side_stereo(spFLACEnc,
            0);
        FLAC__stream_encoder_set_loose_mid_side_stereo(spFLACEnc,
            0);
        FLAC__stream_encoder_set_channels(spFLACEnc,
            iChannels);
        FLAC__stream_encoder_set_bits_per_sample(spFLACEnc,
            iBits);
        FLAC__stream_encoder_set_sample_rate(spFLACEnc,
            iSampleRate);
        FLAC__stream_encoder_set_blocksize(spFLACEnc,
            iFragSize);
        FLAC__stream_encoder_set_max_lpc_order(spFLACEnc,
            iMaxLPCOrder);
        FLAC__stream_encoder_set_qlp_coeff_precision(spFLACEnc,
            0);
        FLAC__stream_encoder_set_do_qlp_coeff_prec_search(spFLACEnc,
            0);
        FLAC__stream_encoder_set_do_escape_coding(spFLACEnc,
            1);  // non-default
        FLAC__stream_encoder_set_do_exhaustive_model_search(spFLACEnc,
            1);  // non-default
        FLAC__stream_encoder_set_min_residual_partition_order(spFLACEnc,
            iMinRiceOrder);
        FLAC__stream_encoder_set_max_residual_partition_order(spFLACEnc,
            iMaxRiceOrder);
        FLAC__stream_encoder_set_rice_parameter_search_dist(spFLACEnc,
            0);
        FLAC__stream_encoder_set_total_samples_estimate(spFLACEnc,
            0);
        FLAC__stream_encoder_set_metadata(spFLACEnc, 
            NULL, 0);
        FLAC__stream_encoder_set_write_callback(spFLACEnc,
            WrapFLACWrite);
        FLAC__stream_encoder_set_metadata_callback(spFLACEnc,
            WrapFLACMetaData);
        FLAC__stream_encoder_set_client_data(spFLACEnc,
            (void *) this);

        iEncState = FLAC__stream_encoder_init(spFLACEnc);
        if (iEncState != FLAC__STREAM_ENCODER_OK)
        {
            Log.Add('!', FLAC__StreamEncoderStateString[iEncState]);
            return false;
        }
        #else
        return false;
        #endif
    }
    else return false;

    return true;
}


#ifdef USE_FLAC
void clComediSrv::Convert (FLAC__int32 *ipDest, const void *vpSrc,
    long lSamples, int iBits)
{
    long lSampleCntr;
    GDT fScale;
    GDT *fpSrc = (GDT *) vpSrc;
    
    fScale = (GDT) (pow(2.0, iBits - 1) - 1.0);
    for (lSampleCntr = 0; lSampleCntr < lSamples; lSampleCntr++)
    {
        ipDest[lSampleCntr] = (FLAC__int32) 
            (fpSrc[lSampleCntr] * fScale + (GDT) 0.5);
    }
}
#endif


clComediSrv::clComediSrv ()
{
    bRun = true;
    iAudioBufSize = 0;
    iBlockCntr = 0;
    #ifdef USE_FLAC
    spFLACEnc = NULL;
    #endif
    Log.Open(COM_LOGFILE);
    Log.Add('*', "Starting");
    Cfg.SetFileName(COM_CFGFILE);
}


clComediSrv::~clComediSrv ()
{
    #ifdef USE_FLAC
    if (spFLACEnc != NULL)
    {
        FLAC__stream_encoder_finish(spFLACEnc);
        FLAC__stream_encoder_delete(spFLACEnc);
    }
    #endif
    Log.Add('*', "Ending");
}


int clComediSrv::Main (int iArgC, char **cpArgV)
{
    int iPort;
    int iSockH;
    void *vpAudioInRes;
    pthread_t ptidAudioIn;
    pthread_t ptidServeClient;
    clSockServ SServ;

    signal(SIGINT, SigHandler);
    signal(SIGHUP, SigHandler);
    signal(SIGTERM, SigHandler);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    if (iArgC >= 2)
    {
        if (strcmp(cpArgV[1], "-D"))
        {
            bDaemon = true;
        }
        else if (strcmp(cpArgV[1], "--version"))
        {
            printf("%s v%i.%i.%i\n", cpArgV[0], GLOBAL_VERSMAJ, 
                GLOBAL_VERSMIN, GLOBAL_VERSPL);
            printf("Copyright (C) 2002 Jussi Laako\n");
            return 0;
        }
        else if (strcmp(cpArgV[1], "--help"))
        {
            printf("%s [-D|--version|--help]\n\n", cpArgV[0]);
            printf("-D         start as daemon\n");
            printf("--version  display version information\n");
            printf("--help     display this help\n");
            return 0;
        }
    }
    if (bDaemon)
    {
        if (fork() != 0)
        {
            exit(0);
        }
        setsid();
        freopen("/dev/null", "r+", stderr);
        freopen("/dev/null", "r+", stdin);
        freopen("/dev/null", "r+", stdout);
    }
    if (!Cfg.GetInt("Port", &iPort))
    {
        Log.Add('!', "\"Port\" not found from configuration file");
        return 1;
    }
    SServ.Bind(iPort);
    pthread_create(&ptidAudioIn, NULL, WrapAudioInThread, NULL);
    while (bRun)
    {
        if (access(COM_SHUTDOWNFILE, F_OK) == 0)
        {
            unlink(COM_SHUTDOWNFILE);
            Stop();
            break;
        }
        iSockH = SServ.WaitForConnect(COM_CONNECT_TIMEOUT);
        if (iSockH >= 0)
        {
            pthread_create(&ptidServeClient, NULL, WrapServeClientThread,
                (void *) iSockH);
            pthread_detach(ptidServeClient);
        }
    }
    pthread_join(ptidAudioIn, &vpAudioInRes);
    if ((int) vpAudioInRes != 0) return ((int) vpAudioInRes);
    return 0;
}


void *clComediSrv::AudioInThread (void *vpParam)
{
    int iChannels;
    int iBits;
    int iFragSize;
    int iSampleCount;
    int iCompress;
    int iComediVersion;
    int iDoubleClock;
    double dSampleRate;
    double dRange;
    char cpDevice[_POSIX_PATH_MAX + 1];
    char cpLogEntry[COM_LOGENTRY_SIZE];
    sigset_t sigsetThis;
    #ifndef BSDSYS
    uid_t uidCurrent;
    struct sched_param sSchedParam;
    #endif
    clAlloc RawBuf;
    clComediIO ComediIO;
    clDSPOp DSP;
    clSoundMsg Msg;

    sigemptyset(&sigsetThis);
    sigaddset(&sigsetThis, SIGPIPE);
    sigaddset(&sigsetThis, SIGINT);
    sigaddset(&sigsetThis, SIGHUP);
    sigaddset(&sigsetThis, SIGFPE);
    pthread_sigmask(SIG_BLOCK, &sigsetThis, NULL);

    GetAudioCfg(cpDevice, &iChannels, &dSampleRate, &iBits, &dRange, 
        &iFragSize);
    if (!Cfg.GetInt("Compress", &iCompress))
        iCompress = MSG_SOUND_COMPRESS_NONE;
    if (!Cfg.GetInt("DoubleClock", &iDoubleClock))
        iDoubleClock = 0;
    sprintf(cpLogEntry, "Request %s: ch %i fs %g wl %i r %g", cpDevice,
        iChannels, dSampleRate, iBits, dRange);
    Log.Add(' ', cpLogEntry);
    if (!ComediIO.Open(cpDevice))
    {
        sprintf(cpLogEntry, "Unable to open device: %s",
            ComediIO.GetErrorMsg());
        Log.Add('!', cpLogEntry);
        Stop();
        return ((void *) 2);
    }
    if (iFragSize > 0)
    {
        if (!ComediIO.PcmBufferSizeSet(iFragSize * 2))
        {
            sprintf(cpLogEntry, "Unable to set buffer size: %s",
                ComediIO.GetErrorMsg());
            Log.Add('#', cpLogEntry);
        }
    }
    else
    {
        iFragSize = ComediIO.PcmBufferSizeGet() / 2;
        if (iFragSize <= 0)
        {
            sprintf(cpLogEntry, 
                "Unable to get fragment size, making guess: %s",
                ComediIO.GetErrorMsg());
            Log.Add('!', cpLogEntry);
            iFragSize = COM_FRAG_SIZE_DEFAULT;
        }
    }
    sprintf(cpLogEntry, "Open %s: frag %i (%i) bytes",
        cpDevice, ComediIO.PcmBufferSizeGet() / 2, iFragSize);
    Log.Add(' ', cpLogEntry);
    iComediVersion = ComediIO.GetVersionCode();
    sprintf(cpLogEntry, "Comedi version %i.%i.%i",
        ((clComediIO::VERS_MASK_MAJ & iComediVersion) >> clComediIO::VERS_SHIFT_MAJ),
        ((clComediIO::VERS_MASK_MIN & iComediVersion) >> clComediIO::VERS_SHIFT_MIN),
        ((clComediIO::VERS_MASK_PL & iComediVersion) >> clComediIO::VERS_SHIFT_PL));
    Log.Add(' ', cpLogEntry);
    sprintf(cpLogEntry, "Driver %s, board %s",
        ComediIO.GetDriverName(),
        ComediIO.GetBoardName());
    Log.Add(' ', cpLogEntry);

    iSampleCount = iFragSize / ComediIO.PcmGetIntSampleSize();
    if (iCompress == MSG_SOUND_COMPRESS_FLAC)
    {
        #ifdef USE_FLAC
        iAudioBufSize = iSampleCount * sizeof(FLAC__int32);
        #else
        iAudioBufSize = 0;
        #endif
    }
    else
    {
        iAudioBufSize = iSampleCount * sizeof(GDT);
    }

    RawBuf.Size(iSampleCount * sizeof(GDT));
    RawBuf.Lock();
    AudioBuf.Size(iAudioBufSize);
    AudioBuf.Lock();

    if (iCompress)
    {
        if (!InitCompress(iChannels, (int) (dSampleRate + 0.5), iBits, 
            iSampleCount, iCompress))
            Stop();
    }

    #ifndef BSDSYS
    uidCurrent = getuid();
    setuid(0);
    sSchedParam.sched_priority = sched_get_priority_min(SCHED_FIFO) + 
        COM_INTHREAD_PRIORITY;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sSchedParam) != 0)
        Log.Add('#', "Warning: Unable to set scheduling parameters");
    setuid(uidCurrent);
    #endif

    if (!ComediIO.PcmInStart(&dSampleRate, iChannels, dRange, COM_USE_DITHER,
        (iDoubleClock) ? true : false))
    {
        sprintf(cpLogEntry, "Unable to start PCM subdevice: %s",
            ComediIO.GetErrorMsg());
        Log.Add('!', cpLogEntry);
        Stop();
    }
    sprintf(cpLogEntry, "Actual fs %g", dSampleRate);
    Log.Add(' ', cpLogEntry);
    sHdr.iChannels = iChannels;
    sHdr.dSampleRate = dSampleRate;
    sHdr.iFragmentSize = iSampleCount;
    sHdr.iCompress = iCompress;

    Log.Add(' ', "AudioIn thread running");
    while (bRun)
    {
        if (ComediIO.PcmInRead((GDT *) RawBuf, iSampleCount) < iSampleCount)
        {
            sprintf(cpLogEntry, "Error reading device: %s",
                ComediIO.GetErrorMsg());
            Log.Add('!', cpLogEntry);
            Stop();
            return ((void *) 2);
        }
        if (iCompress == MSG_SOUND_COMPRESS_FLAC)
        {
            #ifdef USE_FLAC
            Convert(FLACFrame, RawBuf, iSampleCount, iBits);
            if (!FLAC__stream_encoder_process_interleaved(spFLACEnc,
                FLACFrame, iSampleCount / iChannels))
            {
                Log.Add('!', "Compression error");
                Stop();
                return ((void *) 2);
            }
            #endif
        }
        else
        {
            #ifndef USE_RWLOCK
            MtxAudio.Wait();
            #else
            RWLAudio.WaitWrite();
            #endif
            Msg.SetData(AudioBuf, (GDT *) RawBuf, iSampleCount);
            iBlockCntr++;
            CndAudio.NotifyAll();
            #ifndef USE_RWLOCK
            MtxAudio.Release();
            #else
            RWLAudio.Release();
            #endif
        }
    }
    ComediIO.PcmInStop();
    ComediIO.PcmInClose();
    ComediIO.Close();
    Log.Add(' ', "AudioIn thread ending");
    return ((void *) 0);
}


void *clComediSrv::ServeClientThread (void *vpParam)
{
    bool bConnected = true;
    int iFragBufCount;
    int iLocalBlockCntr;
    char cpLogEntry[COM_LOGENTRY_SIZE];
    char cpHdrMsg[GLOBAL_HEADER_LEN];
    socklen_t iAddrLen;
    struct sockaddr_in sPeerAddr;
    sigset_t sigsetThis;
    #ifndef BSDSYS
    uid_t uidCurrent;
    struct sched_param sSchedParam;
    #endif
    clAlloc OutBuf(iAudioBufSize);
    clSoundMsg Msg;
    clSockOp SOp((int) vpParam);

    sigemptyset(&sigsetThis);
    sigaddset(&sigsetThis, SIGPIPE);
    sigaddset(&sigsetThis, SIGINT);
    sigaddset(&sigsetThis, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &sigsetThis, NULL);

    if (!Cfg.GetInt("BufferFrags", &iFragBufCount))
        iFragBufCount = COM_SOCKET_BUF_FRAGS;
    SOp.SetSendBufSize(iFragBufCount * iAudioBufSize);
    SOp.DisableNagle();
    SOp.SetTypeOfService(IPTOS_LOWDELAY);
    iAddrLen = sizeof(sPeerAddr);
    SOp.GetPeerName((struct sockaddr *) &sPeerAddr, &iAddrLen);
    sprintf(cpLogEntry, "Client connected from %s:%i", 
        inet_ntoa(sPeerAddr.sin_addr), ntohs(sPeerAddr.sin_port));
    Log.Add('+', cpLogEntry);
    Msg.SetStart(cpHdrMsg, &sHdr);
    if (SOp.WriteN(cpHdrMsg, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        bConnected = false;
    OutBuf.Lock();
    if (sHdr.iCompress)
    {
        if (SOp.WriteN(CompHead, CompHead.GetSize()) < CompHead.GetSize())
            bConnected = false;
    }

    #ifndef BSDSYS
    uidCurrent = getuid();
    setuid(0);
    sSchedParam.sched_priority = sched_get_priority_min(SCHED_FIFO) + 
        COM_OUTTHREAD_PRIORITY;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sSchedParam) != 0)
        Log.Add('#', "Warning: Unable to set scheduling parameters");
    setuid(uidCurrent);
    #endif

    iLocalBlockCntr = iBlockCntr;
    while (bRun && bConnected)
    {
        MtxAudio.Wait();
        CndAudio.Wait(MtxAudio.GetPtr());
        #ifdef USE_RWLOCK
        MtxAudio.Release();
        RWLAudio.WaitRead();
        #endif
        if (iAudioBufSize > OutBuf.GetSize())
        {
            OutBuf.UnLock();
            OutBuf.Size(iAudioBufSize);
            OutBuf.Lock();
        }
        memcpy(OutBuf, AudioBuf, iAudioBufSize);
        iLocalBlockCntr++;
        if (iLocalBlockCntr != iBlockCntr)
        {
            printf("comedisrv: %i blocks lost\n", 
                iBlockCntr - iLocalBlockCntr);
            iLocalBlockCntr = iBlockCntr;
        }
        #ifndef USE_RWLOCK
        MtxAudio.Release();
        #else
        RWLAudio.Release();
        #endif
        if (SOp.WriteN(OutBuf, iAudioBufSize) < iAudioBufSize)
            bConnected = false;
    }
    sprintf(cpLogEntry, "Client from %s:%i disconnected",
        inet_ntoa(sPeerAddr.sin_addr), ntohs(sPeerAddr.sin_port));
    Log.Add('-', cpLogEntry);
    return NULL;
}


#ifdef USE_FLAC

FLAC__StreamEncoderWriteStatus clComediSrv::FLACWrite (
    const FLAC__StreamEncoder *spFLACEnc, const FLAC__byte *cpBuffer, 
    unsigned uiBytes, unsigned uiSamples, unsigned uiCurrFrame)
{
    static bool bFirst = true;
    FLAC__StreamEncoderWriteStatus iEncStatus = 
        FLAC__STREAM_ENCODER_WRITE_STATUS_OK;

    #ifndef USE_RWLOCK
    MtxAudio.Wait();
    #else
    RWLAudio.WaitWrite();
    #endif
    if (bFirst)
    {
        CompHead.Size(uiBytes);
        memcpy(CompHead, cpBuffer, uiBytes);
        bFirst = false;
    }
    else
    {
        iAudioBufSize = uiBytes;
        if (iAudioBufSize > AudioBuf.GetSize())
        {
            AudioBuf.UnLock();
            AudioBuf.Size(iAudioBufSize);
            AudioBuf.Lock();
        }
        memcpy(AudioBuf, cpBuffer, iAudioBufSize);
        iBlockCntr++;
        CndAudio.NotifyAll();
    }
    #ifndef USE_RWLOCK
    MtxAudio.Release();
    #else
    RWLAudio.Release();
    #endif
    return iEncStatus;
}


void clComediSrv::FLACMetaData (
    const FLAC__StreamEncoder *spFLACEnc, 
    const FLAC__StreamMetadata *spFLACMetaData)
{
}

#endif
