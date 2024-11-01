/*

    Direction calculation from spectrum
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


#include <stdio.h>
#include <math.h>
#include <float.h>

#include "SpectDir.hh"


clSpectDir::clSpectDir ()
{
    lDirCount = 0;
    fpDirections = NULL;
    fPI = acos(-1.0);
    fHalfPI = asin(1.0);
}


clSpectDir::~clSpectDir ()
{
}


void clSpectDir::SetSensorSpacing (GDT fSpacing)
{
    fSensorSpacing = fSpacing;
}


void clSpectDir::SetSoundSpeed (GDT fSoundSpeed)
{
    fArrayFreq = fSoundSpeed / fSensorSpacing / 2;
}


GDT clSpectDir::GetDirection (GDT fFrequency, GDT fDPhase)
{
    GDT fDf;
    GDT fDirection;

    fDf = fArrayFreq / fFrequency;
    fDirection = fDf * (fDPhase / 2);
    return fDirection;
}


void clSpectDir::SetDirectionCount (long lCount)
{
    lDirCount = lCount;
    fpDirections = (GDT *) DirBuf.Size(lDirCount * sizeof(GDT));
    DSP.Zero(fpDirections, lDirCount);
}


void clSpectDir::SetDirection (GDT fFrequency, GDT fLevel, GDT fDPhase)
{
    long lFreqIdx;
    GDT fDirection;
    GDT fDirRes;

    fDirection = GetDirection(fFrequency, fDPhase);
    fDirRes = lDirCount / fPI;
    lFreqIdx = DSP.Round(fDirection * fDirRes) + lDirCount / 2;
    if (lFreqIdx >= 0l && lFreqIdx < lDirCount)
        fpDirections[lFreqIdx] += fLevel;
}


GDT *clSpectDir::GetDirections (GDT *fpDest)
{
    if (fpDest != NULL) DSP.Scale01(fpDest, fpDirections, lDirCount);
    return fpDirections;
}


void clSpectDir::ResetDirections ()
{
    DSP.Zero(fpDirections, lDirCount);
}

