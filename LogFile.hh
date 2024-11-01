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


#ifndef LOGFILE_HH
    #define LOGFILE_HH

    #define LOGFILE_NOERROR         0


    /**
        Log file class
    */
    class clLogFile
    {
            bool bOk;
            int iEC;
            char cpTime[20];
            FILE *FLog;
            void Time();
        public:
            clLogFile();
            clLogFile(const char *);
            ~clLogFile();
            /**
                Open logfile

                \param cpLogFile Filename
                \return Success
            */
            bool Open(const char *);
            /**
                Add new entry to logfile

                \param cLogMark Mark character
                \param cpLogEntry Entry line
                \return Success
            */
            bool Add(char, const char *);
            /**
                Add new entry to logfile with errno

                \param cLogMark Mark character
                \param cpLogEntry Entry line
                \param iErrno errno value
                \return Success
            */
            bool Add(char, const char *, int);
            /**
                Get error code
            */
            int GetError() { return iEC; }
    };

#endif
