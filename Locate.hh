/*

    Algorithm for locating sound sources in array field
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


#include <Alloc.hh>
#include <Condition.hh>
#include <Mutex.hh>
#include <dsp/DSPOp.hh>

#include "Config.h"
#include "CfgFile.hh"
#include "Cluster.hh"
#include "LocateSensor.hh"
#include "LocateSystem.hh"
#include "LogFile.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"
#include "SockServ.hh"


#ifndef LOCATE_HH
    #define LOCATE_HH

    #define LOCATE_HOSTNAME_MAXLEN      256
    #define LOCATE_CONTROL_TAG          1
    #define LOCATE_PARAM_TAG            2
    #define LOCATE_NORMAL_TAG           3

    #define LOCATE_CTRL_ACK             0
    #define LOCATE_CTRL_STOP            1


    /**
        Configuration information message for subnodes.
    */
    typedef struct _stDirCfg
    {
        long lWindowSize;
        float fSoundSpeed;
        float fLowFrequency;
        float fIntegrationTime;
        int iScaling;
        float fScalingExp;
        int iRemoveNoise;
        float fAlpha;
        long lMeanLength;
        long lGapLength;
    } stDirCfg, *stpDirCfg;
    

    /**
        Master node for locate server.
    */
    class clLocate
    {
            volatile bool bRun;
            int iSensorCount;
            int iPort;
            int iMsgSize;
            long lWidth;
            long lHeight;
            GDT fWeight;
            GDT *fpLocMatrix;
            stDirCfg sDirCfg;
            clAlloc LocMatrix;
            clAlloc ResMsg;
            clCondition CndResMsg;
            clMutex MtxResMsg;
            clCfgFile Cfg;
            clDSPOp DSP;
            clLogFile Log;
            clLocateMsg Msg;
            clMPIComm MPICCtrl;
            clMPIComm MPICParam;
            clMPIComm MPICNormal;
            bool Initialize ();
            void SendParams (int, const char *, int, long, long, float);
        public:
            clLocate ();
            ~clLocate ();
            int Main (int, char **);
            void Stop ();
            void *ProcessThread (void *);
            void *ServeClientThread (void *);
    };


    /**
        Slave nodes for locate server.
    */
    class clSubLocate
    {
            int iHostPort;
            int iDirMsgSize;
            long lWidth;
            long lHeight;
            long lPosX;
            long lPosY;
            float fAzimuth;
            char cpHostName[LOCATE_HOSTNAME_MAXLEN];
            GDT *fpLevRes;
            GDT *fpDirRes;
            stDirCfg sDirCfg;
            clAlloc DirMsg;
            clAlloc LevRes;
            clAlloc DirRes;
            clLocateSensor LocSens;
            clMPIComm MPICCtrl;
            clMPIComm MPICParam;
            clMPIComm MPICNormal;
            clDirMsg2 Msg;
            clSockOp SOp;
            bool RecvParams ();
            bool Initialize ();
            bool ConnectDir ();
        public:
            clSubLocate ();
            ~clSubLocate ();
            int Main (int, char **);
    };

#endif

