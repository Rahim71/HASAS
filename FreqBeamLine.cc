/*

    Frequency-domain beamforming for line array
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


#include <math.h>
#include <float.h>
#include <sys/types.h>

#include <dsp/DSPVector.hh>

#include "FreqBeamLine.hh"


clFreqBeamLine::~clFreqBeamLine ()
{
    while (vSensors.size() > 0)
    {
        delete vSensors[vSensors.size() - 1];
        vSensors.pop_back();
    }
}


void clFreqBeamLine::SetDebug (bool bDebug)
{
    size_t uiSensorCntr;

    for (uiSensorCntr = 0; uiSensorCntr < vSensors.size(); uiSensorCntr++)
    {
        if (bDebug) vSensors[uiSensorCntr]->EnableDebug();
        else vSensors[uiSensorCntr]->DisableDebug();
    }
}


bool clFreqBeamLine::Initialize (long lSensors, GDT fSpacing, long lWinSize, 
    GDT fSampleRate)
{
    long lSensorCntr;
    clDSPVector<GDT> dspvShade(lSensors);
    clArraySensor *Sensor;

    lSensorCount = lSensors;
    fSensorSpacing = fSpacing;
    dspvShade.WinDolphChebyshev(
        (GDT) 1.0 / (GDT) pow(10.0, 3.0 * lSensorCount / 20.0), 
        lSensorCount);
    for (lSensorCntr = 0; lSensorCntr < lSensorCount; lSensorCntr++)
    {
        Sensor = new clArraySensor;
        if (!Sensor->Initialize(lSensorCntr * fSpacing, 
            (lSensorCount - 1) - lSensorCntr * fSpacing,
            lWinSize)) return false;
        Sensor->SetSampleRate(fSampleRate);
        Sensor->SetShading(dspvShade[lSensorCntr]);
        vSensors.push_back(Sensor);
    }
    return true;
}


void clFreqBeamLine::SetSoundSpeed (GDT fSoundSpeed)
{
    size_t uiSensorCntr;

    for (uiSensorCntr = 0; uiSensorCntr < vSensors.size(); uiSensorCntr++)
    {
        vSensors[uiSensorCntr]->SetSoundSpeed(fSoundSpeed);
        vSensors[uiSensorCntr]->SetArrayFrequency(
            fSoundSpeed / 2 / fSensorSpacing);
    }
}


void clFreqBeamLine::SetDirection (GDT fDirection, bool bLowPass)
{
    size_t uiSensorCntr;

    for (uiSensorCntr = 0; uiSensorCntr < vSensors.size(); uiSensorCntr++)
    {
        if (fDirection < 0 && uiSensorCntr == (vSensors.size() - 1))
            vSensors[uiSensorCntr]->SetDirection(fDirection, bLowPass);
        else if (fDirection >= 0 && uiSensorCntr == 0)
            vSensors[uiSensorCntr]->SetDirection(fDirection, bLowPass);
        else
            vSensors[uiSensorCntr]->SetDirection(fDirection, true);
    }
}


void clFreqBeamLine::Put (const GDT *fpSrc, long lSrcCount, 
    long lSensorOffs, long lChannels)
{
    size_t uiSensorCntr;
    clDSPVector<GDT> dspvSensorData;
    clDSPVector<GDT> dspvSrc(fpSrc, lSrcCount);

    for (uiSensorCntr = 0; uiSensorCntr < vSensors.size(); uiSensorCntr++)
    {
        dspvSensorData.Extract(dspvSrc, lSensorOffs + uiSensorCntr, lChannels);
        vSensors[uiSensorCntr]->Put(dspvSensorData.Ptr(), 
            dspvSensorData.Size());
        dspvSensorData.Clear();
    }
}


bool clFreqBeamLine::Get (GDT *fpDest, long lDestCount)
{
    size_t uiSensorCntr;
    clDSPVector<GDT> dspvSensorData(lDestCount);
    clDSPVector<GDT> dspvOutData(lDestCount);

    dspvOutData.Zero();
    for (uiSensorCntr = 0; uiSensorCntr < vSensors.size(); uiSensorCntr++)
    {
        if (!vSensors[uiSensorCntr]->Get(dspvSensorData.Ptr(), 
            dspvSensorData.Size())) return false;
        dspvOutData += dspvSensorData;
    }
    dspvOutData *= (GDT) 1.0 / (GDT) lSensorCount;
    return dspvOutData.Get(fpDest, lDestCount);
}
