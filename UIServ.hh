/*

    User interface server
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


#include "CfgFile.hh"
#include "LogFile.hh"
#include "SockServ.hh"


#ifndef UISERV_HH
    #define UISERV_HH

    #define UIS_CONV_BUF_SIZE       255


    /**
        User interface server.

        This works like inetd. It starts waiting for connections in single
        predifined port. When connection arrives it receives process request
        from client, duplicates handles and fork() and exec() the requested
        process and then continues to wait for new connections.

        There are several advatages of this behaviour:
        - Takes only one port
        - SMP friendly
        - Easy to extend
    */
    class clUIServer
    {
            bool bLogShutdown;
            clCfgFile *Cfg;
            clSockServ *SServ;
        public:
            clLogFile *Log;
            clUIServer ();
            ~clUIServer ();
            int Wait ();
            void NoLogShutdown () { bLogShutdown = false; }
    };

#endif
