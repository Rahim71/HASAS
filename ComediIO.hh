/*

    ComediIO
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
#include <comedilib.h>

#include <Alloc.hh>


#ifndef COMEDIIO_HH
    #define COMEDIIO_HH


    /**
        Class for Comedi IO operations
    */
    class clComediIO
    {
            int iInSubDev;
            char *cpErrorMsg;
            comedi_t *comediDev;
            lsampl_t iInMaxValue;
            clAlloc InScanList;
            clAlloc InBuf;
            void SetError ();
            bool PcmStart (int, double *, int, unsigned int *, unsigned int, 
                bool, bool);
            long PcmRead (sampl_t **, long);
        public:
            enum
            {
                VERS_MASK_MAJ = 0x00ff0000,
                VERS_MASK_MIN = 0x0000ff00,
                VERS_MASK_PL = 0x000000ff
            };
            enum
            {
                VERS_SHIFT_MAJ = 16,
                VERS_SHIFT_MIN = 8,
                VERS_SHIFT_PL = 0
            };

            clComediIO ();
            ~clComediIO ();
            /**
                Open device
                
                \param cpDevice Device filename
                \return Success?
            */
            bool Open (const char *);
            /**
                Close device
                
                \return Success?
            */
            bool Close ();
            /**
                Get number of subdevices
                
                \return Number of subdevices
            */
            int GetSubdeviceCount ();
            /**
                Get version code
                
                \return Version code
            */
            int GetVersionCode ();
            /**
                Get driver name
                
                \return Driver name
            */
            const char *GetDriverName ();
            /**
                Get board name
                
                \return Board name
            */
            const char *GetBoardName ();
            /**
                Get error message
                
                \return Error message
            */
            const char *GetErrorMsg ()
                { return cpErrorMsg; }
            /* --- Subdevice specific --- */
            /**
                Open PCM input device
                
                \return Success?
            */
            bool PcmInOpen ();
            /**
                Close PCM input device
                
                \return Success?
            */
            bool PcmInClose ();
            /**
                Get number of channels for PCM input device
                
                \return Number of channels
            */
            int PcmInGetChannelCount ();
            /**
                Start input operation
                
                \param dpSampleRate Samplerate (actual rate is returned)
                \param iChannels Number of channels to use
                \param dRange Input range (in volts)
                \param bDither User dither?
                \param bDoubleClock Use scan/channel timers instead of one?
                \return Success?
            */
            bool PcmInStart (double *, int, double, bool = true, bool = false);
            /**
                Stop input operation
                
                \return Success?
            */
            bool PcmInStop ();
            /**
                Read input samples
                
                \param Buffer
                \param Buffer size (in samples)
                \return Number of samples
            */
            long PcmInRead (float *, long);
            /// \overload
            long PcmInRead (double *, long);
            /**
                Set buffer size for PCM device
                
                \note Buffer size must be multiple of pagesize.
                
                \param uiBufSize Buffer size
                \return Success?
            */
            bool PcmBufferSizeSet (unsigned int);
            /**
                Get buffer size for PCM device
                
                \return Size of buffer
            */
            int PcmBufferSizeGet ();
            /**
                Get internal sample size (for calculating buffer sizes)
                
                \return Intenal sample size
            */
            size_t PcmGetIntSampleSize ()
                { return sizeof(sampl_t); }
    };


#endif
