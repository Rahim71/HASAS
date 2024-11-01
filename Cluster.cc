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

#include "Cluster.hh"


clMPIProc::clMPIProc ()
{
    bInitialized = false;
    iError = MPI_SUCCESS;
}


clMPIProc::~clMPIProc ()
{
    if (bInitialized) Finalize();
}


bool clMPIProc::Initialize (int *ipArgC, char ***cppArgV)
{
    if (bInitialized) return false;
    iError = MPI_Init(ipArgC, cppArgV);
    if (iError != MPI_SUCCESS) return false;
    bInitialized = true;
    return true;
}


bool clMPIProc::Finalize ()
{
    if (!bInitialized) return false;
    iError = MPI_Finalize();
    if (iError != MPI_SUCCESS) return false;
    bInitialized = false;
    return true;
}


bool clMPIProc::GetRank (int *ipRank)
{
    if (!bInitialized) return false;
    iError = MPI_Comm_rank(MPI_COMM_WORLD, ipRank);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIProc::GetNodeCount (int *ipNodeCount)
{
    if (!bInitialized) return false;
    iError = MPI_Comm_size(MPI_COMM_WORLD, ipNodeCount);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


clMPIComm::clMPIComm ()
{
    iTag = 0;
    iError = MPI_SUCCESS;
}


clMPIComm::clMPIComm (int iCommTag)
{
    iTag = iCommTag;
    iError = MPI_SUCCESS;
}


clMPIComm::~clMPIComm ()
{
}


void clMPIComm::SetTag (int iCommTag)
{
    iTag = iCommTag;
}


bool clMPIComm::Send (int iDest, char *cpData, int iCount)
{
    iError = MPI_Send(cpData, iCount, MPI_CHAR, iDest, iTag, 
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Send (int iDest, unsigned char *cpData, int iCount)
{
    iError = MPI_Send(cpData, iCount, MPI_UNSIGNED_CHAR, iDest, iTag,
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Send (int iDest, short *ipData, int iCount)
{
    iError = MPI_Send(ipData, iCount, MPI_SHORT, iDest, iTag,
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Send (int iDest, unsigned short *ipData, int iCount)
{
    iError = MPI_Send(ipData, iCount, MPI_UNSIGNED_SHORT, iDest, iTag,
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Send (int iDest, int *ipData, int iCount)
{
    iError = MPI_Send(ipData, iCount, MPI_INT, iDest, iTag,
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Send (int iDest, unsigned int *ipData, int iCount)
{
    iError = MPI_Send(ipData, iCount, MPI_UNSIGNED, iDest, iTag,
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Send (int iDest, long *lpData, int iCount)
{
    iError = MPI_Send(lpData, iCount, MPI_LONG, iDest, iTag,
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Send (int iDest, unsigned long *lpData, int iCount)
{
    iError = MPI_Send(lpData, iCount, MPI_UNSIGNED_LONG, iDest, iTag,
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Send (int iDest, float *fpData, int iCount)
{
    iError = MPI_Send(fpData, iCount, MPI_FLOAT, iDest, iTag,
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Send (int iDest, double *dpData, int iCount)
{
    iError = MPI_Send(dpData, iCount, MPI_DOUBLE, iDest, iTag,
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Send (int iDest, long double *ldpData, int iCount)
{
    iError = MPI_Send(ldpData, iCount, MPI_LONG_DOUBLE, iDest, iTag,
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Send (int iDest, void *vpData, int iCount)
{
    iError = MPI_Send(vpData, iCount, MPI_BYTE, iDest, iTag,
        MPI_COMM_WORLD);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Probe (int iSrc)
{
    iError = MPI_Probe(iSrc, iTag, MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::ProbeAny (int iSrc)
{
    iError = MPI_Probe(iSrc, MPI_ANY_TAG, MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::ProbeNB (int iSrc)
{
    int iFlag;

    iError = MPI_Iprobe(iSrc, iTag, MPI_COMM_WORLD, &iFlag, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    if (iFlag) return true;
    else return false;
}


bool clMPIComm::ProbeAnyNB (int iSrc)
{
    int iFlag;

    iError = MPI_Iprobe(iSrc, MPI_ANY_TAG, MPI_COMM_WORLD, &iFlag, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    if (iFlag) return true;
    else return false;
}


bool clMPIComm::Recv (int iSrc, char *cpData, int iCount)
{
    iError = MPI_Recv(cpData, iCount, MPI_CHAR, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Recv (int iSrc, unsigned char *cpData, int iCount)
{
    iError = MPI_Recv(cpData, iCount, MPI_UNSIGNED_CHAR, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Recv (int iSrc, short *ipData, int iCount)
{
    iError = MPI_Recv(ipData, iCount, MPI_SHORT, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Recv (int iSrc, unsigned short *ipData, int iCount)
{
    iError = MPI_Recv(ipData, iCount, MPI_UNSIGNED_SHORT, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Recv (int iSrc, int *ipData, int iCount)
{
    iError = MPI_Recv(ipData, iCount, MPI_INT, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Recv (int iSrc, unsigned int *ipData, int iCount)
{
    iError = MPI_Recv(ipData, iCount, MPI_UNSIGNED, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Recv (int iSrc, long *lpData, int iCount)
{
    iError = MPI_Recv(lpData, iCount, MPI_LONG, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Recv (int iSrc, unsigned long *lpData, int iCount)
{
    iError = MPI_Recv(lpData, iCount, MPI_UNSIGNED_LONG, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Recv (int iSrc, float *fpData, int iCount)
{
    iError = MPI_Recv(fpData, iCount, MPI_FLOAT, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Recv (int iSrc, double *dpData, int iCount)
{
    iError = MPI_Recv(dpData, iCount, MPI_DOUBLE, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Recv (int iSrc, long double *ldpData, int iCount)
{
    iError = MPI_Recv(ldpData, iCount, MPI_LONG_DOUBLE, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::Recv (int iSrc, void *vpData, int iCount)
{
    iError = MPI_Recv(vpData, iCount, MPI_BYTE, iSrc, iTag,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, char *cpData, int iCount)
{
    iError = MPI_Recv(cpData, iCount, MPI_CHAR, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, unsigned char *cpData, int iCount)
{
    iError = MPI_Recv(cpData, iCount, MPI_UNSIGNED_CHAR, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, short *ipData, int iCount)
{
    iError = MPI_Recv(ipData, iCount, MPI_SHORT, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, unsigned short *ipData, int iCount)
{
    iError = MPI_Recv(ipData, iCount, MPI_UNSIGNED_SHORT, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, int *ipData, int iCount)
{
    iError = MPI_Recv(ipData, iCount, MPI_INT, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, unsigned int *ipData, int iCount)
{
    iError = MPI_Recv(ipData, iCount, MPI_UNSIGNED, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, long *lpData, int iCount)
{
    iError = MPI_Recv(lpData, iCount, MPI_LONG, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, unsigned long *lpData, int iCount)
{
    iError = MPI_Recv(lpData, iCount, MPI_UNSIGNED_LONG, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, float *fpData, int iCount)
{
    iError = MPI_Recv(fpData, iCount, MPI_FLOAT, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, double *dpData, int iCount)
{
    iError = MPI_Recv(dpData, iCount, MPI_DOUBLE, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, long double *ldpData, int iCount)
{
    iError = MPI_Recv(ldpData, iCount, MPI_LONG_DOUBLE, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::RecvAny (int iSrc, void *vpData, int iCount)
{
    iError = MPI_Recv(vpData, iCount, MPI_BYTE, iSrc, MPI_ANY_TAG,
        MPI_COMM_WORLD, &sStatus);
    if (iError != MPI_SUCCESS) return false;
    return true;
}


bool clMPIComm::GetCount (const std::type_info &TypeInfo, int *ipCount)
{
    if (TypeInfo == typeid(char)) 
        iError = MPI_Get_count(&sStatus, MPI_CHAR, ipCount);
    else if (TypeInfo == typeid(unsigned char)) 
        iError = MPI_Get_count(&sStatus, MPI_UNSIGNED_CHAR, ipCount);
    else if (TypeInfo == typeid(short))
        iError = MPI_Get_count(&sStatus, MPI_SHORT, ipCount);
    else if (TypeInfo == typeid(unsigned short))
        iError = MPI_Get_count(&sStatus, MPI_UNSIGNED_SHORT, ipCount);
    else if (TypeInfo == typeid(int))
        iError = MPI_Get_count(&sStatus, MPI_INT, ipCount);
    else if (TypeInfo == typeid(unsigned int))
        iError = MPI_Get_count(&sStatus, MPI_UNSIGNED, ipCount);
    else if (TypeInfo == typeid(long))
        iError = MPI_Get_count(&sStatus, MPI_LONG, ipCount);
    else if (TypeInfo == typeid(unsigned long))
        iError = MPI_Get_count(&sStatus, MPI_UNSIGNED_LONG, ipCount);
    else if (TypeInfo == typeid(float))
        iError = MPI_Get_count(&sStatus, MPI_FLOAT, ipCount);
    else if (TypeInfo == typeid(double))
        iError = MPI_Get_count(&sStatus, MPI_DOUBLE, ipCount);
    else if (TypeInfo == typeid(long double))
        iError = MPI_Get_count(&sStatus, MPI_LONG_DOUBLE, ipCount);
    else if (TypeInfo == typeid(void *))
        iError = MPI_Get_count(&sStatus, MPI_BYTE, ipCount);
    if (iError != MPI_SUCCESS) return false;
    return true;
}

