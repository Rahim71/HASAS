/*

    Log file class
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
#include <errno.h>
#include <time.h>
#include <string.h>

#include "LogFile.hh"


clLogFile::clLogFile()
{
    bOk = false;
    iEC = LOGFILE_NOERROR;
}


clLogFile::clLogFile(const char *cpLogFile)
{
    bOk = false;
    iEC = LOGFILE_NOERROR;
    Open(cpLogFile);
}


clLogFile::~clLogFile()
{
    if (bOk) fclose(FLog);
}


bool clLogFile::Open(const char *cpLogFile)
{
    FLog = fopen(cpLogFile, "at");
    if (FLog != NULL)
    {
        bOk = true;
        fprintf(FLog, "\n");
    }
    else
    {
        bOk = false;
        iEC = errno;
    }
    return bOk;
}


bool clLogFile::Add(char cLogMark, const char *cpLogEntry)
{
    if (!bOk) return false;
    Time();
    if (fprintf(FLog, "%c %s %s\n", cLogMark, cpTime, cpLogEntry) <= 0)
    {
        return false;
    }
    fflush(FLog);
    return true;
}


bool clLogFile::Add(char cLogMark, const char *cpLogEntry, int iErrno)
{
    if (!bOk) return false;
    Time();
    if (fprintf(FLog, "%c %s %s: (%i) %s\n", cLogMark, cpTime, cpLogEntry,
        iErrno, strerror(iErrno)) <= 0)
    {
        return false;
    }
    fflush(FLog);
    return true;
}


void clLogFile::Time()
{
    time_t ttTime;
    struct tm *spTM;

    ttTime = time(NULL);
    spTM = localtime(&ttTime);
    strftime(cpTime, 20, "%Y/%m/%d %H:%M:%S", spTM);
}

