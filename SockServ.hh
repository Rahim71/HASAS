/*

    Socket server class
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


#include <sys/socket.h>
#include <sys/un.h>


#ifndef SOCKSERV_HH
    #define SOCKSERV_HH

    #ifndef UNIX_PATH_MAX
        #define UNIX_PATH_MAX           108
    #endif

    #define SOCKSERV_LISTENQUEUE_LEN    8

    #if (defined(__QNX__) && !defined(socklen_t))
    typedef size_t socklen_t;
    #endif


    /**
        Socket server class
    */
    class clSockServ
    {
            int iErrno;
            int iListenH;
            bool bLocal;
            char cpLocalSockName[UNIX_PATH_MAX + 1];
        public:
            clSockServ ();
            clSockServ (unsigned short);
            clSockServ (const char *);
            ~clSockServ ();
            /**
                Bind to TCP port.

                \param usPort Port number
                \return Success
            */
            bool Bind (unsigned short);
            /**
                Bind to specific address and port.

                \param cpAddress Address
                \param usPort Port
                \return Success
            */
            bool Bind (const char *, unsigned short);
            /**
                Bind to UNIX domain socket.

                \param cpSockFile Filename
                \return Success
            */
            bool Bind (const char *);
            /**
                Wait for connect.

                \note This has no timeout.

                \return Handle to new connection, -1 on error
            */
            int WaitForConnect ();
            /**
                Wait for connect.

                \note Check errno for timeout/error.

                \param iTimeout Timeout (ms)
                \return Handle to new connection, -1 on error
            */
            int WaitForConnect (int);
            /**
                Close listening socket.
            */
            void Close ();
            /**
                Get errno.

                \return errno
            */
            int GetErrno () { return iErrno; }
    };

#endif
