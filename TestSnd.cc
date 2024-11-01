/*

    Test program for SoundSrv
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
#include <signal.h>

#include <dsp/DSPOp.hh>

#include "Config.h"
#include "Audio.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"


bool bRun = true;


void SigHandler(int iSignal)
{
    switch (iSignal)
    {
        case SIGINT:
            printf("SIGINT received\n");
            bRun = false;
            break;
        default:
            printf("Uknown signal received\n");
    }
}


int main(int argc, char *argv[])
{
    int iFormat = AFMT_S16_LE;
    int iSampleRate;
    int iSockHandle;
    int iBytesRead;
    int iLastReadRes;
    int iDataCount = 8192 / sizeof(GDT);
    const int iMsgSize = 8192;
    char cpMsgBuf[iMsgSize];
    signed short sspOutBuf[iDataCount];
    GDT fDataBuf[iDataCount];
    stSoundStart sSndStart;
    audio_buf_info sBufInfo;
    clSockClie Client;
    clSoundMsg SoundMsg;
    clDSPOp DSP;
    clSockOp *Socket;
    clAudio *Audio;

    if (argc < 3)
    {
        printf("%s <host> <port> [device]\n", argv[0]);
        return 1;
    }
    signal(SIGINT, SigHandler);
    printf("Connecting %s:%s...\n", argv[1], argv[2]);
    iSockHandle = Client.Connect(argv[1], NULL, atoi(argv[2]));
    if (iSockHandle < 0)
    {
        printf("Unable to connect specified host!\n");
        return 1;
    }
    printf("Connected, getting starting message...\n");
    Socket = new clSockOp(iSockHandle);
    if (Socket->Read(cpMsgBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
    {
        printf("Error reading first message!\n");
        return 1;
    }
    SoundMsg.GetStart(cpMsgBuf, &sSndStart);
    printf("Got, opening audio device...\n");
    if (argc > 3)
    {
        iSampleRate = (int) (sSndStart.dSampleRate + 0.5);
        Audio = new clAudio(argv[3], &iFormat, &iSampleRate,
            &sSndStart.iChannels, AUDIO_WRITE);
    }
    else
    {
        iSampleRate = (int) (sSndStart.dSampleRate + 0.5);
        Audio = new clAudio("/dev/dsp", &iFormat, &iSampleRate, 
            &sSndStart.iChannels, AUDIO_WRITE);
    }
    printf("Audio sr: %g  ch: %i  ec: %i\n", sSndStart.dSampleRate, 
        sSndStart.iChannels, Audio->GetError());
    memset(sspOutBuf, 0x00, iDataCount * sizeof(signed short));
    do
    {
        Audio->Write(sspOutBuf, iDataCount * sizeof(signed short));
        Audio->GetOutBufInfo(&sBufInfo);
    } while (sBufInfo.bytes >= (int) (iDataCount * sizeof(signed short)));
    printf("Now playing data, press Ctrl-C to stop...\n");
    while (bRun)
    {
        iBytesRead = 0;
        do {
            iLastReadRes = Socket->Read(&cpMsgBuf[iBytesRead], 
                iMsgSize - iBytesRead);
            iBytesRead += iLastReadRes;
        } while (iBytesRead < iMsgSize && bRun && iLastReadRes > 0);
        if (iLastReadRes < 0)
        {
            printf("Receive error!\n");
            bRun = false;
        }
        SoundMsg.GetData(cpMsgBuf, fDataBuf, iDataCount);
        DSP.Convert(sspOutBuf, fDataBuf, iDataCount, false);
        Audio->Write(sspOutBuf, iDataCount * sizeof(signed short));
    }
    printf("Closing down...\n");
    Socket->Shutdown(2);
    delete Audio;
    delete Socket;
    printf("Exit.\n");
    return 0;
}

