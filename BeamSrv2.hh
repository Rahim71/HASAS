/*

    Beamforming input server
    Copyright (C) 2002 Jussi Laako

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
#include <Condition.hh>
#include <Mutex.hh>
#include <Semaphore.hh>

#include <dsp/DSPOp.hh>
#include <dsp/RecDecimator.hh>

#include "Config.h"
#include "LocalMsg.h"
#include "CfgFile.hh"
#include "FreqBeamDipole.hh"
#include "FreqBeamLine.hh"
#include "LogFile.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"
#include "SockServ.hh"

#include "BeamCommon.hh"


#ifndef BEAMSRV2_HH
#define BEAMSRV2_HH

#   define BS_TAG_READY     1
#   define BS_TAG_RES       2


    /// Communicates beamformer parameters between nodes
    bool BeamCommNodeParams (stBeamNodeParams &);
    bool BeamCommInData (GDT *, int);


    /**
        Beamforming input server, root node.
        
        This node reads data from streamdist, distributes it to slave nodes,
        collects beamformed data and sends it to clients.
        Each beam is represented as a channel at output.
    */
    class clBeamSrv2Master
    {
            int iFilterType;
            int iDecimate;
            int iDecFact;
            int iChOffset;
            int iOutDataCount;
            char cpLogBuf[BS_LOGBUFSIZE];
            stRawDataFirst sInHdr;
            stBeamNodeParams sNodeParams;
            clAlloc FiltWork;
            clCfgFile Cfg;
            clDSPOp DSP;
            clLogFile Log;
            clRecDecimator *FilterBank;
            clSockOp SOpIn;
            clSockServ SServ;
            /* Input data, thread shared */
            clReBuffer InBuf;
            clSemaphore SemIn;
            clMutex MtxIn;
            /* Output data, thread shared */
            clAlloc OutData;
            clCondition CndOut;
            clMutex MtxOut;
            /* - */
            bool ReadConfig ();
            bool InitFilterBank ();
            bool InitProcessing ();
            bool WaitReady ();
            void ProcessLoop ();
            void CompactData (GDT *, const GDT *, long, long, long, long);
            bool FilterData (GDT *, const GDT *, long, long, long, long);
        public:
            clBeamSrv2Master ();
            ~clBeamSrv2Master ();
            int Main (int *, char ***);
            static void Abort (int = 0);
            void *InDataThread (void *);
            void *ServerThread (void *);
            void *ServeClientThread (void *);
    };


    /**
        Beamforming input server, slave node.
        
        Slave nodes do the actual beam processing.
    */
    class clBeamSrv2Slave
    {
            int iRank;
            stBeamNodeParams sNodeParams;
            clFreqBeamDipole FBDipole;
            clFreqBeamLine FBLine;
            bool SendReady ();
            bool SendRes (GDT *, int);
        public:
            clBeamSrv2Slave (int);
            ~clBeamSrv2Slave ();
            int Main (int *, char ***);
    };

#endif
