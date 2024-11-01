/*

    Spectrum based direction server, header
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


#include <limits.h>

#include <dsp/DSPOp.hh>

#include "Config.h"
#include "LocalMsg.h"
#include "CfgFile.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"
#include "SpectDirDipole.hh"
#include "SpectDirLine.hh"


#ifndef DIRECTION2_HH
    #define DIRECTION2_HH


    /**
        Array types
    */
    enum
    {
        DIR2_ARRAY_TYPE_DIPOLE = 0,
        DIR2_ARRAY_TYPE_TRIANGLE = 1,
        DIR2_ARRAY_TYPE_LINE = 2,
        DIR2_ARRAY_TYPE_PLANE = 3,
        DIR2_ARRAY_TYPE_CYLINDER = 4,
        DIR2_ARRAY_TYPE_SPHERE = 5
    };


    /**
        Spectrum based direction server
    */
    class clDirection2
    {
            bool bRun;
            int iFilterType;
            int iArrayType;
            long lSensorCount;
            long lFilterSize;
            long lWindowSize;
            long lStartChannel;
            GDT fSensorSpacing;
            char cpStreamSocket[_POSIX_PATH_MAX + 1];
            stDirReq sReq;
            stDirRes sResHdr;
            stRawDataFirst sRawHdr;
            clCfgFile Cfg;
            clDSPOp DSP;
            clDirMsg Msg;
            clSockClie SClient;
            clSockOp SOpRequest;
            clSockOp SOpResult;
            clSockOp SOpData;
            clSpectDirDipole *SDDipole;
            clSpectDirLine *SDLine;
            bool GetCfg ();
            bool GetRq ();
            bool ConnectStream ();
            bool InitDir ();
            void ProcessLoop ();
        public:
            clDirection2 (int, int);
            ~clDirection2 ();
            int Main ();
    };

#endif

