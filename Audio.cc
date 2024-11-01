/*

    Class for OSS audio IO operations
    Copyright (C) 1998-2002 Jussi Laako

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
#include <unistd.h>
#include <fcntl.h>
#ifdef linux
    #include <sys/ioctl.h>
#elif BSDSYS
    #include <sys/ioctl.h>
#else
    #include <ioctl.h>
#endif
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

//#include "sys/soundcard.h"
#include "Audio.hh"


clAudio::clAudio ()
{
    iDevH = -1;
    iErrorCode = 0;
}


clAudio::clAudio (const char *cpDevName, int *ipFormat, int *ipSampleRate,
    int *ipChannels, int iType)
{
    iDevH = -1;
    iErrorCode = 0;
    Open(cpDevName, ipFormat, ipSampleRate, ipChannels, iType);
}


clAudio::~clAudio ()
{
    Close();
}


bool clAudio::Open (const char *cpDevName, int *ipFormat, int *ipSampleRate, 
    int *ipChannels, int iType)
{
    int iOpenFlags;
   
    iErrorCode = 0;
    switch(iType)
    {
        case AUDIO_READ:
            iOpenFlags = O_RDONLY;
            break;
        case AUDIO_WRITE:
            iOpenFlags = O_WRONLY;
            break;
        case AUDIO_DUPLEX:
            iOpenFlags = O_RDWR;
            break;
        default:
            iOpenFlags = O_RDWR;
    }
    iDevH = open(cpDevName, iOpenFlags);
    if (iDevH < 0)
    {
        iErrorCode = errno;
        return false;
    }
    if (iType == AUDIO_DUPLEX)
    {
        if (ioctl(iDevH, SNDCTL_DSP_SETDUPLEX, 0) < 0)
        {
            iErrorCode = errno;
            return false;
        }
    }
    if (ioctl(iDevH, SNDCTL_DSP_SETFMT, ipFormat) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    if (ioctl(iDevH, SNDCTL_DSP_CHANNELS, ipChannels) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    if (ioctl(iDevH, SNDCTL_DSP_SPEED, ipSampleRate) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


void clAudio::Close ()
{
    if (iDevH >= 0) close(iDevH);
    iDevH = -1;
}


bool clAudio::SetParams (int *ipFormat, int *ipChannels, int *ipSampleRate)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_SETFMT, ipFormat) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    if (ioctl(iDevH, SNDCTL_DSP_CHANNELS, ipChannels) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    if (ioctl(iDevH, SNDCTL_DSP_SPEED, ipSampleRate) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


bool clAudio::SetFormat (int *ipFormat)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_SETFMT, ipFormat) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}



bool clAudio::SetChannels (int *ipChannels)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_CHANNELS, ipChannels) < 0)
    {
        iErrorCode = 0;
        return false;
    }
    return true;
}


bool clAudio::SetSampleRate (int *ipSampleRate)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_SPEED, ipSampleRate) < 0)
    {
        iErrorCode = 0;
        return false;
    }
    return true;
}


int clAudio::Read (void *vpData, int iBytes)
{
    int iStatus;
    int iBytesRead = 0;
    unsigned char *ucpData = (unsigned char *) vpData;

    iErrorCode = 0;
    do
    {
        iStatus = read(iDevH, &ucpData[iBytesRead], 
            (size_t) (iBytes - iBytesRead));
        if (iStatus < 0)
        {
            iErrorCode = errno;
            return iStatus;
        }
        iBytesRead += iStatus;
    } while (iBytesRead < iBytes);
    return iBytesRead;
}


int clAudio::Write (const void *vpData, int iBytes)
{
    int iStatus;
    int iBytesWritten = 0;
    const unsigned char *ucpData = (const unsigned char *) vpData;
   
    iErrorCode = 0;
    do
    {
        iStatus = write(iDevH, &ucpData[iBytesWritten], 
            (size_t) (iBytes - iBytesWritten));
        if (iStatus < 0)
        {
            iErrorCode = errno;
            return iStatus;
        }
        iBytesWritten += iStatus;
    } while (iBytesWritten < iBytes);
    return iBytesWritten;
}


bool clAudio::Sync ()
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_SYNC, 0) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


bool clAudio::Reset ()
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_RESET, 0) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


bool clAudio::Post ()
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_POST, 0) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


int clAudio::GetFragmentSize ()
{
    int iFragSize;

    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_GETBLKSIZE, &iFragSize) < 0)
    {
        iErrorCode = errno;
        return -1;
    }
    return iFragSize;
}


bool clAudio::SubDivide (int *ipSubDivide)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_SUBDIVIDE, ipSubDivide) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


bool clAudio::SetFragment (int iFragSize, int iFragCount)
{
    int iFragSize2P;
    int iFragments;

    iErrorCode = 0;
    iFragSize2P = (int) (log(iFragSize) / log(2.0) + 0.5);
    iFragments = ((iFragCount << 16) | (iFragSize2P & 0xffff));
    if (ioctl(iDevH, SNDCTL_DSP_SETFRAGMENT, &iFragments) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


int clAudio::GetFormats ()
{
    int iFormats;

    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_GETFMTS, &iFormats) < 0)
    {
        iErrorCode = errno;
        return 0;
    }
    return iFormats;
}


bool clAudio::GetInBufInfo (audio_buf_info *spInBufInfo)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_GETISPACE, spInBufInfo) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


bool clAudio::GetOutBufInfo (audio_buf_info *spOutBufInfo)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_GETOSPACE, spOutBufInfo) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


bool clAudio::GetInBufStat (count_info *spInBufStat)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_GETIPTR, spInBufStat) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


bool clAudio::GetOutBufStat (count_info *spOutBufStat)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_GETOPTR, spOutBufStat) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


int clAudio::GetCaps ()
{
    int iCaps;
   
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_GETCAPS, &iCaps) < 0)
    {
        iErrorCode = errno;
        return 0;
    }
    return iCaps;
}


bool clAudio::SetDuplex ()
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_SETDUPLEX, 0) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


bool clAudio::SetNonBlock ()
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_NONBLOCK, 0) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


bool clAudio::SetSynchronous ()
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_SETSYNCRO, 0) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


int clAudio::GetTrigger ()
{
    int iTrigger;

    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_GETTRIGGER, &iTrigger) < 0)
    {
        iErrorCode = errno;
        return 0;
    }
    return iTrigger;
}


bool clAudio::SetTrigger (int iTrigger)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_SETTRIGGER, &iTrigger) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


bool clAudio::MapInBuffer (buffmem_desc *spBufferInfo)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_MAPINBUF, spBufferInfo) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


bool clAudio::MapOutBuffer (buffmem_desc *spBufferInfo)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_MAPOUTBUF, spBufferInfo) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}


int clAudio::GetOutDelay ()
{
    int iDelay;

    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_GETODELAY, &iDelay) < 0)
    {
        iErrorCode = errno;
        return -1;
    }
    return iDelay;
}


#ifndef USE_OSSLITE
bool clAudio::GetErrorInfo (audio_errinfo *spErrorInfo)
{
    iErrorCode = 0;
    if (ioctl(iDevH, SNDCTL_DSP_GETERROR, spErrorInfo) < 0)
    {
        iErrorCode = errno;
        return false;
    }
    return true;
}
#endif


int clAudio::GetVersion ()
{
    int iVersion;

    iErrorCode = 0;
    if (ioctl(iDevH, OSS_GETVERSION, &iVersion) < 0)
    {
        iErrorCode = errno;
        return 0;
    }
    return iVersion;
}


void clAudio::DeIntStereo (const signed short *sspSource, 
    signed short *sspLeft, signed short *sspRight, unsigned int uiSourceSize)
{
    unsigned int uiSrcIdx = 0;
    unsigned int uiDstIdx = 0;
   
    do
    {
        sspLeft[uiDstIdx] = sspSource[uiSrcIdx++];
        sspRight[uiDstIdx++] = sspSource[uiSrcIdx++];
    } while (uiSrcIdx < uiSourceSize);
}


void clAudio::DeIntStereo (const unsigned char *ucpSource,
    unsigned char *ucpLeft, unsigned char *ucpRight, unsigned int uiSourceSize)
{
    unsigned int uiSrcIdx = 0;
    unsigned int uiDstIdx = 0;
   
    do {
        ucpLeft[uiDstIdx] = ucpSource[uiSrcIdx++];
        ucpRight[uiDstIdx++] = ucpSource[uiSrcIdx++];
    } while (uiSrcIdx < uiSourceSize);
}


void clAudio::IntStereo(signed short *sspDest, 
    const signed short *sspLeft, const signed short *sspRight, 
    unsigned int uiSourceSize)
{
    unsigned int uiSrcIdx = 0;
    unsigned int uiDstIdx = 0;
   
    do {
        sspDest[uiDstIdx++] = sspLeft[uiSrcIdx];
        sspDest[uiDstIdx++] = sspRight[uiSrcIdx++];
    } while (uiSrcIdx < uiSourceSize);
}


void clAudio::IntStereo(unsigned char *ucpDest,
    const unsigned char *ucpLeft, const unsigned char *ucpRight,
    unsigned int uiSourceSize)
{
    unsigned int uiSrcIdx = 0;
    unsigned int uiDstIdx = 0;
   
    do {
        ucpDest[uiDstIdx++] = ucpLeft[uiSrcIdx];
        ucpDest[uiDstIdx++] = ucpRight[uiSrcIdx++];
    } while (uiSrcIdx < uiSourceSize);
}

