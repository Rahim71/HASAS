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


#include <Alloc.hh>


#ifndef CFGFILE_HH
    #define CFGFILE_HH


    #define CFGF_MAX_ENTRIES        1024
    #define CFGF_BUFSIZE            1024


    class clCfgFile
    {
            int iEntryCount;
            clAlloc Names[CFGF_MAX_ENTRIES];
            clAlloc Values[CFGF_MAX_ENTRIES];
            void ReadFile (const char *);
            void FreeAll ();
        public:
            clCfgFile ();
            clCfgFile (const char *);
            ~clCfgFile ();
            /**
                Read configuration file.

                You can call this many times. Previous settings are flushed
                from the memory every time this method is called.

                \param cpFileName Name of configuration file
            */
            void SetFileName (const char *);
            /**
                Get value for key.

                \param cpKey Key name
                \param cpVal Value
                \return Success
            */
            bool GetStr (const char *, char *);
            /// \overload
            bool GetInt (const char *, int *);
            /// \overload
            bool GetInt (const char *, long *);
            /// \overload
            bool GetFlt (const char *, float *);
            /// \overload
            bool GetFlt (const char *, double *);
            /// \overload
            int GetFltArray (const char *, float *);
            /// \overload
            int GetFltArray (const char *, double *);
    };

#endif

