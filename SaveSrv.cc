/*

    Save server
    Copyright (C) 2001-2002 Jussi Laako

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
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "SockClie.hh"

#include "SaveSrv.hh"


static bool bDaemon = false;
static const char *cpFileExt[] = { "wav", "aiff", "flac" };
static const char cpFLACMarker[] = "fLaC";
clSaveSrv SaveSrv;

void SigHandler (int iSigNum)
{
    switch (iSigNum)
    {
        case SIGINT:
            SaveSrv.Log.Add('#', "Received SIGINT, terminating...");
            SaveSrv.Stop();
            break;
        case SIGTERM:
            SaveSrv.Log.Add('#', "Received SIGTERM, terminating...");
            SaveSrv.Stop();
            break;
        case SIGSEGV:
            SaveSrv.Log.Add('!', "Oops, received SIGSEGV, terminating...");
            abort();
            break;
    }
}


int main (int argc, char *argv[])
{
    bool bRealTime = false;
    int iArgCntr;
    uid_t uidCurrent;
    struct sched_param sSchedParam;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    signal(SIGINT, SigHandler);
    signal(SIGTERM, SigHandler);
    signal(SIGSEGV, SigHandler);

    for (iArgCntr = 1; iArgCntr < argc; iArgCntr++)
    {
        if (strcmp(argv[iArgCntr], "--rt") == 0)
            bRealTime = true;
    }

    if (bRealTime)
    {
        uidCurrent = getuid();
        setuid(0);
        memset(&sSchedParam, 0x00, sizeof(sSchedParam));
        sSchedParam.sched_priority = sched_get_priority_min(SCHED_FIFO);
        sched_setscheduler(getpid(), SCHED_FIFO, &sSchedParam);
        setuid(uidCurrent);
    }

    return SaveSrv.Main(argc, argv);
}


#ifdef USE_FLAC

FLAC__StreamEncoderWriteStatus WrapFLACWrite (
    const FLAC__StreamEncoder *spFLACEnc, const FLAC__byte cpBuffer[], 
    unsigned uiBytes, unsigned uiSamples, unsigned uiCurrFrame,
    void *vpDataPtr)
{
    clSaveSrv *SaveSrvInst = (clSaveSrv *) vpDataPtr;
    
    return SaveSrvInst->FLACWrite(spFLACEnc, cpBuffer, uiBytes, uiSamples,
        uiCurrFrame);
}


void WrapFLACMetaData (
    const FLAC__StreamEncoder *spFLACEnc, 
    const FLAC__StreamMetadata *spFLACMetaData,
    void *vpDataPtr)
{
    clSaveSrv *SaveSrvInst = (clSaveSrv *) vpDataPtr;

    SaveSrvInst->FLACMetaData(spFLACEnc, spFLACMetaData);
}

#endif


bool clSaveSrv::Initialize ()
{
    char cpSndFileVer[SAVS_SNDFILE_MSGLEN + 1];

    Cfg.SetFileName(SD_CFGFILE);
    if (!Cfg.GetStr("LocalSocket", cpSockName))
    {
        Log.Add('!', "\"LocalSocket\" not found from configuration file");
        return false;
    }
    Cfg.SetFileName(SAVS_CFGFILE);
    if (!Cfg.GetStr("SavePath", cpSavePath))
    {
        Log.Add('!', "\"SavePath\" not found from configuration file");
        return false;
    }
    if (!Cfg.GetInt("Format", &iFormat))
    {
        Log.Add('!', "\"Format\" not found from configuration file");
        return false;
    }
    if (!Cfg.GetInt("Type", &iType))
    {
        Log.Add('!', "\"Type\" not found from configuration file");
        return false;
    }
    if (!Cfg.GetInt("Bits", &iBits))
    {
        Log.Add('!', "\"Bits\" not found from configuration file");
        return false;
    }
    if (!Cfg.GetInt("Dither", &iDither))
        iDither = 0;
    if (iBits != 8 && iBits != 16 && iBits != 24)
    {
        iDither = 0;
        Log.Add(' ', "Unsupported word length for dithering, dither disabled");
    }
    if (!Cfg.GetInt("FrameLength", &lFrameLen))
        lFrameLen = SAVS_DEF_FRAMELEN;
    if (!Cfg.GetInt("FileTime", &iFileTime))
        iFileTime = SAVS_DEF_FILETIME;

    lFrameSize = lFrameLen * sizeof(GDT);
    InFrame.Size(lFrameSize);
    if (iDither == 1)
    {
        OutFrame.Size(lFrameLen * (iBits / 8));
        Log.Add(' ', "Dither enabled");
    }
    else
    {
        OutFrame.Size(lFrameLen * sizeof(double));
        Log.Add(' ', "Dither disabled");
    }
    NoiseFrame.Size(lFrameLen * sizeof(int));
    #ifdef USE_FLAC
    FLACFrame.Size(lFrameLen * sizeof(FLAC__int32));
    #endif

    #ifdef linux
    int iRndH = open("/dev/urandom", O_RDONLY);
    if (iRndH < 0)
    {
        Log.Add('!', "Unable to access /dev/urandom", errno);
        return false;
    }
    if (read(iRndH, &uiRndSeed, sizeof(unsigned int)) == sizeof(unsigned int))
    {
        Log.Add(' ', "Random number generator seeded using /dev/urandom");
    }
    else
    {
        uiRndSeed = time(NULL);
        Log.Add('#', "Unable to read /dev/urandom, seeding with time");
    }
    close(iRndH);
    #else
    uiRndSeed = time(NULL);
    #endif

    int iRandVal;
    int *ipRndPtr = NoiseFrame;
    #if (defined(linux) || defined(BSDSYS))
    srandom(uiRndSeed);
    #else
    srand(uiRndSeed);
    #endif
    for (long lRndCntr = 0; lRndCntr < lFrameLen; lRndCntr++)
    {
        #if (defined(linux) || defined(BSDSYS))
        iRandVal = random();
        #else
        iRandVal = rand();
        #endif
        ipRndPtr[lRndCntr] = (iRandVal & 0x7fffffff);
    }
    #ifdef USE_MERSENNE_TWISTER
    MTR = new MTRand(uiRndSeed);
    Log.Add(' ', "Using Mersenne Twister as PRNG");
    #endif

    iFileTime *= 60;

    sf_command(NULL, SFC_GET_LIB_VERSION, cpSndFileVer, SAVS_SNDFILE_MSGLEN);
    Log.Add(' ', cpSndFileVer);
    return true;
}


bool clSaveSrv::ConnectStream ()
{
    int iSockH;
    char cpLogEntry[SAVS_LOGENTRY_SIZE + 1];
    stRawDataReq sDataReq;
    clSockClie SClient;

    iSockH = SClient.Connect(cpSockName);
    if (iSockH < 0) 
    {
        Log.Add('!', "clSockClie::Connect() failed", SClient.GetErrno());
        return false;
    }
    SOp.SetHandle(iSockH);
    if (SOp.ReadN(&sDataHdr, sizeof(sDataHdr)) != sizeof(sDataHdr))
    {
        Log.Add('!', "clSockOp::ReadN() failed", SOp.GetErrno());
        return false;
    }
    sDataReq.iChannel = -1;
    if (SOp.WriteN(&sDataReq, sizeof(sDataReq)) != sizeof(sDataReq))
    {
        Log.Add('!', "clSockOp::WriteN() failed", SOp.GetErrno());
        return false;
    }
    sprintf(cpLogEntry, "Stream ch %i fs %g", sDataHdr.iChannels,
        sDataHdr.dSampleRate);
    Log.Add(' ', cpLogEntry);
    return true;
}


bool clSaveSrv::CreateFile ()
{
    char cpFileName[_POSIX_PATH_MAX + 1];
    SF_INFO sSndInfo;

    if (spSndFile != NULL) sf_close(spSndFile);

    CreateFileName(cpFileName);

    memset(&sSndInfo, 0x00, sizeof(sSndInfo));
    sSndInfo.samplerate = (int) (sDataHdr.dSampleRate + 0.5);
    sSndInfo.channels = sDataHdr.iChannels;
    switch (iFormat)
    {
        case SAVS_FORMAT_WAV:
            sSndInfo.format = SF_FORMAT_WAV;
            break;
        case SAVS_FORMAT_AIFF:
            sSndInfo.format = SF_FORMAT_AIFF;
            break;
        default:
            iFormat = SAVS_FORMAT_WAV;
            sSndInfo.format = (SF_FORMAT_WAV|SF_FORMAT_PCM_16);
    }
    switch (iType)
    {
        case SAVS_TYPE_PCM:
            switch (iBits)
            {
                case 8:
                    sSndInfo.format |= SF_FORMAT_PCM_U8;
                    break;
                case 16:
                    sSndInfo.format |= SF_FORMAT_PCM_16;
                    break;
                case 24:
                    sSndInfo.format |= SF_FORMAT_PCM_24;
                    break;
                case 32:
                    sSndInfo.format |= SF_FORMAT_PCM_32;
                    break;
            }
            break;
        case SAVS_TYPE_FLOAT:
            sSndInfo.format |= SF_FORMAT_FLOAT;
            break;
        case SAVS_TYPE_ADPCM:
            sSndInfo.format |= SF_FORMAT_IMA_ADPCM;
            break;
        case SAVS_TYPE_MSADPCM:
            sSndInfo.format |= SF_FORMAT_MS_ADPCM;
            break;
        default:
            sSndInfo.format |= SF_FORMAT_FLOAT;
    }
    sSndInfo.format |= SF_ENDIAN_FILE;
    if (!sf_format_check(&sSndInfo))
    {
        Log.Add('!', "Invalid file format");
        return false;
    }
    spSndFile = sf_open(cpFileName, SFM_WRITE, &sSndInfo);
    if (spSndFile == NULL)
    {
        Log.Add('!', "File open failed");
        return false;
    }
    if (!sf_command(spSndFile, SFC_SET_NORM_DOUBLE, NULL, SF_TRUE))
    {
        // ignore results for now, bug in libsndfile, wrong return value
        /*Log.Add('!', "Setting normalization behaviour failed");
        return false;*/
    }
    if (iDither > 1)
    {
        if (!sf_command(spSndFile, SFC_SET_ADD_DITHER_ON_WRITE, NULL, SF_TRUE))
        {
            Log.Add('#', "Warning, failed to enable dithering");
        }
    }
    Log.Add('+', cpFileName);
    return true;
}


