/*

    Spectrum based direction finding for line array
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

#include <Alloc.hh>
#include <dsp/DSPOp.hh>
#include <dsp/RecDecimator.hh>

#include "Config.h"
#include "RemoveNoise.hh"
#include "SpectDir.hh"


#ifndef SPECTDIRLINE_HH
    #define SPECTDIRLINE_HH


    // These should match with the ones defined in Messages.hh!

    /**
        Scaling type
    */
    enum
    {
        SDL_SCALE_LIN = 0,      ///< Linear scaling
        SDL_SCALE_LOG = 1       ///< Logarithmic (dB) scaling
    };

    /**
        Background noise estimation and removal type
    */
    enum
    {
        SDL_BNER_NONE = 0,      ///< No noise removal
        SDL_BNER_TPSW = 1,      ///< Two-Pass Split-Window
        SDL_BNER_OTA = 2,       ///< Order-Truncate-Average
        SDL_BNER_DIFF = 3,      ///< Differential
        SDL_BNER_IDIFF = 4      ///< Inverse differential
    };


    /**
        Spectrum based direction finding for a line array.

        This is 100% my (Jussi Laako) own algorithm. At least I haven't seen
        this anywhere before and I'm not aware if such exists anywhere else.
    */
    class clSpectDirLine : public clSpectDir
    {
            bool bDebug;
            long lSensorCount;
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
            std::vector<GDT *> vfpProcBuf;
            std::vector<GDT *> vfpPrevBuf;
            std::vector<GDT *> vfpRNBuf;
            std::vector<GCDT *> vspSpect;
            clDSPAlloc WinFuncBuf;
            clDSPAlloc ExtBuf;
            std::vector<clDSPAlloc> vProcBuf;
            std::vector<clDSPAlloc> vPrevBuf;
            std::vector<clDSPAlloc> vRNBuf;
            std::vector<clDSPAlloc> vSpectBuf;
            std::vector<clRecDecimator *> vDecimator;
            clRemoveNoise BNER;
            void Calculate (int, stpSpectDirRN);
        public:
            /**
                Constructor.

                \param lSensors Number of sensors in array
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
            clSpectDirLine (long, GDT, GDT, double, long, int, long, long, GDT, 
                bool);
            ~clSpectDirLine ();
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

