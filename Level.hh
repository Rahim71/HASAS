/*

    Level server, header
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


#include <limits.h>

#include <Alloc.hh>
#include <dsp/DSPOp.hh>
#include <dsp/Filter2.hh>

#include "Config.h"
#include "LocalMsg.h"
#include "CfgFile.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"


#ifndef LEVEL_HH
    #define LEVEL_HH

    #define LEVEL_FILTER_WINDOW         true


    /**
        Level server
    */
    class clLevel
    {
            bool bRun;
            long lFilterSize;
            long lRawDataCount;
            long lRawBufSize;
            long lSampleCount;
            char cpStreamSocket[_POSIX_PATH_MAX + 1];
            GDT fResPeakLevel;
            stRawDataFirst sRawHdr;
            stLevelReq sRequest;
            stLevelRes sResult;
            clDSPAlloc RawBuf;
            clDSPAlloc DataBuf;
            clCfgFile CfgFile;
            clDSPOp DSP;
            clFilter2 Filter;
            clSockClie SClient;
            clSockOp SOpRequest;
            clSockOp SOpResult;
            clSockOp SOpRaw;
            clLevelMsg Msg;
            bool GetCfg ();
            bool GetRequest ();
            bool ConnectStream ();
            bool Initialize ();
            bool ReceiveData ();
            bool Process ();
            bool SendResult ();
        public:
            clLevel (int, int);
            ~clLevel ();
            int Main (int, char **);
    };

#endif

