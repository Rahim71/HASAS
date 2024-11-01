/*

    ALSA audio I/O
    Copyright (C) 2000-2002 Jussi Laako

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


#ifdef USE_ALSA09


#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <alsa/asoundlib.h>

#include "AudioA2.hh"


static const char *cpEmptyName = "N/A";


clAudioA2::clAudioA2 ()
{
    sndpcmPcm = NULL;
    sndpcmInfo = NULL;
    sndpcmHwParams = NULL;
    sndpcmSwParams = NULL;
    snd_output_stdio_attach(&sndoutErr, stderr, 0);
}


clAudioA2::~clAudioA2 ()
{
    PcmClose();
    CardClose();
    snd_output_close(sndoutErr);
}


bool clAudioA2::CardOpen (int iCard)
{
    if (snd_card_load(iCard) != 0)
        return false;
    return true;
}


void clAudioA2::CardClose ()
{
}


const char *clAudioA2::CardGetName (int iCard)
{
    char *cpCardName;
    
    if (snd_card_get_name(iCard, &cpCardName) != 0)
        return cpEmptyName;
    return cpCardName;
}


const char *clAudioA2::CardGetLongName (int iCard)
{
    char *cpCardName;
    
    if (snd_card_get_name(iCard, &cpCardName) != 0)
        return cpEmptyName;
    return cpCardName;
}


bool clAudioA2::PcmOpen (const char *cpDeviceId, snd_pcm_stream_t eMode)
{
    if (snd_pcm_open(&sndpcmPcm, cpDeviceId, eMode, 0) != 0)
        return false;
    if (snd_pcm_info_malloc(&sndpcmInfo) != 0)
        return false;
    if (snd_pcm_info(sndpcmPcm, sndpcmInfo) != 0)
        return false;
    if (snd_pcm_hw_params_malloc(&sndpcmHwParams) != 0)
        return false;
    return true;
}


void clAudioA2::PcmClose ()
{
    if (sndpcmSwParams != NULL)
    {
        snd_pcm_sw_params_free(sndpcmSwParams);
        sndpcmSwParams = NULL;
    }
    if (sndpcmHwParams != NULL)
    {
        snd_pcm_hw_params_free(sndpcmHwParams);
        sndpcmHwParams = NULL;
    }
    if (sndpcmInfo != NULL)
    {
        snd_pcm_info_free(sndpcmInfo);
        sndpcmInfo = NULL;
    }
    if (sndpcmPcm != NULL)
    {
        snd_pcm_close(sndpcmPcm);
        sndpcmPcm = NULL;
    }
}


bool clAudioA2::PcmSetSetup (int iChannels, int iSampleRate, int iBits, 
    int iFragSize, int iFragCount)
{
    int iDir;

    uiFrameSize = (iBits / 8) * iChannels;
    snd_pcm_hw_params_any(sndpcmPcm, sndpcmHwParams);
//    snd_pcm_hw_params_dump(sndpcmHwParams, sndoutErr);
    if (snd_pcm_hw_params_set_access(sndpcmPcm, sndpcmHwParams, 
        SND_PCM_ACCESS_RW_INTERLEAVED) != 0)
        return false;
    switch (iBits)
    {
        case 8:
            if (snd_pcm_hw_params_set_format(sndpcmPcm, sndpcmHwParams,
                SND_PCM_FORMAT_U8) != 0)
                return false;
            break;
        case 16:
            if (snd_pcm_hw_params_set_format(sndpcmPcm, sndpcmHwParams,
                SND_PCM_FORMAT_S16) != 0)
                return false;
            break;
        case 24:
            if (snd_pcm_hw_params_set_format(sndpcmPcm, sndpcmHwParams,
                SND_PCM_FORMAT_S24) != 0)
                return false;
            break;
        case 32:
            if (snd_pcm_hw_params_set_format(sndpcmPcm, sndpcmHwParams,
                SND_PCM_FORMAT_S32) != 0)
                return false;
            break;
        /*case 64:
            if (snd_pcm_hw_params_set_format(sndpcmPcm, sndpcmHwParams,
                SND_PCM_FORMAT_FLOAT64) != 0)
                return false;
            break;*/
        default:
            return false;
    }
    if (snd_pcm_hw_params_set_channels(sndpcmPcm, sndpcmHwParams, 
        iChannels) != 0)
        return false;
    iDir = 0;
    uiActRate = snd_pcm_hw_params_set_rate_near(sndpcmPcm, sndpcmHwParams, 
        iSampleRate, &iDir);
    uiActBufSize = snd_pcm_hw_params_set_buffer_size_near(sndpcmPcm,
        sndpcmHwParams, iFragSize * iFragCount / uiFrameSize);
    uiActBufSize *= uiFrameSize;
    iDir = 0;
    uiActFragSize = snd_pcm_hw_params_set_period_size_near(sndpcmPcm,
        sndpcmHwParams, iFragSize / uiFrameSize, &iDir);
    uiActFragSize *= uiFrameSize;
    iDir = 0;
    uiActFragCount = snd_pcm_hw_params_set_periods_near(sndpcmPcm,
        sndpcmHwParams, iFragCount, &iDir);
