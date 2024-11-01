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


#include <sys/time.h>


#ifndef MESSAGES_HH
    #define MESSAGES_HH

    #define MSG_FALSE               0
    #define MSG_TRUE                1


    /**
        Sound: compression
    */
    enum
    {
        MSG_SOUND_COMPRESS_NONE = 0,
        MSG_SOUND_COMPRESS_FLAC = 1
    };

    /**
        Spectrum: type
    */
    enum
    {
        MSG_SPECT_TYPE_FFT = 0,
        MSG_SPECT_TYPE_MRFFT = 1,
        MSG_SPECT_TYPE_GABOR = 2,
        MSG_SPECT_TYPE_WVD = 3,
        MSG_SPECT_TYPE_HANKEL = 4,
        MSG_SPECT_TYPE_AUTOCORR = 5,
        MSG_SPECT_TYPE_CEPSTRUM = 6
    };

    /**
        Spectrum: window
    */
    enum
    {
        MSG_SPECT_WND_RECT = 0,
        MSG_SPECT_WND_BARTLETT = 1,
        MSG_SPECT_WND_BLACKMAN = 2,
        MSG_SPECT_WND_BM_HAR = 3,
        MSG_SPECT_WND_COS_TAPER = 4,
        MSG_SPECT_WND_EXP = 5,
        MSG_SPECT_WND_FLATTOP = 6,
        MSG_SPECT_WND_GEN_COS = 7,
        MSG_SPECT_WND_HAMMING = 8,
        MSG_SPECT_WND_HANNING = 9,
        MSG_SPECT_WND_KAISER = 10,
        MSG_SPECT_WND_KAI_BES = 11,
        MSG_SPECT_WND_TUKEY = 12
    };

    /**
        Spectrum: background noise estimation and removal
    */
    enum
    {
        MSG_SPECT_BNER_NONE = 0,
        MSG_SPECT_BNER_TPSW = 1,
        MSG_SPECT_BNER_OTA = 2,
        MSG_SPECT_BNER_DIFF = 3,
        MSG_SPECT_BNER_IDIFF = 4
    };

    /**
        Direction: algorithm
    */
    enum
    {
        MSG_DIR_ALG_BEAM = 1,
        MSG_DIR_ALG_CORR = 2,
        MSG_DIR_ALG_SPECT = 4
    };

    /**
        Direction: scaling
    */
    enum
    {
        MSG_DIR_SCAL_LIN = 0,
        MSG_DIR_SCAL_LOG = 1,
        MSG_DIR_SCAL_EXP = 2,
        MSG_DIR_SCAL_SIN = 3
    };

    /**
        Direction: background noise estimation and removal
    */
    enum
    {
        MSG_DIR_BNER_NONE = 0,
        MSG_DIR_BNER_TPSW = 1,
        MSG_DIR_BNER_OTA = 2,
        MSG_DIR_BNER_DIFF = 3,
        MSG_DIR_BNER_IDIFF = 4
    };

    /**
        LOFAR: type
    */
    enum
    {
        MSG_LOFAR_TYPE_FFT = 0,
        MSG_LOFAR_TYPE_CEPSTRUM = 1,
        MSG_LOFAR_TYPE_AUTOCORR = 2
    };

    /**
        LOFAR: window
    */
    enum
    {
        MSG_LOFAR_WIN_RECT = 0,
        MSG_LOFAR_WIN_BARTLETT = 1,
        MSG_LOFAR_WIN_BLACKMAN = 2,
        MSG_LOFAR_WIN_BM_HARRIS = 3,
        MSG_LOFAR_WIN_COS_TAPER = 4,
        MSG_LOFAR_WIN_EXP = 5,
        MSG_LOFAR_WIN_FLATTOP = 6,
        MSG_LOFAR_WIN_HAMMING = 7,
        MSG_LOFAR_WIN_HANNING = 8,
        MSG_LOFAR_WIN_KAISER = 9,
        MSG_LOFAR_WIN_KAI_BES = 10,
        MSG_LOFAR_WIN_TUKEY = 11
    };

    /**
        LOFAR: background noise estimation and removal
    */
    enum
    {
        MSG_LOFAR_BNER_NONE = 0,
        MSG_LOFAR_BNER_TPSW = 1,
        MSG_LOFAR_BNER_OTA = 2,
        MSG_LOFAR_BNER_DIFF = 3,
        MSG_LOFAR_BNER_IDIFF = 4,
        MSG_LOFAR_BNER_STDDEV = 5
    };

    /**
        LOFAR: predefined clipping functions
    */
    enum
    {
        MSG_LOFAR_CLIP_NONE = 0,
        MSG_LOFAR_CLIP_LOW = 1,
        MSG_LOFAR_CLIP_BOTH = 2,
        MSG_LOFAR_CLIP_MEAN = 3,
        MSG_LOFAR_CLIP_MEDIAN = 4,
        MSG_LOFAR_CLIP_10DB = 5,
        MSG_LOFAR_CLIP_20DB = 6,
        MSG_LOFAR_CLIP_50P = 7,
        MSG_LOFAR_CLIP_75P = 8,
        MSG_LOFAR_CLIP_OFFSET = 9,
        MSG_LOFAR_CLIP_OFFSET2 = 10,
        MSG_LOFAR_CLIP_OFFSET3 = 11,
        MSG_LOFAR_CLIP_SLIDING = 12
    };

    /**
        Level: algorithm
    */
    enum
    {
        MSG_LEVEL_ALG_PEAK = 0,
        MSG_LEVEL_ALG_RMS = 1,
        MSG_LEVEL_ALG_MEAN = 2,
        MSG_LEVEL_ALG_MEDIAN = 3,
        MSG_LEVEL_ALG_STDDEV = 4
    };


    /**
        Header message from soundsrv
    */
    typedef struct _stSoundStart
    {
        int iChannels;          ///< Number of channels
        double dSampleRate;     ///< Samplerate
        int iFragmentSize;      ///< Fragment size (in samples)
        int iCompress;          ///< Compression
    } stSoundStart, *stpSoundStart;

    /**
        Spectrum: request
    */
    typedef struct _stSpectReq
    {
        int iChannel;           ///< Channel
        int iType;              ///< Type (STFT, Hankel, etc)
        int iWindow;            ///< Window function
        float fWinParam;        ///< Optional window parameter
        long lLength;           ///< Number of points
        int iLowFreq;           ///< Lower frequency limit (Hz)
        int iHighFreq;          ///< Higher frequency limit (Hz)
        float fGain;            ///< Gain (dB)
        float fSlope;           ///< Gain (dB/oct)
        int iOverlap;           ///< Overlap (%)
        bool bLinear;           ///< Linear/logarithmic scaling
        bool bNormalize;        ///< Normalize?
        int iRemoveNoise;       ///< Noise removal algorithm
        float fAlpha;           ///< Alpha for noise removal
        long lMeanLength;       ///< Mean length for noise removal
        long lGapLength;        ///< Gap length for TPSW noise removal
        float fDynRange;        ///< Dynamic range for logarithmic (dB) level
    } stSpectReq, *stpSpectReq;

    /**
        Spectrum: result
    */
    typedef struct _stSpectRes
    {
        struct timeval sTimeStamp;  ///< Timestamp
        int iChannel;           ///< Channel
        long lLength;           ///< Result length
        int iLowFreq;           ///< Lower frequency limit (may differ from request)
        int iHighFreq;          ///< Higher frequency limit (may differ from request)
        int iSampleRate;        ///< Samplerate
        bool bLinear;           ///< Linear/logarithmic
        float fPeakLevel;       ///< Peak level (dB)
        float fLineTime;        ///< Length of spectrum window (s)
    } stSpectRes, *stpSpectRes;

    /**
        Direction: request
    */
    typedef struct _stDirReq
    {
        int iAlgorithm;         ///< Algorithm
        float fSoundSpeed;      ///< Speed of sound (m/s)
        float fLowFreqLimit;    ///< Lower frequency limit (Hz)
        float fIntegrationTime; ///< Itegration time (s)
        int iScaling;           ///< Scaling function
        float fScalingExp;      ///< Exponent for exponential scaling
        bool bNormalize;        ///< Normalization
        float fLeftDir;         ///< Left direction (rad)
        float fRightDir;        ///< Right direction (rad)
        long lSectorCount;      ///< Number of sectors
        int iRemoveNoise;       ///< Background noise estimation and removal algorithm
        float fAlpha;           ///< Alpha for noise removal
        long lMeanLength;       ///< Mean length for noise removal
        long lGapLength;        ///< Gap length for TPSW noise removal algorithm
        bool bDisableFilter;    ///< Disable all filtering of input signal
    } stDirReq, *stpDirReq;

    /**
        Direction: result
    */
    typedef struct _stDirRes
    {
        struct timeval sTimeStamp;  ///< Timestamp
        float fIntegrationTime; ///< Integration time (s)
        float fHighFreqLimit;   ///< Higher frequency limit (Hz)
        float fPeakLevel;       ///< Peak level (dB)
        long lSectorCount;      ///< Number of sectors (number of results)
        bool b3DArray;          ///< 3D array (the non-twosided case)
    } stDirRes, *stpDirRes;

    /**
        Direction3: request

        This is used by locate server.
    */
    typedef struct _stDirReq2
    {
        long lWindowSize;       ///< Window size
        float fSoundSpeed;      ///< Speed of sound (m/s)
        float fLowFreqLimit;    ///< Lower frequecy limit (Hz)
        float fIntegrationTime; ///< Integration time (s)
        int iScaling;           ///< Scaling algorithm
        float fScalingExp;      ///< Exponent for exponential scaling
        int iRemoveNoise;       ///< Background noise estimation and removal algorithm
        float fAlpha;           ///< Alpha for noise removal
        long lMeanLength;       ///< Mean length for noise removal
        long lGapLength;        ///< Gap length for TPSW noise removal algorithm
    } stDirReq2, *stpDirReq2;

    /**
        Direction3: result

        This is used by locate server.
    */
    typedef struct _stDirRes2
    {
        struct timeval sTimeStamp; ///< Timestamp
        long lMinBin;           ///< Lowest used bin index
        long lMaxBin;           ///< Highest used bin index
        float fFreqResolution;  ///< Frequency resolution (Hz/bin)
        long lResultCount;      ///< Number of results
        float fPeakLevel;       ///< Peak level (dB)
        float fIntegrationTime; ///< Integration time (s)
    } stDirRes2, *stpDirRes2;

    /**
        LOFAR: request
    */
    typedef struct _stLofarReq
    {
        int iChannel;           ///< Channel
        int iType;              ///< Type; FFT, Hankel, Cepstrum, etc.
        int iWindow;            ///< Window function
        float fWinParameter;    ///< Parameter for window function
        long lWinLength;        ///< Window length in points
        float fLowFreq;         ///< Lower frequency limit (Hz)
        float fHighFreq;        ///< Higher frequency limit (Hz)
        int iOverlap;           ///< Overlap (%)
        bool bLinear;           ///< Linear/logarithmic scaling
        int iRemoveNoise;       ///< Background noise estimation and removal algorithm
        float fAlpha;           ///< Alpha for noise removal
        long lMeanLength;       ///< Mean length for noise removal
        long lGapLength;        ///< Gap length for TPSW noise removal algorithm
        bool bDemon;            ///< LOFAR/DEMON
        int iClip;              ///< Predefined clipping function
        long lAvgCount;         ///< Number of spectrums to average
    } stLofarReq, *stpLofarReq;

    /**
        LOFAR: result
    */
    typedef struct _stLofarRes
    {
        struct timeval sTimeStamp; ///< Timestamp
        long lSpectLength;      ///< Spectrum length
        float fLowFreq;         ///< Lower frequency limit (Hz)
        float fHighFreq;        ///< Higher frequency limit (Hz)
        float fDemonBand;       ///< DEMON bandwidth (Hz)
        float fLineTime;        ///< Length of spectrum window in time (s)
        float fPeakLevel;       ///< Peak level (dB)
    } stLofarRes, *stpLofarRes;

    /**
        BeamAudio: request
    */
    typedef struct _stBeamAudioReq
    {
        float fDirection;       ///< Direction of beam (rad)
        float fSoundSpeed;      ///< Speed of sound (m/s)
        bool bHighFreq;         ///< High frequency content
    } stBeamAudioReq, *stpBeamAudioReq;

    /**
        BeamAudio: result header
    */
    typedef struct _stBeamAudioFirst
    {
        long lBufLength;        ///< Buffer length
        int iSampleRate;        ///< Samplerate
    } stBeamAudioFirst, *stpBeamAudioFirst;

    /**
        BeamAudio: result
    */
    typedef struct _stBeamAudioRes
    {
        struct timeval sTimeStamp; ///< Timestamp
        long lBufLength;        ///< Buffer length
        float fPeakLevel;       ///< Peak level (dB)
        float fDirection;       ///< Direction (rad)
    } stBeamAudioRes, *stpBeamAudioRes;

    /**
        Level: request
    */
    typedef struct _stLevelReq
    {
        int iChannel;           ///< Channel
        int iAlgorithm;         ///< Algorithm
        float fIntegrationTime; ///< Itegration time (s)
        float fLowFrequency;    ///< Lower frequency limit (Hz)
        float fHighFrequency;   ///< Higer frequency limit (Hz)
    } stLevelReq, *stpLevelReq;

    /**
        Level: result
    */
    typedef struct _stLevelRes
    {
        struct timeval sTimeStamp; ///< Timestamp
        float fIntegrationTime; ///< Integration time (s)
        float fPeakLevel;       ///< Peak level (dB)
        float fResult;          ///< Result
    } stLevelRes, *stpLevelRes;

    /**
        Locate: results header
    */
    typedef struct _stLocateHdr
    {
        int iWidth;             ///< Matrix width
        int iHeight;            ///< Matrix height
    } stLocateHdr, *stpLocateHdr;

    /**
        Locate: result
    */
    typedef struct _stLocateRes
    {
        long lPointCount;       ///< Point count
    } stLocateRes, *stpLocateRes;


    /**
        Base class for all message handling

        For communication between userinterfaces and processing servers
        the usual rule is to first send request message. This is
        null-terminated string of length GLOBAL_HEADER_LEN. For results 
        there is GLOBAL_HEADER_LEN amount of null-terminated string header
        data and then endianess converted result data follows.

        Set/GetRequest generates/parses this string from request struct.
        Set/GetResult generates/parses the result message.
    */
    class clBaseMsg
    {
        protected:
            /**
                Result endian conversion

                \param fpDest Destination
                \param fpSrc Source
            */
            inline void EndianConv (float *, const float *);
            /// \overload
            inline void EndianConv (double *, const double *);
    };


    /**
        Sound server communication
    */
    class clSoundMsg : public clBaseMsg
    {
        public:
            /**
                Start message from server

                \param cpMsgBuf Message buffer
                \param stpSoundStart Message
            */
            void SetStart (char *, const stpSoundStart);
            /// \overload
            void GetStart (const char *, stpSoundStart);
            /** 
                Datastream from server

                \param vpMsgBuf Message buffer
                \param spTimeStamp Time stamp
                \param fpData Data
                \param iDataLen Data length
            */
            void SetData (void *, const struct timeval *, const float *,
                int);
            /// \overload
            void SetData (void *, const struct timeval *, const double *,
                int);
            /// \overload
            void SetData (void *, const float *, int);
            /// \overload
            void SetData (void *, const double *, int);
            /// \overload
            int GetData (const void *, struct timeval *, float *);
            /// \overload
            int GetData (const void *, struct timeval *, double *);
            /// \overload
            void GetData (const void *, float *, int);
            /// \overload
            void GetData (const void *, double *, int);
    };


    /**
        Spectrum server communication
    */
    class clSpectMsg : public clBaseMsg
    {
        public:
            // Request message to server
            void SetRequest (char *, const stpSpectReq);
            void GetRequest (const char *, stpSpectReq);
            // Result message from server
            void SetResult (void *, const stpSpectRes, const float *);
            void SetResult (void *, const stpSpectRes, const double *);
            void GetResult (const void *, stpSpectRes, float *);
            void GetResult (const void *, stpSpectRes, double *);
    };


    /**
        Direction server communication
    */
    class clDirMsg : public clBaseMsg
    {
        public:
            // Request message to server
            void SetRequest (char *, const stpDirReq);
            void GetRequest (const char *, stpDirReq);
            // Result message from server
            void SetResult (void *, const stpDirRes, const float *);
            void SetResult (void *, const stpDirRes, const double *);
            void GetResult (const void *, stpDirRes, float *);
            void GetResult (const void *, stpDirRes, double *);
    };


    /** 
        Direction server communication 2 (for locate/direction3)
    */
    class clDirMsg2 : public clBaseMsg
    {
        public:
            // Request message to server
            void SetRequest (char *, const stpDirReq2);
            void GetRequest (const char *, stpDirReq2);
            // Result message from server
            void SetResult (void *, const stpDirRes2, 
                const float *, const float *);
            void SetResult (void *, const stpDirRes2,
                const double *, const double *);
            void GetResult (const void *, stpDirRes2, float *, float *);
            void GetResult (const void *, stpDirRes2, double *, double *);
    };


    /**
        LOFAR/DEMON server communication
    */
    class clLofarMsg : public clBaseMsg
    {
        public:
            // Request message to server
            void SetRequest (char *, const stpLofarReq);
            void GetRequest (const char *, stpLofarReq);
            // Result message from server
            void SetResult (void *, const stpLofarRes, const float *);
            void SetResult (void *, const stpLofarRes, const double *);
            void GetResult (const void *, stpLofarRes, float *);
            void GetResult (const void *, stpLofarRes, double *);
    };


    /**
        BeamAudio server communication
    */
    class clBeamAudioMsg : public clBaseMsg
    {
        public:
            // Request message to server
            void SetRequest (char *, const stpBeamAudioReq);
            void GetRequest (const char *, stpBeamAudioReq);
            // First message from server
            void SetFirst (char *, const stpBeamAudioFirst);
            void GetFirst (const char *, stpBeamAudioFirst);
            // Result message from server
            void SetResult (void *, const stpBeamAudioRes, const float *);
            void SetResult (void *, const stpBeamAudioRes, const double *);
            void GetResult (const void *, stpBeamAudioRes, float *);
            void GetResult (const void *, stpBeamAudioRes, double *);
    };


    /**
        Level server communication
    */
    class clLevelMsg : public clBaseMsg
    {
        public:
            // Request message to server
            void SetRequest (char *, const stpLevelReq);
            void GetRequest (const char *, stpLevelReq);
            // Result message from server
            void SetResult (void *, const stpLevelRes);
            void GetResult (const void *, stpLevelRes);
    };


    /**
        Locate server communication
    */
    class clLocateMsg : public clBaseMsg
    {
        public:
            // Header message from server
            void SetHeader (char *, const stpLocateHdr);
            void GetHeader (const char *, stpLocateHdr);
            // Result message from server
            void SetResult (void *, const stpLocateRes, const float *);
            void SetResult (void *, const stpLocateRes, const double *);
            void GetResult (const void *, stpLocateRes, float *);
            void GetResult (const void *, stpLocateRes, double *);
    };            

#endif

