/*

    Class for OSS audio IO operations
    Copyright (c) 1998-2002 Jussi Laako

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


#ifndef USE_OSSLITE
#include "sys/soundcard.h"
#else
#include <sys/soundcard.h>
#endif


#ifndef AUDIO_HH
    #define AUDIO_HH

    #ifndef BUF_SIZE
        #define BUF_SIZE        4096
    #endif


    enum
    {
        AUDIO_READ = 0,
        AUDIO_WRITE = 1,
        AUDIO_DUPLEX = 2
    };


    /**
        Class for OSS audio IO operations
    */
    class clAudio
    {
            int iDevH;
            int iErrorCode;
        public:
            clAudio ();
            clAudio (const char *, int *, int *, int *, int);
            ~clAudio ();
            /**
                Open device

                \note Check format, channels and samplerate for changes!

                \param cpDevName Device name
                \param ipFormat Format (SND_FMT_*)
                \param ipSampleRate Samplerate
                \param ipChannels Channel count
                \param iType Open mode, read, write or full-duplex
                \return Success
            */
            bool Open (const char *, int *, int *, int *, int);
            /**
                Close device
            */
            void Close ();
            /**
                Set device parameters

                \param ipFormat Format (SND_FMT_*)
                \param ipChannels Channel count
                \param ipSampleRate Samplerate
                \return Success
            */
            bool SetParams (int *, int *, int *);
            /**
                Set format

                \param ipFormat Format (SND_FMT_*)
                \return Success
            */
            bool SetFormat (int *);
            /**
                Set channel count

                \param ipChannels Channel count
                \return Success
            */
            bool SetChannels (int *);
            /**
                Set samplerate

                \param ipSampleRate Samplerate
                \return Success
            */
            bool SetSampleRate (int *);
            /**
                Read data to buffer

                \param vpData Buffer
                \param iBytes Number of bytes to read
                \return Number of bytes read
            */
            int Read (void *, int);
            /**
                Write data from buffer

                \param vpData Buffer
                \param iBytes Number of bytes to write
                \return Number of bytes written
            */
            int Write (const void *, int);
            /**
                Wait for buffer to be played empty

                \return Success
            */
            bool Sync ();
            /**
                Stops any ongoing action and returns 'normal' state
                (for example stops recording)

                \return Success
            */
            bool Reset ();
            /**
                Indicates driver that there will be pause in IO action

                \return Success
            */
            bool Post ();
            /**
                Get internal buffering fragment size

                \return Internal fragment size in bytes
            */
            int GetFragmentSize ();
            /**
                Undocumented feature of OSS
            */
            bool SubDivide (int *);
            /**
                Set buffering

                \param iFragSize Fragment size x where fragsize = 2^x
                \param iFragCount Number of fragments, 7fff = no limit
            */
            bool SetFragment (int, int);
            /**
                Get supported formats

                \return Formats, 0 on error
            */
            int GetFormats ();
            /**
                Get input buffer info

                \param spInBufInfo Input buffer info (audio_buf_info struct)
                \return Success
            */
            bool GetInBufInfo (audio_buf_info *);
            /**
                Get output buffer info

                \param spOutBufInfo Output buffer info (audio_buf_info struct)
                \return Success
            */
            bool GetOutBufInfo (audio_buf_info *);
            /**
                Get input buffer status

                \param spInBufStat Input buffer status (count_info struct)
                \return Success
            */
            bool GetInBufStat (count_info *);
            /**
                Get output buffer status

                \param spOutBufStat Output buffer status (count_info struct)
                \return Success
            */
            bool GetOutBufStat (count_info *);
            /**
                Get device capabilities

                \return DSP_CAP_* bitmask, 0 on error
            */
            int GetCaps ();
            /**
                Enable full duplex

                \return Success
            */
            bool SetDuplex ();
            /**
                Set always nonblocking io

                \return Success
            */
            bool SetNonBlock ();
            /**
                Set synchronous operation

                \return Success
            */
            bool SetSynchronous ();
            /**
                Get trigger mask

                \return Trigger mask, 0 on error
            */
            int GetTrigger ();
            /**
                Set trigger mask

                \param iTrigger Trigger mask
                \return Success
            */
            bool SetTrigger (int);
            /**
                Map input buffer

                \param spBufferInfo Mapping info (buffmem_desc struct)
                \return Success
            */
            bool MapInBuffer (buffmem_desc *);
            /**
                Map output buffer

                \param spBufferInfo Mapping info (buffmem_desc struct)
                \return Success
            */
            bool MapOutBuffer (buffmem_desc *);
            /**
                Get output delay

                \return Output delay (samples)
            */
            int GetOutDelay ();
            #ifndef USE_OSSLITE
            /**
                Get detailed error information

                \note Commercial OSS driver only!

                \param spErrorInfo Detailed error information
                \return Success
            */
            bool GetErrorInfo (audio_errinfo *);
            #endif
            /**
                Get OSS version

                \return OSS version, 0 on error
            */
            int GetVersion ();
            /**
                Methods for (de)interleaving stereo data
            */
            void DeIntStereo (const signed short *, signed short *, 
                signed short *, unsigned int);
            /// \overload
            void DeIntStereo (const unsigned char *, unsigned char *, 
                unsigned char *, unsigned int);
            /// \overload
            void IntStereo (signed short *, const signed short *,
                const signed short *, unsigned int);
            /// \overload
            void IntStereo (unsigned char *, const unsigned char *,
                const unsigned char *, unsigned int);
            /**
                Get device handle

                \return File descriptor to device
            */
            int GetHandle() { return iDevH; }
            /**
                Get errno code

                \return errno
            */
            int GetError() { return iErrorCode; }
            /// \overload
            int GetErrno() { return iErrorCode; }
   };
   

#endif

