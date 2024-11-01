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


#include <dsp/DSPOp.hh>

#include "Config.h"
#include "ArrayBase.hh"


#ifndef SPECTDIR2_HH
    #define SPECTDIR2_HH


    typedef struct _stSpectDir2RN
    {
        int iType;
        GDT fAlpha;
        long lMeanLength;
        long lGapLength;
    } stSpectDir2RN, *stpSpectDir2RN;


    /**
        Direction calculation from spectrum.

        See clSpectDir for details.
    */
    class clSpectDir2
    {
            long lDirCount;
            GDT fPI;
            GDT fSensorSpacing;
            GDT fArrayFreq;
        public:
            clDSPOp DSP;
            clSpectDir2 ();
            ~clSpectDir2 ();
            // Set sensor spacing
            // Note: Call this before setting speed of sound
            // (sensor spacing)
            void SetSensorSpacing (GDT);
            // Set speed of sound
            // (speed of sound)
            void SetSoundSpeed (GDT);
            // Get array frequency
            GDT GetArrayFrequency () { return fArrayFreq; }
            // Get direction from frequency and phase difference
            // (frequency, phase diff (rad)) = direction (rad)
            GDT GetDirection (GDT, GDT);
    };

#endif

