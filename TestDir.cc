/*

    Test direction finding
    Copyright (C) 1999-2000 Jussi Laako

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
#include <math.h>
#include <float.h>
#include <signal.h>

#include "Config.h"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"


bool bRun = true;


void SigHandler (int iSigNum)
{
    bRun = false;
}


int main (int argc, char *argv[])
{
    int iSockH;
    int iMsgLen;
    int iBeamCntr;
    char cpReqMsgBuf[GLOBAL_HEADER_LEN];
    char *cpMsgBuf;
    GDT fpResData[18];
    stDirReq sRq;
    stDirRes sResHdr;
    clSockClie SClient;
    clSockOp *SOp;
    clDirMsg DirMsg;

    signal(SIGINT, SigHandler);
    if (argc < 3)
    {
        printf("%s <host> <port>\n", argv[0]);
        return 1;
    }
    iSockH = SClient.Connect(argv[1], NULL, atoi(argv[2]));
    if (iSockH < 0)
    {
        printf("Can't connect\n");
        return 1;
    }
    SOp = new clSockOp(iSockH);
    strcpy(cpReqMsgBuf, "direction");
    if (SOp->WriteN(cpReqMsgBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
    {
        printf("Can't send process name\n");
        return 1;
    }
    sRq.iAlgorithm = MSG_DIR_ALG_BEAM;
    sRq.fSoundSpeed = 1430.0F;
    sRq.fLowFreqLimit = 60.0F;
    sRq.fIntegrationTime = 5.0F;
    sRq.iScaling = MSG_DIR_SCAL_LIN;
    sRq.fScalingExp = 0.0F;
    sRq.bNormalize = false;
    sRq.fLeftDir = -(M_PI / 2.0);
    sRq.fRightDir = M_PI / 2.0;
    sRq.lSectorCount = 18;
    DirMsg.SetRequest(cpReqMsgBuf, &sRq);
    if (SOp->WriteN(cpReqMsgBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
    {
        printf("Can't send request message\n");
        return 1;
    }
    iMsgLen = GLOBAL_HEADER_LEN + 18 * sizeof(GDT);
    cpMsgBuf = (char *) alloca(iMsgLen);
    if (cpMsgBuf == NULL)
    {
        printf("Memory allocation error!\n");
        return 2;
    }
    while (bRun)
    {
        if (SOp->ReadN(cpMsgBuf, iMsgLen) == iMsgLen)
        {
            DirMsg.GetResult(cpMsgBuf, &sResHdr, fpResData);
            for (iBeamCntr = 0; iBeamCntr < 18; iBeamCntr++)
            {
                printf(" %.2f", fpResData[iBeamCntr]);
            }
            printf("\n\n");
        }
        else
        {
            printf("Result read error\n");
            break;
        }
    }
    delete SOp;
    return 0;
}

