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
#include "SpectDir.hh"


#ifndef SPECTDIRDIPOLE_HH
    #define SPECTDIRDIPOLE_HH


    // These should match with the ones defined in Messages.hh!

    /**
        Scaling type
    */
    enum
    {
        SDD_SCALE_LIN = 0,      ///< Linear scaling
        SDD_SCALE_LOG = 1       ///< Logarithmic (dB) scaling
    };

    /**
        Background noise estimation and removal type
    */
    enum
    {
        SDD_BNER_NONE = 0,      ///< No noise removal
        SDD_BNER_TPSW = 1,      ///< Two-Pass Split-Window
        SDD_BNER_OTA = 2,       ///< Order-Truncate-Average
        SDD_BNER_DIFF = 3,      ///< Differential
        SDD_BNER_IDIFF = 4      ///< Inverse differential
    };


    /**
        Spectrum based direction finding for a dipole array.

        This is 100% my (Jussi Laako) own algorithm. At least I haven't seen
        this anywhere before and I'm not aware if such exists anywhere else.
    */
    class clSpectDirDipole : public clSpectDir
    {
            bool bDebug;
            long lDirectionCount;
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
            GDT *fpExtBuf;
            GDT *fpProcBuf[2];
            GDT *fpPrevBuf[2];
            GDT *fpRNBuf[2];
            GCDT *spSpect[2];
            clDSPAlloc WinFuncBuf;
            clDSPAlloc ExtBuf;
            clDSPAlloc ProcBuf[2];
            clDSPAlloc PrevBuf[2];
            clDSPAlloc RNBuf[2];
            clDSPAlloc SpectBuf[2];
            clRecDecimator Decimator[2];
            clRemoveNoise BNER;
            void Calculate (int, stpSpectDirRN);
        public:
            /**
                Constructor.

                \param fSensorSpacing Sensor distance (m)
                \param fSoundSpeed Speed of sound (m/s)
                \param dSampleRate Samplerate
                \param lFiltSize Filter size (points)
                \param iFilterType Type of decimation filter
                \param lWindowSize Window size (points)
                \param lDirCount Number of sectors
                \param fIntTimeReq Integration time request (s)
                \param bEnableDebug Enable debug?
            */
            clSpectDirDipole (GDT, GDT, double, long, int, long, long, GDT, 
                bool);
            ~clSpectDirDipole ();
            /**
                Put data into input data FIFO.

                \param fpInputData Input data
                \param lDataCount Number of samples (total)
                \param lStartCh Starting channel index
                \param lChCount Number of channels (total)
            */
            void PutData (const GDT *, long, long, long);
            /**
                Get results.

                \f[V_{f}(x)=\sqrt{\left(\Re{F(x)_{L}}^{2}+\Im{F(x)_{L}}^{2}\right)+\left(\Re{F(x)_{R}}^{2}+\Im{F(x)_{R}}^{2}\right)}\f]
                \f[z_{c}(x)=F(x)_{L}F(x)_{R}^{*}\f]
                \f[\Delta{\varphi}_{f}(x)=\arctan\left(\frac{\Im{z_{c}(x)}}{\Re{z_{c}(x)}}\right)\f]

                \param fpResults Results vector
                \param fLowFreqLimit Lower frequency limit (Hz)
                \param iScaling Scaling type
                \param spRemoveNoise Noise removal parameters
                \return Results available?
            */
            bool GetResults (GDT *, GDT, int, stpSpectDirRN);
            /**
                Reset results
            */
            void ResetResults ();
            /**
                Get integration time (seconds)

                \return Integration time (s)
            */
            GDT GetIntegrationTime () { return fIntTime; }
    };

#endif

