/*

    Test UI for locate
    Copyright (C) 2000 Jussi Laako

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
#include <signal.h>

#include <ptc/ptc.h>

#include <Alloc.hh>

#include "Config.h"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"


bool bRun = true;
Console Con;
Format Fmt(32, 0x00ff0000, 0x0000ff00, 0x000000ff);


void SigHandler (int iSigNum)
{
    bRun = false;
}


void PlotData (Surface &Sfc, GDT *fpData, int iWidth, int iHeight)
{
    int iDataCntr;
    /*unsigned int iR;
    unsigned int iG;
    unsigned int iB;*/
    unsigned int *ipSfc;

    ipSfc = (unsigned int *) Sfc.lock();
    for (iDataCntr = 0; iDataCntr < (iWidth * iHeight); iDataCntr++)
    {
        if (fpData[iDataCntr] > 1) fpData[iDataCntr] = 1;
        ipSfc[iDataCntr] =
            ((unsigned int) (fpData[iDataCntr] * 0xff) << 16) |
            ((unsigned int) (fpData[iDataCntr] * 0xff) << 8) |
            ((unsigned int) (fpData[iDataCntr] * 0xff));
        /*iR = (unsigned int) (fpData[iDataCntr] * 0xff) + 
            sSeabed.pixel_data[iDataCntr * 4];
        if (iR > 0xff) iR = 0xff;
        iG = sSeabed.pixel_data[iDataCntr * 4 + 1];
        iB = sSeabed.pixel_data[iDataCntr * 4 + 2];
        ipSfc[iDataCntr] = ((iR << 16) | (iG << 8) | iB);*/
    }
    Sfc.unlock();
}


int main (int argc, char *argv[])
{
    int iDataSize;
    char cpMsgHdr[GLOBAL_HEADER_LEN];
    stLocateHdr sHdr;
    stLocateRes sRes;
    clAlloc InData;
    clAlloc ResData;
    clSockClie SClient;
    clSockOp SOp;
    clLocateMsg Msg;

    signal(SIGINT, SigHandler);
    SOp.SetHandle(SClient.Connect("localhost", NULL, 30002));
    if (SOp.ReadN(cpMsgHdr, GLOBAL_HEADER_LEN) == GLOBAL_HEADER_LEN)
    {
        Msg.GetHeader(cpMsgHdr, &sHdr);
        iDataSize = GLOBAL_HEADER_LEN + 
            sHdr.iWidth * sHdr.iHeight * sizeof(GDT);
        InData.Size(iDataSize);
        ResData.Size(sHdr.iWidth * sHdr.iHeight * sizeof(GDT));
        Con.open("Locate test", sHdr.iWidth, sHdr.iHeight, Fmt);
        Surface Sfc(sHdr.iWidth, sHdr.iHeight, Fmt);
        while (bRun && !Con.key() && SOp.ReadN(InData, iDataSize) == iDataSize)
        {
            Msg.GetResult(InData, &sRes, (GDT *) ResData);
            PlotData(Sfc, ResData, sHdr.iWidth, sHdr.iHeight);
            Sfc.copy(Con);
            Con.update();
        }
        Con.close();
    }
    return 0;
}

