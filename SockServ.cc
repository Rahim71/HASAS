/*

    Socket server class
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
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Config.h"
#include "SockServ.hh"


clSockServ::clSockServ ()
{
    iErrno = 0;
    iListenH = -1;
}


clSockServ::clSockServ (unsigned short usPort)
{
    iErrno = 0;
    Bind(usPort);
}


clSockServ::clSockServ (const char *cpSockFile)
{
    iErrno = 0;
    Bind(cpSockFile);
}



bool clSockServ::Bind(unsigned short usPort)
{
    int iFlag = 1;
    struct sockaddr_in sServAddr;
    
    bLocal = false;
    iListenH = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (iListenH < 0)
    {
        iErrno = errno;
        return false;
    }
    if (setsockopt(iListenH, SOL_SOCKET, SO_REUSEADDR, 
        &iFlag, sizeof(iFlag)) < 0)
    {
        iErrno = errno;
        return false;
    }
    memset(&sServAddr, 0x00, sizeof(sServAddr));
    sServAddr.sin_family = AF_INET;
    sServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sServAddr.sin_port = htons(usPort);
    if (bind(iListenH, (struct sockaddr *) &sServAddr, sizeof(sServAddr)) < 0)
    {
        iErrno = errno;
        return false;
    }
    if (listen(iListenH, SOCKSERV_LISTENQUEUE_LEN) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


bool clSockServ::Bind(const char *cpAddress, unsigned short usPort)
{
    int iFlag = 1;
    struct sockaddr_in sServAddr;

    bLocal = false;
    iListenH = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (iListenH < 0)
    {
        iErrno = errno;
        return false;
    }
    if (setsockopt(iListenH, SOL_SOCKET, SO_REUSEADDR, 
        &iFlag, sizeof(iFlag)) < 0)
    {
        iErrno = errno;
        return false;
    }
    memset(&sServAddr, 0x00, sizeof(sServAddr));
    sServAddr.sin_family = AF_INET;
    if (cpAddress != NULL)
    {
        sServAddr.sin_addr.s_addr = inet_addr(cpAddress);
    }
    else
    {
        sServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    sServAddr.sin_port = htons(usPort);
    if (bind(iListenH, (struct sockaddr *) &sServAddr, sizeof(sServAddr)) < 0)
    {
        iErrno = errno;
        return false;
    }
    if (listen(iListenH, SOCKSERV_LISTENQUEUE_LEN) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


bool clSockServ::Bind(const char *cpSockFile)
{
    #ifndef __QNX__
    int iFlag = 1;
    mode_t mtPrevMask;
    struct sockaddr_un sServAddr;

    bLocal = true;
    if (access(cpSockFile, W_OK) == 0)
    {
        unlink(cpSockFile);
    }
    iListenH = socket(PF_UNIX, SOCK_STREAM, 0);
    if (iListenH < 0)
    {
        iErrno = errno;
        return false;
    }
    if (setsockopt(iListenH, SOL_SOCKET, SO_REUSEADDR, 
        &iFlag, sizeof(iFlag)) < 0)
    {
        iErrno = errno;
        return false;
    }
    memset(&sServAddr, 0x00, sizeof(sServAddr));
    sServAddr.sun_family = AF_UNIX;
    if ((strlen(GLOBAL_SOCKET_PATH) + strlen(cpSockFile)) > UNIX_PATH_MAX)
    {
        iErrno = 0;
        return false;
    }
    sprintf(sServAddr.sun_path, "%s/%s", GLOBAL_SOCKET_PATH, cpSockFile);
    strcpy(cpLocalSockName, sServAddr.sun_path);
    mtPrevMask = umask(0111);
    if (bind(iListenH, (struct sockaddr *) &sServAddr, sizeof(sServAddr)) < 0)
    {
        umask(mtPrevMask);
        iErrno = errno;
        return false;
    }
    umask(mtPrevMask);
    if (listen(iListenH, SOCKSERV_LISTENQUEUE_LEN) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
    #else
    return Bind(atoi(cpSockFile));
    #endif
}


clSockServ::~clSockServ()
{
    Close();
}


int clSockServ::WaitForConnect()
{
    int iClientH;
    socklen_t iAddrLen;
    struct sockaddr_un sClientAddrU;
    struct sockaddr_in sClientAddrI;
    
    if (bLocal)
    {
        iAddrLen = sizeof(sClientAddrU);
        iClientH = accept(iListenH, (struct sockaddr *) &sClientAddrU, 
            &iAddrLen);
    }
    else
    {
        iAddrLen = sizeof(sClientAddrI);
        iClientH = accept(iListenH, (struct sockaddr *) &sClientAddrI, 
            &iAddrLen);
    }
    if (iClientH < 0)
    {
        iErrno = errno;
        return -1;
    }
    return iClientH;
}


int clSockServ::WaitForConnect(int iTimeout)
{
    int iClientH;
    int iSelectResult;
    struct timeval sTimeout;
    struct sockaddr_un sClientAddrU;
    struct sockaddr_in sClientAddrI;
    socklen_t iAddrLen;
    fd_set fdsetListen;

    sTimeout.tv_sec = iTimeout / 1000;
    sTimeout.tv_usec = iTimeout % 1000 * 1000;
    FD_ZERO(&fdsetListen);
    FD_SET(iListenH, &fdsetListen);
    iSelectResult = select(iListenH + 1, &fdsetListen, NULL, NULL, &sTimeout);
    if (iSelectResult > 0)
    {
        if (bLocal)
        {
            iAddrLen = sizeof(sClientAddrU);
            iClientH = accept(iListenH, (struct sockaddr *) &sClientAddrU,
                &iAddrLen);
        }
        else
        {
            iAddrLen = sizeof(sClientAddrI);
            iClientH = accept(iListenH, (struct sockaddr *) &sClientAddrI,
                &iAddrLen);
        }
        if (iClientH < 0)
        {
            iErrno = errno;
            return -1;
        }
        return iClientH;
    }
    else if (iSelectResult == 0)
    {
        iErrno = 0;
        return -1;
    }
    else
    {
        iErrno = errno;
        return -1;
    }
}


void clSockServ::Close ()
{
    if (iListenH >= 0)
    {
        close(iListenH);
        if (bLocal)
        {
            unlink(cpLocalSockName);
        }
    }
}

