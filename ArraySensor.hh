/*

    Class representing single sensor for array operations
    Copyright (C) 1999-2002 Jussi Laako

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
#include <dsp/DSPVector.hh>
#include <dsp/Filter2.hh>

#include "Config.h"


#ifndef ARRAYSENSOR_HH
    #define ARRAYSENSOR_HH


    /**
        Class representing sensor in sensor array for frequency domain
        beamforming
    */
    class clArraySensor
    {
        protected:
            bool bDebug;
            long lWindowSize;       ///< Processing window size
            long lProcCount;        ///< Number of samples per processing
            GDT fPI;
            GDT fArrayFrequency;    ///< Array frequency
            GDT fSoundSpeed;        ///< Speed of sound
            GDT fSecsPerMeter;      ///< 1.0 / soundspeed
            GDT fSampleRate;        ///< Samplerate
            GDT fSampleSpacing;     ///< 1.0 / samplerate
            GDT fFreqResolution;    ///< Frequency resolution
            GDT fShadeCoeff;        ///< Shading coefficient
            GDT fLeftDistance;      ///< Left side sensor distance
            GDT fRightDistance;     ///< Right side sensor distance
            GDT fLeftTime;          ///< Left side sensor delay
            GDT fRightTime;         ///< Right side sensor delay
            GDT fDirection;         ///< Beam direction for delay
            clDSPOp DSP;
            clFilter2 Filter;
            clDSPVector<GDT> dspvInData;
            clDSPVector<GDT> dspvProcData;
            clDSPVector<GDT> dspvOutData;
            void SinCosC (float, float *, float *);
            void SinCosC (double, double *, double *);
        public:
            clArraySensor ();
            ~clArraySensor () {}
            void EnableDebug () { bDebug = true; }
            void DisableDebug () { bDebug = false; }
            /**
                Initialize sensor

                Frequency resolution is \f$\frac{f_{Nyquist}}{N+1}\f$.

                \param fLDistance Left side sensor distance to end of the array (m)
                \param fRDistance Right side sensor distance to end of the array (m)
                \param lWinSize Processing window size
                \return Success
            */
            bool Initialize (GDT, GDT, long);
            /**
                Set sample rate

                \f[\Delta{t_s}=\frac{1}{f_s}\f]

                \param fFs Samplerate
            */
            void SetSampleRate (GDT);
            /**
                Set sound speed

                \f[c^{-1}=\frac{1}{c}\f]

                \param fSndSpeed Speed of sound in m/s
            */
            void SetSoundSpeed (GDT);
            /**
                Set design frequency of array

                \f$\lambda/2\f$

                \param fFa Array frequency (Hz)
            */
            void SetArrayFrequency (GDT);
            /**
                Set shading coefficient

                \param fCoeff Shading coefficient
            */
            void SetShading (GDT);
            /**
                Set direction (in rad)

                \f[\varphi_{f}=\sin(\theta)\pi\left(\frac{\frac{1}{c}d}{\frac{1}{2f}}\right)\f]
                \f[C_{f}=\cos(\varphi_{f})+j\sin(\varphi_{f})\f]

                \param fDir Direction
                \param bLowPass Low pass filter at array frequency
            */
            void SetDirection (GDT, bool = true);
            /**
                Put sensor data into sensor processor

                \param fpSrc Source vector
                \param lSrcCount Number of samples in source vector
            */
            void Put (const GDT *, long);
            /**
                Get delayed data from the sensor processor

                \param fpDest Destination vector
                \param lDestCount Number of samples to put into destination vector
                \return Success
            */
            bool Get (GDT *, long);
    };

#endif
