/*

    SoundServer2
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

#include "SoundSrv2.hh"


static bool bDaemon = false;
clSoundSrv2 SoundSrv2;


int main (int argc, char *argv[])
{
    return SoundSrv2.Main(argc, argv);
}


void SigHandler (int iSigNum)
{
    switch (iSigNum)
    {
        case SIGINT:
            SoundSrv2.Log.Add('#', "Received SIGINT, terminating...");
            SoundSrv2.Stop();
            break;
        case SIGHUP:
            SoundSrv2.Log.Add('#', "Received SIGHUP, terminating...");
            SoundSrv2.Stop();
            break;
        case SIGTERM:
            SoundSrv2.Log.Add('#', "Received SIGTERM, terminating...");
            SoundSrv2.Stop();
            break;
        default:
            SoundSrv2.Log.Add('!', "Received unknown signal, terminating...");
            SoundSrv2.Stop();
    }
}


void *WrapAudioInThread (void *vpParam)
{
    return SoundSrv2.AudioInThread(vpParam);
}


void *WrapServeClientThread (void *vpParam)
{
    return SoundSrv2.ServeClientThread(vpParam);
}


#ifdef USE_FLAC

FLAC__StreamEncoderWriteStatus WrapFLACWrite (
    const FLAC__StreamEncoder *spFLACEnc, const FLAC__byte cpBuffer[], 
    unsigned uiBytes, unsigned uiSamples, unsigned uiCurrFrame,
    void *vpDataPtr)
{
    clSoundSrv2 *SoundSrv2Inst = (clSoundSrv2 *) vpDataPtr;
    
    return SoundSrv2Inst->FLACWrite(spFLACEnc, cpBuffer, uiBytes, uiSamples,
        uiCurrFrame);
}


void WrapFLACMetaData (
    const FLAC__StreamEncoder *spFLACEnc, 
    const FLAC__StreamMetadata *spFLACMetaData,
    void *vpDataPtr)
{
    clSoundSrv2 *SoundSrv2Inst = (clSoundSrv2 *) vpDataPtr;

    SoundSrv2Inst->FLACMetaData(spFLACEnc, spFLACMetaData);
}

#endif


bool clSoundSrv2::GetAudioCfg (char *cpDevice, int *ipChannels,
    int *ipSampleRate, int *ipBits, int *ipFragSize)
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
    if (!Cfg.GetInt("SampleRate", ipSampleRate))
    {
        Log.Add('!', "\"SampleRate\" not found from configuration file");
        return false;
    }
    if (!Cfg.GetInt("Bits", ipBits))
    {
        Log.Add('!', "\"Bits\" not found from configuration file");
        return false;
    }
    if (!Cfg.GetInt("FragSize", ipFragSize))
    {
        Log.Add('!', "\"FragSize\" not found from configuration file");
        return false;
    }
    return true;
}


bool clSoundSrv2::InitCompress (int iChannels, int iSampleRate, int iBits,
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
void clSoundSrv2::Convert (FLAC__int32 *ipDest, const void *vpSrc,
    long lSamples, int iBits)
{
    long lSampleCntr;
    
    if (iBits == 8)
    {
        unsigned char *ipSrc = (unsigned char *) vpSrc;
        
        for (lSampleCntr = 0; lSampleCntr < lSamples; lSampleCntr++)
        {
            ipDest[lSampleCntr] = (FLAC__int32) ipSrc[lSampleCntr] - 0x80;
        }
    }
    else if (iBits > 8 && iBits <= 16)
    {
        signed short *ipSrc = (signed short *) vpSrc;
        
        for (lSampleCntr = 0; lSampleCntr < lSamples; lSampleCntr++)
        {
            ipDest[lSampleCntr] = (FLAC__int32) ipSrc[lSampleCntr];
        }
    }
    else if (iBits > 16)
    {
        signed int *ipSrc = (signed int *) vpSrc;
        
        for (lSampleCntr = 0; lSampleCntr < lSamples; lSampleCntr++)
        {
            ipDest[lSampleCntr] = 
                ((FLAC__int32) ipSrc[lSampleCntr] >> (32 - iBits));
        }
    }
}
#endif


clSoundSrv2::clSoundSrv2 ()
{
    bRun = true;
    iAudioBufSize = 0;
    iBlockCntr = 0;
    #ifdef USE_FLAC
    spFLACEnc = NULL;
    #endif
    Log.Open(SS2_LOGFILE);
    Log.Add('*', "Starting");
    Cfg.SetFileName(SS2_CFGFILE);
}


clSoundSrv2::~clSoundSrv2 ()
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


int clSoundSrv2::Main (int iArgC, char **cpArgV)
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
            printf("Copyright (C) 2000-2002 Jussi Laako\n");
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
        if (access(SS2_SHUTDOWNFILE, F_OK) == 0)
        {
            unlink(SS2_SHUTDOWNFILE);
            Stop();
            break;
        }
        iSockH = SServ.WaitForConnect(SS2_CONNECT_TIMEOUT);
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


void *clSoundSrv2::AudioInThread (void *vpParam)
{
    int iFormat;
    int iSampleRate;
    int iChannels;
    int iBits;
    int iSampleSize;
    int iFragSize;
    int iSampleCount;
    int iCompress;
    int iOSSVersion;
    char cpDevice[_POSIX_PATH_MAX + 1];
    char cpLogEntry[SS2_LOGENTRY_SIZE];
    sigset_t sigsetThis;
    #ifndef BSDSYS
    uid_t uidCurrent;
    struct sched_param sSchedParam;
    #endif
    clAlloc RawBuf;
    clAlloc ConvBuf;
    clAudio Audio;
    clDSPOp DSP;
    clSoundMsg Msg;

    sigemptyset(&sigsetThis);
    sigaddset(&sigsetThis, SIGPIPE);
    sigaddset(&sigsetThis, SIGINT);
    sigaddset(&sigsetThis, SIGHUP);
    sigaddset(&sigsetThis, SIGFPE);
    pthread_sigmask(SIG_BLOCK, &sigsetThis, NULL);

    GetAudioCfg(cpDevice, &iChannels, &iSampleRate, &iBits, &iFragSize);
    if (!Cfg.GetInt("Compress", &iCompress))
        iCompress = MSG_SOUND_COMPRESS_NONE;
    switch (iBits)
    {
        case 8:
            iFormat = AFMT_U8;
            iSampleSize = 1;
            break;
        case 16:
            iFormat = AFMT_S16_NE;
            iSampleSize = 2;
            break;
        #ifndef USE_OSSLITE
        case 24:
        case 32:
            iFormat = AFMT_S32_NE;
            iSampleSize = 4;
            break;
        #endif
        default:
            iFormat = AFMT_S16_NE;
            iSampleSize = 2;
    }
    sprintf(cpLogEntry, "Request %s: ch %i fs %i fmt %xh", cpDevice,
        iChannels, iSampleRate, iFormat);
    Log.Add(' ', cpLogEntry);
    if (!Audio.Open(cpDevice, &iFormat, &iSampleRate, &iChannels, AUDIO_READ))
    {
        Log.Add('!', "Unable to open device");
        Stop();
        return ((void *) 2);
    }
    if (iFragSize > 0)
    {
        if (!Audio.SetFragment(iFragSize, 0x7fff))
        {
            Log.Add('#', "Warning: Unable to set fragment size");
        }
    }
    else
    {
        iFragSize = Audio.GetFragmentSize();
        if (iFragSize <= 0)
        {
            Log.Add('!', "Unable to get fragment size, making guess");
            iFragSize = SS2_FRAG_SIZE_DEFAULT;
        }
    }
    sprintf(cpLogEntry, "Open %s: ch %i fs %i fmt %xh frag %i (%i) bytes",
        cpDevice, iChannels, iSampleRate, iFormat, 
        Audio.GetFragmentSize(), iFragSize);
    Log.Add(' ', cpLogEntry);
    iOSSVersion = Audio.GetVersion();
    sprintf(cpLogEntry, "OSS version %i.%i.%i (compiled with %i.%i.%i)",
        SS2_OSS_MAJOR(iOSSVersion), SS2_OSS_MINOR(iOSSVersion),
        SS2_OSS_PL(iOSSVersion),
        SS2_OSS_MAJOR(SOUND_VERSION), SS2_OSS_MINOR(SOUND_VERSION),
        SS2_OSS_PL(SOUND_VERSION));
    Log.Add(' ', cpLogEntry);

    iSampleCount = iFragSize / iSampleSize;
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
    sHdr.iChannels = iChannels;
    sHdr.dSampleRate = iSampleRate;
    sHdr.iFragmentSize = iSampleCount;
    sHdr.iCompress = iCompress;

    RawBuf.Size(iFragSize);
    RawBuf.Lock();
    ConvBuf.Size(iAudioBufSize);
    ConvBuf.Lock();
    AudioBuf.Size(iAudioBufSize);
    AudioBuf.Lock();

    if (iCompress)
    {
        if (!InitCompress(iChannels, iSampleRate, iBits, iSampleCount, 
            iCompress))
            Stop();
    }

    #ifndef BSDSYS
    uidCurrent = getuid();
    setuid(0);
    sSchedParam.sched_priority = sched_get_priority_min(SCHED_FIFO) + 
        SS2_INTHREAD_PRIORITY;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sSchedParam) != 0)
        Log.Add('#', "Warning: Unable to set scheduling parameters");
    setuid(uidCurrent);
    #endif

    Log.Add(' ', "AudioIn thread running");
    while (bRun)
    {
        if (Audio.Read(RawBuf, iFragSize) < iFragSize)
        {
            Log.Add('!', "Error reading device");
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
            switch (iSampleSize)
            {
                case 1:
                    DSP.Convert((GDT *) ConvBuf, (unsigned char *) RawBuf, 
                        iSampleCount);
                    break;
                case 2:
                    DSP.Convert((GDT *) ConvBuf, (signed short *) RawBuf, 
                        iSampleCount, false);
                    break;
                case 4:
                    DSP.Convert((GDT *) ConvBuf, (signed int *) RawBuf, 
                        iSampleCount, true);
                    break;
                default:
                    sprintf(cpLogEntry, "Fatal error @%s~%i", 
                        __FILE__, __LINE__);
                    Log.Add('!', cpLogEntry);
                    Stop();
                    return ((void *) 2);
            }
            #ifndef USE_RWLOCK
            MtxAudio.Wait();
            #else
            RWLAudio.WaitWrite();
            #endif
            Msg.SetData(AudioBuf, (GDT *) ConvBuf, iSampleCount);
            iBlockCntr++;
            CndAudio.NotifyAll();
            #ifndef USE_RWLOCK
            MtxAudio.Release();
            #else
            RWLAudio.Release();
            #endif
        }
    }
    Log.Add(' ', "AudioIn thread ending");
    return ((void *) 0);
}


void *clSoundSrv2::ServeClientThread (void *vpParam)
{
    bool bConnected = true;
    int iFragBufCount;
    int iLocalBlockCntr;
    char cpLogEntry[SS2_LOGENTRY_SIZE];
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
        iFragBufCount = SS2_SOCKET_BUF_FRAGS;
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
        SS2_OUTTHREAD_PRIORITY;
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
            printf("soundsrv2: %i blocks lost\n", 
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

FLAC__StreamEncoderWriteStatus clSoundSrv2::FLACWrite (
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


void clSoundSrv2::FLACMetaData (
    const FLAC__StreamEncoder *spFLACEnc, 
    const FLAC__StreamMetadata *spFLACMetaData)
{
}

#endif
