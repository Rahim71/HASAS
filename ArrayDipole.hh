/*

    Class for dipole array operations
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
#include <dsp/Filter.hh>

#include "Config.h"
#include "ArrayBase.hh"


#ifndef ARRAYDIPOLE_HH
    #define ARRAYDIPOLE_HH


    /**
        Class for dipole array operations
    */
    class clArrayDipole : private clArrayBase
    {
            bool bInitialized;
            long lWindowSize;  ///< for filtering and array processing = out size
            long lBufferSize;  ///< total size of history buffer n * WindowSize
            long lBaseIdx;  ///< Start of non-delayed window
            int iSampleRate;
            GDT fSpacing;
            GDT fDelay;
            /// Raw data
            clDSPAlloc RawBuf[2];
            /// Filtered data (BP filtered)
            clDSPAlloc FiltBuf[2];
            /// High frequency data (HP filtered)
            clDSPAlloc HFBuf[2];
            /// 0,1 BP filters, 2,3 HP filters
            clFilter FilterBank[4];
        public:
            clArrayDipole ();
            ~clArrayDipole ();
            /**
                Initialize array

                \f[\Delta{T_{d}}=dc^{-1}\f]

                \param fSensorSpacing Sensor spacing (m)
                \param fSndSpeed Speed of sound (m/s)
                \param iSRate Samplerate
                \param lWinSize Size of window
                \param fLowFreqLimit Low frequency limit (Hz)
                \param bEnableDebug Enable debug printout
                \return Success
            */
            bool Initialize (GDT, GDT, int, long, GDT, bool);
            /**
                Uninitialize array
            */
            void Uninitialize ();
            /**
                Get pointer to data buffer

                \param lSensor Number of sensor
                \return Pointer to data buffer
            */
            GDT *GetRawPtr (long);
            /// \overload
            GDT *GetFiltPtr (long);
            /// \overload
            GDT *GetHFPtr (long);
            /**
                Get maximum delay in samples

                \f[\Delta{T_{max}}=\frac{\Delta{T_d}}{\Delta{t_s}}\f]

                \return Maximum delay in samples
            */
            long GetMaxDelay ();
            /**
                Get upper frequency limit of array

                \f[f_a=\frac{\Delta{T_{d}^{-1}}}{2}\f]
                or
                \f[f_a=\frac{c^{-1}d}{2}\f]

                \return Upper frequency limit of array (Hz)
            */
            GDT GetArrayFreq ();
            /**
                Get delay for specified direction (rad)

                \f[\Delta{T}=\sin(\theta)\Delta{T_d}\f]
                or
                \f[\Delta{T}=\frac{\sin(\theta)d}{c}\f]

                \param fDirection Direction (rad)
                \return Delay (time)
            */
            GDT GetDelay (GDT);
            /**
                Get delay length in seconds for specified sensor to specified
                direction (rad)

                \f[\Delta{T_n}=\Delta{T}n\f]

                \param lSensor Sensor number
                \param fDirection Direction (rad)
                \return Delay (time)
            */
            GDT GetDelayTime (long, GDT);
            /**
                Get delay length in samples for specified sensor to specified
                direction (rad)

                \param lSensor Sensor number
                \param fDirection Direction (rad)
                \return Delay (samples)
            */
            long GetDelaySamples (long, GDT);
            /**
                Set lower frequency limit

                \param fLowFreqLimit Lower frequency limit (Hz)
            */
            void SetLowFreqLimit (GDT);
            /**
                Add data to processing buffer
                
                \note There must be enough data to fill each channel with
                lWindowSize amount of data.

                \param fpSource Source vector
                \param lStartIndex Starting channel index
                \param lChannels Number of channels
            */
            void AddData (const GDT *, long, long);
            /**
                Get raw (non filtered) data for specified direction

                \param fpaDest Destination vectors (array of pointers)
                \param fDirection Direction (rad)
            */
            void GetRawData (GDT **, GDT);
            /**
                \overload
                \param lSensor Sensor number
                \param fDirection Direction (rad)
                \return Pointer to data
            */
            GDT *GetRawDataPtr (long, GDT);
            /**
                Get filtered data for specified direction

                \param fpaDest Destination vectors (array of pointers)
                \param fDirection Direction (rad)
            */
            void GetFilteredData (GDT **, GDT);
            /**
                \overload
                \param lSensor Sensor number
                \param fDirection Direction (rad)
                \return Pointer to data
            */
            GDT *GetFilteredDataPtr (long, GDT);
            /**
                Get audio data for specified direction

                \param fpDest Destination vector
                \param fDirection Direction (rad)
                \param bFullBand Full bandwidth or array-limited
            */
            void GetAudioData (GDT *, GDT, bool);
    };

#endif
