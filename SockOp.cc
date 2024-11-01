/*

    Socket I/O operations
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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include "SockOp.hh"


clSockOp::clSockOp()
{
    bCloseOnDestruct = true;
    iSockH = -1;
}


clSockOp::clSockOp(int iHandle)
{
    bCloseOnDestruct = true;
    iSockH = iHandle;
}


clSockOp::~clSockOp()
{
    if (bCloseOnDestruct && (iSockH >= 0)) close(iSockH);
}


void clSockOp::SetHandle(int iHandle)
{
    if (iSockH >= 0) Close();
    iSockH = iHandle;
}


void clSockOp::Close()
{
    if (iSockH >= 0) 
    {
        close(iSockH);
        iSockH = -1;
    }
}


bool clSockOp::ReadSelect(int iTimeoutMS)
{
    int iSelectRes;
    struct timeval sTimeout;
    fd_set fdsetSelect;

    sTimeout.tv_sec = iTimeoutMS / 1000;
    sTimeout.tv_usec = iTimeoutMS % 1000 * 1000;
    FD_ZERO(&fdsetSelect);
    FD_SET(iSockH, &fdsetSelect);
    iSelectRes = select(iSockH + 1, &fdsetSelect, NULL, NULL, &sTimeout);
    if (iSelectRes == 0)
    {
        iErrno = 0;
        return false;
    }
    else if (iSelectRes < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


bool clSockOp::WriteSelect(int iTimeoutMS)
{
    int iSelectRes;
    struct timeval sTimeout;
    fd_set fdsetSelect;

    sTimeout.tv_sec = iTimeoutMS / 1000;
    sTimeout.tv_usec = iTimeoutMS % 1000 * 1000;
    FD_ZERO(&fdsetSelect);
    FD_SET(iSockH, &fdsetSelect);
    iSelectRes = select(iSockH + 1, NULL, &fdsetSelect, NULL, &sTimeout);
    if (iSelectRes == 0)
    {
        iErrno = 0;
        return false;
    }
    else if (iSelectRes < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


int clSockOp::Read(void *vpBuf, int iBufLen)
{
    iRetVal = read(iSockH, vpBuf, iBufLen);
    iErrno = errno;
    return iRetVal;
}


int clSockOp::Write(const void *vpBuf, int iBufLen)
{
    iRetVal = write(iSockH, vpBuf, iBufLen);
    iErrno = errno;
    return iRetVal;
}


int clSockOp::ReadN(void *vpBuf, int iBufLen)
{
    int iBytesRead = 0;
    char *cpBufPtr = (char *) vpBuf;

    do {
#       ifndef LINUXSYS
        iRetVal = recv(iSockH, &cpBufPtr[iBytesRead], iBufLen - iBytesRead, 0);
#       else
        iRetVal = recv(iSockH, &cpBufPtr[iBytesRead], iBufLen - iBytesRead,
            MSG_NOSIGNAL);
#       endif
        iBytesRead += iRetVal;
    } while (iBytesRead < iBufLen && iRetVal > 0);  // was iRetVal >= 0
    iErrno = errno;
    return iBytesRead;
}


int clSockOp::WriteN(const void *vpBuf, int iBufLen)
{
    int iBytesWritten = 0;
    char *cpBufPtr = (char *) vpBuf;

    do {
#       ifndef LINUXSYS
        iRetVal = send(iSockH, &cpBufPtr[iBytesWritten], 
            iBufLen - iBytesWritten, 0);
#       else
        iRetVal = send(iSockH, &cpBufPtr[iBytesWritten], 
            iBufLen - iBytesWritten, MSG_NOSIGNAL);
#       endif
        iBytesWritten += iRetVal;
    } while (iBytesWritten < iBufLen && iRetVal > 0);  // was iRetVal >= 0
    iErrno = errno;
    return iBytesWritten;
}


int clSockOp::Receive(void *vpBuf, int iBufLen, int iFlags)
{
    iRetVal = recv(iSockH, vpBuf, iBufLen, iFlags);
    iErrno = errno;
    return iRetVal;
}


int clSockOp::Send(const void *vpBuf, int iBufLen, int iFlags)
{
    iRetVal = send(iSockH, vpBuf, iBufLen, iFlags);
    iErrno = errno;
    return iRetVal;
}


int clSockOp::ReceiveMsg(struct msghdr *sMsgHdr, int iFlags)
{
    iRetVal = recvmsg(iSockH, sMsgHdr, iFlags);
    iErrno = errno;
    return iRetVal;
}


int clSockOp::SendMsg(const struct msghdr *sMsgHdr, int iFlags)
{
    iRetVal = sendmsg(iSockH, sMsgHdr, iFlags);
    iErrno = errno;
    return iRetVal;
}


int clSockOp::Shutdown(int iShutdownDir)
{
    iRetVal = shutdown(iSockH, iShutdownDir);
    iErrno = errno;
    return iRetVal;
}


int clSockOp::GetSockName(struct sockaddr *spAddr, socklen_t *ipAddrLen)
{
    iRetVal = getsockname(iSockH, spAddr, ipAddrLen);
    iErrno = errno;
    return iRetVal;
}


int clSockOp::GetPeerName(struct sockaddr *spAddr, socklen_t *ipAddrLen)
{
    iRetVal = getpeername(iSockH, spAddr, ipAddrLen);
    iErrno = errno;
    return iRetVal;
}


bool clSockOp::SetBlocking(bool bBlocking)
{
    int iMode;

    iMode = fcntl(iSockH, F_GETFL);
    if (iMode < 0)
    {
        iErrno = errno;
        return false;
    }
    if (bBlocking)
    {
        iMode &= ~(O_NONBLOCK);
    }
    else
    {
        iMode |= O_NONBLOCK;
    }
    if (fcntl(iSockH, F_SETFL, iMode) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


bool clSockOp::SetKeepAlive()
{
    int iFlag = 1;
    
    if (setsockopt(iSockH, SOL_SOCKET, SO_KEEPALIVE, &iFlag, 
        sizeof(iFlag)) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


bool clSockOp::SetLinger(int iLingerTime)
{
    struct linger sLinger;

    sLinger.l_onoff = 1;
    sLinger.l_linger = iLingerTime;
    if (setsockopt(iSockH, SOL_SOCKET, SO_LINGER, &sLinger, 
        sizeof(sLinger)) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


int clSockOp::GetRecvBufSize()
{
    int iRecvBufSize;
    socklen_t iValSize;

    iValSize = sizeof(iRecvBufSize);
    if (getsockopt(iSockH, SOL_SOCKET, SO_RCVBUF, &iRecvBufSize, 
        &iValSize) < 0)
    {
        iErrno = errno;
        return -1;
    }
    return iRecvBufSize;
}


bool clSockOp::SetRecvBufSize(int iBufSize)
{
    if (setsockopt(iSockH, SOL_SOCKET, SO_RCVBUF, &iBufSize, 
        sizeof(iBufSize)) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


int clSockOp::GetSendBufSize()
{
    int iSendBufSize;
    socklen_t iValSize;

    iValSize = sizeof(iSendBufSize);
    if (getsockopt(iSockH, SOL_SOCKET, SO_SNDBUF, &iSendBufSize, 
        &iValSize) < 0)
    {
        iErrno = errno;
        return -1;
    }
    return iSendBufSize;
}


bool clSockOp::SetSendBufSize(int iBufSize)
{
    if (setsockopt(iSockH, SOL_SOCKET, SO_SNDBUF, &iBufSize,
        sizeof(iBufSize)) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


int clSockOp::GetRecvBufLowWater()
{
    int iBufLowWater;
    socklen_t iValSize;

    iValSize = sizeof(iBufLowWater);
    if (getsockopt(iSockH, SOL_SOCKET, SO_RCVLOWAT, &iBufLowWater,
        &iValSize) < 0)
    {
        iErrno = errno;
        return -1;
    }
    return iBufLowWater;
}


bool clSockOp::SetRecvBufLowWater(int iBufLowWater)
{
    if (setsockopt(iSockH, SOL_SOCKET, SO_RCVLOWAT, &iBufLowWater,
        sizeof(iBufLowWater)) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


int clSockOp::GetSendBufLowWater()
{
    int iBufLowWater;
    socklen_t iValSize;

    iValSize = sizeof(iBufLowWater);
    if (getsockopt(iSockH, SOL_SOCKET, SO_SNDLOWAT, &iBufLowWater,
        &iValSize) < 0)
    {
        iErrno = errno;
        return -1;
    }
    return iBufLowWater;
}


bool clSockOp::SetSendBufLowWater(int iBufLowWater)
{
    if (setsockopt(iSockH, SOL_SOCKET, SO_SNDLOWAT, &iBufLowWater,
        sizeof(iBufLowWater)) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


bool clSockOp::SetRecvTimeout(int iTimeoutMS)
{
    struct timeval sTimeout;

    sTimeout.tv_sec = iTimeoutMS / 1000;
    sTimeout.tv_usec = iTimeoutMS % 1000 * 1000;
    if (setsockopt(iSockH, SOL_SOCKET, SO_RCVTIMEO, &sTimeout,
        sizeof(sTimeout)) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


bool clSockOp::SetSendTimeout(int iTimeoutMS)
{
    struct timeval sTimeout;

    sTimeout.tv_sec = iTimeoutMS / 1000;
    sTimeout.tv_usec = iTimeoutMS % 1000 * 1000;
    if (setsockopt(iSockH, SOL_SOCKET, SO_SNDTIMEO, &sTimeout,
        sizeof(sTimeout)) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


bool clSockOp::SetTCPKeepAlive(int iTimeSec)
{
    #ifdef HAVE_TCP_KEEPALIVE
    if (setsockopt(iSockH, IPPROTO_TCP, TCP_KEEPALIVE, &iTimeSec,
        sizeof(iTimeSec)) < 0)
    {
        iErrno = errno;
        return false;
    }
    #endif
    return true;
}


bool clSockOp::DisableNagle()
{
    int iFlag = 1;
    
    if (setsockopt(iSockH, IPPROTO_TCP, TCP_NODELAY, &iFlag, 
        sizeof(iFlag)) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


bool clSockOp::SetNagle(bool bNagle)
{
    int iFlag = (bNagle) ? 0 : 1;
    
    if (setsockopt(iSockH, IPPROTO_TCP, TCP_NODELAY, &iFlag,
        sizeof(iFlag)) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


bool clSockOp::SetTypeOfService(int iTOS)
{
    if (setsockopt(iSockH, IPPROTO_IP, IP_TOS, &iTOS,
        sizeof(iTOS)) < 0)
    {
        iErrno = errno;
        return false;
    }
    return true;
}


void clSockOp::SetCloseOnDestruct(bool bSetting)
{
    bCloseOnDestruct = bSetting;
}

