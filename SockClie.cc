/*

    Socket client class
    Copyright (C) 1999-2001 Jussi Laako

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
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Config.h"
#include "SockClie.hh"


#ifndef UNIX_PATH_MAX
    #define UNIX_PATH_MAX           108
#endif


clSockClie::clSockClie()
{
    iErrno = 0;
}


clSockClie::~clSockClie()
{
}


int clSockClie::Connect(const char *cpHostName, const char *cpHostAddr,
    int iHostPort)
{
    int iServH;
    struct sockaddr_in sClientAddr;
    struct in_addr *spInAddr;
    struct hostent *spHostEntry;

    memset(&sClientAddr, 0x00, sizeof(sClientAddr));
    iServH = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (iServH < 0)
    {
        iErrno = errno;
        return -1;
    }
    if (cpHostAddr != NULL)
    {
        sClientAddr.sin_family = AF_INET;
        sClientAddr.sin_port = htons(iHostPort);
        if (inet_aton(cpHostAddr, &sClientAddr.sin_addr) == 0)
        {
            return -1;
        }
    }
    else
    {
        sClientAddr.sin_family = AF_INET;
        sClientAddr.sin_port = htons(iHostPort);
        #ifndef __QNX__
        spHostEntry = gethostbyname2(cpHostName, AF_INET);
        #else
        spHostEntry = gethostbyname(cpHostName);
        #endif
        if (spHostEntry != NULL)
        {
            spInAddr = (struct in_addr *) spHostEntry->h_addr_list[0];
            if (spInAddr != NULL)
            {
                sClientAddr.sin_addr.s_addr = spInAddr->s_addr;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }
    if (connect(iServH, (struct sockaddr *) &sClientAddr, 
        sizeof(sClientAddr)) < 0)
    {
        iErrno = errno;
        return -1;
    }
    return iServH;
}


int clSockClie::Connect(const char *cpSockName)
{
    #ifndef __QNX__
    int iServH;
    struct sockaddr_un sClientAddr;

    memset(&sClientAddr, 0x00, sizeof(sClientAddr));
    iServH = socket(PF_UNIX, SOCK_STREAM, 0);
    if (iServH < 0)
    {
        iErrno = errno;
        return -1;
    }
    sClientAddr.sun_family = AF_UNIX;
    if ((strlen(GLOBAL_SOCKET_PATH) + strlen(cpSockName)) > UNIX_PATH_MAX)
    {
        iErrno = 0;
        return -1;
    }
    sprintf(sClientAddr.sun_path, "%s/%s", GLOBAL_SOCKET_PATH, cpSockName);
    if (connect(iServH, (struct sockaddr *) &sClientAddr,
        sizeof(sClientAddr)) < 0)
    {
        iErrno = errno;
        return -1;
    }
    return iServH;
    #else
    return Connect("localhost", NULL, atoi(cpSockName));
    #endif
}
            
