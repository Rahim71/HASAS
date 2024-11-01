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


#include <sys/types.h>

#include <Alloc.hh>
#include <Condition.hh>
#include <Mutex.hh>
#include <Semaphore.hh>

#include <dsp/DSPOp.hh>
#include <dsp/ReBuffer.hh>
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


#ifndef BEAMSRV_HH
#define BEAMSRV_HH

#   define BS_PROCINFOV_T       std::vector<stBeamProcInfo>


    typedef struct _stBeamProcInfo
    {
        pid_t pidProc;
        int iSockH;
    } stBeamProcInfo, *stpBeamProcInfo;


    /**
        Beamforming input server, main process.
        
        This process reads data from streamdist, distributes it to slave
        processes, collects beamformed data and sends it to clients.
        Each beam is represented as a channel at output.
    */
    class clBeamSrvMaster
    {
            int iFilterType;
            int iDecimate;
            int iDecFact;
            int iChOffset;
            int iOutDataCount;
            volatile int iBlockCntr;
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
            void ProcessLoop ();
            bool SendNodeParams ();
            bool WaitReady ();
            bool SendInData (const GDT *, int);
            void CompactData (GDT *, const GDT *, long, long, long, long);
            bool FilterData (GDT *, const GDT *, long, long, long, long);
        public:
            clBeamSrvMaster ();
            ~clBeamSrvMaster ();
            int Main (int *, char ***);
            static void Abort ();
            void *InDataThread (void *);
            void *ServerThread (void *);
            void *ServeClientThread (void *);
    };


    /**
        Beamforming input server, slave process.
        
        Slave processes do the actual beam processing.
    */
    class clBeamSrvSlave
    {
            int iProcess;
            int iSockH;
            stBeamNodeParams sNodeParams;
            clFreqBeamDipole FBDipole;
            clFreqBeamLine FBLine;
            bool RecvParams ();
            bool RecvInData (GDT *, int);
            bool SendReady ();
            bool SendRes (const GDT *, int);
        public:
            clBeamSrvSlave (int, int);
            ~clBeamSrvSlave ();
            int Main (int *, char ***);
    };

#endif
