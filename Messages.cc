/*

    Messaging
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


#ifdef HAVE_ENDIAN_H
    #include <endian.h>
#endif
#include <stdio.h>
#ifdef USE_G_SNPRINTF
    #include <glib.h>
#endif

#include "Config.h"
#include "Messages.hh"


inline void clBaseMsg::EndianConv (float *fpDest, const float *fpSrc)
{
    #ifdef LITTLE_ENDIAN
        unsigned int *upDest = (unsigned int *) fpDest;
        unsigned int *upSrc = (unsigned int *) fpSrc;

        #ifndef __alpha__
            asm ("bswap %0" : "=r" (*upDest) : "0" (*upSrc));
        #else
            #ifndef __G_LIB_H__
            #error "glib needed"
            #endif
            *upDest = GUINT32_SWAP_LE_BE_CONSTANT(*upSrc);
        #endif
    #else
        *fpDest = *fpSrc;
    #endif
}


inline void clBaseMsg::EndianConv (double *dpDest, const double *dpSrc)
{
    #ifdef LITTLE_ENDIAN
        #ifndef __alpha__
        unsigned int *upDest = (unsigned int *) dpDest;
        unsigned int *upSrc = (unsigned int *) dpSrc;

            asm ("bswap %0" : "=r" (upDest[1]) : "0" (upSrc[0]));
            asm ("bswap %0" : "=r" (upDest[0]) : "0" (upSrc[1]));
        #else
            #ifndef __G_LIB_H__
            #error "glib needed"
            #endif
            unsigned long long *upDest = (unsigned long long *) dpDest;
            unsigned long long *upSrc = (unsigned long long *) dpSrc;
            
            *upDest = GUINT64_SWAP_LE_BE_CONSTANT(*upSrc);
        #endif
    #else
        *dpDest = *dpSrc;
    #endif
}


void clSoundMsg::SetStart (char *cpMsgBuf, const stpSoundStart spSndStart)
{
    #ifndef USE_G_SNPRINTF
        sprintf(cpMsgBuf, "%i %g %i %i", 
            spSndStart->iChannels, 
            spSndStart->dSampleRate, 
            spSndStart->iFragmentSize,
            spSndStart->iCompress);
    #else
        g_snprintf(cpMsgBuf, GLOBAL_HEADER_LEN, "%i %g %i %i", 
            spSndStart->iChannels, 
            spSndStart->dSampleRate,
            spSndStart->iFragmentSize,
            spSndStart->iCompress);
    #endif
}


void clSoundMsg::GetStart (const char *cpMsgBuf, stpSoundStart spSndStart)
{
    sscanf(cpMsgBuf, "%i %lf %i %i", 
        &spSndStart->iChannels, 
        &spSndStart->dSampleRate,
        &spSndStart->iFragmentSize,
        &spSndStart->iCompress);
}


void clSoundMsg::SetData (void *vpMsgBuf, const struct timeval *spTimeStamp, 
    const float *fpData, int iDataLen)
{
    int iLoopCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];
    
    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr, "%li:%li %i", (long) spTimeStamp->tv_sec, 
            (long) spTimeStamp->tv_usec, iDataLen);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN, "%li:%li %i", 
            (long) spTimeStamp->tv_sec, (long) spTimeStamp->tv_usec, iDataLen);
    #endif
    for (iLoopCntr = 0; iLoopCntr < iDataLen; iLoopCntr++)
    {
        EndianConv(&fpDataPtr[iLoopCntr], &fpData[iLoopCntr]);
    }
}


void clSoundMsg::SetData (void *vpMsgBuf, const struct timeval *spTimeStamp, 
    const double *dpData, int iDataLen)
{
    int iLoopCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr, "%li:%li %i", (long) spTimeStamp->tv_sec, 
            (long) spTimeStamp->tv_usec, iDataLen);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN, "%li:%li %i", 
            (long) spTimeStamp->tv_sec, (long) spTimeStamp->tv_usec, iDataLen);
    #endif
    for (iLoopCntr = 0; iLoopCntr < iDataLen; iLoopCntr++)
    {
        EndianConv(&dpDataPtr[iLoopCntr], &dpData[iLoopCntr]);
    }
}


void clSoundMsg::SetData (void *vpMsgBuf, const float *fpData, int iDataLen)
{
    int iLoopCntr;
    float *fpDataPtr = (float *) vpMsgBuf;

    for (iLoopCntr = 0; iLoopCntr < iDataLen; iLoopCntr++)
    {
        EndianConv(&fpDataPtr[iLoopCntr], &fpData[iLoopCntr]);
    }
}


void clSoundMsg::SetData (void *vpMsgBuf, const double *dpData, int iDataLen)
{
    int iLoopCntr;
    double *dpDataPtr = (double *) vpMsgBuf;

    for (iLoopCntr = 0; iLoopCntr < iDataLen; iLoopCntr++)
    {
        EndianConv(&dpDataPtr[iLoopCntr], &dpData[iLoopCntr]);
    }
}


int clSoundMsg::GetData (const void *vpMsgBuf, struct timeval *spTimeStamp, 
    float *fpData)
{
    int iDataLen;
    int iLoopCntr;
    const char *cpHdrPtr = (char *) vpMsgBuf;
    const float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef __QNX__
    sscanf(cpHdrPtr, "%li:%li %i", &spTimeStamp->tv_sec, 
        &spTimeStamp->tv_usec, &iDataLen);
    #else
    sscanf(cpHdrPtr, "%i:%i %i", &spTimeStamp->tv_sec,
        &spTimeStamp->tv_usec, &iDataLen);
    #endif
    for (iLoopCntr = 0; iLoopCntr < iDataLen; iLoopCntr++)
    {
        EndianConv(&fpData[iLoopCntr], &fpDataPtr[iLoopCntr]);
    }
    return iDataLen;
}


int clSoundMsg::GetData (const void *vpMsgBuf, struct timeval *spTimeStamp,
    double *dpData)
{
    int iDataLen;
    int iLoopCntr;
    const char *cpHdrPtr = (char *) vpMsgBuf;
    const double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef __QNX__
    sscanf(cpHdrPtr, "%li:%li %i", &spTimeStamp->tv_sec,
        &spTimeStamp->tv_usec, &iDataLen);
    #else
    sscanf(cpHdrPtr, "%i:%i %i", &spTimeStamp->tv_sec,
        &spTimeStamp->tv_usec, &iDataLen);
    #endif
    for (iLoopCntr = 0; iLoopCntr < iDataLen; iLoopCntr++)
    {
        EndianConv(&dpData[iLoopCntr], &dpDataPtr[iLoopCntr]);
    }
    return iDataLen;
}


void clSoundMsg::GetData (const void *vpMsgBuf, float *fpData, int iDataLen)
{
    int iLoopCntr;
    const float *fpDataPtr = (float *) vpMsgBuf;

    for (iLoopCntr = 0; iLoopCntr < iDataLen; iLoopCntr++)
    {
        EndianConv(&fpData[iLoopCntr], &fpDataPtr[iLoopCntr]);
    }
}


void clSoundMsg::GetData (const void *vpMsgBuf, double *dpData, int iDataLen)
{
    int iLoopCntr;
    const double *dpDataPtr = (double *) vpMsgBuf;

    for (iLoopCntr = 0; iLoopCntr < iDataLen; iLoopCntr++)
    {
        EndianConv(&dpData[iLoopCntr], &dpDataPtr[iLoopCntr]);
    }
}


void clSpectMsg::SetRequest (char *cpReqMsg, const stpSpectReq spSpectReq)
{
    int iLinear;
    int iNormalize;

    iLinear = (spSpectReq->bLinear) ? MSG_TRUE : MSG_FALSE;
    iNormalize = (spSpectReq->bNormalize) ? MSG_TRUE : MSG_FALSE;
    #ifndef USE_G_SNPRINTF
        sprintf(cpReqMsg, 
            "%i %i %i %g %li %i %i %g %g %i %i %i %i %g %li %li %g",
            spSpectReq->iChannel,
            spSpectReq->iType,
            spSpectReq->iWindow,
            spSpectReq->fWinParam,
            spSpectReq->lLength,
            spSpectReq->iLowFreq,
            spSpectReq->iHighFreq,
            spSpectReq->fGain,
            spSpectReq->fSlope,
            spSpectReq->iOverlap,
            iLinear,
            iNormalize,
            spSpectReq->iRemoveNoise,
            spSpectReq->fAlpha,
            spSpectReq->lMeanLength,
            spSpectReq->lGapLength,
            spSpectReq->fDynRange);
    #else
        g_snprintf(cpReqMsg, GLOBAL_HEADER_LEN,
            "%i %i %i %g %li %i %i %g %g %i %i %i %i %g %li %li %g",
            spSpectReq->iChannel,
            spSpectReq->iType,
            spSpectReq->iWindow,
            spSpectReq->fWinParam,
            spSpectReq->lLength,
            spSpectReq->iLowFreq,
            spSpectReq->iHighFreq,
            spSpectReq->fGain,
            spSpectReq->fSlope,
            spSpectReq->iOverlap,
            iLinear,
            iNormalize,
            spSpectReq->iRemoveNoise,
            spSpectReq->fAlpha,
            spSpectReq->lMeanLength,
            spSpectReq->lGapLength,
            spSpectReq->fDynRange);
    #endif
}


void clSpectMsg::GetRequest (const char *cpReqMsg, stpSpectReq spSpectReq)
{
    int iLinear;
    int iNormalize;

    sscanf(cpReqMsg, "%i %i %i %g %li %i %i %g %g %i %i %i %i %g %li %li %g",
        &spSpectReq->iChannel,
        &spSpectReq->iType,
        &spSpectReq->iWindow,
        &spSpectReq->fWinParam,
        &spSpectReq->lLength,
        &spSpectReq->iLowFreq,
        &spSpectReq->iHighFreq,
        &spSpectReq->fGain,
        &spSpectReq->fSlope,
        &spSpectReq->iOverlap,
        &iLinear,
        &iNormalize,
        &spSpectReq->iRemoveNoise,
        &spSpectReq->fAlpha,
        &spSpectReq->lMeanLength,
        &spSpectReq->lGapLength,
        &spSpectReq->fDynRange);
    spSpectReq->bLinear = (iLinear == MSG_TRUE) ? true : false;
    spSpectReq->bNormalize = (iNormalize == MSG_TRUE) ? true : false;
}


void clSpectMsg::SetResult (void *vpMsgBuf, const stpSpectRes spSpectHdr, 
    const float *fpSpectData)
{
    int iLinear;
    long lLoopCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    iLinear = (spSpectHdr->bLinear) ? MSG_TRUE : MSG_FALSE;
    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr, "%li:%li %i %li %i %i %i %i %.3f %.3f",
            (long) spSpectHdr->sTimeStamp.tv_sec, 
            (long) spSpectHdr->sTimeStamp.tv_usec,
            spSpectHdr->iChannel,
            spSpectHdr->lLength,
            spSpectHdr->iLowFreq,
            spSpectHdr->iHighFreq,
            spSpectHdr->iSampleRate,
            iLinear,
            spSpectHdr->fPeakLevel,
            spSpectHdr->fLineTime);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN,
            "%li:%li %i %li %i %i %i %i %.3f %.3f",
            (long) spSpectHdr->sTimeStamp.tv_sec, 
            (long) spSpectHdr->sTimeStamp.tv_usec,
            spSpectHdr->iChannel,
            spSpectHdr->lLength,
            spSpectHdr->iLowFreq,
            spSpectHdr->iHighFreq,
            spSpectHdr->iSampleRate,
            iLinear,
            spSpectHdr->fPeakLevel,
            spSpectHdr->fLineTime);
    #endif
    for (lLoopCntr = 0; lLoopCntr < spSpectHdr->lLength; lLoopCntr++)
    {
        EndianConv(&fpDataPtr[lLoopCntr], &fpSpectData[lLoopCntr]);
    }
}


void clSpectMsg::SetResult (void *vpMsgBuf, const stpSpectRes spSpectHdr,
    const double *dpSpectData)
{
    int iLinear;
    long lLoopCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    iLinear = (spSpectHdr->bLinear) ? MSG_TRUE : MSG_FALSE;
    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr, "%li:%li %i %li %i %i %i %i %.3f %.3f",
            (long) spSpectHdr->sTimeStamp.tv_sec, 
            (long) spSpectHdr->sTimeStamp.tv_usec,
            spSpectHdr->iChannel,
            spSpectHdr->lLength,
            spSpectHdr->iLowFreq,
            spSpectHdr->iHighFreq,
            spSpectHdr->iSampleRate,
            iLinear,
            spSpectHdr->fPeakLevel,
            spSpectHdr->fLineTime);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN,
            "%li:%li %i %li %i %i %i %i %.3f %.3f",
            (long) spSpectHdr->sTimeStamp.tv_sec, 
            (long) spSpectHdr->sTimeStamp.tv_usec,
            spSpectHdr->iChannel,
            spSpectHdr->lLength,
            spSpectHdr->iLowFreq,
            spSpectHdr->iHighFreq,
            spSpectHdr->iSampleRate,
            iLinear,
            spSpectHdr->fPeakLevel,
            spSpectHdr->fLineTime);
    #endif
    for (lLoopCntr = 0; lLoopCntr < spSpectHdr->lLength; lLoopCntr++)
    {
        EndianConv(&dpDataPtr[lLoopCntr], &dpSpectData[lLoopCntr]);
    }
}


void clSpectMsg::GetResult (const void *vpMsgBuf, stpSpectRes spSpectHdr,
    float *fpSpectData)
{
    int iLinear;
    long lLoopCntr;
    const char *cpHdrPtr = (char *) vpMsgBuf;
    const float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef __QNX__
    sscanf(cpHdrPtr, "%li:%li %i %li %i %i %i %i %f %f",
    #else
    sscanf(cpHdrPtr, "%i:%i %i %li %i %i %i %i %f %f",
    #endif
        &spSpectHdr->sTimeStamp.tv_sec, &spSpectHdr->sTimeStamp.tv_usec,
        &spSpectHdr->iChannel,
        &spSpectHdr->lLength,
        &spSpectHdr->iLowFreq,
        &spSpectHdr->iHighFreq,
        &spSpectHdr->iSampleRate,
        &iLinear,
        &spSpectHdr->fPeakLevel,
        &spSpectHdr->fLineTime);
    spSpectHdr->bLinear = (iLinear == MSG_TRUE) ? true : false;
    for (lLoopCntr = 0; lLoopCntr < spSpectHdr->lLength; lLoopCntr++)
    {
        EndianConv(&fpSpectData[lLoopCntr], &fpDataPtr[lLoopCntr]);
    }
}


void clSpectMsg::GetResult (const void *vpMsgBuf, stpSpectRes spSpectHdr,
    double *dpSpectData)
{
    int iLinear;
    long lLoopCntr;
    const char *cpHdrPtr = (char *) vpMsgBuf;
    const double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef __QNX__
    sscanf(cpHdrPtr, "%li:%li %i %li %i %i %i %i %f %f",
    #else
    sscanf(cpHdrPtr, "%i:%i %i %li %i %i %i %i %f %f",
    #endif
        &spSpectHdr->sTimeStamp.tv_sec, &spSpectHdr->sTimeStamp.tv_usec,
        &spSpectHdr->iChannel,
        &spSpectHdr->lLength,
        &spSpectHdr->iLowFreq,
        &spSpectHdr->iHighFreq,
        &spSpectHdr->iSampleRate,
        &iLinear,
        &spSpectHdr->fPeakLevel,
        &spSpectHdr->fLineTime);
    spSpectHdr->bLinear = (iLinear == MSG_TRUE) ? true: false;
    for (lLoopCntr = 0; lLoopCntr < spSpectHdr->lLength; lLoopCntr++)
    {
        EndianConv(&dpSpectData[lLoopCntr], &dpDataPtr[lLoopCntr]);
    }
}


void clDirMsg::SetRequest (char *cpMsgBuf, const stpDirReq spDirRq)
{
    int iNormalize;
    int iDisableFilter;

    iNormalize = (spDirRq->bNormalize) ? MSG_TRUE : MSG_FALSE;
    iDisableFilter = (spDirRq->bDisableFilter) ? MSG_TRUE : MSG_FALSE;
    #ifndef USE_G_SNPRINTF
        sprintf(cpMsgBuf, 
            "%i %.3f %.3f %.5f %i %.3f %i %.5f %.5f %li %i %g %li %li %i",
            spDirRq->iAlgorithm,
            spDirRq->fSoundSpeed,
            spDirRq->fLowFreqLimit,
            spDirRq->fIntegrationTime,
            spDirRq->iScaling,
            spDirRq->fScalingExp,
            iNormalize,
            spDirRq->fLeftDir,
            spDirRq->fRightDir,
            spDirRq->lSectorCount,
            spDirRq->iRemoveNoise,
            spDirRq->fAlpha,
            spDirRq->lMeanLength,
            spDirRq->lGapLength,
            iDisableFilter);
    #else
        g_snprintf(cpMsgBuf, GLOBAL_HEADER_LEN,
            "%i %.3f %.3f %.5f %i %.3f %i %.5f %.5f %li %i %g %li %li %i",
            spDirRq->iAlgorithm,
            spDirRq->fSoundSpeed,
            spDirRq->fLowFreqLimit,
            spDirRq->fIntegrationTime,
            spDirRq->iScaling,
            spDirRq->fScalingExp,
            iNormalize,
            spDirRq->fLeftDir,
            spDirRq->fRightDir,
            spDirRq->lSectorCount,
            spDirRq->iRemoveNoise,
            spDirRq->fAlpha,
            spDirRq->lMeanLength,
            spDirRq->lGapLength,
            iDisableFilter);
    #endif
}


void clDirMsg::GetRequest (const char *cpMsgBuf, stpDirReq spDirRq)
{
    int iNormalize;
    int iDisableFilter;

    sscanf(cpMsgBuf, "%i %f %f %f %i %f %i %f %f %li %i %g %li %li %i",
        &spDirRq->iAlgorithm,
        &spDirRq->fSoundSpeed,
        &spDirRq->fLowFreqLimit,
        &spDirRq->fIntegrationTime,
        &spDirRq->iScaling,
        &spDirRq->fScalingExp,
        &iNormalize,
        &spDirRq->fLeftDir,
        &spDirRq->fRightDir,
        &spDirRq->lSectorCount,
        &spDirRq->iRemoveNoise,
        &spDirRq->fAlpha,
        &spDirRq->lMeanLength,
        &spDirRq->lGapLength,
        &iDisableFilter);
    spDirRq->bNormalize = (iNormalize == MSG_TRUE) ? true : false;
    spDirRq->bDisableFilter = (iDisableFilter == MSG_TRUE) ? true : false;
}


void clDirMsg::SetResult (void *vpMsgBuf, const stpDirRes spResHdr,
    const float *fpResData)
{
    long lDataCntr;
    int i3DArray;
    char *cpHdrPtr = (char *) vpMsgBuf;
    float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    i3DArray = (spResHdr->b3DArray) ? MSG_TRUE : MSG_FALSE;
    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr, "%li:%li %.5f %.3f %.3f %li %i",
            (long) spResHdr->sTimeStamp.tv_sec, 
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->fIntegrationTime,
            spResHdr->fHighFreqLimit,
            spResHdr->fPeakLevel,
            spResHdr->lSectorCount,
            i3DArray);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN,
            "%li:%li %.5f %.3f %.3f %li %i",
            (long) spResHdr->sTimeStamp.tv_sec, 
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->fIntegrationTime,
            spResHdr->fHighFreqLimit,
            spResHdr->fPeakLevel,
            spResHdr->lSectorCount,
            i3DArray);
    #endif
    for (lDataCntr = 0; lDataCntr < spResHdr->lSectorCount; lDataCntr++)
    {
        EndianConv(&fpDataPtr[lDataCntr], &fpResData[lDataCntr]);
    }
}


void clDirMsg::SetResult (void *vpMsgBuf, const stpDirRes spResHdr,
    const double *dpResData)
{
    long lDataCntr;
    int i3DArray;
    char *cpHdrPtr = (char *) vpMsgBuf;
    double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    i3DArray = (spResHdr->b3DArray) ? MSG_TRUE : MSG_FALSE;
    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr, "%li:%li %.5f %.3f %.3f %li %i",
            (long) spResHdr->sTimeStamp.tv_sec, 
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->fIntegrationTime,
            spResHdr->fHighFreqLimit,
            spResHdr->fPeakLevel,
            spResHdr->lSectorCount,
            i3DArray);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN,
            "%li:%li %.5f %.3f %.3f %li %i",
            (long) spResHdr->sTimeStamp.tv_sec, 
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->fIntegrationTime,
            spResHdr->fHighFreqLimit,
            spResHdr->fPeakLevel,
            spResHdr->lSectorCount,
            i3DArray);
    #endif
    for (lDataCntr = 0; lDataCntr < spResHdr->lSectorCount; lDataCntr++)
    {
        EndianConv(&dpDataPtr[lDataCntr], &dpResData[lDataCntr]);
    }
}


void clDirMsg::GetResult (const void *vpMsgBuf, stpDirRes spResHdr,
    float *fpResData)
{
    long lDataCntr;
    int i3DArray;
    char *cpHdrPtr = (char *) vpMsgBuf;
    float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef __QNX__
    sscanf(cpHdrPtr, "%li:%li %f %f %f %li %i",
    #else
    sscanf(cpHdrPtr, "%i:%i %f %f %f %li %i",
    #endif
        &spResHdr->sTimeStamp.tv_sec, &spResHdr->sTimeStamp.tv_usec,
        &spResHdr->fIntegrationTime,
        &spResHdr->fHighFreqLimit,
        &spResHdr->fPeakLevel,
        &spResHdr->lSectorCount,
        &i3DArray);
    spResHdr->b3DArray = (i3DArray == MSG_TRUE) ? true : false;
    for (lDataCntr = 0; lDataCntr < spResHdr->lSectorCount; lDataCntr++)
    {
        EndianConv(&fpResData[lDataCntr], &fpDataPtr[lDataCntr]);
    }
}


void clDirMsg::GetResult (const void *vpMsgBuf, stpDirRes spResHdr,
    double *dpResData)
{
    long lDataCntr;
    int i3DArray;
    char *cpHdrPtr = (char *) vpMsgBuf;
    double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef __QNX__
    sscanf(cpHdrPtr, "%li:%li %f %f %f %li %i",
    #else
    sscanf(cpHdrPtr, "%i:%i %f %f %f %li %i",
    #endif
        &spResHdr->sTimeStamp.tv_sec, &spResHdr->sTimeStamp.tv_usec,
        &spResHdr->fIntegrationTime,
        &spResHdr->fHighFreqLimit,
        &spResHdr->fPeakLevel,
        &spResHdr->lSectorCount,
        &i3DArray);
    spResHdr->b3DArray = (i3DArray == MSG_TRUE) ? true : false;
    for (lDataCntr = 0; lDataCntr < spResHdr->lSectorCount; lDataCntr++)
    {
        EndianConv(&dpResData[lDataCntr], &dpDataPtr[lDataCntr]);
    }
}


void clDirMsg2::SetRequest (char *cpMsgBuf, const stpDirReq2 spDirRq)
{
    #ifndef USE_G_SNPRINTF
        sprintf(cpMsgBuf, 
            "%li %.3f %.3f %.5f %i %.3f %i %g %li %li",
            spDirRq->lWindowSize,
            spDirRq->fSoundSpeed,
            spDirRq->fLowFreqLimit,
            spDirRq->fIntegrationTime,
            spDirRq->iScaling,
            spDirRq->fScalingExp,
            spDirRq->iRemoveNoise,
            spDirRq->fAlpha,
            spDirRq->lMeanLength,
            spDirRq->lGapLength);
    #else
        g_snprintf(cpMsgBuf, GLOBAL_HEADER_LEN,
            "%li %.3f %.3f %.5f %i %.3f %i %g %li %li",
            spDirRq->lWindowSize,
            spDirRq->fSoundSpeed,
            spDirRq->fLowFreqLimit,
            spDirRq->fIntegrationTime,
            spDirRq->iScaling,
            spDirRq->fScalingExp,
            spDirRq->iRemoveNoise,
            spDirRq->fAlpha,
            spDirRq->lMeanLength,
            spDirRq->lGapLength);
    #endif
}


void clDirMsg2::GetRequest (const char *cpMsgBuf, stpDirReq2 spDirRq)
{
    sscanf(cpMsgBuf,
        "%li %f %f %f %i %f %i %g %li %li",
        &spDirRq->lWindowSize,
        &spDirRq->fSoundSpeed,
        &spDirRq->fLowFreqLimit,
        &spDirRq->fIntegrationTime,
        &spDirRq->iScaling,
        &spDirRq->fScalingExp,
        &spDirRq->iRemoveNoise,
        &spDirRq->fAlpha,
        &spDirRq->lMeanLength,
        &spDirRq->lGapLength);
}


void clDirMsg2::SetResult (void *vpMsgBuf, const stpDirRes2 spDirHdr,
    const float *fpLevRes, const float *fpDirRes)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    float *fpLevResPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];
    float *fpDirResPtr = (float *) &fpLevResPtr[spDirHdr->lResultCount];

    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr, 
            "%li:%li %li %li %g %li %.3f %.5f",
            (long) spDirHdr->sTimeStamp.tv_sec,
            (long) spDirHdr->sTimeStamp.tv_usec,
            spDirHdr->lMinBin,
            spDirHdr->lMaxBin,
            spDirHdr->fFreqResolution,
            spDirHdr->lResultCount,
            spDirHdr->fPeakLevel,
            spDirHdr->fIntegrationTime);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN, 
            "%li:%li %li %li %g %li %.3f %.5f",
            (long) spDirHdr->sTimeStamp.tv_sec,
            (long) spDirHdr->sTimeStamp.tv_usec,
            spDirHdr->lMinBin,
            spDirHdr->lMaxBin,
            spDirHdr->fFreqResolution,
            spDirHdr->lResultCount,
            spDirHdr->fPeakLevel,
            spDirHdr->fIntegrationTime);
    #endif
    for (lDataCntr = 0; lDataCntr < spDirHdr->lResultCount; lDataCntr++)
    {
        EndianConv(&fpLevResPtr[lDataCntr], &fpLevRes[lDataCntr]);
        EndianConv(&fpDirResPtr[lDataCntr], &fpDirRes[lDataCntr]);
    }
}


void clDirMsg2::SetResult (void *vpMsgBuf, const stpDirRes2 spDirHdr,
    const double *dpLevRes, const double *dpDirRes)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    double *dpLevResPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];
    double *dpDirResPtr = (double *) &dpLevResPtr[spDirHdr->lResultCount];

    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr,
            "%li:%li %li %li %g %li %.3f %.5f",
            (long) spDirHdr->sTimeStamp.tv_sec,
            (long) spDirHdr->sTimeStamp.tv_usec,
            spDirHdr->lMinBin,
            spDirHdr->lMaxBin,
            spDirHdr->fFreqResolution,
            spDirHdr->lResultCount,
            spDirHdr->fPeakLevel,
            spDirHdr->fIntegrationTime);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN,
            "%li:%li %li %li %g %li %.3f %.5f",
            (long) spDirHdr->sTimeStamp.tv_sec,
            (long) spDirHdr->sTimeStamp.tv_usec,
            spDirHdr->lMinBin,
            spDirHdr->lMaxBin,
            spDirHdr->fFreqResolution,
            spDirHdr->lResultCount,
            spDirHdr->fPeakLevel,
            spDirHdr->fIntegrationTime);
    #endif
    for (lDataCntr = 0; lDataCntr < spDirHdr->lResultCount; lDataCntr++)
    {
        EndianConv(&dpLevResPtr[lDataCntr], &dpLevRes[lDataCntr]);
        EndianConv(&dpDirResPtr[lDataCntr], &dpDirRes[lDataCntr]);
    }
}


void clDirMsg2::GetResult (const void *vpMsgBuf, stpDirRes2 spDirHdr,
    float *fpLevRes, float *fpDirRes)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;

    sscanf(cpHdrPtr,
        #ifndef __QNX__
        "%li:%li %li %li %g %li %f %f",
        #else
        "%i:%i %li %li %g %li %f %f",
        #endif
        &spDirHdr->sTimeStamp.tv_sec,
        &spDirHdr->sTimeStamp.tv_usec,
        &spDirHdr->lMinBin,
        &spDirHdr->lMaxBin,
        &spDirHdr->fFreqResolution,
        &spDirHdr->lResultCount,
        &spDirHdr->fPeakLevel,
        &spDirHdr->fIntegrationTime);
    
    float *fpLevResPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];
    float *fpDirResPtr = (float *) &fpLevResPtr[spDirHdr->lResultCount];
    for (lDataCntr = 0; lDataCntr < spDirHdr->lResultCount; lDataCntr++)
    {
        EndianConv(&fpLevRes[lDataCntr], &fpLevResPtr[lDataCntr]);
        EndianConv(&fpDirRes[lDataCntr], &fpDirResPtr[lDataCntr]);
    }
}


void clDirMsg2::GetResult (const void *vpMsgBuf, stpDirRes2 spDirHdr,
    double *dpLevRes, double *dpDirRes)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;

    sscanf(cpHdrPtr,
        #ifndef __QNX__
        "%li:%li %li %li %g %li %f %f",
        #else
        "%i:%i %li %li %g %li %f %f",
        #endif
        &spDirHdr->sTimeStamp.tv_sec,
        &spDirHdr->sTimeStamp.tv_usec,
        &spDirHdr->lMinBin,
        &spDirHdr->lMaxBin,
        &spDirHdr->fFreqResolution,
        &spDirHdr->lResultCount,
        &spDirHdr->fPeakLevel,
        &spDirHdr->fIntegrationTime);

    double *dpLevResPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];
    double *dpDirResPtr = (double *) &dpLevResPtr[spDirHdr->lResultCount];
    for (lDataCntr = 0; lDataCntr < spDirHdr->lResultCount; lDataCntr++)
    {
        EndianConv(&dpLevRes[lDataCntr], &dpLevResPtr[lDataCntr]);
        EndianConv(&dpDirRes[lDataCntr], &dpDirResPtr[lDataCntr]);
    }
}


void clLofarMsg::SetRequest (char *cpMsgBuf, const stpLofarReq spLofarRq)
{
    int iLinear;
    int iDemon;

    iLinear = (spLofarRq->bLinear) ? MSG_TRUE : MSG_FALSE;
    iDemon = (spLofarRq->bDemon) ? MSG_TRUE : MSG_FALSE;
    #ifndef USE_G_SNPRINTF
        sprintf(cpMsgBuf, 
            "%i %i %i %g %li %.3f %.3f %i %i %i %g %li %li %i %i %li",
            spLofarRq->iChannel,
            spLofarRq->iType,
            spLofarRq->iWindow,
            spLofarRq->fWinParameter,
            spLofarRq->lWinLength,
            spLofarRq->fLowFreq,
            spLofarRq->fHighFreq,
            spLofarRq->iOverlap,
            iLinear,
            spLofarRq->iRemoveNoise,
            spLofarRq->fAlpha,
            spLofarRq->lMeanLength,
            spLofarRq->lGapLength,
            iDemon,
            spLofarRq->iClip,
            spLofarRq->lAvgCount);
    #else
        g_snprintf(cpMsgBuf, GLOBAL_HEADER_LEN,
            "%i %i %i %g %li %.3f %.3f %i %i %i %g %li %li %i %i %li",
            spLofarRq->iChannel,
            spLofarRq->iType,
            spLofarRq->iWindow,
            spLofarRq->fWinParameter,
            spLofarRq->lWinLength,
            spLofarRq->fLowFreq,
            spLofarRq->fHighFreq,
            spLofarRq->iOverlap,
            iLinear,
            spLofarRq->iRemoveNoise,
            spLofarRq->fAlpha,
            spLofarRq->lMeanLength,
            spLofarRq->lGapLength,
            iDemon,
            spLofarRq->iClip,
            spLofarRq->lAvgCount);
    #endif
}


void clLofarMsg::GetRequest (const char *cpMsgBuf, stpLofarReq spLofarRq)
{
    int iLinear;
    int iDemon;

    sscanf(cpMsgBuf, "%i %i %i %g %li %f %f %i %i %i %g %li %li %i %i %li",
        &spLofarRq->iChannel,
        &spLofarRq->iType,
        &spLofarRq->iWindow,
        &spLofarRq->fWinParameter,
        &spLofarRq->lWinLength,
        &spLofarRq->fLowFreq,
        &spLofarRq->fHighFreq,
        &spLofarRq->iOverlap,
        &iLinear,
        &spLofarRq->iRemoveNoise,
        &spLofarRq->fAlpha,
        &spLofarRq->lMeanLength,
        &spLofarRq->lGapLength,
        &iDemon,
        &spLofarRq->iClip,
        &spLofarRq->lAvgCount);
    spLofarRq->bLinear = (iLinear == MSG_TRUE) ? true : false;
    spLofarRq->bDemon = (iDemon == MSG_TRUE) ? true : false;
}


void clLofarMsg::SetResult (void *vpMsgBuf, const stpLofarRes spResHdr,
    const float *fpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr, "%li:%li %li %.3f %.3f %.3f %g %.3f",
            (long) spResHdr->sTimeStamp.tv_sec,
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->lSpectLength,
            spResHdr->fLowFreq,
            spResHdr->fHighFreq,
            spResHdr->fDemonBand,
            spResHdr->fLineTime,
            spResHdr->fPeakLevel);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN,
            "%li:%li %li %.3f %.3f %.3f %g %.3f",
            (long) spResHdr->sTimeStamp.tv_sec,
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->lSpectLength,
            spResHdr->fLowFreq,
            spResHdr->fHighFreq,
            spResHdr->fDemonBand,
            spResHdr->fLineTime,
            spResHdr->fPeakLevel);
    #endif
    for (lDataCntr = 0; lDataCntr < spResHdr->lSpectLength; lDataCntr++)
    {
        EndianConv(&fpDataPtr[lDataCntr], &fpResData[lDataCntr]);
    }
}


void clLofarMsg::SetResult (void *vpMsgBuf, const stpLofarRes spResHdr,
    const double *dpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr, "%li:%li %li %.3f %.3f %.3f %g %.3f",
            (long) spResHdr->sTimeStamp.tv_sec,
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->lSpectLength,
            spResHdr->fLowFreq,
            spResHdr->fHighFreq,
            spResHdr->fDemonBand,
            spResHdr->fLineTime,
            spResHdr->fPeakLevel);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN,
            "%li:%li %li %.3f %.3f %.3f %g %.3f",
            (long) spResHdr->sTimeStamp.tv_sec,
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->lSpectLength,
            spResHdr->fLowFreq,
            spResHdr->fHighFreq,
            spResHdr->fDemonBand,
            spResHdr->fLineTime,
            spResHdr->fPeakLevel);
    #endif
    for (lDataCntr = 0; lDataCntr < spResHdr->lSpectLength; lDataCntr++)
    {
        EndianConv(&dpDataPtr[lDataCntr], &dpResData[lDataCntr]);
    }
}


void clLofarMsg::GetResult (const void *vpMsgBuf, stpLofarRes spResHdr,
    float *fpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef __QNX__
    sscanf(cpHdrPtr, "%li:%li %li %f %f %f %g %f",
    #else
    sscanf(cpHdrPtr, "%i:%i %li %f %f %f %g %f",
    #endif
        &spResHdr->sTimeStamp.tv_sec,
        &spResHdr->sTimeStamp.tv_usec,
        &spResHdr->lSpectLength,
        &spResHdr->fLowFreq,
        &spResHdr->fHighFreq,
        &spResHdr->fDemonBand,
        &spResHdr->fLineTime,
        &spResHdr->fPeakLevel);
    for (lDataCntr = 0; lDataCntr < spResHdr->lSpectLength; lDataCntr++)
    {
        EndianConv(&fpResData[lDataCntr], &fpDataPtr[lDataCntr]);
    }
}


void clLofarMsg::GetResult (const void *vpMsgBuf, stpLofarRes spResHdr,
    double *dpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef __QNX__
    sscanf(cpHdrPtr, "%li:%li %li %f %f %f %g %f",
    #else
    sscanf(cpHdrPtr, "%i:%i %li %f %f %f %g %f",
    #endif
        &spResHdr->sTimeStamp.tv_sec,
        &spResHdr->sTimeStamp.tv_usec,
        &spResHdr->lSpectLength,
        &spResHdr->fLowFreq,
        &spResHdr->fHighFreq,
        &spResHdr->fDemonBand,
        &spResHdr->fLineTime,
        &spResHdr->fPeakLevel);
    for (lDataCntr = 0; lDataCntr < spResHdr->lSpectLength; lDataCntr++)
    {
        EndianConv(&dpResData[lDataCntr], &dpDataPtr[lDataCntr]);
    }
}


void clBeamAudioMsg::SetRequest (char *cpMsgBuf, 
    const stpBeamAudioReq spReqHdr)
{
    int iHighFreq;

    iHighFreq = (spReqHdr->bHighFreq) ? MSG_TRUE : MSG_FALSE;
    #ifndef USE_G_SNPRINTF
        sprintf(cpMsgBuf, "%f %.2f %i", 
            spReqHdr->fDirection,
            spReqHdr->fSoundSpeed,
            iHighFreq);
    #else
        g_snprintf(cpMsgBuf, GLOBAL_HEADER_LEN, "%f %.2f %i", 
            spReqHdr->fDirection,
            spReqHdr->fSoundSpeed,
            iHighFreq);
    #endif
}


void clBeamAudioMsg::GetRequest (const char *cpMsgBuf, 
    stpBeamAudioReq spReqHdr)
{
    int iHighFreq;

    sscanf(cpMsgBuf, "%f %f %i", 
        &spReqHdr->fDirection,
        &spReqHdr->fSoundSpeed,
        &iHighFreq);
    spReqHdr->bHighFreq = (iHighFreq == MSG_TRUE) ? true : false;
}


void clBeamAudioMsg::SetFirst (char *cpMsgBuf, 
    const stpBeamAudioFirst spFirst)
{
    #ifndef USE_G_SNPRINTF
        sprintf(cpMsgBuf, "%li %i", 
            spFirst->lBufLength,
            spFirst->iSampleRate);
    #else
        g_snprintf(cpMsgBuf, GLOBAL_HEADER_LEN, "%li %i", 
            spFirst->lBufLength,
            spFirst->iSampleRate);
    #endif
}


void clBeamAudioMsg::GetFirst (const char *cpMsgBuf,
    stpBeamAudioFirst spFirst)
{
    sscanf(cpMsgBuf, "%li %i", 
        &spFirst->lBufLength,
        &spFirst->iSampleRate);
}


void clBeamAudioMsg::SetResult (void *vpMsgBuf, 
    const stpBeamAudioRes spResHdr, const float *fpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr, "%li:%li %li %.3f %f", 
            (long) spResHdr->sTimeStamp.tv_sec,
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->lBufLength,
            spResHdr->fPeakLevel,
            spResHdr->fDirection);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN, "%li:%li %li %.3f %f",
            (long) spResHdr->sTimeStamp.tv_sec,
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->lBufLength,
            spResHdr->fPeakLevel,
            spResHdr->fDirection);
    #endif
    for (lDataCntr = 0; lDataCntr < spResHdr->lBufLength; lDataCntr++)
    {
        EndianConv(&fpDataPtr[lDataCntr], &fpResData[lDataCntr]);
    }
}


void clBeamAudioMsg::SetResult (void *vpMsgBuf,
    const stpBeamAudioRes spResHdr, const double *dpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr, "%li:%li %li %.3f %f",
            (long) spResHdr->sTimeStamp.tv_sec,
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->lBufLength,
            spResHdr->fPeakLevel,
            spResHdr->fDirection);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN, "%li:%li %li %.3f %f",
            (long) spResHdr->sTimeStamp.tv_sec,
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->lBufLength,
            spResHdr->fPeakLevel,
            spResHdr->fDirection);
    #endif
    for (lDataCntr = 0; lDataCntr < spResHdr->lBufLength; lDataCntr++)
    {
        EndianConv(&dpDataPtr[lDataCntr], &dpResData[lDataCntr]);
    }
}


void clBeamAudioMsg::GetResult (const void *vpMsgBuf,
    stpBeamAudioRes spResHdr, float *fpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef __QNX__
    sscanf(cpHdrPtr, "%li:%li %li %f %f",
    #else
    sscanf(cpHdrPtr, "%i:%i %li %f %f",
    #endif
        &spResHdr->sTimeStamp.tv_sec,
        &spResHdr->sTimeStamp.tv_usec,
        &spResHdr->lBufLength,
        &spResHdr->fPeakLevel,
        &spResHdr->fDirection);
    for (lDataCntr = 0; lDataCntr < spResHdr->lBufLength; lDataCntr++)
    {
        EndianConv(&fpResData[lDataCntr], &fpDataPtr[lDataCntr]);
    }
}


void clBeamAudioMsg::GetResult (const void *vpMsgBuf,
    stpBeamAudioRes spResHdr, double *dpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef __QNX__
    sscanf(cpHdrPtr, "%li:%li %li %f %f",
    #else
    sscanf(cpHdrPtr, "%i:%i %li %f %f",
    #endif
        &spResHdr->sTimeStamp.tv_sec,
        &spResHdr->sTimeStamp.tv_usec,
        &spResHdr->lBufLength,
        &spResHdr->fPeakLevel,
        &spResHdr->fDirection);
    for (lDataCntr = 0; lDataCntr < spResHdr->lBufLength; lDataCntr++)
    {
        EndianConv(&dpResData[lDataCntr], &dpDataPtr[lDataCntr]);
    }
}


void clLevelMsg::SetRequest (char *cpRequest, const stpLevelReq spRequest)
{
    #ifndef USE_G_SNPRINTF
        sprintf(cpRequest, "%i %i %f %.3f %.3f",
            spRequest->iChannel,
            spRequest->iAlgorithm,
            spRequest->fIntegrationTime,
            spRequest->fLowFrequency,
            spRequest->fHighFrequency);
    #else
        g_snprintf(cpRequest, GLOBAL_HEADER_LEN, "%i %i %f %.3f %.3f",
            spRequest->iChannel,
            spRequest->iAlgorithm,
            spRequest->fIntegrationTime,
            spRequest->fLowFrequency,
            spRequest->fHighFrequency);
    #endif
}


void clLevelMsg::GetRequest (const char *cpRequest, stpLevelReq spRequest)
{
    sscanf(cpRequest, "%i %i %f %f %f",
        &spRequest->iChannel,
        &spRequest->iAlgorithm,
        &spRequest->fIntegrationTime,
        &spRequest->fLowFrequency,
        &spRequest->fHighFrequency);
}


void clLevelMsg::SetResult (void *vpMsgBuf, const stpLevelRes spResHdr)
{
    char *cpHeader = (char *) vpMsgBuf;

    #ifndef USE_G_SNPRINTF
        sprintf(cpHeader, "%li:%li %f %.3f %g",
            (long) spResHdr->sTimeStamp.tv_sec,
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->fIntegrationTime,
            spResHdr->fPeakLevel,
            spResHdr->fResult);
    #else
        g_snprintf(cpHeader, GLOBAL_HEADER_LEN, "%li:%li %f %.3f %g",
            (long) spResHdr->sTimeStamp.tv_sec,
            (long) spResHdr->sTimeStamp.tv_usec,
            spResHdr->fIntegrationTime,
            spResHdr->fPeakLevel,
            spResHdr->fResult);
    #endif
}


void clLevelMsg::GetResult (const void *vpMsgBuf, stpLevelRes spResHdr)
{
    char *cpHeader = (char *) vpMsgBuf;

    #ifndef __QNX__
    sscanf(cpHeader, "%li:%li %f %f %f",
    #else
	sscanf(cpHeader, "%i:%i %f %f %f",
    #endif
        &spResHdr->sTimeStamp.tv_sec,
        &spResHdr->sTimeStamp.tv_usec,
        &spResHdr->fIntegrationTime,
        &spResHdr->fPeakLevel,
        &spResHdr->fResult);
}


void clLocateMsg::SetHeader (char *cpMsgBuf, const stpLocateHdr spHdr)
{
    #ifndef USE_G_SNPRINTF
        sprintf(cpMsgBuf, "%i %i",
            spHdr->iWidth,
            spHdr->iHeight);
    #else
        g_snprintf(cpMsgBuf, GLOBAL_HEADER_LEN, 
            "%i %i",
            spHdr->iWidth,
            spHdr->iHeight);
    #endif
}


void clLocateMsg::GetHeader (const char *cpMsgBuf, stpLocateHdr spHdr)
{
    sscanf(cpMsgBuf, "%i %i",
        &spHdr->iWidth,
        &spHdr->iHeight);
}


void clLocateMsg::SetResult (void *vpMsgBuf, const stpLocateRes spResHdr,
    const float *fpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr,
            "%li",
            spResHdr->lPointCount);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN,
            "%li",
            spResHdr->lPointCount);
    #endif
    for (lDataCntr = 0; lDataCntr < spResHdr->lPointCount; lDataCntr++)
    {
        EndianConv(&fpDataPtr[lDataCntr], &fpResData[lDataCntr]);
    }
}


void clLocateMsg::SetResult (void *vpMsgBuf, const stpLocateRes spResHdr,
    const double *dpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    #ifndef USE_G_SNPRINTF
        sprintf(cpHdrPtr,
            "%li",
            spResHdr->lPointCount);
    #else
        g_snprintf(cpHdrPtr, GLOBAL_HEADER_LEN,
            "%li",
            spResHdr->lPointCount);
    #endif
    for (lDataCntr = 0; lDataCntr < spResHdr->lPointCount; lDataCntr++)
    {
        EndianConv(&dpDataPtr[lDataCntr], &dpResData[lDataCntr]);
    }
}


void clLocateMsg::GetResult (const void *vpMsgBuf, stpLocateRes spResHdr,
    float *fpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    float *fpDataPtr = (float *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    sscanf(cpHdrPtr,
        "%li",
        &spResHdr->lPointCount);
    for (lDataCntr = 0; lDataCntr < spResHdr->lPointCount; lDataCntr++)
    {
        EndianConv(&fpResData[lDataCntr], &fpDataPtr[lDataCntr]);
    }
}


void clLocateMsg::GetResult (const void *vpMsgBuf, stpLocateRes spResHdr,
    double *dpResData)
{
    long lDataCntr;
    char *cpHdrPtr = (char *) vpMsgBuf;
    double *dpDataPtr = (double *) &cpHdrPtr[GLOBAL_HEADER_LEN];

    sscanf(cpHdrPtr,
        "%li",
        &spResHdr->lPointCount);
    for (lDataCntr = 0; lDataCntr < spResHdr->lPointCount; lDataCntr++)
    {
        EndianConv(&dpResData[lDataCntr], &dpDataPtr[lDataCntr]);
    }
}

