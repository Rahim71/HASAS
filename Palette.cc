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


#include "Palette.hh"


clPalette::clPalette ()
{
    iPalSize = 0;
    upPalette = NULL;
}


clPalette::~clPalette ()
{
}


void clPalette::GenBW()
{
    unsigned int uIdxCntr;

    iPalSize = 256;
    upPalette = (unsigned int *) Palette.Size(iPalSize * sizeof(unsigned int));
    for (uIdxCntr = 0; uIdxCntr < (unsigned int) iPalSize; uIdxCntr++)
    {
        upPalette[uIdxCntr] =
            (((0xff - uIdxCntr) << 16) |
            ((0xff - uIdxCntr) << 8) |
            0xff - uIdxCntr);
    }
}


void clPalette::GenHSV()
{
    int iIdxCntr;
    unsigned int uColCntr;

    iPalSize = 256 * 5;
    upPalette = (unsigned int *) Palette.Size(iPalSize * sizeof(unsigned int));
    iIdxCntr = 0;
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (uColCntr << 16);
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (0x00ff0000 | (uColCntr << 8));
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (((0xff - uColCntr) << 16) | 0x0000ff00);
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (0x0000ff00 | uColCntr);
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (((0xff - uColCntr) << 8) | 0x000000ff);
    }
}


void clPalette::GenLight()
{
    int iIdxCntr;
    unsigned int uColCntr;

    iPalSize = 256 * 4;
    upPalette = (unsigned int *) Palette.Size(iPalSize * sizeof(unsigned int));
    iIdxCntr = 0;
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = ((uColCntr << 8) | 0x000000ff);
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (0x0000ff00 | (0xff - uColCntr));
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = ((uColCntr << 16) | ((0xff - uColCntr) << 8));
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (0x00ff0000 | uColCntr);
    }
}


void clPalette::GenTemp()
{
    int iIdxCntr;
    unsigned int uColCntr;

    iPalSize = 256 * 3;
    upPalette = (unsigned int *) Palette.Size(iPalSize * sizeof(unsigned int));
    iIdxCntr = 0;
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = uColCntr;
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (0x000000ff | (uColCntr << 8));
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (0x0000ffff | (uColCntr << 16));
    }
}


void clPalette::GenDir()
{
    int iIdxCntr;
    unsigned int uColCntr;

    iPalSize = 256 + 2;
    upPalette = (unsigned int *) Palette.Size(iPalSize * sizeof(unsigned int));
    iIdxCntr = 0;
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (uColCntr | (uColCntr << 8) |
            (uColCntr << 16));
    }
    upPalette[iIdxCntr++] = 0x007f7fff;
    upPalette[iIdxCntr++] = 0x000000ff;
}


void clPalette::GenGreen()
{
    int iIdxCntr;
    unsigned int uColCntr;

    iPalSize = 256 * 2;
    upPalette = (unsigned int *) Palette.Size(iPalSize * sizeof(unsigned int));
    iIdxCntr = 0;
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (uColCntr << 8);
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (((0xff - uColCntr) << 8) | uColCntr);
    }
}


void clPalette::GenGreen2()
{
    int iIdxCntr;
    unsigned int uColCntr;

    iPalSize = 256 * 2;
    upPalette = (unsigned int *) Palette.Size(iPalSize * sizeof(unsigned int));
    iIdxCntr = 0;
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (uColCntr << 8);
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = ((uColCntr << 16) | 0xff00 | uColCntr);
    }
}


void clPalette::GenGreen3 ()
{
    int iIdxCntr;
    unsigned int uColCntr;
    
    iPalSize = 256 * 2;
    upPalette = (unsigned int *) Palette.Size(iPalSize * sizeof(unsigned int));
    iIdxCntr = 0;
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (uColCntr << 8);
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (0xff00 | uColCntr);
    }
}


void clPalette::GenGreen4 ()
{
    int iIdxCntr;
    unsigned int uColCntr;
    
    iPalSize = 256 * 3;
    upPalette = (unsigned int *) Palette.Size(iPalSize * sizeof(unsigned int));
    iIdxCntr = 0;
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (uColCntr << 8);
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (((0xff - uColCntr) << 8) | uColCntr);
    }
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = ((uColCntr << 8) | 0xff);
    }
}


void clPalette::GenPureGreen()
{
    int iIdxCntr;
    unsigned int uColCntr;

    iPalSize = 256;
    upPalette = (unsigned int *) Palette.Size(iPalSize * sizeof(unsigned int));
    iIdxCntr = 0;
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] = (uColCntr << 8);
    }
}


void clPalette::GenWB()
{
    int iIdxCntr;
    unsigned int uColCntr;

    iPalSize = 256;
    upPalette = (unsigned int *) Palette.Size(iPalSize * sizeof(unsigned int));
    iIdxCntr = 0;
    for (uColCntr = 0; uColCntr < 256; uColCntr++)
    {
        upPalette[iIdxCntr++] =
            ((uColCntr << 16) | (uColCntr << 8) | uColCntr);
    }
}

