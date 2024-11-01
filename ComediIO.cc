/*

    ComediIO
    Copyright (C) 2002 Jussi Laako
    
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


#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ComediIO.hh"


void clComediIO::SetError ()
{
    if (cpErrorMsg != NULL)
        free(cpErrorMsg);
    cpErrorMsg = strdup(comedi_strerror(comedi_errno()));
}


bool clComediIO::PcmStart (int iSubDev, double *dpSampleRate, int iChannels,
    unsigned int *uipScanList, unsigned int uiListLength, bool bDither,
    bool bDoubleClock)
{
    struct comedi_cmd_struct sComediCmd;
    
    memset(&sComediCmd, 0x00, sizeof(sComediCmd));
    sComediCmd.subdev = (unsigned int) iSubDev;
    sComediCmd.flags = ((bDither) ? TRIG_DITHER : 0) | TRIG_DEGLITCH;
    sComediCmd.start_src = TRIG_NOW;
    sComediCmd.start_arg = 0;
    sComediCmd.scan_begin_src = TRIG_TIMER;
    sComediCmd.scan_begin_arg = (unsigned int) 
        (1.0 / *dpSampleRate * 1.0e9 + 0.5);
    /*sComediCmd.scan_begin_src = TRIG_NONE;
    sComediCmd.scan_begin_arg = 0;*/
    if (bDoubleClock)
    {
        sComediCmd.convert_src = TRIG_TIMER;
        sComediCmd.convert_arg = (unsigned int)
            (1.0 / (*dpSampleRate * iChannels) * 1.0e9 + 0.5);
    }
    else
    {
        sComediCmd.convert_src = TRIG_NONE;
        sComediCmd.convert_arg = 0;
    }
    sComediCmd.scan_end_src = TRIG_NONE;
    sComediCmd.scan_end_arg = 0;
    sComediCmd.stop_src = TRIG_NONE;
    sComediCmd.stop_arg = 0;
    sComediCmd.chanlist = uipScanList;
    sComediCmd.chanlist_len = uiListLength;
    
    comedi_command_test(comediDev, &sComediCmd);
    *dpSampleRate =
        1.0 / ((double) sComediCmd.scan_begin_arg / 1.0e9);
    /**dpSampleRate =
        1.0 / ((double) sComediCmd.convert_arg * iChannels / 1.0e9);*/
    if (comedi_command(comediDev, &sComediCmd) != 0)
    {
        SetError();
        return false;
    }
    return true;
}


long clComediIO::PcmRead (sampl_t **ipDest, long lCount)
{
    int iInSize;
    int iReadRes;
    int iBytesRead = 0;
    char *cpInBuf;
    sampl_t *ipInBuf;
    
    iInSize = lCount * sizeof(sampl_t);
    ipInBuf = (sampl_t *) InBuf.Size(lCount * sizeof(sampl_t));
    cpInBuf = InBuf;
    *ipDest = ipInBuf;
    do
    {
        iReadRes = read(comedi_fileno(comediDev), 
            &cpInBuf[iBytesRead], iInSize - iBytesRead);
        iBytesRead += iReadRes;
    } while (iBytesRead < iInSize && iReadRes >= 0);
    return (iBytesRead / sizeof(sampl_t));
}


clComediIO::clComediIO ()
{
    cpErrorMsg = NULL;
    comediDev = NULL;
    iInSubDev = -1;
}


clComediIO::~clComediIO ()
{
    Close();
}


bool clComediIO::Open (const char *cpDevice)
{
    comediDev = comedi_open(cpDevice);
    if (comediDev == NULL)
    {
        SetError();
        return false;
    }
    return true;
}


bool clComediIO::Close ()
{
    if (iInSubDev >= 0)
    {
        if (comedi_unlock(comediDev, (unsigned int) iInSubDev) != 0)
        {
            SetError();
            return false;
        }
        iInSubDev = -1;
    }
    if (comediDev != NULL)
    {
        if (comedi_close(comediDev) != 0)
        {
            SetError();
            return false;
        }
        comediDev = NULL;
    }
    if (cpErrorMsg != NULL)
    {
        free(cpErrorMsg);
        cpErrorMsg = NULL;
    }
    return true;
}


int clComediIO::GetSubdeviceCount ()
{
    return comedi_get_n_subdevices(comediDev);
}


