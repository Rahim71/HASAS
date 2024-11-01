/*

    User interface for beam audio server
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


#include <pthread.h>
#include <gtk/gtk.h>

#include <Alloc.hh>
#include <Mutex.hh>
#include <dsp/DSPOp.hh>

#include "Config.h"
#ifndef __QNX__
#include "Audio.hh"
#endif
#ifndef BSDSYS
#ifdef USE_ALSA05
#include "AudioA.hh"
#else
#include "AudioA2.hh"
#endif
#endif
#include "Audio3D.hh"
#include "CfgFile.hh"
#include "GtkUtils.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"


#ifndef BEAMAUDIOUI_HH
    #define BEAMAUDIOUI_HH


    #define BAUI_VER_MAJ        GLOBAL_VERSMAJ
    #define BAUI_VER_MIN        GLOBAL_VERSMIN
    #define BAUI_VER_PL         GLOBAL_VERSPL
    #define BAUI_PADDING        8
    #define BAUI_SERVER_MAXLEN  255
    #define BAUI_CONV_BUF_LEN   255
    #define BAUI_AUDIO_BUFCOUNT 10


    static const char *cpWindowTxt = "Beam audio UI";
    static const char *cpLServerTxt = "Server";
    static const char *cpBConnectTxt = "Connect";
    static const char *cpLSoundSpeedTxt = "Sound speed";
    static const char *cpCBHighFreqTxt = "High frequencies";
    static const char *cpCBDitherTxt = "Dither";
    static const char *cpCB3DAudioTxt = "3D";
    static const char *cpLDirectionTxt = "Direction";
    //static const char *cpLDistanceTxt = "Distance";
        

    /**
        User interface for beam audio server
    */
    class clBeamAudioUI
    {
            volatile bool bConnected;
            bool bDither;
            bool b3DAudio;
            bool bALSA;
            int iSampleRate;
            int iRandH;
            int iPrevHeading;
            long lFragSize;
            long lDataCount;
            long lMsgSize;
            long lClips;
            volatile long lBufIdx;
            GDT *fpInAudio;
            GDT *fpSrcAudio;
            BAUI_SND_DATATYPE *ipOutAudio[BAUI_AUDIO_BUFCOUNT];
            pthread_t ptidAudio;
            stBeamAudioReq sRequest;
            /* Glib variables */
            guint guSbCtxt;
            gint giGdkTag;
            GList *glServer;
            /* Gtk+ variables */
            GtkWidget *gwWindow;
            GtkWidget *gwVBox;
            // Table 1
            GtkWidget *gwTable1;
            GtkWidget *gwLServer;
            GtkWidget *gwCServer;
            GtkWidget *gwBConnect;
            // Table 2
            GtkWidget *gwTable2;
            GtkWidget *gwLSoundSpeed;
            GtkWidget *gwESoundSpeed;
            GtkWidget *gwCBHighFreq;
            GtkWidget *gwCBDither;
            GtkWidget *gwCB3DAudio;
            // -
            GtkWidget *gwLDirection;
            GtkObject *goADirection;
            GtkWidget *gwHSDirection;
            //GtkWidget *gwLDistance;
            //GtkObject *goADistance;
            //GtkWidget *gwHSDistance;
            GtkWidget *gwStatusBar;
            /* - */
            clAlloc MessageBuf;
            clAlloc InAudioBuf;
            clAlloc SrcAudioBuf;
            clAlloc DitherBuf;
            clAlloc DitherRandBuf;
            clAlloc OutAudioBuf[BAUI_AUDIO_BUFCOUNT];
            //clAudio Audio;
            clAudio3D Audio3D;
            clCfgFile Cfg;
            clDSPOp DSP;
            clBeamAudioMsg Msg;
            clMutex OutBufMutex[BAUI_AUDIO_BUFCOUNT];
            clGtkUtils GtkUtils;
            clSockClie SClient;
            clSockOp SOp;
            void Build ();
            void BuildTable1 ();
            void BuildTable2 ();
            void ConnectSignals ();
            bool ParseServerStr (char *, int *, const char *);
            bool InitConnection (const char *, int);
            bool SendSettings ();
            void AllocateBuffers (const stpBeamAudioFirst);
            #if (!defined(BSDSYS) && !defined(__QNX__))
            #ifdef USE_ALSA05
            bool InitAudio (clAudio &, clAudioA &);
            #else
            bool InitAudio (clAudio &, clAudioA2 &);
            #endif
            #elif defined(BSDSYS)
            bool InitAudio (clAudio &);
            #elif defined(__QNX__)
            bool InitAudio (clAudioA &);
            #endif
            void UpdateSettings ();
            void ConvertMS ();
            void Process3D ();
            void Dither ();
            void ConvertFromDither();
        public:
            clBeamAudioUI (int *, char ***);
            ~clBeamAudioUI ();
            int Main ();
            gboolean OnDelete (GtkWidget *, GdkEvent *, gpointer);
            void OnConnectClick (GtkButton *, gpointer);
            void OnValueChanged (GtkAdjustment *, gpointer);
            void OnToggled (GtkToggleButton *, gpointer);
            void OnGdkInput (gpointer, gint, GdkInputCondition);
            void *AudioOutThread (void *);
    };

#endif
