/*

    Base class for array operations
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


#include <dsp/DSPOp.hh>

#include "Config.h"


#ifndef ARRAYBASE_HH
    #define ARRAYBASE_HH


    /**
        Types for shading window
    */
    enum
    {
        AB_SHADE_RECTANGLE = 0,
        AB_SHADE_BLACKMAN = 1,
        AB_SHADE_KAISER_BESSEL = 2
    };


    /**
        Base class for array operations
    */
    class clArrayBase
    {
        public:
            bool bDebug;
            GDT fSoundSpeed;
            GDT fSecsPerMeter;  ///< 1.0 / soundspeed
            GDT fSampleSpacing;  ///< 1.0 / samplerate
            clDSPOp DSP;
            clArrayBase () { bDebug = false; }
            ~clArrayBase () {}
            void EnableDebug () { bDebug = true; }
            void DisableDebug () { bDebug = false; }
            /**
                Set sample rate

                \f[\Delta{t_s}=\frac{1}{f_s}\f]

                \param iSampleRate Samplerate
            */
            void SetSampleRate (int);
            /**
                Set sound speed

                \f[c^{-1}=\frac{1}{c}\f]

                \param fSndSpeed Speed of sound in m/s
            */
            void SetSoundSpeed (GDT);
            /**
                Generate shading coefficients

                \param fpCoeffs Shading coefficients
                \param iShadeType Type of shade window
                \param lSensCount Number of sensors
            */
            void SetShading (GDT *, int, long);
    };

#endif
