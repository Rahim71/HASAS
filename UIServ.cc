/*

    User interface server
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
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Config.h"
#include "UIServ.hh"
#include "SockOp.hh"


bool bRun = true;
clUIServer *UIServer;
clSockOp *SOp;


void SigHandler(int iSigNum)
{
    switch (iSigNum)
    {
        case SIGHUP:
        case SIGINT:
        case SIGTERM:
            bRun = false;
            break;
        default:
            bRun = false;
    }
}


int main(int argc, char *argv[])
{
    int iArgCntr;
    int iClientH;
    int iTOS;
    socklen_t iTOSSize;
    bool bDaemon = false;
    bool bDebug = false;
    char cpMsg[GLOBAL_HEADER_LEN];

    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    if (argc >= 2)
    {
        for (iArgCntr = 1; iArgCntr < argc; iArgCntr++)
        {
            if (strcmp(argv[iArgCntr], "-D") == 0)
            {
                bDaemon = true;
            }
            if (strcmp(argv[iArgCntr], "--debug") == 0)
            {
                bDebug = true;
            }
            if (strcmp(argv[iArgCntr], "--version") == 0)
            {
                printf("UIServ v%i.%i.%i\n", UIS_VERSMAJ, UIS_VERSMIN,
                    UIS_VERSPL);
                printf("Copyright (C) 1999-2001 Jussi Laako\n");
                return 0;
            }
            if (strcmp(argv[iArgCntr], "--help") == 0)
            {
                printf("%s [-D|--debug|--version|--help]\n\n", argv[0]);
                printf("-D         start as daemon\n");
                printf("--debug    emit debug output from servers\n");
                printf("--version  display version information\n");
                printf("--help     display this help\n");
                return 0;
            }
        }
    }
    if (bDaemon)
    {
        signal(SIGHUP, SigHandler);
        if (fork() != 0)
        {
            exit(0);
        }
        setsid();
        freopen("/dev/null", "r+", stderr);
        freopen("/dev/null", "r+", stdin);
        freopen("/dev/null", "r+", stdout);
    }
    else
    {
        signal(SIGINT, SigHandler);
    }
    signal(SIGTERM, SigHandler);
    UIServer = new clUIServer();
    while (bRun)
    {
        if (bDebug) printf(" - Waiting for connections\n");
        iClientH = UIServer->Wait();
        if (iClientH >= 0)
        {
            if (fork() == 0)
            {
                UIServer->NoLogShutdown();
                delete UIServer;
                dup2(iClientH, 3);
                dup2(iClientH, 4);
                SOp = new clSockOp(iClientH);
                if (!SOp->ReadSelect(UIS_MSG_TIMEOUT))
                {
                    exit(1);
                }
                if (SOp->ReadN(cpMsg, GLOBAL_HEADER_LEN) == GLOBAL_HEADER_LEN)
                {
                    if (!SOp->DisableNagle())
                    {
                        if (bDebug) 
                            printf("clSockOp::DisableNagle() failed\n");
                    }
                    if (!SOp->SetTypeOfService(IPTOS_LOWDELAY))
                    {
                        if (bDebug)
                            printf("clSockOp::SetTypeOfService() failed\n");
                    }
                    iTOS = 0;
                    iTOSSize = sizeof(iTOS);
                    getsockopt(3, IPPROTO_IP, IP_TOS, &iTOS, &iTOSSize);
                    if (iTOS != IPTOS_LOWDELAY)
                    {
                        if (bDebug)
                            printf("Problem with socket options & dup2()!\n");
                    }

                    if (!bDebug)
                    {
                        if (!bDaemon)
                        {
                            execl(cpMsg, cpMsg, NULL);
                        }
                        else 
                        {
                            execl(cpMsg, cpMsg, "-D", NULL);
                        }
                    }
                    else
                    {
                        printf(" - execl(%s)\n", cpMsg);
                        execl(cpMsg, cpMsg, "--debug", NULL);
                    }
                    exit(1);
                }
            }
            else
            {
                close(iClientH);
            }
        }
        else
        {
            bRun = false;
        }
    }
    delete UIServer;
    if (bDebug) printf(" - Exit\n");
    return 0;
}


clUIServer::clUIServer()
{
    int iPortNum;
    char cpLogBuf[UIS_CONV_BUF_SIZE];
    
    bLogShutdown = true;
    Cfg = new clCfgFile(UIS_CFGFILE);
    Log = new clLogFile(UIS_LOGFILE);
    if (Log->GetError() != LOGFILE_NOERROR)
    {
        printf("Logfile open failed!\n");
        exit(1);
    }
    Log->Add('*', "Starting");
    if (!Cfg->GetInt("Port", &iPortNum))
    {
        iPortNum = UIS_DEFAULT_PORT;
    }
    sprintf(cpLogBuf, "Listening for connections on port %i", iPortNum);
    Log->Add(' ', cpLogBuf);
    SServ = new clSockServ(iPortNum);
    if (SServ->GetErrno() != 0)
    {
        Log->Add('!', "clSockServ::clSockServ() error", SServ->GetErrno());
        return;
    }
}


clUIServer::~clUIServer()
{
    if (bLogShutdown) Log->Add('*', "Shutdown");
    delete SServ;
    delete Log;
    delete Cfg;
}


int clUIServer::Wait()
{
    int iHandle;
    char cpLogBuf[UIS_CONV_BUF_SIZE];
    socklen_t iPeerAddrLen;
    struct sockaddr_in sPeerAddr;
    clSockOp SOp;
    
    while (bRun)
    {
        if (access(UIS_SHUTDOWNFILE, F_OK) == 0)
        {
            unlink(UIS_SHUTDOWNFILE);
            bRun = false;
            break;
        }
        iHandle = SServ->WaitForConnect(UIS_TIMEOUT);
        if (iHandle >= 0)
        {
            SOp.SetHandle(iHandle);
            SOp.SetCloseOnDestruct(false);
            iPeerAddrLen = sizeof(sPeerAddr);
            SOp.GetPeerName((struct sockaddr *) &sPeerAddr, &iPeerAddrLen);
            sprintf(cpLogBuf, "Client connected from %s:%i",
                inet_ntoa(sPeerAddr.sin_addr), ntohs(sPeerAddr.sin_port));
            Log->Add('+', cpLogBuf);
            return iHandle;
        }
        else
        {
            if (SServ->GetErrno() != 0)
            {
                Log->Add('!', "clSockServ::WaitForConnect() error",
                    SServ->GetErrno());
                return -1;
            }
        }
        waitpid(0, NULL, WNOHANG);
    }
    return -1;
}

