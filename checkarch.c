/*

    Check architecture and compiler properties
    Copyright (C) 1999 Jussi Laako

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


int main()
{
    int iCharSize;
    int iShortSize;
    int iIntSize;
    int iLongSize;
    int iFloatSize;
    int iDoubleSize;
    int iEndianTest;
    char *cpEndianTest;
    #ifdef SPECIAL
    int iLongLongSize;
    int iLongDoubleSize;
    #endif

    iCharSize = sizeof(unsigned char);
    iShortSize = sizeof(signed short);
    iIntSize = sizeof(int);
    iLongSize = sizeof(long);
    iFloatSize = sizeof(float);
    iDoubleSize = sizeof(double);
    cpEndianTest = (char *) &iEndianTest;
    #ifdef SPECIAL
    iLongLongSize = sizeof(long long);
    iLongDoubleSize = sizeof(long double);
    #endif

    printf("Architecture check v1.1\n");
    printf("Copyright (c) 1999 by Jussi Laako. All rights reserved.\n\n");
    if ((iIntSize == iLongSize) && (iIntSize == 4))
    {
        printf("This should be 32-bit system.\n\n");
    }
    else if ((iIntSize < iLongSize) && (iIntSize == 4) && (iLongSize == 8))
    {
        printf("This should be 64-bit system.\n\n");
    }
    else
    {
        printf("This is some strange unsupported compiler/architecture!\n");
        printf("This will need some porting efforts.\n\n");
    }
    printf("char size %i - %i-bits\n", iCharSize, iCharSize * 8);
    printf("short size %i - %i-bits\n", iShortSize, iShortSize * 8);
    printf("int size %i - %i-bits\n", iIntSize, iIntSize * 8);
    printf("long size %i - %i-bits\n", iLongSize, iLongSize * 8);
    #ifdef SPECIAL
    printf("long long size %i - %i-bits\n", iLongLongSize, iLongLongSize * 8);
    #endif
    printf("float size %i - %i-bits\n", iFloatSize, iFloatSize * 8);
    printf("double size %i - %i-bits\n", iDoubleSize, iDoubleSize * 8);
    #ifdef SPECIAL
    printf("long double size %i - %i-bits\n", iLongDoubleSize, 
        iLongDoubleSize * 8);
    #endif
    iEndianTest = 0x01020304;
    if (cpEndianTest[0] == 0x01)
    {
        printf("This is big endian machine.\n");
    }
    else
    {
        printf("This is little endian machine.\n");
    }
    return 0;
}
