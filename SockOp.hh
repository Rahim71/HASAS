/*

    Socket I/O operations
    Copyright (C) 1999-2002 Jussi Laako

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


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>


#ifndef SOCKOP_HH
    #define SOCKOP_HH

    #if (defined(__QNX__) && !defined(socklen_t))
    typedef size_t socklen_t;
    #endif


    /**
        Socket I/O operations
    */
    class clSockOp
    {
            int iErrno;
            int iRetVal;
            int iSockH;
            bool bCloseOnDestruct;
        public:
            clSockOp ();
            clSockOp (int);
            ~clSockOp ();
            /**
                Set handle.

                Closes previously open handle.

                \param iHandle Handle
            */
            void SetHandle (int);
            /**
                Close handle/connection.
            */
            void Close ();
            /**
                Wait for socket to become readable.

                \param iTimeoutMS Timeout (ms)
                \return Readable?
            */
            bool ReadSelect (int);
            /**
                Wait for socket to become writable.

                \param iTimeoutMS Timeout (ms)
                \return Writable?
            */
            bool WriteSelect (int);
            /**
                Read data.

                \param vpBuf Buffer
                \param iBufLen Buffer size
                \return Bytes read
            */
            int Read (void *, int);
            /**
                Write data.

                \param vpBuf Buffer
                \param iBufLen Buffer size
                \return Bytes written
            */
            int Write (const void *, int);
            /**
                Read N bytes.

                Ensures that N bytes are read, unless error occurs.

                \param vpBuf Buffer
                \param iBufLen Buffer size
                \return Bytes read
            */
            int ReadN (void *, int);
            /**
                Write N bytes.

                Ensures that N bytes are written, unless error occurs.

                \param vpBuf Buffer
                \param iBufLen Buffer size
                \return Bytes written
            */
            int WriteN (const void *, int);
            /**
                Receive data.

                \param vpBuf Buffer
                \param iBufLen Buffer size
                \param iFlags Flags
                \return Bytes received
            */
            int Receive (void *, int, int);
            /**
                Send data.

                \param vpBuf Buffer
                \param iBufLen Buffer size
                \param iFlags Flags
                \return Bytes sent
            */
            int Send (const void *, int, int);
            /**
                Receive message.

                \param spMsgHdr Message header
                \param iFlags Flags
                \return Bytes received
            */
            int ReceiveMsg (struct msghdr *, int);
            /**
                Send message.

                \param spMsgHdr Message header
                \param iFlags Flags
                \return Bytes sent
            */
            int SendMsg (const struct msghdr *, int);
            /**
                Shutdown connection.

                \param iShutdownDir Direction flag
                \return 0 on success, -1 on error
            */
            int Shutdown (int);
            /**
                Get address of this end.

                \param spAddr Address
                \param ipAddrLen Address length
                \return 0 on success, -1 on error
            */
            int GetSockName (struct sockaddr *, socklen_t *);
            /**
                Get address of remote end.

                \param spAddr Address
                \param ipAddrLen Address length
                \return 0 on succes, -1 on error
            */
            int GetPeerName (struct sockaddr *, socklen_t *);
            /**
                Set blocking/nonblocking IO.

                \param bBlocking Blocking?
                \return Success
            */
            bool SetBlocking (bool);
            /**
                Enable keepalive messages.

                \return Success
            */
            bool SetKeepAlive ();
            /**
                Enable linger option.

                \param iLingerTime Linger time
                \return Success
            */
            bool SetLinger (int);
            /**
                Get receive buffer size.

                \return Size of receive buffer
            */
            int GetRecvBufSize ();
            /**
                Set receive buffer size.

                \param iBufSize Size of receive buffer
                \return Success
            */
            bool SetRecvBufSize (int);
            /**
                Get send buffer size.

                \return Size of send buffer
            */
            int GetSendBufSize ();
            /**
                Set send buffer size.

                \param iBufSize Size of send buffer
                \return Success
            */
            bool SetSendBufSize (int);
            /**
                Get position of receive buffer low-water sign.

                \return Low-water position
            */
            int GetRecvBufLowWater ();
            /**
                Set receive buffer low-water sign.

                \param iBufLowWater Position of receive low-water sign
                \return Success
            */
            bool SetRecvBufLowWater (int);
            /**
                Get position of send buffer low-water sign.

                \return Low-water position
            */
            int GetSendBufLowWater ();
            /**
                Set send buffer low-water sign.

                \param iBufLowWater Position of send low-water sign
                \return Success
            */
            bool SetSendBufLowWater (int);
            /**
                Set receive timeout.

                \param iTimeoutMS Timeout (ms)
                \return Success
            */
            bool SetRecvTimeout (int);
            /**
                Set send timeout.

                \param iTimeoutMS Timeout (ms)
                \return Success
            */
            bool SetSendTimeout (int);
            /**
                Set interval of TCP keepalive messages.

                \param iTimeSec Time between keepalive messages (s)
                \return Success
            */
            bool SetTCPKeepAlive (int);
            /**
                Disable TCP nagle-algorithm.

                \return Success
            */
            bool DisableNagle ();
            /// \overload
            bool SetNagle (bool);
            /**
                Set type-of-service flag.
                
                \param iTOS Type-of-service flag (IPTOS_*)
                \return Success
            */
            bool SetTypeOfService (int);
            /**
                Close handle/connection in destructor.

                \param bSetting Close in destructor?
            */
            void SetCloseOnDestruct (bool);
            /**
                Get handle (file descriptor).

                \return Handle
            */
            int GetHandle () { return iSockH; }
            /**
                Get errno.

                \return errno
            */
            int GetErrno () { return iErrno; }
    };

#endif
