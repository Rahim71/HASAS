/*

    Audio stream distributor
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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "StreamDist.hh"


static bool bDaemon;
clStreamDist StreamDist;


int main (int argc, char *argv[])
{
    return StreamDist.Main(argc, argv);
}


void SigHandler (int iSigNum)
{
    switch (iSigNum)
    {
        case SIGINT:
            StreamDist.Log.Add('#', "Received SIGINT, terminating...");
            StreamDist.Stop();
            break;
        case SIGHUP:
            StreamDist.Log.Add('#', "Received SIGHUP, terminating...");
            StreamDist.Stop();
            break;
        case SIGTERM:
            StreamDist.Log.Add('#', "Received SIGTERM, terminating...");
            StreamDist.Stop();
            break;
        default:
            StreamDist.Log.Add('!', 
                "Received unknown signal, terminating...");
            StreamDist.Stop();
    }
}


void *WrapAudioInThread (void *vpParam)
{
    return StreamDist.AudioInThread(vpParam);
}


void *WrapServeClientThread (void *vpParam)
{
    return StreamDist.ServeClientThread(vpParam);
}


#ifdef USE_FLAC

FLAC__StreamDecoderReadStatus WrapFLACRead (
    const FLAC__StreamDecoder *spFLACDec, 
    FLAC__byte cpBuffer[], unsigned *uipBytes, void *vpDataPtr)
{
    clStreamDist *StreamDistInst = (clStreamDist *) vpDataPtr;
    
    return StreamDistInst->FLACRead(spFLACDec, cpBuffer, uipBytes);
}


FLAC__StreamDecoderWriteStatus WrapFLACWrite (
    const FLAC__StreamDecoder *spFLACDec, 
    const FLAC__Frame *spFrame, const FLAC__int32 * const ippBuffer[], 
    void *vpDataPtr)
{
    clStreamDist *StreamDistInst = (clStreamDist *) vpDataPtr;

    return StreamDistInst->FLACWrite(spFLACDec, spFrame, ippBuffer);
}


void WrapFLACMetaData (
    const FLAC__StreamDecoder *spFLACDec, 
    const FLAC__StreamMetadata *spMetaData, void *vpDataPtr)
{
    clStreamDist *StreamDistInst = (clStreamDist *) vpDataPtr;

    StreamDistInst->FLACMetaData(spFLACDec, spMetaData);    
}


void WrapFLACError (
    const FLAC__StreamDecoder *spFLACDec, 
    FLAC__StreamDecoderErrorStatus iErrorStatus, void *vpDataPtr)
{
    clStreamDist *StreamDistInst = (clStreamDist *) vpDataPtr;

    StreamDistInst->FLACError(spFLACDec, iErrorStatus);    
}

#endif


inline void clStreamDist::CopyChannel (GDT *fpDest, const GDT *fpSrc, 
    int iChannel)
{
    int iSampleCntr;
    int iSampleCount;

    iSampleCount = iFragmentSize / sHdr.iChannels;
    for (iSampleCntr = 0; iSampleCntr < iSampleCount; iSampleCntr++)
    {
        fpDest[iSampleCntr] = fpSrc[iSampleCntr * sHdr.iChannels + iChannel];
    }
}


bool clStreamDist::InitCompress (int iCompress)
{
    if (iCompress == MSG_SOUND_COMPRESS_FLAC)
    {
        #ifdef USE_FLAC
        FLAC__StreamDecoderState iDecState;
        
        spFLACDec = FLAC__stream_decoder_new();
        if (spFLACDec == NULL)
        {
            Log.Add('!', "FLAC constructor failed");
            return false;
        }
        FLAC__stream_decoder_set_read_callback(spFLACDec,
            WrapFLACRead);
        FLAC__stream_decoder_set_write_callback(spFLACDec,
            WrapFLACWrite);
        FLAC__stream_decoder_set_metadata_callback(spFLACDec,
            WrapFLACMetaData);
        FLAC__stream_decoder_set_error_callback(spFLACDec,
            WrapFLACError);
        FLAC__stream_decoder_set_client_data(spFLACDec,
            (void *) this);
        
        iDecState = FLAC__stream_decoder_init(spFLACDec);
        if (iDecState != FLAC__STREAM_DECODER_SEARCH_FOR_METADATA)
        {
            Log.Add('!', FLAC__StreamDecoderStateString[iDecState]);
            return false;
        }
        #endif
    }
    else
    {
        return false;
    }
    
    return true;
}


clStreamDist::clStreamDist ()
{
    bRun = true;
    iAudioBufSize = 0;
    iBlockCntr = 0;
    #ifdef USE_FLAC
    spFLACDec = NULL;
    #endif
    Log.Open(SD_LOGFILE);
    Log.Add('*', "Starting");
    Cfg.SetFileName(SD_CFGFILE);
}


clStreamDist::~clStreamDist ()
{
    #ifdef USE_FLAC
    if (spFLACDec != NULL)
    {
        FLAC__stream_decoder_delete(spFLACDec);
    }
    #endif
    Log.Add('*', "Ending");
}


int clStreamDist::Main (int iArgC, char **cpArgV)
{
    int iSockH;
    void *vpAudioInRes;
    char cpSocket[_POSIX_PATH_MAX + 1];
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
            printf("Copyright (C) 2000-2001 Jussi Laako\n");
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
    if (!Cfg.GetStr("LocalSocket", cpSocket))
    {
        Log.Add('!', "\"LocalSocket\" not found from configuration file");
        return 1;
    }
    if (!SServ.Bind(cpSocket))
    {
        Log.Add('!', strerror(SServ.GetErrno()));
        return 2;
    }
    pthread_create(&ptidAudioIn, NULL, WrapAudioInThread, NULL);
    while (bRun)
    {
        if (access(SD_SHUTDOWNFILE, F_OK) == 0)
        {
            unlink(SD_SHUTDOWNFILE);
            Stop();
            break;
        }
        iSockH = SServ.WaitForConnect(SD_CONNECT_TIMEOUT);
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


void *clStreamDist::AudioInThread (void *vpParam)
{
    int iSockH;
    int iHostPort;
    char cpHostAddr[SD_HOSTADDR_MAX];
    char cpHdrMsg[GLOBAL_HEADER_LEN];
    char cpLogEntry[SD_LOGENTRY_SIZE];
    stSoundStart sInHdr;
    sigset_t sigsetThis;
    #ifndef BSDSYS
    uid_t uidCurrent;
    struct sched_param sSchedParam;
    #endif
    clAlloc InBuf;
    clSockClie SClie;
    clSockOp SOp;
    clSoundMsg Msg;

    sigemptyset(&sigsetThis);
    sigaddset(&sigsetThis, SIGPIPE);
    sigaddset(&sigsetThis, SIGINT);
    sigaddset(&sigsetThis, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &sigsetThis, NULL);

    if (!Cfg.GetStr("Server", cpHostAddr))
    {
        Log.Add('!', "\"Server\" not found from configuration file");
        return ((void *) 2);
    }
    if (!Cfg.GetInt("ServerPort", &iHostPort))
    {
        Log.Add('!', "\"ServerPort\" not found from configuration file");
        return ((void *) 2);
    }
    iSockH = SClie.Connect(cpHostAddr, NULL, iHostPort);
    if (iSockH < 0)
    {
        Log.Add('!', "Failed to connect to specified server", 
            SClie.GetErrno());
        return ((void *) 2);
    }
    SOp.SetHandle(iSockH);
    if (SOp.ReadN(cpHdrMsg, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
    {
        Log.Add('!', "Failed to receive data header", SOp.GetErrno());
        return ((void *) 2);
    }
    Msg.GetStart(cpHdrMsg, &sInHdr);
    sprintf(cpLogEntry, "Receiving data: ch %i fs %g frag %i samples",
        sInHdr.iChannels, sInHdr.dSampleRate, sInHdr.iFragmentSize);
    Log.Add(' ', cpLogEntry);

    if (sInHdr.iCompress)
    {
        if (!InitCompress(sInHdr.iCompress))
            return ((void *) 2);
    }

    iFragmentSize = sInHdr.iFragmentSize;
    iAudioBufSize = sInHdr.iFragmentSize * sizeof(GDT);
    sHdr.iChannels = sInHdr.iChannels;
    sHdr.dSampleRate = sInHdr.dSampleRate;
    InBuf.Size(iAudioBufSize);
    InBuf.Lock();
    AudioBuf.Size(iAudioBufSize);
    AudioBuf.Lock();
    Log.Add(' ', "AudioIn thread running");

    #ifndef BSDSYS
    uidCurrent = getuid();
    setuid(0);
    sSchedParam.sched_priority = sched_get_priority_min(SCHED_FIFO) + 
        SD_INTHREAD_PRIORITY;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sSchedParam) != 0)
        Log.Add('#', "Warning: Unable to set scheduling parameters");
    setuid(uidCurrent);
    #endif

    if (sInHdr.iCompress == MSG_SOUND_COMPRESS_FLAC)
    {
        #ifdef USE_FLAC
        InSOp = &SOp;
        FLAC__stream_decoder_process_until_end_of_stream(spFLACDec);
        #endif
    }

    while (bRun)
    {
        if (sInHdr.iCompress == MSG_SOUND_COMPRESS_FLAC)
        {
            sleep(1);
        }
        else
        {
            if (SOp.ReadN(InBuf, iAudioBufSize) < iAudioBufSize)
            {
                Log.Add('!', "Read error on input stream", SOp.GetErrno());
                return ((void *) 2);
            }
            #ifndef USE_RWLOCK
            MtxAudio.Wait();
            #else
            RWLAudio.WaitWrite();
            #endif
            Msg.GetData(InBuf, (GDT *) AudioBuf, sInHdr.iFragmentSize);
            iBlockCntr++;
            CndAudio.NotifyAll();
            #ifndef USE_RWLOCK
            MtxAudio.Release();
            #else
            RWLAudio.Release();
            #endif
        }
    }

    if (sInHdr.iCompress == MSG_SOUND_COMPRESS_FLAC)
    {
        #ifdef USE_FLAC
        FLAC__stream_decoder_finish(spFLACDec);
        #endif
    }

    Log.Add(' ', "AudioIn thread ending");
    return ((void *) 0);
}


void *clStreamDist::ServeClientThread (void *vpParam)
{
    bool bConnected = true;
    int iOutSize;
    int iLocalBlockCntr;
    stRawDataReq sReq;
    sigset_t sigsetThis;
    #ifndef BSDSYS
    uid_t uidCurrent;
    struct sched_param sSchedParam;
    #endif
    clAlloc OutBuf(iAudioBufSize);
    clSockOp SOp((int) vpParam);

    sigemptyset(&sigsetThis);
    sigaddset(&sigsetThis, SIGPIPE);
    sigaddset(&sigsetThis, SIGINT);
    sigaddset(&sigsetThis, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &sigsetThis, NULL);
    Log.Add('+', "Client connected");
    if (SOp.WriteN(&sHdr, sizeof(sHdr)) < (int) sizeof(sHdr))
        bConnected = false;
    if (SOp.ReadN(&sReq, sizeof(sReq)) < (int) sizeof(sReq))
        bConnected = false;
    if (sReq.iChannel >= sHdr.iChannels) 
    {
        bConnected = false;
        Log.Add('!', "Client requested non-existing channel");
    }
    if (sReq.iChannel < 0)
        iOutSize = iAudioBufSize;
    else
        iOutSize = iAudioBufSize / sHdr.iChannels;

    #ifndef BSDSYS
    uidCurrent = getuid();
    setuid(0);
    sSchedParam.sched_priority = sched_get_priority_min(SCHED_FIFO) + 
        SD_OUTTHREAD_PRIORITY;
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
        if (sReq.iChannel < 0)
        {
            memcpy(OutBuf, AudioBuf, iAudioBufSize);
        }
        else
        {
            CopyChannel(OutBuf, AudioBuf, sReq.iChannel);
        }
        iLocalBlockCntr++;
        if (iLocalBlockCntr != iBlockCntr)
        {
            printf("streamdist: %i blocks lost\n", 
                iBlockCntr - iLocalBlockCntr);
            iLocalBlockCntr = iBlockCntr;
        }
        #ifndef USE_RWLOCK
        MtxAudio.Release();
        #else
        RWLAudio.Release();
        #endif
        if (SOp.WriteN(OutBuf, iOutSize) < iOutSize)
            bConnected = false;
    }
    Log.Add('-', "Client disconnected");
    return NULL;
}


#ifdef USE_FLAC

FLAC__StreamDecoderReadStatus clStreamDist::FLACRead (
    const FLAC__StreamDecoder *spFLACDec, 
    FLAC__byte *cpBuffer, unsigned *uipBytes)
{
    int iReadRes;

    iReadRes = InSOp->Read(cpBuffer, (int) *uipBytes);
    if (iReadRes < 0)
    {
        Log.Add('!', "Read error on input stream", InSOp->GetErrno());
        *uipBytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
    if (iReadRes == 0 || !bRun)
    {
        *uipBytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    *uipBytes = (unsigned) iReadRes;
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}


FLAC__StreamDecoderWriteStatus clStreamDist::FLACWrite (
    const FLAC__StreamDecoder *spFLACDec, 
    const FLAC__Frame *spFrame, const FLAC__int32 * const *ippBuffer)
{
    unsigned uiChannels;
    unsigned uiBits;
    unsigned uiBlockSize;
    unsigned uiSamples;
    unsigned uiChCntr;
    unsigned uiSampleCntr;
    GDT fScale;
    GDT *fpAudioBuf;

    uiChannels = FLAC__stream_decoder_get_channels(spFLACDec);
    uiBits = FLAC__stream_decoder_get_bits_per_sample(spFLACDec);
    uiBlockSize = FLAC__stream_decoder_get_blocksize(spFLACDec);
    uiSamples = uiBlockSize * uiChannels;

    #ifndef USE_RWLOCK
    MtxAudio.Wait();
    #else
    RWLAudio.WaitWrite();
    #endif

    if ((int) uiSamples != iFragmentSize)
    {
        iFragmentSize = uiSamples;
        iAudioBufSize = iFragmentSize * sizeof(GDT);
        AudioBuf.UnLock();
        AudioBuf.Size(iAudioBufSize);
        AudioBuf.Lock();
    }

    switch (uiBits)
    {
        case 8:
            fScale = 1.0f / 0x7f;
            break;
        case 16:
            fScale = 1.0f / 0x7fff;
            break;
        case 24:
            fScale =  1.0f / 0x7fffff;
            break;
        default:
            Log.Add('!', "Unsupported sample length");
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    fpAudioBuf = AudioBuf;
    for (uiChCntr = 0; uiChCntr < uiChannels; uiChCntr++)
    {
        for (uiSampleCntr = 0; uiSampleCntr < uiBlockSize; uiSampleCntr++)
        {
            fpAudioBuf[uiSampleCntr * uiChannels + uiChCntr] =
                (GDT) ippBuffer[uiChCntr][uiSampleCntr] * fScale;
        }
    }

    iBlockCntr++;

    CndAudio.NotifyAll();
    #ifndef USE_RWLOCK
    MtxAudio.Release();
    #else
    RWLAudio.Release();
    #endif

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


void clStreamDist::FLACMetaData (
    const FLAC__StreamDecoder *spFLACDec, 
    const FLAC__StreamMetadata *spMetadata)
{
}


void clStreamDist::FLACError (
    const FLAC__StreamDecoder *spFLACDec, 
    FLAC__StreamDecoderErrorStatus iErrorStatus)
{
    Log.Add('!', FLAC__StreamDecoderErrorStatusString[iErrorStatus]);
}

#endif
