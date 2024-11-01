/*

    RGB palette class
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


#ifndef PALETTE_HH
    #define PALETTE_HH


    /**
        RGB palette class
    */
    class clPalette
    {
            clAlloc Palette;
        protected:
            int iPalSize;
            unsigned int *upPalette;
        public:
            clPalette ();
            ~clPalette ();
            /**
                Generate white-to-black palette.
            */
            void GenBW ();
            /**
                Generate black-blue-cyan-green-yellow-red palette.
            */
            void GenHSV ();
            /**
                Generate palette matching to light's spectrum.
            */
            void GenLight ();
            /**
                Generate black-red-yellow-white palette.
            */
            void GenTemp ();
            /**
                Generate black-white palette with two highest values red.
            */
            void GenDir ();
            /**
                Generate black-green-red palette. "NATO-style"
            */
            void GenGreen ();
            /**
                Generate black-green-white palette.
            */
            void GenGreen2 ();
            /**
                Generate black-green-yellow palette.
            */
            void GenGreen3 ();
            /**
                Generate black-green-red-yellow palette.
            */
            void GenGreen4 ();
            /**
                Generate black-green palette.
            */
            void GenPureGreen ();
            /**
                Generate black-white palette.
            */
            void GenWB ();
            /**
                Get number of colors in palette.

                \return Number of colors
            */
            int Size () { return iPalSize; }
            /**
                Return color from palette LUT.

                \param iValue Palette LUT index.
                \return Color
            */
            unsigned int Color (int iValue) { return upPalette[iValue]; }
            /**
                Return color from palette LUT.
            */
            unsigned int operator [] (int iValue)
                { return upPalette[iValue]; }
    };

#endif

