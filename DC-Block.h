/*

    DC-blocking IIR filter
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


#ifdef _MSC_VER
#pragma warning(disable:4305)
#endif


#include "Config.h"


static const GDT fpDCBlock[][5] = {
{ // 0
    0.999471546671221,
    -1.99894309334244,
    0.999471546671221,
    1.99892336443267,
    -0.998962822252213
},
{ // 1
    0.998450100273063,
    -1.99690020054613,
    0.998450100273063,
    1.99688049179904,
    -0.996919909293219
},
{ // 2
    0.997472599022871,
    -1.99494519804574,
    0.997472599022871,
    1.99492550859388,
    -0.994964887497603
},
{ // 3
    0.996565379457685,
    -1.99313075891537,
    0.996565379457685,
    1.99311108737143,
    -0.993150430459315
},
{ // 4
    0.995752746555527,
    -1.99150549311105,
    0.995752746555527,
    1.99148583760795,
    -0.99152514861416
},
{ // 5
    0.995056358671025,
    -1.99011271734205,
    0.995056358671025,
    1.99009307558518,
    -0.99013235909892
},
{ // 6
    0.994494691463981,
    -1.98898938292796,
    0.994494691463981,
    1.98896975225803,
    -0.98900901359789
},
{ // 7
    0.994082589601246,
    -1.98816517920249,
    0.994082589601246,
    1.98814555666718,
    -0.988184801737801
},
{ // 8
    0.993830912590808,
    -1.98766182518162,
    0.993830912590808,
    1.98764220761425,
    -0.987681442748987
},
{ // 9
    0.996868235580257,
    -0.996868235580257,
    0,
    0.993736471160514,
    0
}
};

static const long lDCBlockCount = 10;

