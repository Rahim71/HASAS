/*

    Sound card input server
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sched.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
//#include <sys/time.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pth.h>

#include "Config.h"
#include "SoundSrv.hh"


clSoundSrv *SoundSrv;


void *Input(void *vpNone)
{
    SoundSrv->InputExec();
    return NULL;
}


void *WaitConnect(void *vpNone)
{
    SoundSrv->WaitConnectExec();
    return NULL;
}


void *ServeClient(void *vpHandle)
{
    SoundSrv->ServeClientExec(vpHandle);
    return NULL;
}


int main(int argc, char *argv[])
{
    bool bDaemon = false;
    int iArgCntr;
    char cpDevice[_POSIX_PATH_MAX + 1];
    #ifndef BSDSYS
    uid_t uidCurrent;
    struct sched_param sSchedParam;
    #endif
    
    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    #ifndef BSDSYS
    uidCurrent = getuid();
    setuid(0);
    sSchedParam.sched_priority = SS_SCHED_PRIORITY;
    sched_setscheduler(0, SCHED_FIFO, &sSchedParam);
    setuid(uidCurrent);
    #endif
    if (pth_init() < 0)
    {
        fprintf(stderr, "fatal: pth_init() failed\n");
        exit(1);
    }
    strcpy(cpDevice, SS_SND_DEVICE);
    if (argc >= 2)
    {
        for (iArgCntr = 1; iArgCntr < argc; iArgCntr++)
        {
            if (strcmp(argv[iArgCntr], "-D") == 0)
            {
                bDaemon = true;
            }
            else if (strcmp(argv[iArgCntr], "--version") == 0)
            {
                printf("SoundSrv v%i.%i.%i\n", SS_VERSMAJ, SS_VERSMIN,
                    SS_VERSPL);
                printf("Copyright (C) 1999-2001 Jussi Laako\n");
                return 0;
            }
            else if (strcmp(argv[iArgCntr], "--help") == 0)
            {
                printf("%s [-D|--version|--help|device]\n\n", argv[0]);
                printf("-D         start as daemon\n");
                printf("--version  display version information\n");
                printf("--help     display this help\n");
                printf("device     use device (eg. dsp0)\n");
                return 0;
            }
            else
            {
                strcpy(cpDevice, argv[iArgCntr]);
            }
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
    SoundSrv = new clSoundSrv(argv[0], cpDevice);
    SoundSrv->Exec();
    delete SoundSrv;
    pth_kill();
    return 0;
}


clSoundSrv::clSoundSrv(const char *cpName, const char *cpAudioDev)
{
    int iBits;
    int iLoopCntr;
    int iOSSVersion;
    long lPthVersion;
    char cpCfgName[_POSIX_PATH_MAX + 1];
    char cpLogName[_POSIX_PATH_MAX + 1];
    char cpDevName[_POSIX_PATH_MAX + 1];
    char cpLogTxt[256];
    
    bRun = true;
    for (iLoopCntr = 0; iLoopCntr < SS_MAXCLIENTS; iLoopCntr++)
    {
        ipClientH[iLoopCntr] = -1;
    }
    strcpy(cpProgName, cpName);
    sprintf(cpCfgName, "soundsrv.%s.cfg", cpAudioDev);
    CfgFile = new clCfgFile(cpCfgName);
    sprintf(cpLogName, "%s.%s", SS_LOGFILE, cpAudioDev);
    LogFile = new clLogFile(cpLogName);
    LogFile->Add('*', "Starting");
    lPthVersion = pth_version();
    sprintf(cpLogTxt, "Pth version %lu.%lu.%lu (compiled with %u.%u.%u)",
        ((lPthVersion & 0xf00000) >> 20),
        ((lPthVersion & 0x0ff000) >> 12),
        (lPthVersion & 0x0000ff),
        SS_PTH_MAJ, SS_PTH_MIN, SS_PTH_PL);
    LogFile->Add(' ', cpLogTxt);
    sprintf(cpDevName, "/dev/%s", cpAudioDev);
    if (!CfgFile->GetInt("SampleRate", &iAudioSr))
    {
        iAudioSr = SS_SND_SAMPLERATE;
    }
    if (!CfgFile->GetInt("Channels", &iAudioCh))
    {
        iAudioCh = SS_SND_CHANNELS;
    }
    if (CfgFile->GetInt("Bits", &iBits))
    {
        switch (iBits)
        {
            case 8:
                iAudioFrmt = AFMT_U8;
                iAudioTypeSize = 1;
                break;
            case 16:
                iAudioFrmt = AFMT_S16_NE;
                iAudioTypeSize = 2;
                break;
            #ifndef USE_OSSLITE
            case 24:
            case 32:
                iAudioFrmt = AFMT_S32_NE;
                iAudioTypeSize = 4;
                break;
            #endif
            default:
                iAudioFrmt = SS_SND_FORMAT;
                iAudioTypeSize = SS_SND_FORMAT_SIZE;
        }
    }
    else
    {
        iAudioFrmt = SS_SND_FORMAT;
        iAudioTypeSize = SS_SND_FORMAT_SIZE;
    }
    sprintf(cpLogTxt, "Request %s fs %i ch %i fmt %xh", cpAudioDev, 
        iAudioSr, iAudioCh, iAudioFrmt);
    LogFile->Add(' ', cpLogTxt);
    Audio = new clAudio(cpDevName, &iAudioFrmt, &iAudioSr, &iAudioCh,
        AUDIO_READ);
    sprintf(cpLogTxt, "Open %s fs %i ch %i fmt %xh", cpAudioDev, 
        iAudioSr, iAudioCh, iAudioFrmt);
    LogFile->Add(' ', cpLogTxt, Audio->GetError());
    iOSSVersion = Audio->GetVersion();
    sprintf(cpLogTxt, "OSS version %i.%i.%i (compiled with %i.%i.%i)",
        ((iOSSVersion >> 16) & 0xff), ((iOSSVersion >> 8) & 0xff),
        (iOSSVersion & 0xff),
        ((SOUND_VERSION >> 16) & 0xff), ((SOUND_VERSION >> 8) & 0xff),
        (SOUND_VERSION & 0xff));
    LogFile->Add(' ', cpLogTxt);
    sprintf(cpLogTxt, "Fragment size %i bytes", Audio->GetFragmentSize());
    LogFile->Add(' ', cpLogTxt);
    iSampleCount = Audio->GetFragmentSize() / iAudioTypeSize;
    iOutBufSize = iSampleCount * sizeof(GDT);
    cpOutBuf = (char *) malloc(iOutBufSize);
    if (cpOutBuf == NULL)
    {
        LogFile->Add('!', "OUT OF MEMORY");
        exit(1);
    }
    mlock(cpOutBuf, iOutBufSize);
}


clSoundSrv::~clSoundSrv()
{
    delete Audio;
    LogFile->Add('*', "Shutdown");
    delete LogFile;
    delete CfgFile;
    munlock(cpOutBuf, iOutBufSize);
    if (cpOutBuf != NULL) free(cpOutBuf);
}


void clSoundSrv::Exec()
{
    unsigned int iInputStackSize;
    int iSigNum;
    void *vpReturnValue;
    pth_attr_t pthaInput;
    pth_attr_t pthaWaitConnect;
    pth_event_t ptheInputDead;
    pth_event_t ptheWaitConnectDead;
    pth_event_t ptheThreadDead;

    sigemptyset(&sigsetQuit);
    sigaddset(&sigsetQuit, SIGINT);
    sigaddset(&sigsetQuit, SIGHUP);
    sigaddset(&sigsetQuit, SIGTERM);
    iInputStackSize = 1024 * 32;
    pthaInput = pth_attr_new();
    pth_attr_set(pthaInput, PTH_ATTR_NAME, "Input");
    pth_attr_set(pthaInput, PTH_ATTR_STACK_SIZE, iInputStackSize);
    tidInput = pth_spawn(pthaInput, Input, NULL);
    pthaWaitConnect = pth_attr_new();
    pth_attr_set(pthaWaitConnect, PTH_ATTR_NAME, "WaitConnect");
    tidWaitConnect = pth_spawn(PTH_ATTR_DEFAULT, WaitConnect, NULL);
    ptheInputDead = pth_event(PTH_EVENT_TID|PTH_UNTIL_TID_DEAD, tidInput);
    ptheWaitConnectDead = pth_event(PTH_EVENT_TID|PTH_UNTIL_TID_DEAD,
        tidWaitConnect);
    ptheThreadDead = pth_event_concat(ptheInputDead, ptheWaitConnectDead, 
        NULL);
    LogFile->Add(' ', "Running");
    pth_sigwait_ev(&sigsetQuit, &iSigNum, ptheThreadDead);
    if (pth_event_occurred(ptheThreadDead))
    {
        LogFile->Add('!', "Dead of thread detected");
        Quit();
    }
    else
    {
        Quit(iSigNum);
    }
    pth_cancel(tidInput);
    pth_join(tidInput, &vpReturnValue);
    pth_join(tidWaitConnect, &vpReturnValue);
    LogFile->Add(' ', "Stopping");
}


void clSoundSrv::Quit()
{
    bRun = false;
}


void clSoundSrv::Quit(int iSigNum)
{
    bRun = false;
    switch (iSigNum)
    {
        case SIGHUP:
            LogFile->Add(' ', "Received SIGHUP");
            break;
        case SIGINT:
            LogFile->Add(' ', "Received SIGINT");
            break;
        default:
            LogFile->Add('!', "Received unknown signal");
    }
}


void clSoundSrv::InputExec()
{
    int iDevH;
    int iInBufSize;
    int iConvBufSize;
    int iBytesRead = 0;
    int iInputErrors = 0;
    void *pInBuf;
    GDT *pConvBuf;

    iInBufSize = iSampleCount * iAudioTypeSize;
    iConvBufSize = iSampleCount * sizeof(GDT);
    pInBuf = malloc(iInBufSize);
    pConvBuf = (GDT *) malloc(iConvBufSize);
    if (pInBuf == NULL || pConvBuf == NULL)
    {
        LogFile->Add('!', "OUT OF MEMORY!");
        return;
    }
    mlock(pInBuf, iInBufSize);
    mlock(pConvBuf, iConvBufSize);
    pth_cancel_state(PTH_CANCEL_DEFERRED, NULL);
    iDevH = SoundSrv->Audio->GetHandle();
    LogFile->Add(' ', "Input thread running");
    // We do one normal read to trigger input operation, this is trying to be
    // workaround for some buggy driver implementations...
    iBytesRead = read(iDevH, pInBuf, iInBufSize);
    while (bRun)
    {
        if (access(SS_SHUTDOWNFILE, F_OK) == 0)
        {
            unlink(SS_SHUTDOWNFILE);
            Quit();
            break;
        }
        iBytesRead += pth_read(iDevH, pInBuf, iInBufSize - iBytesRead);
        if (!bRun) break;
        if (iBytesRead == iInBufSize)
        {
            switch (iAudioTypeSize)
            {
                case 1:
                    DSP.Convert(pConvBuf, (unsigned char *) pInBuf, 
                        iSampleCount);
                    break;
                case 2:
                    DSP.Convert(pConvBuf, (signed short *) pInBuf,
                        iSampleCount, false);
                    break;
                case 4:
                    DSP.Convert(pConvBuf, (signed int *) pInBuf,
                        iSampleCount, true);
                    break;
            }
            MutexThis.Wait();
            SoundMsg.SetData(cpOutBuf, pConvBuf, iSampleCount);
            CondDataAvail.Notify(TRUE);
            MutexThis.Release();
            iBytesRead = 0;
        }
        else if (iBytesRead > iInBufSize)
        {
            LogFile->Add('!', 
                "pth_read() overflowed input buffer, process unstable");
            Quit();
        }
        else if (iBytesRead < iInBufSize && iBytesRead >= 0)
        {
            iInputErrors++;
            LogFile->Add('!', "pth_read() input buffer underrun");
        }
        else
        {
            iInputErrors++;
            LogFile->Add('!', "input device pth_read() error", errno);
        }
        if (iInputErrors >= SS_MAXERRORS)
        {
            LogFile->Add('!', "Too many errors on input device");
            Quit();
        }
    }
    munlock(pInBuf, iInBufSize);
    munlock(pConvBuf, iOutBufSize);
    free(pInBuf);
    free(pConvBuf);
    LogFile->Add(' ', "Input thread ending");
}


void clSoundSrv::WaitConnectExec()
{
    int iLoopCntr;
    int iListenH;
    int iPort;
    socklen_t iAddrLen;
    char cpLogEntry[256];
    void *vpReturnValue;
    struct protoent *spProtocol = NULL;
    struct servent *spService = NULL;
    struct sockaddr_in sServAddr;
    struct sockaddr_in sClieAddr;
    pth_attr_t pthaServeClient;
    pth_event_t ptheInputDead;

    LogFile->Add(' ', "WaitConnect thread running");
    pthaServeClient = pth_attr_new();
    pth_attr_set(pthaServeClient, PTH_ATTR_NAME, "ServeClient");
    ptheInputDead = pth_event(PTH_EVENT_TID|PTH_UNTIL_TID_DEAD, tidInput);
    spProtocol = getprotobyname("tcp");
    if (spProtocol != NULL)
    {
        iListenH = socket(AF_INET, SOCK_STREAM, spProtocol->p_proto);
    }
    else
    {
        iListenH = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        LogFile->Add('#', 
            "Warning: no entry returned for TCP by getprotobyname()");
    }
    if (iListenH < 0)
    {
        LogFile->Add('!', "socket() failed for listening fd", errno);
        return;
    }
    memset(&sServAddr, 0x00, sizeof(sServAddr));
    sServAddr.sin_family = AF_INET;
    sServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (CfgFile->GetInt("Port", &iPort))
    {
        sServAddr.sin_port = htons(iPort);
    }
    else
    {
        spService = getservbyname(cpProgName, "tcp");
        if (spService != NULL)
        {
            sServAddr.sin_port = spService->s_port;
        }
        else
        {
            sServAddr.sin_port = htons(SS_DEFAULT_PORT);
        }
    }
    if (bind(iListenH, (struct sockaddr *) &sServAddr, sizeof(sServAddr)) < 0)
    {
        LogFile->Add('!', "bind() failed for listening fd", errno);
        close(iListenH);
        return;
    }
    if (listen(iListenH, SS_MAXCLIENTS) < 0)
    {
        LogFile->Add('!', "listen() failed for listening fd", errno);
        close(iListenH);
        return;
    }
    sprintf(cpLogEntry, "Listening for connections on port %i", 
        ntohs(sServAddr.sin_port));
    LogFile->Add(' ', cpLogEntry);
    while (bRun)
    {
        iClientIdx = FindFreeClient();
        memset(&sClieAddr, 0x00, sizeof(sClieAddr));
        iAddrLen = sizeof(sClieAddr);
        ipClientH[iClientIdx] = pth_accept_ev(iListenH, 
            (struct sockaddr *) &sClieAddr, &iAddrLen, ptheInputDead);
        if (!pth_event_occurred(ptheInputDead))
        {
            if (ipClientH[iClientIdx] >= 0)
            {
                sprintf(cpLogEntry, "%s connected", 
                    inet_ntoa(sClieAddr.sin_addr));
                LogFile->Add('+', cpLogEntry);
                ptidServeClient[iClientIdx] =
                    pth_spawn(pthaServeClient, ServeClient, 
                        &ipClientH[iClientIdx]);
            }
        }
    }
    for (iLoopCntr = 0; iLoopCntr < SS_MAXCLIENTS; iLoopCntr++)
    {
        if (ipClientH[iLoopCntr] >= 0)
            pth_join(ptidServeClient[iLoopCntr], &vpReturnValue);
    }
    pth_event_free(ptheInputDead, PTH_FREE_THIS);
    LogFile->Add(' ', "WaitConnect thread ending");
}


void clSoundSrv::ServeClientExec(void *vpHandle)
{
    bool bThreadRun = true;
    char cpFirstMsg[GLOBAL_HEADER_LEN];
    char *cpLocalOutBuf;
    int iHandleIdx;
    const int iSockH = *((int *) vpHandle);
    int iSockBufSize;
    //int iSockLowWater;
    int iSockNagle;
    int iSockTOS;
    int iBytesSent;
    int iLastRes;
    int iWriteErrno;
    socklen_t iSockOptLen;
    stSoundStart sSndStart;
    pth_event_t ptheInputDead;

    LogFile->Add(' ', "ServeClient thread running");
    iHandleIdx = FindThisHandle(iSockH);
    if (iHandleIdx < 0)
    {
        LogFile->Add('!', "FindThisHandle() returned error");
        return;
    }
    cpLocalOutBuf = (char *) malloc(iOutBufSize);
    if (cpLocalOutBuf == NULL)
    {
        LogFile->Add('!', "malloc() for local output buffer failed");
        return;
    }
    mlock(cpLocalOutBuf, iOutBufSize);
    ptheInputDead = pth_event(PTH_EVENT_TID|PTH_UNTIL_TID_DEAD, tidInput);
    sSndStart.iChannels = iAudioCh;
    sSndStart.dSampleRate = iAudioSr;
    sSndStart.iFragmentSize = iSampleCount;
    SoundMsg.SetStart(cpFirstMsg, &sSndStart);
    if (pth_write_ev(iSockH, cpFirstMsg, GLOBAL_HEADER_LEN, ptheInputDead) <
        GLOBAL_HEADER_LEN)
    {
        LogFile->Add('?', "Unable to send data header message to client",
            errno);
        close(iSockH);
        ipClientH[iHandleIdx] = -1;
        return;
    }
    iSockOptLen = sizeof(iSockBufSize);
    if (getsockopt(iSockH, SOL_SOCKET, SO_SNDBUF, &iSockBufSize, 
        &iSockOptLen) < 0)
    {
        LogFile->Add('!', "getsockopt() error getting send buffer size",
            errno);
    }
    if (iSockBufSize < (SS_SOCKET_BUF_FRAGS * iOutBufSize))
    {
        iSockBufSize = SS_SOCKET_BUF_FRAGS * iOutBufSize;
        if (setsockopt(iSockH, SOL_SOCKET, SO_SNDBUF, &iSockBufSize,
            sizeof(iSockBufSize)) < 0)
        {
            LogFile->Add('!', "setsockopt() error setting send buffer size",
                errno);
        }
    }
    /*iSockOptLen = sizeof(iSockLowWater);
    if (getsockopt(iSockH, SOL_SOCKET, SO_SNDLOWAT, &iSockLowWater,
        &iSockOptLen) < 0)
    {
        LogFile->Add('!', "getsockopt() error getting send low water mark",
            errno);
    }
    if (iSockLowWater < iOutBufSize)
    {
        iSockLowWater = iOutBufSize;
        if (setsockopt(iSockH, SOL_SOCKET, SO_SNDLOWAT, &iSockLowWater,
            sizeof(iSockLowWater)) < 0)
        {
            LogFile->Add('!', "setsockopt() error setting send low water mark",
                errno);
        }
    }*/
    iSockNagle = 1;
    if (setsockopt(iSockH, IPPROTO_TCP, TCP_NODELAY, &iSockNagle,
        sizeof(iSockNagle)) < 0)
    {
        LogFile->Add('!', "setsockopt() error disabling nagle algorithm",
            errno);
    }
    iSockTOS = IPTOS_LOWDELAY;
    if (setsockopt(iSockH, IPPROTO_IP, IP_TOS, &iSockTOS,
        sizeof(iSockTOS)) < 0)
    {
        LogFile->Add('!', "setsockopt() error setting TOS flag",
            errno);
    }
    while (bRun && bThreadRun)
    {
        MutexThis.Wait();
        CondDataAvail.Wait(MutexThis.GetPtr(), ptheInputDead);
        memcpy(cpLocalOutBuf, cpOutBuf, iOutBufSize);
        MutexThis.Release();
        if (!pth_event_occurred(ptheInputDead))
        {
            iBytesSent = 0;
            do {
                iLastRes = pth_write_ev(iSockH, &cpLocalOutBuf[iBytesSent], 
                    iOutBufSize - iBytesSent, ptheInputDead);
                iBytesSent += iLastRes;
                if (pth_event_occurred(ptheInputDead)) 
                {
                    bThreadRun = false;
                    break;
                }
            } while (iBytesSent < iOutBufSize && iLastRes >= 0);
            if (!bThreadRun) break;
            iWriteErrno = (iLastRes < 0) ? errno : 0;
            if (iWriteErrno != 0)
            {
                if (iWriteErrno == EPIPE)
                {
                    LogFile->Add('-', "Client disconnect");
                }
                else
                {
                    LogFile->Add('-', 
                        "pth_write() returned error on client socket",
                        iWriteErrno);
                }
                bThreadRun = false;
            }
        }
        else break;
    }
    close(iSockH);
    ipClientH[iHandleIdx] = -1;
    pth_event_free(ptheInputDead, PTH_FREE_THIS);
    munlock(cpLocalOutBuf, iOutBufSize);
    free(cpLocalOutBuf);
    LogFile->Add(' ', "ServeClient thread ending");
}


int inline clSoundSrv::FindFreeClient()
{
    int iLoopCntr;

    for (iLoopCntr = 0; iLoopCntr < SS_MAXCLIENTS; iLoopCntr++)
    {
        if (ipClientH[iLoopCntr] < 0)
        {
            return iLoopCntr;
        }
    }
    return -1;
}


int inline clSoundSrv::FindThisHandle(int iFindH)
{
    int iLoopCntr;

    for (iLoopCntr = 0; iLoopCntr < SS_MAXCLIENTS; iLoopCntr++)
    {
        if (ipClientH[iLoopCntr] == iFindH)
        {
            return iLoopCntr;
        }
    }
    return -1;
}

