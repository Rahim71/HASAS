/*

    Sound user interface, header
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
#include <limits.h>
#include <gtk/gtk.h>

#include <Condition.hh>
#include <Mutex.hh>
#include <Semaphore.hh>
#include <dsp/DSPOp.hh>
#include <dsp/Filter.hh>

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
#include "CfgFile.hh"
#include "GtkUtils.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"


#ifndef SOUNDUI_HH
    #define SOUNDUI_HH


    #define SUI_VER_MAJ         GLOBAL_VERSMAJ
    #define SUI_VER_MIN         GLOBAL_VERSMIN
    #define SUI_VER_PL          GLOBAL_VERSPL
    #define SUI_PADDING         8
    #define SUI_CONV_LEN        256
    #define SUI_SERV_MAXLEN     255
    #define SUI_CH_VALUE        1.0
    #define SUI_CH_LOWER        1.0
    #define SUI_CH_HIGHER       16.0
    #define SUI_IN_VALUE        0.0
    #define SUI_IN_LOWER        -72.0
    #define SUI_IN_HIGHER       0.0
    #define SUI_OUT_VALUE       0.0
    #define SUI_OUT_LOWER       -72.0
    #define SUI_OUT_HIGHER      0.0
    #define SUI_OUT_RANGE       12.0
    #define SUI_OUT_STEP        0.1
    #define SUI_EQ_RANGE        12.0
    #define SUI_EQ_STEP         0.1
    #define SUI_EQ_MAXOCTS      15
    #define SUI_FILTER_WINDOW   true
    

    /**
        Per channel GUI for clSoundUI
    */
    class clSoundChGUI
    {
            clGtkUtils GtkUtils;
        public:
            clSoundChGUI (GtkWidget *, int, int, long);
            ~clSoundChGUI ();
            // glib types
            GList *glServers;
            // gtk+ types
            GtkWidget *gwFrame;
            GtkWidget *gwVBox;
            // Table 1
            GtkWidget *gwTable1;
            GtkWidget *gwLServer;
            GtkWidget *gwCServer;
            GtkWidget *gwLChannel;
            GtkObject *goAChannel;
            GtkWidget *gwSBChannel;
            GtkWidget *gwBConnect;
            // -
            GtkWidget *gwLInputLevel;
            GtkWidget *gwPBInputLevel;
            GtkWidget *gwCBEq;
            // Table Eq
            GtkWidget *gwTableEq;
            GtkWidget *gwLOutputLevel;
            GtkObject *goAOutputLevel;
            GtkWidget *gwVSOutputLevel;
            GtkWidget *gwPBOutputLevel;
            GtkWidget *gwaLEqLevel[SUI_EQ_MAXOCTS];
            GtkObject *goaAEqLevel[SUI_EQ_MAXOCTS];
            GtkWidget *gwaVSEqLevel[SUI_EQ_MAXOCTS];
            // Curve Eq
            GtkWidget *gwCurveEq;
            GtkWidget *gwBApplyCurve;
    };
    

    /**
        Sound user interface
    */
    class clSoundUI
    {
            bool bALSA;
            bool bFirstTimeout;
            int iVuTimeout;
            // These are read-only after startup, thus not protected
            int iALSACard;
            int iALSADevice;
            int iALSASubDevice;
            int iDeviceBase;
            int iDCBlock;
            char cpDevice[_POSIX_PATH_MAX];
            // -
            pthread_t ptidSoundOut;
            pthread_t ptidSoundIn[SUI_MAX_CHANNELS];
            // Thread communication, protected
            volatile bool bRun;
            volatile int iChCount;
            volatile int iSampleRate;
            volatile int iOctaveCount;
            volatile long lDataRefCount;
            volatile long lSampleCount;
            volatile bool bpConnected[SUI_MAX_CHANNELS];
            volatile bool bpEqEnabled[SUI_MAX_CHANNELS];
            volatile int ipSockH[SUI_MAX_CHANNELS];
            GDT fpLevelCoeff[SUI_MAX_CHANNELS];
            clDSPAlloc ChData[SUI_MAX_CHANNELS];
            clDSPAlloc EqCoeffs[SUI_MAX_CHANNELS];  // Non-protected
            // glib types
            guint guiStatusbarCtxt;
            gfloat fpInputLevel[SUI_MAX_CHANNELS];  // Protected
            gfloat fpOutputLevel[SUI_MAX_CHANNELS];  // Protected
            // Gtk+ types
            GtkWidget *gwWindow;
            GtkWidget *gwVBox;
            GtkWidget *gwHBox;
            GtkWidget *gwStatusbar;
            // Classes
            #ifndef __QNX__
            clAudio Audio;
            #endif
            #ifndef BSDSYS
            #ifdef USE_ALSA05
            clAudioA AudioA;
            #else
            clAudioA2 AudioA;
            #endif
            #endif
            clCfgFile Cfg;
            clCondition CondData[SUI_MAX_CHANNELS];
            clFilter Filters[SUI_MAX_CHANNELS];  // Protected, MutexFilter
            clGtkUtils GtkUtils;
            clMutex MutexData;
            clMutex MutexLevel;
            clMutex MutexChData[SUI_MAX_CHANNELS];
            clMutex MutexFilter[SUI_MAX_CHANNELS];
            clSemaphore SemStart1;
            clSemaphore SemStart2;
            clSockClie SClient;
            clSoundChGUI *SoundChGUI[SUI_MAX_CHANNELS];
            // Methods
            void GetCfg ();
            void BuildGUI ();
            void ConnectSignals ();
            bool ParseServerStr (char *, int *, const char *);
        public:
            clSoundUI (int *, char ***);
            ~clSoundUI ();
            int Exec ();
            gboolean OnDeleteEvent (GtkWidget *gwSender, GdkEvent *geEvent,
                gpointer gpData);
            void OnClickedEvent (GtkButton *, gpointer);
            gint OnTimeoutEvent (gpointer);
            void OnToggledEvent (GtkToggleButton *, gpointer);
            void OnValueChangedEvent (GtkAdjustment *, gpointer);
            void OnApplyCurveClicked (GtkButton *, gpointer);
            void OnMotionCurve (GtkWidget *, GdkEventMotion *, gpointer);
            void *SoundOutThread (void *vpData);
            void *SoundInThread (void *vpData);
    };


#endif

