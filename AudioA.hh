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


#include <sys/asoundlib.h>


#ifndef AUDIOA_HH
    #define AUDIOA_HH

    #define AA_MODE_PLAY            SND_PCM_OPEN_PLAYBACK
    #define AA_MODE_RECORD          SND_PCM_OPEN_CAPTURE
    #define AA_MODE_DUPLEX          SND_PCM_OPEN_DUPLEX

    #define AA_FRAG_RECORD_LW       1
    #define AA_FRAG_PLAY_LW         3
    #define AA_FRAG_PLAY_HW         1


    /**
        ALSA PCM audio I/O for ALSA 0.5 API
    */
    class clAudioA
    {
            bool bStream;
            int iCard;
            int iDevice;
            int iSubDevice;
            int iMode;
            snd_ctl_t *spSndCtlH;
            snd_pcm_t *spPcmH;
            snd_ctl_hw_info_t sHWInfo;
            snd_pcm_info_t sPCMInfo;
            snd_pcm_channel_info_t sPCMChInfo;
            snd_pcm_channel_setup_t sPCMSetup;
        public:
            clAudioA ();
            ~clAudioA ();
            /* - Card specifics - */
            /**
                Open card.

                \param iOCard Card number
                \return Success
            */
            bool CardOpen (int);
            /**
                Close card.
            */
            void CardClose ();
            /**
                Get number of cards in system.
            
                \note This can be called without opening card

                \return Number of cards in system
            */
            int CardGetCount ();
            /**
                Get name of card (short)

                \return Name of card
            */
            const char *CardGetName ();
            /**
                Get full name of card

                \return Name of card
            */
            const char *CardGetLongName ();
            /**
                Number of channels on card

                \return Number of channels
            */
            int CardGetChannelCount ();
            /* - Device specifics - */
            /**
                Open device

                \param iODevice Device number
                \param iOSubDevice Subdevice number (or -1)
                \param iOMode Open mode; play, record or fullduplex
                \return Success
            */
            bool PcmOpen (int, int, int);
            /**
                Close device
            */
            void PcmClose ();
            /**
                Setup device

                \param iChannels Number of channels
                \param iSampleRate Samplerate
                \param iBits Wordlength (in bits)
                \param iFragSize Fragment size (in bytes)
                \param bStreamMode Stream or block mode
                \return Success
            */
            bool PcmSetSetup (int, int, int, int, bool);
            /**
                Get channel count

                \return Channel count
            */
            int PcmGetChannels ();
            /**
                Get samplerate

                \return Samplerate
            */
            int PcmGetSampleRate ();
            /**
                Get wordlength

                \return Wordlength (in bits)
            */
            int PcmGetBits ();
            /**
                Get fragment size

                \return Fragment size (in bytes)
            */
            int PcmGetFragmentSize ();
            /**
                Get name of format

                \return Name of format
            */
            const char *PcmGetFormatName ();
            /**
                Get device status

                \return Status
            */
            int PcmGetStatus ();
            /**
                Get device status as string

                \return Status string
            */
            const char *PcmGetStatusStr (int);
            /**
                Get device name

                \return Device name
            */
            const char *PcmGetName ();
            /**
                Get information about channel

                \return Channel information (struct pointer)
            */
            const snd_pcm_channel_info_t *PcmGetChannelInfo ();
            /**
                Get channel status

                \param spChStatus Channel status (snd_pcm_channel_status_t struct)
                \return Success
            */
            bool PcmGetChannelStatus (snd_pcm_channel_status_t *);
            /**
                Get buffer usage

                \return Buffer usage (in bytes)
            */
            int PcmGetBufUsed ();
            /**
                Get buffer free space

                \return Buffer free space (in bytes)
            */
            int PcmGetBufFree ();
            /**
                Prepare device for operation

                \return Success
            */
            bool PcmPrepare ();
            /**
                Start device operation

                \return Success
            */
            bool PcmGo ();
            /**
                Drain buffers

                \return Success
            */
            bool PcmDrain ();
            /**
                Flush buffers

                \return Success
            */
            bool PcmFlush ();
            /**
                Get size of transfer unit

                \return Size of transfer unit
            */
            ssize_t PcmGetTransferSize ();
            /**
                Read data from device

                \param vpBuffer Buffer
                \param iBufSize Size of buffer
                \return Number of bytes read
            */
            ssize_t PcmRead (void *, size_t);
            /**
                Write data to device

                \param vpBuffer Buffer
                \param iBufSize Size of buffer
                \return Number of bytes written
            */
            ssize_t PcmWrite (const void *, size_t);
    };

#endif

#endif  // USE_ALSA05