//    snd_pcm_hw_params_dump(sndpcmHwParams, sndoutErr);
    if (snd_pcm_hw_params(sndpcmPcm, sndpcmHwParams) != 0)
        return false;
        
    if (snd_pcm_sw_params_malloc(&sndpcmSwParams) != 0)
        return false;
    if (snd_pcm_sw_params_current(sndpcmPcm, sndpcmSwParams) != 0)
        return false;
//    snd_pcm_sw_params_dump(sndpcmSwParams, sndoutErr);
    /*if (snd_pcm_sw_params_set_xrun_mode(sndpcmPcm, sndpcmSwParams,
        SND_PCM_XRUN_NONE) != 0)
        return false;*/
    if (snd_pcm_sw_params_set_stop_threshold(sndpcmPcm, sndpcmSwParams,
        0xffffffff) != 0)
        return false;
//    snd_pcm_sw_params_dump(sndpcmSwParams, sndoutErr);
    if (snd_pcm_sw_params(sndpcmPcm, sndpcmSwParams) != 0)
        return false;
    return true;
}


int clAudioA2::PcmGetChannels ()
{
    return snd_pcm_hw_params_get_channels(sndpcmHwParams);
}


int clAudioA2::PcmGetSampleRate ()
{
    return ((int) uiActRate);
}


int clAudioA2::PcmGetBits ()
{
    switch (snd_pcm_hw_params_get_format(sndpcmHwParams))
    {
        case SND_PCM_FORMAT_U8:
            return 8;
        case SND_PCM_FORMAT_S16:
            return 16;
        case SND_PCM_FORMAT_S24:
            return 24;
        case SND_PCM_FORMAT_S32:
            return 32;
        /*case SND_PCM_FORMAT_FLOAT64:
            return 64;*/
        default:
            return -1;
    }
}


int clAudioA2::PcmGetBufferSize ()
{
    return ((int) uiActBufSize);
}


int clAudioA2::PcmGetFragmentSize ()
{
    return ((int) uiActFragSize);
}


int clAudioA2::PcmGetFragmentCount ()
{
    return ((int) uiActFragCount);
}


int clAudioA2::PcmGetStatus ()
{
    return ((int) snd_pcm_state(sndpcmPcm));
}


const char *clAudioA2::PcmGetStatusStr (int iStatusCode)
{
    snd_pcm_state_t iState;
    
    iState = snd_pcm_state(sndpcmPcm);
    return snd_pcm_state_name(iState);
}


const char *clAudioA2::PcmGetName ()
{
    return snd_pcm_info_get_name(sndpcmInfo);
}


bool clAudioA2::PcmLink (clAudioA2 &Device)
{
    if (snd_pcm_link(sndpcmPcm, Device.GetPcmHandle()) != 0)
        return false;
    return true;
}


bool clAudioA2::PcmUnlink ()
{
    if (snd_pcm_unlink(sndpcmPcm) != 0)
        return false;
    return true;
}


bool clAudioA2::PcmPrepare ()
{
    if (snd_pcm_prepare(sndpcmPcm) != 0)
        return false;
    return true;
}


bool clAudioA2::PcmStart ()
{
    if (snd_pcm_start(sndpcmPcm) != 0)
        return false;
    return true;
}


bool clAudioA2::PcmReset ()
{
    if (snd_pcm_reset(sndpcmPcm) != 0)
        return false;
    return true;
}


bool clAudioA2::PcmDrain ()
{
    if (snd_pcm_drain(sndpcmPcm) != 0)
        return false;
    return true;
}


int clAudioA2::PcmRead (void *vpBuffer, int iBufSize)
{
    int iReadSize;
    
    iReadSize = snd_pcm_readi(sndpcmPcm, vpBuffer, iBufSize / uiFrameSize);
    if (iReadSize < 0)
        return iReadSize;
    return (iReadSize * uiFrameSize);
}


int clAudioA2::PcmWrite (const void *vpBuffer, int iBufSize)
{
    int iWriteSize;
    
    iWriteSize = snd_pcm_writei(sndpcmPcm, vpBuffer, iBufSize / uiFrameSize);
    if (iWriteSize < 0)
        return iWriteSize;
    return (iWriteSize * uiFrameSize);
}

#endif  // USE_ALSA09
