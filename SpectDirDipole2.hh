/*

    Spectrum based direction finding for dipole array
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


#include <Alloc.hh>
#include <dsp/DSPOp.hh>
#include <dsp/RecDecimator.hh>

#include "Config.h"
#include "RemoveNoise.hh"
#include "SpectDir2.hh"


#ifndef SPECTDIRDIPOLE2_HH
    #define SPECTDIRDIPOLE2_HH


    // These should match with the ones defined in Messages.hh!

    enum
    {
        SDD2_SCALE_LIN = 0,
        SDD2_SCALE_LOG = 1
    };

    enum
    {
        SDD2_BNER_NONE = 0,
        SDD2_BNER_TPSW = 1,
        SDD2_BNER_OTA = 2,
        SDD2_BNER_DIFF = 3,
        SDD2_BNER_IDIFF = 4
    };


    /**
        Spectrum based direction finding for dipole array.

        This is used by clDirection3.

        \note See clSpectDirDipole for details!
    */
    class clSpectDirDipole2 : public clSpectDir2
    {
            bool bDebug;
            long lFilterSize;
            long lFFTSize;
            long lDecimation;
            long lSpectSize;
            long lMinBin;
            long lMaxBin;
            long lNewData;
            long lOldData;
            float fOverlap;
            GDT fFreqRes;
            GDT fIntTime;
            GDT *fpWinFunc;
            GDT *fpLevRes;
            GDT *fpDirRes;
            GDT *fpExtBuf;
            GDT *fpProcBuf[2];
            GDT *fpPrevBuf[2];
            GDT *fpRNBuf[2];
            GCDT *spSpect[2];
            clDSPAlloc WinFuncBuf;
            clDSPAlloc LevResBuf;
            clDSPAlloc DirResBuf;
            clDSPAlloc ExtBuf;
            clDSPAlloc ProcBuf[2];
            clDSPAlloc PrevBuf[2];
            clDSPAlloc RNBuf[2];
            clDSPAlloc SpectBuf[2];
            clRecDecimator Decimator[2];
            clRemoveNoise BNER;
            void Calculate (int, stpSpectDir2RN);
        public:
            /**
                Constructor.

                \param fSensorSpacing Sensor spacing (m)
                \param fSoundSpeed Speed of sound (m/s)
                \param dSampleRate Samplerate
                \param lFiltSize Size of filter (points)
                \param iFilterType Type of decimation filter
                \param lWindowSize Size of window (points)
                \param fIntTimeReq Integration time request (s)
                \param bEnableDebug Enable debug?
            */
            clSpectDirDipole2 (GDT, GDT, double, long, int, long, GDT, bool);
            ~clSpectDirDipole2 ();
            /**
                Put data into input FIFO.

                \param fpInputData Input data vector
                \param lSampleCount Number of sample in input vector (total)
                \param lStartCh Starting channel index
                \param lChCount Channel count, total
            */
            void PutData (const GDT *, long, long, long);
            /**
                Get copy of results.

                \param fpLevelResults Levels vector (can be NULL)
                \param fpDirResults Directions vector (can be NULL)
                \param fLowFreqLimit Low frequency limit (Hz)
                \param iScaling Scaling type
                \param spRemoveNoise Noise removal parameters
                \return Results available?
            */
            bool GetResults (GDT *, GDT *, GDT, int, stpSpectDir2RN);
            /**
                Get pointer to level results.

                \return Pointer to level results
            */
            GDT *GetLevels () { return fpLevRes; }
            /**
                Get pointer to direction results.

                \return Pointer to direction results
            */
            GDT *GetDirections () { return fpDirRes; }
            /**
                Get integration time (seconds).

                \return Integration time (s)
            */
            GDT GetIntegrationTime () { return fIntTime; }
            /**
                Get frequency resolution (Hz).

                \return Frequency resolution (Hz)
            */
            GDT GetFreqResolution () { return fFreqRes; }
            /**
                Get minimum used spectrum bin.

                \return Index to lowest used bin
            */
            long GetMinBin () { return lMinBin; }
            /**
                Get maximum used spectrum bin.

                \return Index to highest used bin
            */
            long GetMaxBin () { return lMaxBin; }
            /**
                Get result count.

                \return Result vector length
            */
            long GetResultCount () { return lSpectSize; }
    };

#endif

