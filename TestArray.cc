/*

    Test array classes
    Copyright (C) 1999-2000 Jussi Laako

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

#include "ArrayDipole.hh"


int main(int argc, char *argv[])
{
    clArrayDipole Dipole;

    Dipole.Initialize(2.0, 1430.0, 44100, 512, 100.0, true);
    printf("Sensor 0: 0 %.5f/%li 45 %.5f/%li 90 %.5f/%li\n", 
        Dipole.GetDelayTime(0, 0.0) * 1000.0, Dipole.GetDelaySamples(0, 0.0),
        Dipole.GetDelayTime(0, 0.785) * 1000.0, Dipole.GetDelaySamples(0, 0.785),
        Dipole.GetDelayTime(0, 1.571) * 1000.0, Dipole.GetDelaySamples(0, 1.571));
    printf("Sensor 1: 0 %.5f/%li 45 %.5f/%li 90 %.5f/%li\n",
        Dipole.GetDelayTime(1, 0.0) * 1000.0, Dipole.GetDelaySamples(1, 0.0),
        Dipole.GetDelayTime(1, 0.785) * 1000.0, Dipole.GetDelaySamples(1, 0.785),
        Dipole.GetDelayTime(1, 1.571) * 1000.0, Dipole.GetDelaySamples(1, 1.571));
    printf("Sensor 0: -0 %.5f/%li -45 %.5f/%li -90 %.5f/%li\n",
        Dipole.GetDelayTime(0, -0.0) * 1000.0, Dipole.GetDelaySamples(0, -0.0),
        Dipole.GetDelayTime(0, -0.785) * 1000.0, Dipole.GetDelaySamples(0, -0.785),
        Dipole.GetDelayTime(0, -1.571) * 1000.0, Dipole.GetDelaySamples(0, -1.571));
    printf("Sensor 1: -0 %.5f/%li -45 %.5f/%li -90 %.5f/%li\n",
        Dipole.GetDelayTime(1, -0.0) * 1000.0, Dipole.GetDelaySamples(1, -0.0),
        Dipole.GetDelayTime(1, -0.785) * 1000.0, Dipole.GetDelaySamples(1, -0.785),
        Dipole.GetDelayTime(1, -1.571) * 1000.0, Dipole.GetDelaySamples(1, -1.571));
    Dipole.Uninitialize();
}

