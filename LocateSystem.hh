/*

    Result combining of locate matrixes
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


#ifndef LOCATESYTEM_HH
    #define LOCATESYSTEM_HH


    /**
        Result combining of locate matrixes
    */
    class clLocateSystem
    {
            long lWidth;
            long lHeight;
            long lPointCount;
            long lResultCount;
            GDT fWeight;
            GDT *fpResults;
            GDT *fpFinal;
            clAlloc Results;
            clAlloc Final;
            clDSPOp DSP;
        public:
            clLocateSystem ();
            ~clLocateSystem ();
            /**
                Initialize.

                \param lW Matrix width
                \param lH Matrix height
                \param fCoeff Weighting coefficient
            */
            bool Initialize (long, long, GDT);
            /**
                Add subresult.

                \param fpSubRes Result matrix
            */
            void Add (const GDT *);
            /**
                Process final result.
            */
            void Process ();
            /**
                Get results

                \param fpDest Result matrix
            */
            void GetResults (GDT *);
    };

#endif

