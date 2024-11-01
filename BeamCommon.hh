/*

    Beamforming input server, common
    Copyright (C) 2002 Jussi Laako

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


#ifndef BEAMCOMMON_HH
#define BEAMCOMMON_HH

#   define BS_LOGBUFSIZE    4096
#   define BS_ACCEPT_TO     250


    /**
        Array types for beamforming server.
    */
    enum
    {
        BS_ARRAY_TYPE_DIPOLE = 0,
        BS_ARRAY_TYPE_TRIANGLE = 1,
        BS_ARRAY_TYPE_LINE = 2,
        BS_ARRAY_TYPE_PLANE = 3,
        BS_ARRAY_TYPE_CYLINDER = 4,
        BS_ARRAY_TYPE_SPHERE = 5
    };


    /**
        Node parameters for beamforming cluster.
    */
    typedef struct _stBeamNodeParams
    {
        int iType;          ///< Array type
        int iSensors;       ///< Number of sensors
        float fSpacing;     ///< Sensor spacing
        float fSoundSpeed;  ///< Speed of sound
        int iBeamCount;     ///< Beam count
        int iWindowSize;    ///< Size of window
        int iBlockSize;     ///< Cluster communication block size
        float fSampleRate;  ///< Samplerate of incoming data
    } stBeamNodeParams, *stpBeamNodeParams;

#endif
