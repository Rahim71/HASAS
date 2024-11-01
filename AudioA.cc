/*

    ALSA audio I/O
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


#ifdef USE_ALSA05


#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/asoundlib.h>

#include "AudioA.hh"


static const int iALSAStatusMax = 6;
static const char *cpALSAStatus[] =
{
    "Not ready",
    "Ready for prepare",
    "Ready to go",
    "Running",
    "Underrun",
    "Overrun",
    "Paused"
};
static const char *cpALSANullStatus = "Unknown";


clAudioA::clAudioA ()
{
    iCard = -1;
    iDevice = -1;
    spSndCtlH = NULL;
    spPcmH = NULL;
}


clAudioA::~clAudioA ()
{
    PcmClose();
    CardClose();
}


bool clAudioA::CardOpen (int iOCard)
{
    if (snd_ctl_open(&spSndCtlH, iOCard) < 0) return false;
    if (snd_ctl_hw_info(spSndCtlH, &sHWInfo) < 0) return false;
    iCard = iOCard;
    return true;
}


void clAudioA::CardClose ()
{
    if (spSndCtlH != NULL) snd_ctl_close(spSndCtlH);
}


int clAudioA::CardGetCount ()
{
    return snd_cards();
}


const char *clAudioA::CardGetName ()
{
    return ((const char *) sHWInfo.name);
}


const char *clAudioA::CardGetLongName ()
{
    return ((const char *) sHWInfo.longname);
}


int clAudioA::CardGetChannelCount ()
{
    return sHWInfo.pcmdevs;
}


bool clAudioA::PcmOpen (int iODevice, int iOSubDevice, int iOMode)
{
    if (iOSubDevice < 0)
    {
        if (snd_pcm_open(&spPcmH, iCard, iODevice, iOMode) < 0) return false;
    }
    else
    {
        if (snd_pcm_open_subdevice(&spPcmH, iCard, iODevice, iOSubDevice, 
            iOMode) < 0) return false;
    }
    memset(&sPCMInfo, 0x00, sizeof(sPCMInfo));
    if (snd_pcm_info(spPcmH, &sPCMInfo) < 0) return false;
    memset(&sPCMChInfo, 0x00, sizeof(sPCMChInfo));
    sPCMChInfo.subdevice = iOSubDevice;
    sPCMChInfo.channel = (iOMode == AA_MODE_PLAY) ?
        SND_PCM_CHANNEL_PLAYBACK : SND_PCM_CHANNEL_CAPTURE;
    #ifndef __QNX__
    sPCMChInfo.mode = -1;
    #endif
    if (snd_pcm_channel_info(spPcmH, &sPCMChInfo) < 0) return false;
    iDevice = iODevice;
    iSubDevice = iOSubDevice;
    iMode = iOMode;
    return true;
}


void clAudioA::PcmClose ()
{
    if (spPcmH != NULL) snd_pcm_close(spPcmH);
}


bool clAudioA::PcmSetSetup (int iChannels, int iSampleRate, int iBits, 
    int iFragSize, bool bStreamMode)
{
    snd_pcm_channel_params_t sChParams;

    bStream = bStreamMode;
    PcmFlush();
    memset(&sChParams, 0x00, sizeof(sChParams));
    sChParams.channel = (iMode == AA_MODE_PLAY) ?
        SND_PCM_CHANNEL_PLAYBACK : SND_PCM_CHANNEL_CAPTURE;
    sChParams.mode = (bStream) ? 
        SND_PCM_MODE_STREAM : SND_PCM_MODE_BLOCK;
    sChParams.format.interleave = 1;
    switch (iBits)
    {
        case 8:
            sChParams.format.format = SND_PCM_SFMT_U8;
            break;
        case 16:
            sChParams.format.format = SND_PCM_SFMT_S16;
            break;
        case 24:
            sChParams.format.format = SND_PCM_SFMT_S24;
            break;
        case 32:
            sChParams.format.format = SND_PCM_SFMT_S32;
            break;
        default:
            sChParams.format.format = SND_PCM_SFMT_S16;
    }
    sChParams.format.rate = iSampleRate;
    sChParams.format.voices = iChannels;
    sChParams.start_mode = (iMode == AA_MODE_PLAY) ? 
        SND_PCM_START_FULL : SND_PCM_START_GO;  // SND_PCM_START_DATA
    sChParams.stop_mode = SND_PCM_STOP_ROLLOVER;
    if (bStream)
    {
        sChParams.buf.stream.queue_size = iFragSize;
        sChParams.buf.stream.fill = (iMode == AA_MODE_PLAY) ? 
            SND_PCM_FILL_SILENCE : SND_PCM_FILL_NONE;
        sChParams.buf.stream.max_fill = iFragSize;
    }
    else
    {
        sChParams.buf.block.frag_size = iFragSize;
        sChParams.buf.block.frags_min = AA_FRAG_PLAY_LW;
        sChParams.buf.block.frags_max = AA_FRAG_PLAY_HW;
    }
    if (snd_pcm_channel_params(spPcmH, &sChParams) < 0) return false;
    memset(&sPCMSetup, 0x00, sizeof(sPCMSetup));
    sPCMSetup.channel = (iMode == AA_MODE_PLAY) ?
        SND_PCM_CHANNEL_PLAYBACK : SND_PCM_CHANNEL_CAPTURE;
    sPCMSetup.mode = (bStream) ?
        SND_PCM_MODE_STREAM : SND_PCM_MODE_BLOCK;
    if (snd_pcm_channel_setup(spPcmH, &sPCMSetup) < 0) return false;
    return true;
}


int clAudioA::PcmGetChannels ()
{
    return sPCMSetup.format.voices;
}


int clAudioA::PcmGetSampleRate ()
{
    return sPCMSetup.format.rate;
}


int clAudioA::PcmGetBits ()
{
    switch (sPCMSetup.format.format)
    {
        case SND_PCM_SFMT_U8:
            return 8;
        case SND_PCM_SFMT_S16:
            return 16;
        case SND_PCM_SFMT_S24:
            return 24;
        case SND_PCM_SFMT_S32:
            return 32;
        default:
            return -1;
    }
}


int clAudioA::PcmGetFragmentSize ()
{
    return sPCMSetup.buf.block.frag_size;
}


const char *clAudioA::PcmGetFormatName ()
{
    return snd_pcm_get_format_name(sPCMSetup.format.format);
}


int clAudioA::PcmGetStatus ()
{
    snd_pcm_channel_status_t sChStatus;

    if (!PcmGetChannelStatus(&sChStatus)) return -1;
    return sChStatus.status;
}


const char *clAudioA::PcmGetStatusStr (int iStatusCode)
{
    if (iStatusCode >= 0 && iStatusCode <= iALSAStatusMax)
        return cpALSAStatus[iStatusCode];
    else
        return cpALSANullStatus;
}


const char *clAudioA::PcmGetName ()
{
    return ((const char *) sPCMInfo.name);
}


const snd_pcm_channel_info_t *clAudioA::PcmGetChannelInfo ()
{
    return &sPCMChInfo;
}


bool clAudioA::PcmGetChannelStatus (snd_pcm_channel_status_t *spChStatus)
{
    memset(spChStatus, 0x00, sizeof(snd_pcm_channel_status_t));
    spChStatus->channel = (iMode == AA_MODE_PLAY) ?
        SND_PCM_CHANNEL_PLAYBACK : SND_PCM_CHANNEL_CAPTURE;
    spChStatus->mode = (bStream) ?
        SND_PCM_MODE_STREAM : SND_PCM_MODE_BLOCK;
    if (snd_pcm_channel_status(spPcmH, spChStatus) < 0) return false;
    return true;
}


int clAudioA::PcmGetBufUsed ()
{
    snd_pcm_channel_status_t sChStatus;

    if (!PcmGetChannelStatus(&sChStatus)) return -1;
    return sChStatus.count;
}


int clAudioA::PcmGetBufFree ()
{
    snd_pcm_channel_status_t sChStatus;

    if (!PcmGetChannelStatus(&sChStatus)) return -1;
    return sChStatus.free;
}


bool clAudioA::PcmPrepare ()
{
    if (iMode == AA_MODE_PLAY)
    {
        if (snd_pcm_playback_prepare(spPcmH) < 0) return false;
    }
    else
    {
        if (snd_pcm_capture_prepare(spPcmH) < 0) return false;
    }
    return true;
}


bool clAudioA::PcmGo ()
{
    if (iMode == AA_MODE_PLAY)
    {
        if (snd_pcm_playback_go(spPcmH) < 0) return false;
    }
    else
    {
        if (snd_pcm_capture_go(spPcmH) < 0) return false;
    }
    return true;
}


bool clAudioA::PcmDrain ()
{
    if (iMode == AA_MODE_PLAY)
    {
        if (snd_pcm_playback_drain(spPcmH) < 0) return false;
    }
    return true;
}


bool clAudioA::PcmFlush ()
{
    if (iMode == AA_MODE_PLAY)
    {
        if (snd_pcm_playback_flush(spPcmH) < 0) return false;
    }
    else
    {
        if (snd_pcm_capture_flush(spPcmH) < 0) return false;
    }
    return true;
}


ssize_t clAudioA::PcmGetTransferSize ()
{
    if (iMode == AA_MODE_PLAY)
        return snd_pcm_transfer_size(spPcmH, SND_PCM_CHANNEL_PLAYBACK);
    else
        return snd_pcm_transfer_size(spPcmH, SND_PCM_CHANNEL_CAPTURE);
}


ssize_t clAudioA::PcmRead (void *vpBuffer, size_t iBufSize)
{
    return snd_pcm_read(spPcmH, vpBuffer, iBufSize);
}


ssize_t clAudioA::PcmWrite (const void *vpBuffer, size_t iBufSize)
{
    return snd_pcm_write(spPcmH, vpBuffer, iBufSize);
}

#endif  // USE_ALSA05