bool clSaveSrv::CreateFile2 ()
{
    #ifdef USE_FLAC
    int iMaxLPCOrder;
    int iMinRiceOrder;
    int iMaxRiceOrder;
    char cpFileName[_POSIX_PATH_MAX + 1];
    FLAC__StreamEncoderState iEncState;
    
    if (spFLACEnc != NULL && iFileH >= 0) 
    {
        FLAC__stream_encoder_finish(spFLACEnc);
        close(iFileH);
    }

    if (!Cfg.GetInt("FLACMaxLPCOrder", &iMaxLPCOrder))
        iMaxLPCOrder = 0;
    if (iMaxLPCOrder > (int) FLAC__MAX_LPC_ORDER)
        iMaxLPCOrder = FLAC__MAX_LPC_ORDER;
    if (!Cfg.GetInt("FLACMinRiceOrder", &iMinRiceOrder))
        iMinRiceOrder = 0;
    if (!Cfg.GetInt("FLACMaxRiceOrder", &iMaxRiceOrder))
        iMaxRiceOrder = 0;
    if (iMaxRiceOrder > (int) FLAC__MAX_RICE_PARTITION_ORDER)
        iMaxRiceOrder = FLAC__MAX_RICE_PARTITION_ORDER;

    CreateFileName(cpFileName);
    
    iFileH = open(cpFileName, O_WRONLY|O_CREAT, 
        S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (iFileH < 0)
    {
        Log.Add('!', "File open failed");
        return false;
    }

    if (spFLACEnc == NULL)
    {
        spFLACEnc = FLAC__stream_encoder_new();
        if (spFLACEnc == NULL)
        {
            Log.Add('!', "FLAC constructor failed");
            return false;
        }
    }
    FLAC__stream_encoder_set_streamable_subset(spFLACEnc,
        1);
    FLAC__stream_encoder_set_do_mid_side_stereo(spFLACEnc,
        0);
    FLAC__stream_encoder_set_loose_mid_side_stereo(spFLACEnc,
        0);
    FLAC__stream_encoder_set_channels(spFLACEnc,
        sDataHdr.iChannels);
    FLAC__stream_encoder_set_bits_per_sample(spFLACEnc,
        iBits);
    FLAC__stream_encoder_set_sample_rate(spFLACEnc,
        (int) (sDataHdr.dSampleRate + 0.5));
    FLAC__stream_encoder_set_blocksize(spFLACEnc,
        lFrameLen);
    FLAC__stream_encoder_set_max_lpc_order(spFLACEnc,
        iMaxLPCOrder);
    FLAC__stream_encoder_set_qlp_coeff_precision(spFLACEnc,
        0);
    FLAC__stream_encoder_set_do_qlp_coeff_prec_search(spFLACEnc,
        0);
    FLAC__stream_encoder_set_do_escape_coding(spFLACEnc,
        1);  // non-default
    FLAC__stream_encoder_set_do_exhaustive_model_search(spFLACEnc,
        1);  // non-default
    FLAC__stream_encoder_set_min_residual_partition_order(spFLACEnc,
        iMinRiceOrder);
    FLAC__stream_encoder_set_max_residual_partition_order(spFLACEnc,
        iMaxRiceOrder);
    FLAC__stream_encoder_set_rice_parameter_search_dist(spFLACEnc,
        0);
    /*FLAC__stream_encoder_set_total_samples_estimate(spFLACEnc,
        sDataHdr.dSampleRate * iFileTime * 60);*/
    FLAC__stream_encoder_set_total_samples_estimate(spFLACEnc,
        0);
    FLAC__stream_encoder_set_metadata(spFLACEnc, 
        NULL, 0);
    FLAC__stream_encoder_set_write_callback(spFLACEnc,
        WrapFLACWrite);
    FLAC__stream_encoder_set_metadata_callback(spFLACEnc,
        WrapFLACMetaData);
    FLAC__stream_encoder_set_client_data(spFLACEnc,
        (void *) this);

    iEncState = FLAC__stream_encoder_init(spFLACEnc);
    if (iEncState != FLAC__STREAM_ENCODER_OK)
    {
        Log.Add('!', FLAC__StreamEncoderStateString[iEncState]);
        return false;
    }
    
    Log.Add('+', cpFileName);
    return true;
    #else
    return false;
    #endif
}


void clSaveSrv::CreateFileName (char *cpFileName)
{
    int iFileNumber;
    char cpTime[SAVS_TIMELEN + 1];
    time_t ttTime;

    ttTime = time(NULL);
    ttFileStarted = ttTime - ttTime % iFileTime;
    strftime(cpTime, SAVS_TIMELEN + 1, "%d%m%Y%H%M", localtime(&ttFileStarted));
    iFileNumber = 0;
    do
    {
        sprintf(cpFileName, "%s%s-%i.%s", 
            cpSavePath, cpTime, iFileNumber, cpFileExt[iFormat]);
        iFileNumber++;
    } while(access(cpFileName, F_OK) == 0);
}


void clSaveSrv::ProcessLoop ()
{
    time_t ttTime;

    Log.Add(' ', "Entering process loop");
    if (iFormat == SAVS_FORMAT_WAV ||
        iFormat == SAVS_FORMAT_AIFF)
    {
        if (!CreateFile()) Stop();
    }
    else if (iFormat == SAVS_FORMAT_FLAC)
    {
        if (!CreateFile2()) Stop();
    }
    while (bRun)
    {
        if (access(SAVS_SHUTDOWNFILE, F_OK) == 0)
        {
            unlink(SAVS_SHUTDOWNFILE);
            Stop();
            break;
        }
        if (SOp.ReadN(InFrame, lFrameSize) != lFrameSize)
        {
            Log.Add('!', "clSockOp::ReadN() error", SOp.GetErrno());
            break;
        }
        if (iFormat == SAVS_FORMAT_WAV || 
            iFormat == SAVS_FORMAT_AIFF)
        {
            if (!WriteData()) break;
        }
        else if (iFormat == SAVS_FORMAT_FLAC)
        {
            if (!WriteData2()) break;
        }
        ttTime = time(NULL);
        if ((ttTime - ttTime % iFileTime) > ttFileStarted)
        {
            if (iFormat == SAVS_FORMAT_WAV ||
                iFormat == SAVS_FORMAT_AIFF)
            {
                if (!CreateFile()) break;
            }
            else if (iFormat == SAVS_FORMAT_FLAC)
            {
                if (!CreateFile2()) break;
            }
        }
    }
    Log.Add(' ', "Leaving process loop");
}


void clSaveSrv::Dither ()
{
    int iSignMask;
    int iNoiseMask;
    long lSampleCntr;
    long lTempSample;
    GDT fDithScale;
    #ifndef USE_MERSENNE_TWISTER
    int *ipNoiseFrame = NoiseFrame;
    #endif
    GDT *fpInFrame = InFrame;

    if (iBits == 8)
    {
        fDithScale = 0x7f000000;
        iSignMask  = 0x00800000;
        iNoiseMask = 0x007fffff;
        unsigned char *ipOutFrame8 = OutFrame;
        for (lSampleCntr = 0; lSampleCntr < lFrameLen; lSampleCntr++)
        {
            #ifndef USE_MERSENNE_TWISTER
            int iRndVal = ipNoiseFrame[lSampleCntr];
            #else
            int iRndVal = (int) (MTR->randInt() & 0x7fffffff);
            #endif
            int iDithVal = (iRndVal & iSignMask) ?
                -(iRndVal & iNoiseMask) : (iRndVal & iNoiseMask);
            lTempSample = 
                (long) (fpInFrame[lSampleCntr] * fDithScale) + iDithVal;
            ipOutFrame8[lSampleCntr] = (unsigned char)
                ((lTempSample >> 24) + 0x7f);
        }
    }
    else if (iBits == 16)
    {
        fDithScale = 0x7fff0000;
        iSignMask  = 0x00008000;
        iNoiseMask = 0x00007fff;
        signed short *ipOutFrame16 = OutFrame;
        for (lSampleCntr = 0; lSampleCntr < lFrameLen; lSampleCntr++)
        {
            #ifndef USE_MERSENNE_TWISTER
            int iRndVal = ipNoiseFrame[lSampleCntr];
            #else
            int iRndVal = (int) (MTR->randInt() & 0x7fffffff);
            #endif
            int iDithVal = (iRndVal & iSignMask) ?
                -(iRndVal & iNoiseMask) : (iRndVal & iNoiseMask);
            lTempSample = 
                (long) (fpInFrame[lSampleCntr] * fDithScale) + iDithVal;
            ipOutFrame16[lSampleCntr] = (signed short) 
                (lTempSample >> 16);
        }
    }
    else if (iBits == 24)
    {
        fDithScale = 0x7fffff00;
        iSignMask  = 0x00000080;
        iNoiseMask = 0x0000007f;
        signed int *ipOutFrame24 = OutFrame;
        for (lSampleCntr = 0; lSampleCntr < lFrameLen; lSampleCntr++)
        {
            #ifndef USE_MERSENNE_TWISTER
            int iRndVal = ipNoiseFrame[lSampleCntr];
            #else
            int iRndVal = (int) (MTR->randInt() & 0x7fffffff);
            #endif
            int iDithVal = (iRndVal & iSignMask) ?
                -(iRndVal & iNoiseMask) : (iRndVal & iNoiseMask);
            lTempSample = 
                (long) (fpInFrame[lSampleCntr] * fDithScale) + iDithVal;
            ipOutFrame24[lSampleCntr] = (signed int)
                (lTempSample >> 8);
        }
    }
}


bool clSaveSrv::WriteData ()
{
    char cpSndFileError[SAVS_SNDFILE_MSGLEN + 1];

    if (iDither == 1)
    {
        Dither();
        if (iBits == 8)
        {
            if (sf_write_raw(spSndFile, OutFrame, (sf_count_t) lFrameLen) != 
                (sf_count_t) lFrameLen)
            {
                sf_error_str(spSndFile, cpSndFileError, SAVS_SNDFILE_MSGLEN);
                Log.Add('!', cpSndFileError);
                return false;
            }
        }
        else if (iBits == 16)
        {
            if (sf_write_short(spSndFile, OutFrame, (sf_count_t) lFrameLen) != 
                (sf_count_t) lFrameLen)
            {
                sf_error_str(spSndFile, cpSndFileError, SAVS_SNDFILE_MSGLEN);
                Log.Add('!', cpSndFileError);
                return false;
            }
        }
        else if (iBits == 24)
        {
            if (sf_write_int(spSndFile, OutFrame, (sf_count_t) lFrameLen) != 
                (sf_count_t) lFrameLen)
            {
                sf_error_str(spSndFile, cpSndFileError, SAVS_SNDFILE_MSGLEN);
                Log.Add('!', cpSndFileError);
                return false;
            }
        }
    }
    else
    {
        GDT *fpInFrame = InFrame;
        double *dpOutFrame = OutFrame;
        for (long lSampleCntr = 0; lSampleCntr < lFrameLen; lSampleCntr++)
        {
            dpOutFrame[lSampleCntr] = fpInFrame[lSampleCntr];
        }
        if (sf_write_double(spSndFile, dpOutFrame, (sf_count_t) lFrameLen) != 
            (sf_count_t) lFrameLen)
        {
            sf_error_str(spSndFile, cpSndFileError, SAVS_SNDFILE_MSGLEN);
            Log.Add('!', cpSndFileError);
            return false;
        }
    }
    return true;
}


bool clSaveSrv::WriteData2 ()
{
    #ifdef USE_FLAC
    long lSampleCntr;
    FLAC__int32 *ipDest = FLACFrame;

    if (iDither == 1)
    {
        Dither();
    }
    else
    {
        float *fpInFrame = InFrame;
        if (iBits == 8)
            DSP.Convert((unsigned char *) OutFrame, fpInFrame, lFrameLen);
        if (iBits == 16)
            DSP.Convert((signed short *) OutFrame, fpInFrame, lFrameLen, false);
        if (iBits == 24)
            DSP.Convert((signed int *) OutFrame, fpInFrame, lFrameLen, true);
    }
    if (iBits == 8)
    {
        unsigned char *ipSrc = OutFrame;

        for (lSampleCntr = 0; lSampleCntr < lFrameLen; lSampleCntr++)
            ipDest[lSampleCntr] = (FLAC__int32) ipSrc[lSampleCntr] - 0x80;
    }
    else if (iBits == 16)
    {
        signed short *ipSrc = OutFrame;

        for (lSampleCntr = 0; lSampleCntr < lFrameLen; lSampleCntr++)
            ipDest[lSampleCntr] = (FLAC__int32) ipSrc[lSampleCntr];
    }
    else if (iBits == 24)
    {
        signed int *ipSrc = OutFrame;

        for (lSampleCntr = 0; lSampleCntr < lFrameLen; lSampleCntr++)
            ipDest[lSampleCntr] = 
                ((FLAC__int32) ipSrc[lSampleCntr] >> 8);
    }
    if (!FLAC__stream_encoder_process_interleaved(spFLACEnc,
        ipDest, lFrameLen / sDataHdr.iChannels))
        return false;
    return true;
    #else
    return false;
    #endif
}



clSaveSrv::clSaveSrv ()
{
    bRun = true;
    iFileH = -1;
    spSndFile = NULL;
    #ifdef USE_FLAC
    spFLACEnc = NULL;
    #endif
    #ifdef USE_MERSENNE_TWISTER
    MTR = NULL;
    #endif
    Log.Open(SAVS_LOGFILE);
    Log.Add('*', "Starting");
}


clSaveSrv::~clSaveSrv ()
{
    #ifdef USE_MERSENNE_TWISTER
    if (MTR != NULL) delete MTR;
    #endif
    #ifdef USE_FLAC
    if (spFLACEnc != NULL) 
    {
        if (iFileH >= 0)
        {
            FLAC__stream_encoder_finish(spFLACEnc);
            close(iFileH);
        }
        FLAC__stream_encoder_delete(spFLACEnc);
    }
    #endif
    if (spSndFile != NULL) sf_close(spSndFile);
    Log.Add('*', "Ending");
}


int clSaveSrv::Main (int iArgC, char **cpArgV)
{
    if (iArgC >= 2)
    {
        if (strcmp(cpArgV[1], "-D"))
        {
            bDaemon = true;
        }
        else if (strcmp(cpArgV[1], "--version"))
        {
            printf("%s v%i.%i.%i\n", cpArgV[0], SAVS_VERSMAJ,
                SAVS_VERSMIN, SAVS_VERSPL);
            printf("Copyright (C) 2001-2002 Jussi Laako\n");
            return 0;
        }
        else if (strcmp(cpArgV[1], "--help"))
        {
            printf("%s [-D|--version|--help]\n\n", cpArgV[0]);
            printf("-D         start as daemon\n");
            printf("--version  display version information\n");
            printf("--help     display this help\n");
            return 0;
        }
    }
    if (bDaemon)
    {
        if (fork() != 0)
        {
            exit(0);
        }
        setsid();
        freopen("/dev/null", "r+", stderr);
        freopen("/dev/null", "r+", stdin);
        freopen("/dev/null", "r+", stdout);
    }
    if (!Initialize()) return 1;
    if (!ConnectStream()) return 2;
    ProcessLoop();
    return 0;
}


#ifdef USE_FLAC

FLAC__StreamEncoderWriteStatus clSaveSrv::FLACWrite (
    const FLAC__StreamEncoder *spFLACEnc, const FLAC__byte *cpBuffer, 
    unsigned uiBytes, unsigned uiSamples, unsigned uiCurrFrame)
{
    if (write(iFileH, cpBuffer, uiBytes) != (int) uiBytes)
        return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}


void clSaveSrv::FLACMetaData (
    const FLAC__StreamEncoder *spFLACEnc, 
    const FLAC__StreamMetadata *spFLACMetaData)
{
    //lseek(iFileH, 4, SEEK_SET);
    //write(iFileH, spFLACMetaData, sizeof(FLAC__StreamMetaData));
}

#endif