int clComediIO::GetVersionCode ()
{
    int iRetVal;
    
    iRetVal = comedi_get_version_code(comediDev);
    if (iRetVal < 0)
        SetError();
    return iRetVal;
}


const char *clComediIO::GetDriverName ()
{
    return comedi_get_driver_name(comediDev);
}


const char *clComediIO::GetBoardName ()
{
    return comedi_get_board_name(comediDev);
}


bool clComediIO::PcmInOpen ()
{
    iInSubDev = comedi_get_read_subdevice(comediDev);
    if (iInSubDev < 0)
    {
        SetError();
        return false;
    }
    if (comedi_lock(comediDev, (unsigned int) iInSubDev) != 0)
    {
        iInSubDev = -1;
        SetError();
        return false;
    }
    iInMaxValue = comedi_get_maxdata(comediDev, (unsigned int) iInSubDev, 0);
    if (iInMaxValue == 0)
    {
        SetError();
        return false;
    }
    return true;
}


bool clComediIO::PcmInClose ()
{
    if (iInSubDev >= 0)
    {
        if (comedi_unlock(comediDev, (unsigned int) iInSubDev) != 0)
        {
            SetError();
            return false;
        }
        iInSubDev = -1;
    }
    return true;
}


int clComediIO::PcmInGetChannelCount ()
{
    int iRetVal;
    
    iRetVal = comedi_get_n_channels(comediDev, (unsigned int) iInSubDev);
    if (iRetVal < 0)
        SetError();
    return iRetVal;
}


bool clComediIO::PcmInStart (double *dpSampleRate, int iChannels,
    double dRange, bool bDither, bool bDoubleClock)
{
    int iRange;
    int iChanCntr;
    unsigned int *uipScanList;
    
    iRange = comedi_find_range(comediDev, (unsigned int) iInSubDev, 0, 
        UNIT_volt, -dRange, dRange);
    if (iRange < 0)
    {
        SetError();
        return false;
    }
    uipScanList = (unsigned int *) 
        InScanList.Size(iChannels * sizeof(unsigned int));
    for (iChanCntr = 0; iChanCntr < iChannels; iChanCntr++)
        uipScanList[iChanCntr] = CR_PACK(iChanCntr, iRange, AREF_GROUND);
    return PcmStart(iInSubDev, dpSampleRate, iChannels, uipScanList,
        iChannels, bDither, bDoubleClock);
}


bool clComediIO::PcmInStop ()
{
    if (comedi_cancel(comediDev, (unsigned int) iInSubDev) != 0)
    {
        SetError();
        return false;
    }
    return true;
}


long clComediIO::PcmInRead (float *fpDest, long lCount)
{
    int iSampleCntr;
    long lReadRes;
    sampl_t *ipInBuf;
    float fScale;
    
    fScale = 1.0f / iInMaxValue;
    lReadRes = PcmRead(&ipInBuf, lCount);
    if (lReadRes < 0)
    {
        SetError();
        return lReadRes;
    }
    for (iSampleCntr = 0; iSampleCntr < lReadRes; iSampleCntr++)
        fpDest[iSampleCntr] = (float) ipInBuf[iSampleCntr] * fScale;
    return lReadRes;
}


long clComediIO::PcmInRead (double *dpDest, long lCount)
{
    int iSampleCntr;
    long lReadRes;
    sampl_t *ipInBuf;
    double dScale;
    
    dScale = 1.0 / iInMaxValue;
    lReadRes = PcmRead(&ipInBuf, lCount);
    if (lReadRes < 0)
    {
        SetError();
        return lReadRes;
    }
    for (iSampleCntr = 0; iSampleCntr < lReadRes; iSampleCntr++)
        dpDest[iSampleCntr] = (double) ipInBuf[iSampleCntr] * dScale;
    return lReadRes;
}


bool clComediIO::PcmBufferSizeSet (unsigned int uiBufSize)
{
    /*if (comedi_set_max_buffer_size(comediDev, (unsigned int) iInSubDev, 
        uiBufSize) < 0)
    {
        SetError();
        return false;
    }*/
    if (comedi_set_buffer_size(comediDev, (unsigned int) iInSubDev, 
        uiBufSize) < 0)
    {
        SetError();
        return false;
    }
    return true;
}


int clComediIO::PcmBufferSizeGet ()
{
    int iRetVal;
    
    iRetVal = comedi_get_buffer_size(comediDev, (unsigned int) iInSubDev);
    if (iRetVal < 0)
        SetError();
    return iRetVal;
}
