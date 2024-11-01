/*

    Frequency-domain beamforming for dipole array
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

#include <dsp/DSPVector.hh>

#include "FreqBeamDipole.hh"


void clFreqBeamDipole::SetDebug (bool bDebug)
{
    long lSensorCntr;

    for (lSensorCntr = 0; lSensorCntr < 2; lSensorCntr++)
    {
        if (bDebug) Sensors[lSensorCntr].EnableDebug();
        else Sensors[lSensorCntr].DisableDebug();
    }
}


bool clFreqBeamDipole::Initialize (GDT fSpacing, long lWinSize, 
    GDT fSampleRate)
{
    long lSensorCntr;

    fSensorSpacing = fSpacing;
    if (!Sensors[0].Initialize(0, fSpacing, lWinSize) ||
        !Sensors[1].Initialize(fSpacing, 0, lWinSize))
        return false;
    for (lSensorCntr = 0; lSensorCntr < 2; lSensorCntr++)
    {
        Sensors[lSensorCntr].SetSampleRate(fSampleRate);
    }
    return true;
}


void clFreqBeamDipole::SetSoundSpeed (GDT fSoundSpeed)
{
    long lSensorCntr;

    for (lSensorCntr = 0; lSensorCntr < 2; lSensorCntr++)
    {
        Sensors[lSensorCntr].SetSoundSpeed(fSoundSpeed);
        Sensors[lSensorCntr].SetArrayFrequency(
            fSoundSpeed / 2 / fSensorSpacing);
    }
}


void clFreqBeamDipole::SetDirection (GDT fDirection, bool bLowPass)
{
    long lSensorCntr;

    for (lSensorCntr = 0; lSensorCntr < 2; lSensorCntr++)
    {
        if (fDirection < 0 && lSensorCntr == 1)
            Sensors[lSensorCntr].SetDirection(fDirection, bLowPass);
        else if (fDirection >= 0 && lSensorCntr == 0)
            Sensors[lSensorCntr].SetDirection(fDirection, bLowPass);
        else
            Sensors[lSensorCntr].SetDirection(fDirection, true);
    }
}


void clFreqBeamDipole::Put (const GDT *fpSrc, long lSrcCount, 
    long lSensorOffs, long lChannels)
{
    long lSensorCntr;
    clDSPVector<GDT> dspvSensorData;
    clDSPVector<GDT> dspvSrc(fpSrc, lSrcCount);

    for (lSensorCntr = 0; lSensorCntr < 2; lSensorCntr++)
    {
        dspvSensorData.Extract(dspvSrc, lSensorOffs + lSensorCntr, lChannels);
        Sensors[lSensorCntr].Put(dspvSensorData.Ptr(), dspvSensorData.Size());
        dspvSensorData.Clear();
    }
}


bool clFreqBeamDipole::Get (GDT *fpDest, long lDestCount)
{
    long lSensorCntr;
    clDSPVector<GDT> dspvSensorData(lDestCount);
    clDSPVector<GDT> dspvOutData(lDestCount);

    dspvOutData.Zero();
    for (lSensorCntr = 0; lSensorCntr < 2; lSensorCntr++)
    {
        if (!Sensors[lSensorCntr].Get(dspvSensorData.Ptr(), 
            dspvSensorData.Size())) return false;
        dspvOutData += dspvSensorData;
    }
    dspvOutData *= (GDT) 0.5;
    return dspvOutData.Get(fpDest, lDestCount);
}
