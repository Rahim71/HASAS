/*

    Base class for array operations
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


#include <stdio.h>
#include <math.h>
#include <float.h>

#include "ArrayBase.hh"


static const char *cpaShadingTypes[] = { "Rectangle", "Blackman",
    "Kaiser-Bessel" };


void clArrayBase::SetSampleRate (int iSampleRate)
{
    fSampleSpacing = (GDT) 1.0 / (GDT) iSampleRate;
    if (bDebug) printf("Sample spacing %g ms\n", fSampleSpacing * 1000.0);
}


void clArrayBase::SetSoundSpeed (GDT fSndSpeed)
{
    fSoundSpeed = fSndSpeed;
    fSecsPerMeter = (GDT) 1.0 / fSndSpeed;
    if (bDebug) printf("Sound speed %g ms/m\n", fSecsPerMeter * 1000.0);
}


void clArrayBase::SetShading (GDT *fpCoeffs, int iShadeType, long lSensCount)
{
    if (bDebug) printf("Generate %s shading coefficients for %li sensors\n",
        cpaShadingTypes[iShadeType], lSensCount);
    switch (iShadeType)
    {
        case AB_SHADE_RECTANGLE:
            DSP.Set(fpCoeffs, (GDT) 1.0, lSensCount);
            break;
        case AB_SHADE_BLACKMAN:
            DSP.WinExactBlackman(fpCoeffs, lSensCount);
            break;
        case AB_SHADE_KAISER_BESSEL:
            DSP.WinKaiserBessel(fpCoeffs, AB_KBWIN_ALPHA, lSensCount);
            break;
        default:
            DSP.Set(fpCoeffs, (GDT) 1.0, lSensCount);
    }
}

