/*

    Direction calculation from spectrum, header
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

#include "Config.h"
#include "ArrayBase.hh"


#ifndef SPECTDIR_HH
    #define SPECTDIR_HH


    // Following struct is not used here, but defined for global use
    /**
        SpectDir: Noise removal parameters
    */
    typedef struct _stSpectDirRN
    {
        int iType;
        GDT fAlpha;
        long lMeanLength;
        long lGapLength;
    } stSpectDirRN, *stpSpectDirRN;


    /**
        Direction calculation from spectrum.

        This is 100% my (Jussi Laako) own algorithm. At least I haven't seen
        this anywhere before and I'm not aware if such exists anywhere else.
    */
    class clSpectDir
    {
            long lDirCount;
            GDT fPI;
            GDT fHalfPI;
            GDT fSensorSpacing;
            GDT fArrayFreq;
            GDT *fpDirections;
            clAlloc DirBuf;
        public:
            clDSPOp DSP;
            clSpectDir ();
            ~clSpectDir ();
            /**
                Set sensor spacing.

                \note Call this before setting speed of sound.

                \param fSpacing Sensor spacing (m)
            */
            void SetSensorSpacing (GDT);
            /**
                Set speed of sound.

                \param fSoundSpeed Speed of sound (m/s)
            */
            void SetSoundSpeed (GDT);
            /**
                Get array frequency.

                Get maximum frequency that can be handled by the array
                without false directions. It is \f$\frac{\lambda}{2}\f$.

                \return Array frequency
            */
            GDT GetArrayFrequency () { return fArrayFreq; }
            /**
                Get direction from frequency and phase difference.

                \f[\Delta{f}=\frac{f_{a}}{f}\f]
                \f[\theta=\Delta{f}\frac{\Delta\varphi}{2}\f]

                \param fFrequency Frequency (Hz)
                \param fDPhase Phase difference (rad)
                \return Direction (rad)
            */
            GDT GetDirection (GDT, GDT);
            /**
                Set number of directions.

                180 / n = resolution.

                \param lCount Number of directions (sectors)
            */
            void SetDirectionCount (long);
            /**
                Set direction from frequency, power/magnitude and phase diff.

                \note Results are integrated over subsequent calls.

                \param fFrequency Frequency (Hz)
                \param fLevel Level
                \param fDPhase Phase difference
            */
            void SetDirection (GDT, GDT, GDT);
            /**
                Get vector containing results of subsequent SetDirection()'s.

                \note Destination pointer can be NULL, destination copy is
                normalized to [0:1], returned pointer is pointer to raw 
                results.

                \param fpDest Directions
                \return Pointer to directions
            */
            GDT *GetDirections (GDT *);
            /**
                Reset directions
            */
            void ResetDirections ();
    };

#endif

