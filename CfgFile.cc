/*

    Configuration file parser
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
#include <stdlib.h>
#include <string.h>

#include "CfgFile.hh"


void clCfgFile::ReadFile (const char *cpFileName)
{
    char cpInBuf[CFGF_BUFSIZE + 1];
    char cpName[CFGF_BUFSIZE + 1];
    char cpValue[CFGF_BUFSIZE + 1];
    char *cpTokPtr;
    FILE *FpCfgFile;

    iEntryCount = 0;
    FpCfgFile = fopen(cpFileName, "rt");
    if (FpCfgFile == NULL)
    {
        return;
    }
    do {
        if (iEntryCount >= CFGF_MAX_ENTRIES - 2)
        {
            break;
        }
        fgets(cpInBuf, CFGF_BUFSIZE, FpCfgFile);
        cpTokPtr = strtok(cpInBuf, " \t\n");
        if (cpTokPtr != NULL)
        {
            strcpy(cpName, cpTokPtr);
            cpTokPtr = strtok(NULL, " \t\n");
            if (cpTokPtr != NULL)
            {
                strcpy(cpValue, cpTokPtr);
                Names[iEntryCount].Size(strlen(cpName) + 1);
                Values[iEntryCount].Size(strlen(cpValue) + 1);
                strcpy(Names[iEntryCount], cpName);
                strcpy(Values[iEntryCount], cpValue);
                iEntryCount++;
            }
        }
    } while (feof(FpCfgFile) == 0);
    fclose(FpCfgFile);
}


void clCfgFile::FreeAll ()
{
    int iLoopCntr;

    for (iLoopCntr = 0; iLoopCntr < iEntryCount; iLoopCntr++)
    {
        Names[iLoopCntr].Free();
        Values[iLoopCntr].Free();
    }
    iEntryCount = 0;
}


clCfgFile::clCfgFile ()
{
    iEntryCount = 0;
}


clCfgFile::clCfgFile (const char *cpFileName)
{
    iEntryCount = 0;
    ReadFile(cpFileName);
}


void clCfgFile::SetFileName (const char *cpFileName)
{
    FreeAll();
    ReadFile(cpFileName);
}


clCfgFile::~clCfgFile ()
{
    try
    {
        FreeAll();
    }
    catch (...)
    {
    }
}


bool clCfgFile::GetStr (const char *cpKey, char *cpVal)
{
    int iLoopCntr;

    for (iLoopCntr = 0; iLoopCntr < iEntryCount; iLoopCntr++)
    {
        if (strcmp(Names[iLoopCntr], cpKey) == 0)
        {
            strcpy(cpVal, Values[iLoopCntr]);
            return true;
        }
    }
    return false;
}


bool clCfgFile::GetInt (const char *cpKey, int *ipVal)
{
    int iLoopCntr;

    for (iLoopCntr = 0; iLoopCntr < iEntryCount; iLoopCntr++)
    {
        if (strcmp(Names[iLoopCntr], cpKey) == 0)
        {
            sscanf(Values[iLoopCntr], "%i", ipVal);
            return true;
        }
    }
    return false;
}


bool clCfgFile::GetInt (const char *cpKey, long *lpVal)
{
    int iLoopCntr;

    for (iLoopCntr = 0; iLoopCntr < iEntryCount; iLoopCntr++)
    {
        if (strcmp(Names[iLoopCntr], cpKey) == 0)
        {
            sscanf(Values[iLoopCntr], "%li", lpVal);
            return true;
        }
    }
    return false;
}


bool clCfgFile::GetFlt (const char *cpKey, float *fpVal)
{
    int iLoopCntr;

    for (iLoopCntr = 0; iLoopCntr < iEntryCount; iLoopCntr++)
    {
        if (strcmp(Names[iLoopCntr], cpKey) == 0)
        {
            sscanf(Values[iLoopCntr], "%f", fpVal);
            return true;
        }
    }
    return false;
}


bool clCfgFile::GetFlt (const char *cpKey, double *dpVal)
{
    int iLoopCntr;

    for (iLoopCntr = 0; iLoopCntr < iEntryCount; iLoopCntr++)
    {
        if (strcmp(Names[iLoopCntr], cpKey) == 0)
        {
            sscanf(Values[iLoopCntr], "%lf", dpVal);
            return true;
        }
    }
    return false;
}


int clCfgFile::GetFltArray (const char *cpKey, float *fpVals)
{
    int iLoopCntr;
    int iValCntr = 0;
    char *cpTokPtr;

    for (iLoopCntr = 0; iLoopCntr < iEntryCount; iLoopCntr++)
    {
        if (strcmp(Names[iLoopCntr], cpKey) == 0)
        {
            cpTokPtr = strtok(Values[iLoopCntr], ", \t\n");
            while (cpTokPtr != NULL)
            {
                sscanf(cpTokPtr, "%f", &fpVals[iValCntr++]);
                cpTokPtr = strtok(NULL, ", \t\n");
            }
        }
    }
    return iValCntr;
}

