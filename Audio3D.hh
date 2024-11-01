/*

    3D audio engine
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
#include <dsp/DSPOp.hh>
#include <dsp/Filter.hh>

#include "Config.h"
#include "HRTF.h"


#ifndef AUDIO3D_HH
    #define AUDIO3D_HH

    #define AUD3D_USE_WINDOW        true
    

    /**
        3D audio engine
    */
    class clAudio3D
    {
            bool bInitialized;
            long lWindowSize;
            long lFFTSize;
            long lSpectSize;
            GDT fDistance;
            GDT fAmp;
            GDT *fpLeftFilt;
            GDT *fpRightFilt;
            GCDT *spLeftFilt;
            GCDT *spRightFilt;
            clAlloc LeftData;
            clAlloc RightData;
            clAlloc LeftFilt;
            clAlloc RightFilt;
            clAlloc CLeftFilt;
            clAlloc CRightFilt;
            clDSPOp DSP;
            clFilter FilterLeft;
            clFilter FilterRight;
            void CopyToLocal (GDT *, const float *);
            void HeadingElevN40 (int) {}
            void HeadingElevN30 (int) {}
            void HeadingElevN20 (int) {}
            void HeadingElevN10 (int) {}
            void HeadingElev0 (int);
            void HeadingElev10 (int) {}
            void HeadingElev20 (int) {}
            void HeadingElev30 (int) {}
            void HeadingElev40 (int) {}
            void HeadingElev50 (int) {}
            void HeadingElev60 (int) {}
            void HeadingElev70 (int) {}
            void HeadingElev80 (int) {}
            void HeadingElev90 ();
            void Prepare (bool);
            void ProcessDistance (GDT *, GDT *, long);
        public:
            clAudio3D ();
            ~clAudio3D ();
            /**
                Initialize

                \note Reinitialization without Uninitialize() is allowed

                \param lSize Size of window
            */
            void Initialize (long);
            /**
                Uninitialize
            */
            void Uninitialize ();
            /**
                Set sound direction

                \note Heading and pitch in degrees, distance 0 - 1

                \param iHeading Heading (deg)
                \param iPitch Pitch (deg)
                \param fDist Distance
            */
            void SetAngles (int, int, GDT);
            /**
                Process interleaved data

                \param fpData New data
            */
            void Process (GDT *);
            /**
                Process noninterleaved data

                \param fpLeftData Left channel data
                \param fpRightData Right channel data
            */
            void Process (GDT *, GDT *);
            /**
                Get size of processing window

                \return Size of processing window
            */
            long GetWindowSize () { return lWindowSize; }
    };

#endif

