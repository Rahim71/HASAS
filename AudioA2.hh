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


#include <sys/types.h>
#include <alsa/asoundlib.h>


#ifndef AUDIOA_HH
    #define AUDIOA_HH

    #define AA_MODE_PLAY            SND_PCM_STREAM_PLAYBACK
    #define AA_MODE_RECORD          SND_PCM_STREAM_CAPTURE


    /**
        ALSA PCM audio I/O for ALSA 0.9 API
    */
    class clAudioA2
    {
            unsigned int uiActRate;
            unsigned int uiActBufSize;
            unsigned int uiActFragSize;
            unsigned int uiActFragCount;
            size_t uiFrameSize;
            snd_output_t *sndoutErr;
            snd_pcm_t *sndpcmPcm;
            snd_pcm_info_t *sndpcmInfo;
            snd_pcm_hw_params_t *sndpcmHwParams;
            snd_pcm_sw_params_t *sndpcmSwParams;
        public:
            clAudioA2 ();
            ~clAudioA2 ();
            /* - Internal stuff - */
            snd_pcm_t *GetPcmHandle ()
                {
                    return sndpcmPcm;
                }
            /* - Card specifics - */
            /**
                Open card.

                \return Success
            */
            bool CardOpen (int);
            /**
                Close card.
            */
            void CardClose ();
            /**
                Get name of card (short)

                \param iCard Card number
                \return Name of card
            */
            const char *CardGetName (int);
            /**
                Get name of card (long)
                
                \param iCard Card number
                \return Name of card
            */
            const char *CardGetLongName (int);
            /**
                Number of channels on card

                \return Number of channels
            */
            int CardGetChannelCount ();
            /* - Device specifics - */
            /**
                Open device

                \param cpDeviceId Device identifier
                \param eMode Playback/capture mode
                \return Success
            */
            bool PcmOpen (const char *, snd_pcm_stream_t eMode);
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
                \param iFragCount Number of framents
                \return Success
            */
            bool PcmSetSetup (int, int, int, int, int);
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
                Get hardware buffer size
                
                \return Buffer size (in bytes)
            */
            int PcmGetBufferSize ();
            /**
                Get fragment size

                \return Fragment size (in bytes)
            */
            int PcmGetFragmentSize ();
            /**
                Get fragment count
                
                \return Fragment count
            */
            int PcmGetFragmentCount ();
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
                Link device to group
                
                \param Device Other device to link to
            */
            bool PcmLink (clAudioA2 &);
            /**
                Unlink device from group
                
                \param Device Other device to unlink from
            */
            bool PcmUnlink ();
            /**
                Prepare device for operation

                \return Success
            */
            bool PcmPrepare ();
            /**
                Start device operation

                \return Success
            */
            bool PcmStart ();
            /**
                Reset device
                
                \return Success
            */
            bool PcmReset ();
            /**
                Drain buffers

                \return Success
            */
            bool PcmDrain ();
            /**
                Read data from device

                \param vpBuffer Buffer
                \param iBufSize Size of buffer
                \return Number of bytes read
            */
            int PcmRead (void *, int);
            /**
                Write data to device

                \param vpBuffer Buffer
                \param iBufSize Size of buffer
                \return Number of bytes written
            */
            int PcmWrite (const void *, int);
    };

#endif

#endif  // USE_ALSA09
