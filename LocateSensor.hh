/*

    Sensor matrix processing for locating sound sources
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


#ifndef LOCATESENSOR_HH
    #define LOCATESENSOR_HH


    /**
        Sensor matrix processing for locating sound sources
    */
    class clLocateSensor
    {
            bool bInitialized;
            bool b3D;
            long lWidth;
            long lHeight;
            long lPosX;
            long lPosY;
            GDT fPI;
            GDT fHalfPI;
            GDT fAzimuth;
            GDT *fpLocMatrix;
            clAlloc LocMatrix;
            clDSPOp DSP;
            void Clear ();
            void SetValue (long, long, GDT);
            void SetDirection (GDT, GDT, GDT);
        public:
            clLocateSensor ();
            ~clLocateSensor ();
            /**
                Initialize.

                \param lW Matrix width
                \param lH Matrix height
                \param lX Sensor X position
                \param lY Sensor Y position
                \param fA Sensor azimuth (direction) (rad)
                \param bTD Three dimensional array (non-twosided case)
                \return Success
            */
            bool Initialize (long, long, long, long, GDT, bool);
            /**
                Uninitialize.
            */
            void Uninitialize ();
            /**
                Set direction values.

                We just walk through matrix and adjust level using

                \f[\alpha=0.05\frac{f}{1000}^{1.4}\f]
                \f[R=15\log(s)+\alpha{s}10^{-3}\f]
                \f[V_{n}=V_{0}10^{\frac{R}{20}}\f]

                We definitely need way better algorithm, this is stupidly
                simple. Ajust multiplier for R for environment, 10 gives
                cylindral spreading and 20 spherical spreading.

                \param fpLevelValues Levels
                \param fpDirectionValues Directions
                \param lValueCount number of levels & directions
                \param lMinBin Results starting at index
                \param lMaxBin Results end at index
                \param fFreqResolution Frequency resolution
            */
            void SetDirectionValues (const GDT *, const GDT *, long,
                long, long, GDT);
            /**
                Get result matrix.

                \return Pointer to result matrix
            */
            GDT *GetResultMatrix () { return fpLocMatrix; }
    };

#endif

