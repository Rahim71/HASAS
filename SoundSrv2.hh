/*

    SoundServer2
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
#include <dsp/DSPOp.hh>

#include "Config.h"
#include "Audio.hh"
#include "CfgFile.hh"
#include "LogFile.hh"
#include "Messages.hh"
#include "SockServ.hh"
#include "SockOp.hh"


#ifndef SOUNDSRV2_HH
    #define SOUNDSRV2_HH

    #define SS2_LOGENTRY_SIZE       256

    #define SS2_OSS_MAJOR(x)        ((x >> 16) & 0xff)
    #define SS2_OSS_MINOR(x)        ((x >> 8) & 0xff)
    #define SS2_OSS_PL(x)           (x & 0xff)


    /**
        SoundServer2

        This uses freely spinning architecture based on condition variables.
        When thread gets buffer read from the soundcard it copies it's
        contents to shared buffer and broadcasts condition. Then all
        client servicing threads (which are waiting for condition) copy it's
        contents to private buffers and start sending out. Thus, if one
        of client servicing threads get stuck on blocking socket it doesn't
        take any CPU time and just misses n blocks of data.

        Soundproxy and streamdist also use this architecture. This ensures,
        that one client can't affect others performance.
    */
    class clSoundSrv2
    {
            volatile bool bRun;
            volatile int iAudioBufSize;
            volatile int iBlockCntr;
            stSoundStart sHdr;
            #ifdef USE_FLAC
            FLAC__StreamEncoder *spFLACEnc;
            #endif
            clAlloc AudioBuf;
            clAlloc CompHead;
            clAlloc FLACFrame;
            clMutex MtxAudio;
            #ifdef USE_RWLOCK
            clRWLock RWLAudio;
            #endif
            clCondition CndAudio;
            clCfgFile Cfg;
            bool GetAudioCfg (char *, int *, int *, int *, int *);
            bool InitCompress (int, int, int, int, int);
            #ifdef USE_FLAC
            void Convert (FLAC__int32 *, const void *, long, int);
            #endif
        public:
            clLogFile Log;
            clSoundSrv2 ();
            ~clSoundSrv2 ();
            int Main (int, char **);
            void *AudioInThread (void *);
            void *ServeClientThread (void *);
            void Stop() { bRun = false; }
            #ifdef USE_FLAC
            FLAC__StreamEncoderWriteStatus FLACWrite (
                const FLAC__StreamEncoder *, const FLAC__byte *, unsigned,
                unsigned, unsigned);
            void FLACMetaData (const FLAC__StreamEncoder *, 
                const FLAC__StreamMetadata *);
            #endif
    };

#endif
