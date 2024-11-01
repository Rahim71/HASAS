/*

    Spectrum server
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


#include <Alloc.hh>
#include <dsp/DSPOp.hh>
#include <dsp/Filter.hh>
#include <dsp/Hankel.hh>
#include <dsp/RecDecimator.hh>
#ifndef DSP_USE_DEQUE
#include <dsp/ReBuffer.hh>
#else
#include <dsp/ReBuffer2.hh>
#endif

#include "Config.h"
#include "LocalMsg.h"
#include "CfgFile.hh"
#include "Messages.hh"
#include "RemoveNoise.hh"
#include "SockClie.hh"
#include "SockOp.hh"


#ifndef SPECTRUM_HH
    #define SPECTRUM_HH


    /**
        Spectrum server
    */
    class clSpectrum
    {
            bool bRun;
            bool bReverseOrder;
            int iFilterType;
            int iInDataSize;
            long lInDataCount;
            long lFilterSize;
            long lDecimation;
            long lOldDataCount;
            long lNewDataCount;
            long lSpectPoints;
            long lSpectLen;
            long lResSize;
            float fLowCorner;
            float fHighCorner;
            char cpStreamSocket[_POSIX_PATH_MAX + 1];
            stSpectReq sReq;
            stSpectRes sResHdr;
            stRawDataFirst sRawHdr;
            clDSPAlloc InData;
            clDSPAlloc Window;
            clDSPAlloc SpectIn;
            clDSPAlloc SpectOut;
            clDSPAlloc ResMsg;
            clCfgFile Cfg;
            clDSPOp DSP;
            clDSPOp FFT;
            clHankel Hankel;
            #ifndef DSP_USE_DEQUE
            clReBuffer ReBuffer;
            #else
            clReBuffer2 ReBuffer;
            #endif
            clRecDecimator Decimator;
            clRecDecimator WVDDec;
            clRemoveNoise RN;
            clSpectMsg Msg;
            clSockClie SClient;
            clSockOp SOpRequest;
            clSockOp SOpResult;
            clSockOp SOpData;
            bool GetCfg ();
            bool GetRq ();
            bool ConnectStream ();
            bool Init ();
            void CreateFilter ();
            void ProcessLoop ();
            bool GetData ();
            bool PullData (GDT *, long);
        public:
            clSpectrum (int, int);
            ~clSpectrum ();
            int Main ();
            void Stop () { bRun = false; }
    };

#endif
