/*

    Sound service proxy, header
    Copyright (C) 2000-2001 Jussi Laako

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

#include <Alloc.hh>
#include <Condition.hh>
#include <Mutex.hh>

#include "Config.h"
#include "CfgFile.hh"
#include "LogFile.hh"
#include "SockClie.hh"
#include "SockOp.hh"
#include "SockServ.hh"


#ifndef SOUNDPROXY_HH
    #define SOUNDPROXY_HH

    #define SP_CONV_BUF_LEN         255


    /**
        Sound service proxy

        \note See soundsrv2 for architecture description.
    */
    class clSoundProxy
    {
            volatile bool bRun;  // Protected
            bool bServeClient[SP_MAXCLIENTS];  // Protected
            int iServerPort;
            int iServicePort;
            int iClientSockH[SP_MAXCLIENTS];  // Protected
            char cpLogFile[_POSIX_PATH_MAX + 1];
            char cpServerHost[SP_SERV_MAXLEN + 1];
            char *cpFirstMsg;  // Protected
            char *cpDataMsg;  // Protected
            pthread_t ptidSoundIn;
            pthread_t ptidWaitConnect;
            pthread_t ptidServeClient[SP_MAXCLIENTS];
            clAlloc FirstMsg;
            clAlloc DataMsg;
            clCfgFile Cfg;
            clCondition CondData;
            clLogFile Log;
            clMutex MutexData;
            clMutex MutexClass;
            void AddToLog (char, const char *);
            void AddToLog (char, const char *, int);
            int FindFreeSlot ();
        public:
            clSoundProxy ();
            ~clSoundProxy ();
            int Exec ();
            void Stop ();
            void *SoundInThread (void *);
            void *WaitConnectThread (void *);
            void *ServeClientThread (void *);
    };

#endif

