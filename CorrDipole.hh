/*

    Correlator class for dipole array
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

#include "Config.h"
#include "ArrayDipole.hh"


#ifndef CORRDIPOLE_HH
    #define CORRDIPOLE_HH


    /**
        Correlator class for dipole array
    */
    class clCorrDipole
    {
            bool bDebug;
            bool bFilter;
            long lSampleCount;  ///< Sample count for specified integration time
            long lBaseIdx;  ///< Starting index = maximum delay
            long lWinSize;  ///< Window size (input data)
            GDT fIntTime;  ///< Integration time in seconds
            clDSPAlloc Data[2];  ///< Data buffers for integration
            clDSPOp DSP;
            clDSPOp DSPBank[2];
            clArrayDipole *Array;
        public:
            /**
                Constructor.

                \param ArrayPtr Pointer to clArrayDipole
                \param fIntegrationTime Integration time (s)
                \param iSampleRate Samplerate
                \param lWindowSize Size of window
                \param bDisableFilter Disable source data filtering
                \param bEnableDebug Enable debugging output
            */
            clCorrDipole (clArrayDipole *, GDT, int, long, bool, bool);
            ~clCorrDipole ();
            /**
                Add data to integration buffers.
                
                Call this after calling clArrayDipole::AddData()
                Returns >= 1 when integration buffers are full and ready to
                be processed by Process(), call until returns 0 before
                calling clArrayDipole::AddData() again.
            */
            bool AddData ();
            /**
                Process data in integration buffers and return result.
                
                This can be called many times for different directions.

                \param fDirection Direction (rad)
                \return Normalized cross-correlation
            */
            GDT Process (GDT);
            /**
                Set history part of buffer.

                Call this between last call of Process() and next call of
                AddData(), this will copy data from end of integration buffer
                to start of integration buffer.
            */
            void SetHistory ();
            /**
                Get peak level (in dB) of integration buffer.

                \return Peak level (dB)
            */
            GDT GetPeakLevel ();
            /**
                Get real integration time.

                \return Integration time (s)
            */
            GDT GetIntegrationTime () { return fIntTime; }
    };

#endif
