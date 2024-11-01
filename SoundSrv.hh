/*

    Sound card input server, header
    Copyright (C) 1999-2001 Jussi Laako

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
#include <signal.h>
#include <pth.h>

#include <PthMutex.hh>
#include <PthCond.hh>
#include <dsp/DSPOp.hh>

#include "Messages.hh"
#include "CfgFile.hh"
#include "LogFile.hh"
#include "Audio.hh"


#ifndef SOUNDSRV_HH
    #define SOUNDSRV_HH

    #define SS_PTH_MAJ      ((PTH_VERSION&0xf00000)>>20)
    #define SS_PTH_MIN      ((PTH_VERSION&0x0ff000)>>12)
    #define SS_PTH_PL       (PTH_VERSION&0x0000ff)


    /**
        Sound card input server.

        This uses great GNU Pth library for threads. This seems to be more
        efficient in some cases than soundsrv2 due to nonpre-emptive
        behaviour of used threading library. It, however, requires select()
        support from soundcard which, unfortunately, all soundcard drivers
        don't support...
    */
    class clSoundSrv
    {
            int iAudioFrmt;
            int iAudioSr;
            int iAudioCh;
            int iAudioTypeSize;
            int iErrorCount;
            int iClientIdx;
            int ipClientH[SS_MAXCLIENTS];
            int iSampleCount;
            int iOutBufSize;
            bool bRun;
            char cpProgName[_POSIX_PATH_MAX];
            char *cpOutBuf;
            sigset_t sigsetQuit;
            pth_t tidInput;
            pth_t tidWaitConnect;
            pth_t ptidServeClient[SS_MAXCLIENTS];
            clPthMutex MutexThis;
            clPthCond CondDataAvail;
            clSoundMsg SoundMsg;
            clDSPOp DSP;
            clCfgFile *CfgFile;
            clLogFile *LogFile;
            clAudio *Audio;
            int FindFreeClient();
            int FindThisHandle(int);
        public:
            clSoundSrv (const char *, const char *);
                // (audio device)
            ~clSoundSrv ();
            void Exec ();
            void Quit ();
            void Quit (int);
            void InputExec ();
            void WaitConnectExec ();
            void ServeClientExec (void *);
            bool Run () { return bRun; }
    };

#endif

