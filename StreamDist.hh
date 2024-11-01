/*

    Audio stream distributor
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
#include <Condition.hh>
#include <Mutex.hh>
#ifdef USE_RWLOCK
#include <RWLock.hh>
#endif
#ifdef USE_FLAC
    #include <FLAC/all.h>
#endif

#include "Config.h"
#include "LocalMsg.h"
#include "CfgFile.hh"
#include "LogFile.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"
#include "SockServ.hh"


#ifndef STREAMDIST_HH
    #define STREAMDIST_HH

    #define SD_LOGENTRY_SIZE        256
    #define SD_HOSTADDR_MAX         256


    /**
        Audio stream distributor

        \note See soundsrv2 for architecture description.
    */
    class clStreamDist
    {
            volatile bool bRun;
            volatile int iFragmentSize;
            volatile int iAudioBufSize;
            volatile int iBlockCntr;
            stRawDataFirst sHdr;
            #ifdef USE_FLAC
            FLAC__StreamDecoder *spFLACDec;
            #endif
            clCfgFile Cfg;
            clAlloc AudioBuf;
            clMutex MtxAudio;
            #ifdef USE_RWLOCK
            clRWLock RWLAudio;
            #endif
            #ifdef USE_FLAC
            clSockOp *InSOp;
            #endif
            clCondition CndAudio;
            void CopyChannel (GDT *, const GDT *, int);
            bool InitCompress (int);
        public:
            clLogFile Log;
            clStreamDist ();
            ~clStreamDist ();
            int Main (int, char **);
            void *AudioInThread (void *);
            void *ServeClientThread (void *);
            void Stop () { bRun = false; }
            #ifdef USE_FLAC
            FLAC__StreamDecoderReadStatus FLACRead (
                const FLAC__StreamDecoder *, FLAC__byte *, unsigned *);
            FLAC__StreamDecoderWriteStatus FLACWrite (
                const FLAC__StreamDecoder *, const FLAC__Frame *, 
                const FLAC__int32 * const *);
            void FLACMetaData (
                const FLAC__StreamDecoder *, const FLAC__StreamMetadata *);
            void FLACError (
                const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus);
            #endif
    };
    
#endif

