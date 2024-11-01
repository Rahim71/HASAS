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


#include <math.h>
#include <float.h>

#include "LocateSystem.hh"


clLocateSystem::clLocateSystem ()
{
}


clLocateSystem::~clLocateSystem ()
{
}


bool clLocateSystem::Initialize (long lW, long lH, GDT fCoeff)
{
    lWidth = lW;
    lHeight = lH;
    fWeight = fCoeff;
    lPointCount = lWidth * lHeight;
    try
    {
        fpResults = (GDT *) Results.Size(lPointCount * sizeof(GDT));
        fpFinal = (GDT *) Final.Size(lPointCount * sizeof(GDT));
    }
    catch (...)
    {
        return false;
    }
    DSP.Zero(fpResults, lPointCount);
    DSP.Zero(fpFinal, lPointCount);
    lResultCount = 0;
    return true;
}


void clLocateSystem::Add (const GDT *fpSubRes)
{
    DSP.Add(fpResults, fpSubRes, lPointCount);
    lResultCount++;
}


void clLocateSystem::Process ()
{
    GDT fScale = (GDT) 1 / lResultCount;

    DSP.Mul(fpFinal, (GDT) 1 - fWeight, lPointCount);
    DSP.Mul(fpResults, fScale * fWeight, lPointCount);
    DSP.Add(fpFinal, fpResults, lPointCount);
    //DSP.Copy(fpFinal, fpResults, lPointCount);
    
    DSP.Zero(fpResults, lPointCount);
    lResultCount = 0;
}


void clLocateSystem::GetResults (GDT *fpDest)
{
    DSP.Scale01(fpDest, fpFinal, lPointCount);
}

