/*

    Server for beamformed audio
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

#include <dsp/DSPOp.hh>

#include "Config.h"
#include "LocalMsg.h"
#include "CfgFile.hh"
#include "FreqBeamDipole.hh"
#include "FreqBeamLine.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"


#ifndef BEAMAUDIO_HH
    #define BEAMAUDIO_HH


    enum
    {    
        BA_ARRAY_TYPE_DIPOLE = 0,
        BA_ARRAY_TYPE_TRIANGLE = 1,
        BA_ARRAY_TYPE_LINE = 2,
        BA_ARRAY_TYPE_PLANE = 3,
        BA_ARRAY_TYPE_CYLINDER = 4,
        BA_ARRAY_TYPE_SPHERE = 5
    };
    

    /**
        Server for beamformed audio
    */
    class clBeamAudio
    {
            bool bRun;
            // Configuration parameters
            int iArrayType;
            int iShadingType;
            long lStartCh;
            long lWindowSize;
            long lSensorCount;
            GDT fSensorSpacing;
            char cpStreamSocket[_POSIX_PATH_MAX + 1];
            //-
            GDT fAttenuation;
            stBeamAudioReq sRequest;
            stBeamAudioRes sResHdr;
            stRawDataFirst sRawHdr;
            clCfgFile CfgFile;
            clBeamAudioMsg Msg;
            clDSPOp DSP;
            clSockClie SClient;
            clSockOp SOpRaw;
            clSockOp SOpRequest;
            clSockOp SOpResult;
            clFreqBeamDipole FBeamDipole;
            clFreqBeamLine FBeamLine;
            bool GetCfg ();
            bool GetRq ();
            bool ConnectStream ();
            bool InitBeam ();
            bool SendFirst ();
            void SetDirection ();
            void ProcessLoop ();
            bool SendResult (GDT *, long);
        public:
            clBeamAudio (int, int);
            ~clBeamAudio ();
            int Main ();
            void Stop () { bRun = false; }
    };

#endif

