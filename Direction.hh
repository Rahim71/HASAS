/*

    Direction calculation
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


#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <Alloc.hh>
#include <Condition.hh>
#include <Mutex.hh>
#include <dsp/DSPOp.hh>

#include "Config.h"
#include "LocalMsg.h"
#define AB_COMPILE
#include "ArrayBase.hh"
#include "ArrayDipole.hh"
#include "BeamDipole.hh"
#include "CorrDipole.hh"
#include "CfgFile.hh"
#include "Messages.hh"
#include "RemoveNoise.hh"
#include "SockClie.hh"
#include "SockOp.hh"


#ifndef DIRECTION_HH
    #define DIRECTION_HH


    /**
        Array types
    */
    enum
    {
        DIR_ARRAY_TYPE_DIPOLE = 0,
        DIR_ARRAY_TYPE_TRIANGLE = 1,
        DIR_ARRAY_TYPE_LINE = 2,
        DIR_ARRAY_TYPE_PLANE = 3,
        DIR_ARRAY_TYPE_CYLINDER = 4,
        DIR_ARRAY_TYPE_SPHERE = 5
    };


    /**
        Direction calculation server
    */
    class clDirection
    {
            // Variables
            volatile bool bRun;
            volatile bool bConnected;
            int iProcessorCount;
            int iStartChannel;
            int iArrayType;
            int iShadingType;
            int iHorizSensors;
            int iVertSensors;
            int iRawBufSize;
            volatile int iResultsRefCount;
            long lWindowSize;
            double dTimingReal;
            GDT fSensorSpacing;
            GDT fpHorizSpacing[BF_MAX_X_SENSORS];
            GDT fpVertSpacing[BF_MAX_Y_SENSORS];
            clock_t ctTiming;
            pthread_t tidReceiveData;
            pthread_t tidpProcessData[DIR_MAX_CPUS];
            pthread_t tidSendResults;
            // Structures
            stDirReq sDirRequest;
            stDirRes sDirResHdr;
            stRawDataFirst sDataFirst;
            // Classes
            clCondition CndDataReady;
            clCondition CndResultsReady;
            clMutex MtxClassData;
            clMutex MtxDataReady;
            clMutex MtxResultsReady;
            clCfgFile *CfgStreamDist;
            clCfgFile *CfgDirection;
            clSockClie SClient;
            clSockOp *SOpRequest;
            clSockOp *SOpResult;
            clSockOp *SOpData;
            clDirMsg DirMsg;
            clDSPAlloc RawData;
            clDSPAlloc DirResData;
            clDSPOp DSP;
            clRemoveNoise NoiseEstRem;
            clArrayDipole ArDipole;
            clBeamDipole *BeDipole[DIR_MAX_CPUS];
            clCorrDipole *CoDipole[DIR_MAX_CPUS];
            // Methods
            bool GetRequest ();
            bool GetFirstMsg ();
            bool InitArray ();
            void Stop ();
            void Scale (GDT *);
            void CalcDipoleBeams (int, long, long, bool);
            void CalcDipoleCorrs (int, long, long, bool);
            void StartTiming ();
            void StopTiming ();
        public:
            clDirection (int, int);
            ~clDirection ();
            /**
                Main method, call this only
            */
            int Exec ();
            /**
                Receive data thread
            */
            void *ReceiveData (void *);
            /**
                Dataprocessing thread (there can be multiple instances of these)
            */
            void *ProcessData (void *);
            /**
                Send results thread
            */
            void *SendResults (void *);
    };

#endif

