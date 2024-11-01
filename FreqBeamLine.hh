/*

    Frequency-domain beamforming for line array, header
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


#include <vector>

#include "Config.h"
#include "ArraySensor.hh"


#ifndef FREQBEAMLINE_HH
    #define FREQBEAMLINE_HH


    /**
        Frequency-domain beamforming for line array.

        Array is shaded using Dolph-Chebyshev window.
    */
    class clFreqBeamLine
    {
        protected:
            long lSensorCount;
            GDT fSensorSpacing;
            std::vector<clArraySensor *> vSensors;
        public:
            clFreqBeamLine () {}
            ~clFreqBeamLine ();
            /**
                Enable/disable debug printouts

                \param bDebug Enable/disable debug printouts
            */
            void SetDebug (bool);
            /**
                Initialize dipole array

                \param lSensors Number of sensors in array
                \param fSpacing Sensor spacing (m)
                \param lWinSize Window size
                \param fSampleRate Sampling rate
                \return Success
            */
            bool Initialize (long, GDT, long, GDT);
            /**
                Set speed of sound

                \param fSoundSpeed Speed of sound (m/s)
            */
            void SetSoundSpeed (GDT);
            /**
                Set direction

                \param fDirection Direction (rad)
                \param bLowPass Lowpass filter
            */
            void SetDirection (GDT, bool);
            /**
                Put data into beamformer

                \param fpSrc Source vector
                \param lSrcCount Number of samples in source vector
                \param lSensorOffs Sensor offset in source vector
                \param lChannels Number of channels in source vector
            */
            void Put (const GDT *, long, long, long);
            /**
                Get data from beamformer

                \param fpDest Destination vector
                \param lDestCount Number of samples to put into destination vector
                \return Success
            */
            bool Get (GDT *, long);
    };

#endif

