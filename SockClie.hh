/*

    Socket client class
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


#ifndef SOCKCLIE_HH
    #define SOCKCLIE_HH

    #if (defined(__QNX__) && !defined(socklen_t))
    typedef size_t socklen_t;
    #endif


    /**
        Socket client class
    */
    class clSockClie
    {
            int iErrno;
        public:
            clSockClie ();
            ~clSockClie ();
            /**
                Connect to TCP server, set either hostname or hostaddr and
                the other to NULL.

                \param cpHostName Hostname
                \param cpHostAddr Host address
                \param iHostPort Host port
                \return Handle, -1 on error
            */
            int Connect (const char *, const char *, int);
            /**
                Connect to Unix domain server.

                In QNX (which doesn't support domain sockets) this is
                faked to TCP socket and filename should be port number.

                \param cpSockName Socket filename
                \return Handle, -1 on error
            */
            int Connect (const char *);
            /**
                Get errno

                \return errno
            */
            int GetErrno () { return iErrno; }
    };

#endif
