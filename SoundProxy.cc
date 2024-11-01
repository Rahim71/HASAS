/*

    Sound service proxy
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <glib.h>

#include "SoundProxy.hh"


static bool bDaemon;
static bool bDebug;
clSoundProxy *SoundProxy;


int main (int argc, char *argv[])
{
    int iRetVal;

    bDaemon = false;
    bDebug = false;
    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    if (argc > 1)
    {
        if (strcmp(argv[1], "-D") == 0)
            bDaemon = true;
        if (strcmp(argv[1], "--debug") == 0)
            bDebug = true;
        if (strcmp(argv[1], "--version") == 0)
        {
            printf("SoundProxy v%i.%i.%i\n", 
                SP_VERSMAJ, SP_VERSMIN, SP_VERSPL);
            printf("Copyright (C) 2000-2001 Jussi Laako\n\n");
        }
        if (strcmp(argv[1], "--help") == 0)
        {
            printf("%s [-D|--debug|--version|--help]\n", argv[0]);
        }
    }
    if (bDaemon)
    {
        if (fork() == 0)
        {
            setsid();
            freopen("/dev/null", "r", stdin);
            freopen("/dev/null", "a", stdout);
            freopen("/dev/null", "a", stderr);
        }
        else
        {
            return 0;
        }
    }
    SoundProxy = new clSoundProxy();
    iRetVal = SoundProxy->Exec();
    delete SoundProxy;
    return iRetVal;
}


void *WrapSoundInThread (void *vpData)
{
    return SoundProxy->SoundInThread(vpData);
}


void *WrapWaitConnectThread (void *vpData)
{
    return SoundProxy->WaitConnectThread(vpData);
}


void *WrapServeClientThread (void *vpData)
{
    return SoundProxy->ServeClientThread(vpData);
}


inline void clSoundProxy::AddToLog (char cMark, const char *cpLogEntry)
{
    MutexClass.Wait();
    Log.Add(cMark, cpLogEntry);
    MutexClass.Release();
}


inline void clSoundProxy::AddToLog (char cMark, const char *cpLogEntry, 
    int iErrno)
{
    MutexClass.Wait();
    Log.Add(cMark, cpLogEntry, iErrno);
    MutexClass.Release();
}


int clSoundProxy::FindFreeSlot ()
{
    int iResIdx = -1;
    int iLoopCntr;

    MutexClass.Wait();
    for (iLoopCntr = 0; iLoopCntr < SP_MAXCLIENTS; iLoopCntr++)
    {
        if (!bServeClient[iLoopCntr])
        {
            iResIdx = iLoopCntr;
            break;
        }
    }
    MutexClass.Release();
    return iResIdx;
}


clSoundProxy::clSoundProxy ()
{
    int iLoopCntr;

    bRun = true;
    Cfg.SetFileName(SP_CFGFILE);
    if (!Cfg.GetStr("LogFile", cpLogFile))
        strcpy(cpLogFile, SP_DEF_LOGFILE);
    Cfg.GetStr("ServerHost", cpServerHost);
    Cfg.GetInt("ServerPort", &iServerPort);
    Cfg.GetInt("Port", &iServicePort);
    Log.Open(cpLogFile);
    cpFirstMsg = (char *) FirstMsg.Size(GLOBAL_HEADER_LEN);
    cpDataMsg = (char *) DataMsg.Size(SP_BUFFER_SIZE);
    DataMsg.Lock();
    for (iLoopCntr = 0; iLoopCntr < SP_MAXCLIENTS; iLoopCntr++)
        bServeClient[iLoopCntr] = false;
}


clSoundProxy::~clSoundProxy ()
{
}


int clSoundProxy::Exec ()
{
    int iSigNum;
    sigset_t ssSignals;

    sigemptyset(&ssSignals);
    sigaddset(&ssSignals, SIGHUP);
    sigaddset(&ssSignals, SIGINT);
    sigaddset(&ssSignals, SIGTERM);
    sigprocmask(SIG_BLOCK, &ssSignals, NULL);
    AddToLog('*', "Started");
    pthread_create(&ptidSoundIn, NULL, WrapSoundInThread, NULL);
    pthread_create(&ptidWaitConnect, NULL, WrapWaitConnectThread, NULL);
    sigwait(&ssSignals, &iSigNum);
    switch (iSigNum)
    {
        case SIGHUP:
            AddToLog(' ', "Received SIGHUP");
            break;
        case SIGINT:
            AddToLog(' ', "Received SIGINT");
            break;
        case SIGTERM:
            AddToLog(' ', "Received SIGTERM");
            break;
        default:
            AddToLog(' ', "Received unexpected signal");
            break;
    }
    Stop();
    pthread_join(ptidWaitConnect, NULL);
    pthread_join(ptidSoundIn, NULL);
    AddToLog('*', "Stopped");
    return 0;
}


void clSoundProxy::Stop ()
{
    MutexClass.Wait();
    bRun = false;
    MutexClass.Release();
}


void *clSoundProxy::SoundInThread (void *vpData)
{
    bool bLocalRun;
    int iSockH;
    char cpConvBuf[SP_CONV_BUF_LEN];
    char *cpMsgBuf;
    #ifndef BSDSYS
    uid_t uidCurrent;
    struct sched_param sSchedParam;
    #endif
    clAlloc MsgBuf;
    clSockClie SClient;
    clSockOp SOp;

    iSockH = SClient.Connect(cpServerHost, NULL, iServerPort);
    if (iSockH < 0)
    {
        g_snprintf(cpConvBuf, SP_CONV_BUF_LEN, 
            "Unable to connect to %s:%i", cpServerHost, iServerPort);
        AddToLog('!', cpConvBuf, SClient.GetErrno());
        Stop();
        return NULL;
    }
    SOp.SetHandle(iSockH);
    g_snprintf(cpConvBuf, SP_CONV_BUF_LEN, "Connected to %s:%i",
        cpServerHost, iServerPort);
    AddToLog('#', cpConvBuf);
    if (SOp.ReadSelect(SP_1ST_MSG_TIMEOUT))
    {
        MutexData.Wait();
        if (SOp.ReadN(cpFirstMsg, GLOBAL_HEADER_LEN) != GLOBAL_HEADER_LEN)
        {
            AddToLog('!', "Unable to get first message from server",
                SOp.GetErrno());
            Stop();
        }
        MutexData.Release();
    }
    else
    {
        AddToLog('!', "Unable to get first message from server (timeout)");
        Stop();
    }
    cpMsgBuf = (char *) MsgBuf.Size(SP_BUFFER_SIZE);
    MsgBuf.Lock();
    AddToLog(' ', "SoundIn thread running");
    #ifndef BSDSYS
    uidCurrent = getuid();
    setuid(0);
    sSchedParam.sched_priority = sched_get_priority_min(SCHED_FIFO) + 
        SP_SCHED_PRIORITY;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &sSchedParam);
    setuid(uidCurrent);
    #endif
    SOp.DisableNagle();
    SOp.SetTypeOfService(IPTOS_LOWDELAY);
    MutexClass.Wait();
    bLocalRun = bRun;
    MutexClass.Release();
    while (bLocalRun)
    {
        if (SOp.ReadSelect(SP_MSG_TIMEOUT))
        {
            if (SOp.ReadN(cpMsgBuf, SP_BUFFER_SIZE) == SP_BUFFER_SIZE)
            {
                MutexData.Wait();
                memcpy(cpDataMsg, cpMsgBuf, SP_BUFFER_SIZE);
                CondData.NotifyAll();
                MutexData.Release();
            }
            else
            {
                AddToLog('!', "Message receive error", SOp.GetErrno());
                Stop();
            }
        }
        MutexClass.Wait();
        bLocalRun = bRun;
        MutexClass.Release();
    }
    SOp.Shutdown(2);
    SOp.Close();
    AddToLog(' ', "SoundIn thread ending");
    return NULL;
}


void *clSoundProxy::WaitConnectThread (void *vpData)
{
    bool bLocalRun = true;
    int iSockH;
    int iSlotIdx;
    clSockServ SServer;

    AddToLog(' ', "WaitConnect thread running");
    SServer.Bind(iServicePort);
    while (bLocalRun)
    {
        iSlotIdx = FindFreeSlot();
        if (iSlotIdx >= 0)
        {
            iSockH = SServer.WaitForConnect(SP_WAIT_CONN_TIMEOUT);
            if (iSockH >= 0)
            {
                MutexClass.Wait();
                bServeClient[iSlotIdx] = true;
                iClientSockH[iSlotIdx] = iSockH;
                MutexClass.Release();
                pthread_create(&ptidServeClient[iSlotIdx], NULL,
                    WrapServeClientThread, GINT_TO_POINTER(iSlotIdx));
            }
        }
        else
        {
            sched_yield();
        }
        MutexClass.Wait();
        bLocalRun = bRun;
        MutexClass.Release();
    }
    AddToLog(' ', "WaitConnect thread ending");
    return NULL;
}


void *clSoundProxy::ServeClientThread (void *vpData)
{
    bool bLocalRun = true;
    int iThisIdx = GPOINTER_TO_INT(vpData);
    int iSockH;
    char *cpMsgBuf;
    char cpHdrBuf[GLOBAL_HEADER_LEN];
    char cpLogBuf[SP_CONV_BUF_LEN];
    socklen_t iPeerAddrLen;
    struct sockaddr_in sPeerAddr;
    #ifndef BSDSYS
    uid_t uidCurrent;
    struct sched_param sSchedParam;
    #endif
    clAlloc MsgBuf;
    clSockOp SOp;

    MutexClass.Wait();
    iSockH = iClientSockH[iThisIdx];
    MutexClass.Release();
    SOp.SetHandle(iSockH);
    //SOp.SetSendBufSize(SP_BUFFER_SIZE * 2);
    iPeerAddrLen = sizeof(sPeerAddr);
    SOp.GetPeerName((struct sockaddr *) &sPeerAddr, &iPeerAddrLen);
    g_snprintf(cpLogBuf, SP_CONV_BUF_LEN, "Client connected from %s:%i", 
        inet_ntoa(sPeerAddr.sin_addr), ntohs(sPeerAddr.sin_port));
    AddToLog('+', cpLogBuf);
    if (!SOp.DisableNagle())
    {
        AddToLog('#', "Unable to disable nagle algorithm", SOp.GetErrno());
    }
    if (!SOp.SetTypeOfService(IPTOS_LOWDELAY))
    {
        AddToLog('#', "Unable set type of service flag", SOp.GetErrno());
    }
    cpMsgBuf = (char *) MsgBuf.Size(SP_BUFFER_SIZE);
    MsgBuf.Lock();
    MutexData.Wait();
    memcpy(cpHdrBuf, cpFirstMsg, GLOBAL_HEADER_LEN);
    MutexData.Release();
    if (SOp.WriteSelect(SP_1ST_MSG_TIMEOUT))
    {
        if (SOp.WriteN(cpHdrBuf, GLOBAL_HEADER_LEN) != GLOBAL_HEADER_LEN)
        {
            AddToLog('!', "Unable to send first message", SOp.GetErrno());
            bLocalRun = false;
        }
    }
    else
    {
        AddToLog('!', "Unable to send first message (timeout)");
        bLocalRun = false;
    }
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    AddToLog(' ', "ServeClient thread running");
    #ifndef BSDSYS
    uidCurrent = getuid();
    setuid(0);
    sSchedParam.sched_priority = sched_get_priority_min(SCHED_FIFO) + 
        SP_SCHED_PRIORITY;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &sSchedParam);
    setuid(uidCurrent);
    #endif
    while (bLocalRun)
    {
        MutexData.Wait();
        CondData.Wait(MutexData.GetPtr());
        memcpy(cpMsgBuf, cpDataMsg, SP_BUFFER_SIZE);
        MutexData.Release();
        MutexClass.Wait();
        bLocalRun = bRun;
        MutexClass.Release();
        if (!bLocalRun) break;
        if (SOp.WriteSelect(SP_MSG_TIMEOUT))
        {
            if (SOp.WriteN(cpMsgBuf, SP_BUFFER_SIZE) != SP_BUFFER_SIZE)
            {
                AddToLog('-', "Client disconnected?", SOp.GetErrno());
                break;
            }
        }
    }
    SOp.Close();
    AddToLog(' ', "ServeClient thread ending");
    return NULL;
}

