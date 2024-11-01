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


#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>

#include "LocateSensor.hh"


inline void clLocateSensor::Clear ()
{
    memset(fpLocMatrix, 0x00, lWidth * lHeight * sizeof(GDT));
}


inline void clLocateSensor::SetValue (long lX, long lY, GDT fValue)
{
    if (lX < 0 || lX >= lWidth) return;
    if (lY < 0 || lY >= lHeight) return;
    fpLocMatrix[lY * lWidth + lX] += fValue;
}


void clLocateSensor::SetDirection (GDT fDirection, GDT fLevel, GDT fFrequency)
{
    long lX1;
    long lY1;
    long lX2 = 0;
    long lY2 = 0;
    GDT fDistance;
    GDT fThisDir;
    GDT fAbsCoeff;
    GDT fPropLoss;
    GDT fNewLevel;

    if (!b3D && (fDirection < -fHalfPI || fDirection > fHalfPI))
        return;
    fDistance = 1;
    fAbsCoeff = 0.05 * pow(fFrequency / 1000, 1.4);
    do {
        //fNewLevel = fLevel;
        fPropLoss = 15 * log10(fDistance) + fAbsCoeff * 
            fDistance * pow(10, -3);
        fNewLevel = fLevel * pow(10, fPropLoss / 20);
        fThisDir = fDirection + fAzimuth;
        lX1 = DSP.Round(sin(fThisDir) * fDistance) + lPosX;
        lY1 = DSP.Round(cos(fThisDir) * fDistance) + lPosY;
        SetValue(lX1, lY1, fNewLevel);
        if (!b3D)
        {
            fThisDir = fPI - fDirection + fAzimuth;
            lX2 = DSP.Round(sin(fThisDir) * fDistance) + lPosX;
            lY2 = DSP.Round(cos(fThisDir) * fDistance) + lPosY;
            SetValue(lX2, lY2, fNewLevel);
        }
        fDistance += 1;
    } while ((lX1 >= 0 && lX1 < lWidth && lY1 >= 0 && lY1 < lHeight) ||
        (lX2 >= 0 && lX2 < lWidth && lY2 >= 0 && lY2 < lHeight));
}


clLocateSensor::clLocateSensor ()
{
    bInitialized = false;
    fPI = acos(-1.0);
    fHalfPI = acos(-1.0) / 2.0;
}


clLocateSensor::~clLocateSensor ()
{
    if (bInitialized) Uninitialize();
}


bool clLocateSensor::Initialize (long lW, long lH, long lX, long lY, 
    GDT fA, bool bTD)
{
    lWidth = lW;
    lHeight = lH;
    lPosX = lX;
    lPosY = lY;
    fAzimuth = -fA;
    b3D = bTD;
    try 
    {
        fpLocMatrix = (GDT *) LocMatrix.Size(lWidth * lHeight * sizeof(GDT));
    } 
    catch (...)
    {
        return false;
    }
    Clear();
    return true;
}


void clLocateSensor::Uninitialize ()
{
    // NOP
}


void clLocateSensor::SetDirectionValues (const GDT *fpLevelValues, 
    const GDT *fpDirectionValues, long lValueCount,
    long lMinBin, long lMaxBin, GDT fFreqResolution)
{
    long lValueCntr;

    Clear();
    for (lValueCntr = lMinBin; lValueCntr < lMaxBin; lValueCntr++)
    {
        /*printf("%f %f %f\n", fpDirectionValues[lValueCntr], 
            20 * log10(fpLevelValues[lValueCntr]),
            (GDT) lValueCntr * fFreqResolution);*/
        SetDirection(fpDirectionValues[lValueCntr], fpLevelValues[lValueCntr],
            (GDT) lValueCntr * fFreqResolution);
    }
}

