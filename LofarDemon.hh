/*

    LOFAR/DEMON calculation server, header
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


#include <Alloc.hh>
#include <dsp/DSPOp.hh>
#include <dsp/Filter2.hh>
#include <dsp/ReBuffer.hh>
#include <dsp/RecDecimator.hh>
#include <dsp/IIRCascade.hh>

#include "Config.h"
#include "LocalMsg.h"
#include "CfgFile.hh"
#include "Messages.hh"
#include "RemoveNoise.hh"
#include "SockClie.hh"
#include "SockOp.hh"


#ifndef LOFARDEMON_HH
    #define LOFARDEMON_HH


    /**
        LOFAR/DEMON calculation server
    */
    class clLofarDemon
    {
            bool bReverseOrder;
            int iFilterType;
            int iDCBlock;
            int iNormIdx;
            int iNormHistCount;
            long lFilterSize;
            long lDecimation;
            long lDemonDecimation;
            long lWinSize;
            long lSpectSize;
            long lNewDataSize;
            long lOldDataSize;
            long lAvgCntr;
            long lHistCntr;
            GDT *fpWinFunc;
            GDT *fpDataBuf;
            GDT *fpSpectRes;
            GDT *fpLofarRes;
            GDT *fpNormHist;
            stRawDataFirst sDataHdr;
            stLofarReq sLofarRq;
            stLofarRes sLofarResHdr;
            clDSPAlloc WinFunc;
            clDSPAlloc DataBuf;
            clDSPAlloc SpectRes;
            clDSPAlloc LofarRes;
            clDSPAlloc HistoryBuf;
            clDSPAlloc NormHist;
            clCfgFile Cfg;
            clDSPOp DSP;
            clLofarMsg LofarMsg;
            clReBuffer ReBuffer;  ///< This is used instead of decimator if decimation factor is < 2
            clRecDecimator Decimator;  ///< This is always the last decimation
            clRecDecimator DemonDecimator;  ///< DEMON BP decimation
            clIIRCascade DCBlockIIR;  ///< DC notch for DEMON
            clRemoveNoise RemoveNoise;
            clSockClie SockClie;
            clSockOp SOpRequest;
            clSockOp SOpResult;
            clSockOp SOpData;
            bool GetRequestMsg ();
            bool GetDataHdr ();
            bool Initialize ();
            bool InitFilter ();
            bool InitFFT ();
            int MainLoop ();
            void CalcSpect ();
            bool SendResults ();
            void PutData (GDT *, long);
            bool PullData (GDT *, long);
            void DemonProc (GDT *, long);
            void SpectStdDev (GDT *);
        public:
            volatile bool bRun;
            clLofarDemon (int, int);
            ~clLofarDemon ();
            int Exec ();
    };

#endif

