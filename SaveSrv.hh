/*

    Save server
    Copyright (C) 2001-2002 Jussi Laako

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


#include <stdio.h>
#include <limits.h>
#include <time.h>

#include <sndfile.h>
#include <Alloc.hh>
#include <dsp/DSPOp.hh>
#ifdef USE_FLAC
    #include <FLAC/all.h>
#endif
#ifdef USE_MERSENNE_TWISTER
#include <MersenneTwister.h>
#endif

#include "Config.h"
#include "LocalMsg.h"
#include "CfgFile.hh"
#include "LogFile.hh"
#include "SockOp.hh"


#ifndef SAVESRV_HH
#define SAVESRV_HH

    #define SAVS_SNDFILE_MSGLEN     255
    #define SAVS_LOGENTRY_SIZE      255
    #define SAVS_TIMELEN            12
    #define SAVS_DEF_FRAMELEN       8192
    #define SAVS_DEF_FILETIME       60


    /**
        File format
    */
    enum
    {
        SAVS_FORMAT_WAV = 0,
        SAVS_FORMAT_AIFF = 1,
        SAVS_FORMAT_FLAC = 2
    };

    /**
        File datatype
    */
    enum
    {
        SAVS_TYPE_PCM = 0,
        SAVS_TYPE_FLOAT = 1,
        SAVS_TYPE_ADPCM = 2,
        SAVS_TYPE_MSADPCM = 3
    };


    /**
        Save server
    */
    class clSaveSrv
    {
            bool bRun;
            int iFormat;
            int iType;
            int iBits;
            int iDither;
            int iFileTime;
            int iFileH;
            unsigned int uiRndSeed;
            long lFrameLen;
            long lFrameSize;
            char cpSockName[_POSIX_PATH_MAX + 1];
            char cpSavePath[_POSIX_PATH_MAX + 1];
            time_t ttFileStarted;
            stRawDataFirst sDataHdr;
            SNDFILE *spSndFile;
            #ifdef USE_FLAC
            FLAC__StreamEncoder *spFLACEnc;
            #endif
            clAlloc InFrame;
            clAlloc OutFrame;
            clAlloc NoiseFrame;
            clAlloc FLACFrame;
            clCfgFile Cfg;
            clDSPOp DSP;
            clSockOp SOp;
            #ifdef USE_MERSENNE_TWISTER
            MTRand *MTR;
            #endif
            bool Initialize ();
            bool ConnectStream ();
            bool CreateFile ();
            bool CreateFile2 ();
            void CreateFileName (char *);
            void ProcessLoop ();
            void Dither ();
            bool WriteData ();
            bool WriteData2 ();
        public:
            clLogFile Log;
            clSaveSrv ();
            ~clSaveSrv ();
            int Main (int, char **);
            void Stop () { bRun = false; }
            #ifdef USE_FLAC
            FLAC__StreamEncoderWriteStatus FLACWrite (
                const FLAC__StreamEncoder *, const FLAC__byte *, unsigned,
                unsigned, unsigned);
            void FLACMetaData (const FLAC__StreamEncoder *, 
                const FLAC__StreamMetadata *);
            #endif
    };

#endif
