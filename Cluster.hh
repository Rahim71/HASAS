/*

    MPI class implementation for clustering
    Copyright (C) 2000-2001 Jussi Laako

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


#include <typeinfo>

#include <mpi.h>


#ifndef CLUSTER_HH
    #define CLUSTER_HH


    /**
        MPI process
    */
    class clMPIProc
    {
            bool bInitialized;
            int iError;
        public:
            clMPIProc ();
            ~clMPIProc ();
            /**
                Initialize MPI process

                \param argc Parameter count
                \param argv Parameter vector
                \return Success
            */
            bool Initialize (int *, char ***);
            /**
                Uninitialize MPI process

                \return Success
            */
            bool Finalize ();
            /**
                Get rank of this MPI node

                \param ipRank Rank
                \return Success
            */
            bool GetRank (int *);
            /**
                Get number of nodes for this MPI process

                \param ipNodeCount Number of nodes
                \return Success
            */
            bool GetNodeCount (int *);
            /**
                Get error code

                \return Error code
            */
            int GetError () { return iError; }
    };


    /**
        MPI communication
    */
    class clMPIComm
    {
            int iTag;
            int iError;
            MPI_Status sStatus;
        public:
            clMPIComm ();
            clMPIComm (int);
            ~clMPIComm ();
            /**
                Set message tag

                \param iTag Tag
            */
            void SetTag (int);
            /**
                Send message

                \param iDest Receiver rank
                \param Data Message buffer
                \param iCount Item count
                \return Success
            */
            bool Send (int, char *, int);
            /// \overload
            bool Send (int, unsigned char *, int);
            /// \overload
            bool Send (int, short *, int);
            /// \overload
            bool Send (int, unsigned short *, int);
            /// \overload
            bool Send (int, int *, int);
            /// \overload
            bool Send (int, unsigned int *, int);
            /// \overload
            bool Send (int, long *, int);
            /// \overload
            bool Send (int, unsigned long *, int);
            /// \overload
            bool Send (int, float *, int);
            /// \overload
            bool Send (int, double *, int);
            /// \overload
            bool Send (int, long double *, int);
            /// \overload
            bool Send (int, void *, int);
            /**
                Synchronize message and return info (with our tag or wildcard)

                Check/wait for message.

                \param iSrc Source rank
                \return Status
            */
            bool Probe (int);
            /// \overload
            bool ProbeAny (int);
            /// \overload
            bool ProbeNB (int);
            /// \overload
            bool ProbeAnyNB (int);
            /**
                Receive message

                \param iSrc Source rank
                \param Data Message buffer
                \param iCount Buffer size in items
                \return Success
            */
            bool Recv (int, char *, int);
            /// \overload
            bool Recv (int, unsigned char *, int);
            /// \overload
            bool Recv (int, short *, int);
            /// \overload
            bool Recv (int, unsigned short *, int);
            /// \overload
            bool Recv (int, int *, int);
            /// \overload
            bool Recv (int, unsigned int *, int);
            /// \overload
            bool Recv (int, long *, int);
            /// \overload
            bool Recv (int, unsigned long *, int);
            /// \overload
            bool Recv (int, float *, int);
            /// \overload
            bool Recv (int, double *, int);
            /// \overload
            bool Recv (int, long double *, int);
            /// \overload
            bool Recv (int, void *, int);
            /**
                Receive message with any tag

                \param iSrc Sender rank
                \param Data Message buffer
                \param iCount Size of message buffer in items
            */
            bool RecvAny (int, char *, int);
            /// \overload
            bool RecvAny (int, unsigned char *, int);
            /// \overload
            bool RecvAny (int, short *, int);
            /// \overload
            bool RecvAny (int, unsigned short *, int);
            /// \overload
            bool RecvAny (int, int *, int);
            /// \overload
            bool RecvAny (int, unsigned int *, int);
            /// \overload
            bool RecvAny (int, long *, int);
            /// \overload
            bool RecvAny (int, unsigned long *, int);
            /// \overload
            bool RecvAny (int, float *, int);
            /// \overload
            bool RecvAny (int, double *, int);
            /// \overload
            bool RecvAny (int, long double *, int);
            /// \overload
            bool RecvAny (int, void *, int);
            /**
                Get length of last received message (number of elements)

                \note This is not to be used extensively. This uses RTTI, so
                it could be very slow.

                \param TypeInfo Type information
                \param ipCount Number of elements
                \return Success
            */
            bool GetCount (const std::type_info &, int *);
            /**
                Get rank of sender of last received message

                \return Sender rank
            */
            int GetSenderRank () { return sStatus.MPI_SOURCE; }
            /**
                Get tag of last received message

                \return Last received tag
            */
            int GetSenderTag () { return sStatus.MPI_TAG; }
            /**
                Get MPI returned error code

                \return Error code
            */
            int GetError () { return iError; }
            /**
                Get MPI error value from status

                \return Error code
            */
            int GetError2 () { return sStatus.MPI_ERROR; }
    };

#endif

