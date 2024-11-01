/*

    Transient spectrum GUI
    Copyright (C) 1999-2003 Jussi Laako

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


#include <gtk/gtk.h>

#include <Alloc.hh>
#include <dsp/DSPOp.hh>

#include "Config.h"
#include "CfgFile.hh"
#include "GtkUtils.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"
#include "FrameBuf.hh"


#ifndef GUISPECT_HH
    #define GUISPECT_HH


    #define SGUI_VER_MAJ            GLOBAL_VERSMAJ
    #define SGUI_VER_MIN            GLOBAL_VERSMIN
    #define SGUI_VER_PL             GLOBAL_VERSPL
    #define SGUI_CONV_MAXLEN        255
    #define SGUI_DEF_WIDTH          1024
    #define SGUI_DEF_HEIGHT         700
    #define SGUI_PADDING            8
    #define SGUI_SERVER_MAXLEN      255
    #define SGUI_CH_LOWER           1.0
    #define SGUI_CH_UPPER           16.0
    #define SGUI_GRAM_BG            0x00ffffff
    #define SGUI_GRAM_FG            0x00000000
    #define SGUI_SPECT_BG           0x00ffffff
    #define SGUI_SPECT_FG           0x000000ff

    #define SGUI_TYPE_ITEMS         7
    #define SGUI_WINDOW_ITEMS       13
    #define SGUI_WINLEN_ITEMS       10
    #define SGUI_REMOVE_NOISE_ITEMS 5
    #define SGUI_PALETTE_ITEMS      9


    /**
        Fit types available
    */
    enum
    {
        SGUI_FIT_NONE = 0,
        SGUI_FIT_NEIGHBOR = 1,
        SGUI_FIT_AVERAGE = 2
    };


    /**
        Palettes available
    */
    enum
    {
        SGUI_PAL_BW = 0,
        SGUI_PAL_HSV = 1,
        SGUI_PAL_LIGHT = 2,
        SGUI_PAL_TEMP = 3,
        SGUI_PAL_DIR = 4,
        SGUI_PAL_GREEN = 5,
        SGUI_PAL_GREEN2 = 6,
        SGUI_PAL_PUREGREEN = 7,
        SGUI_PAL_WB = 8
    };


    /**
        Transient spectrum GUI
    */
    class clSpectGUI
    {
            bool bRun;
            bool bConnected;
            bool bFreezed;
            bool bReConfig;
            int iSockH;
            int iRcvMsgSize;
            int iFit;
            int iPalette;
            int iGramW;
            int iGramH;
            int iSpectW;
            int iSpectH;
            int iScaleFactor;
            int iSpectPoints;
            int iClips;
            int iTIFFCompression;
            int iJPEGQuality;
            int iBeamCount;
            float fGramX;
            float fGramY;
            float fSpectX;
            float fSpectY;
            char cpGramXTime[20];
            char *cpRcvMsgBuf;
            GDT *fpSpect;
            GDT *fpIntSpect;
            gint giGdkTag;
            GList *glServers;
            // - Top level
            GtkWidget *gwWindow;
            GtkWidget *gwVBox;
            GtkWidget *gwCBHide;
            GtkWidget *gwVPaned;
            GtkWidget *gwScrolledW1;
            GtkWidget *gwScrolledW2;
            GtkWidget *gwStatusBar;
            guint guCtxtSB;
            // - Table 1
            GtkWidget *gwTable1;
            GtkWidget *gwLServer;
            GtkWidget *gwCServer;
            GtkWidget *gwLChannel;
            GtkObject *goAChannel;
            GtkWidget *gwSBChannel;
            GtkWidget *gwBConnect;
            GtkWidget *gwBDisconnect;
            GtkWidget *gwCBFreeze;
            // - Table 2
            GtkWidget *gwTable2;
            GtkWidget *gwLType;
            GtkWidget *gwOMType;
            GtkWidget *gwMType;
            GtkWidget *gwaMIType[SGUI_TYPE_ITEMS];
            GtkWidget *gwLWindow;
            GtkWidget *gwOMWindow;
            GtkWidget *gwMWindow;
            GtkWidget *gwaMIWindow[SGUI_WINDOW_ITEMS];
            GtkWidget *gwLWindowParam;
            GtkWidget *gwEWindowParam;
            GtkWidget *gwLWindowLen;
            GtkWidget *gwOMWindowLen;
            GtkWidget *gwMWindowLen;
            GtkWidget *gwaMIWindowLen[SGUI_WINLEN_ITEMS];
            GtkWidget *gwLLowFreq;
            GtkWidget *gwELowFreq;
            GtkWidget *gwLHighFreq;
            GtkWidget *gwEHighFreq;
            GtkWidget *gwLGain;
            GtkWidget *gwEGain;
            GtkWidget *gwLSlope;
            GtkWidget *gwESlope;
            GtkWidget *gwLOverlap;
            GtkWidget *gwEOverlap;
            GtkWidget *gwCBLinear;
            GtkWidget *gwCBNormalize;
            //GtkWidget *gwBApply;
            // - Table 3
            GtkWidget *gwTable3;
            GtkWidget *gwLRemoveNoise;
            GtkWidget *gwOMRemoveNoise;
            GtkWidget *gwMRemoveNoise;
            GtkWidget *gwaMIRemoveNoise[SGUI_REMOVE_NOISE_ITEMS];
            GtkWidget *gwLAlpha;
            GtkWidget *gwEAlpha;
            GtkWidget *gwLMeanLength;
            GtkWidget *gwEMeanLength;
            GtkWidget *gwLGapLength;
            GtkWidget *gwEGapLength;
            GtkWidget *gwLDynRange;
            GtkWidget *gwEDynRange;
            GtkWidget *gwLPalette;
            GtkWidget *gwOMPalette;
            GtkWidget *gwMPalette;
            GtkWidget *gwaMIPalette[SGUI_PALETTE_ITEMS];
            GtkWidget *gwBSave;
            GtkWidget *gwFSSave;
            // - Table Gram
            GtkWidget *gwTableGram;
            GtkWidget *gwHRTime;
            GtkWidget *gwVRFreq;
            GtkWidget *gwDASpectrogram;
            // - Table Spect
            GtkWidget *gwTableSpect;
            GtkWidget *gwHRFreq;
            GtkWidget *gwVRLevel;
            GtkWidget *gwDASpectrum;
            //-
            // Drawing primitives and cursors
            GdkGC *ggcGramBG;
            GdkGC *ggcGramFG;
            GdkGC *ggcSpectBG;
            GdkGC *ggcSpectFG;
            GdkCursor *gcCrossHair;
            //-
            stSpectReq sSRequest;
            stSpectRes sSResult;
            clAlloc RcvMsgBuf;
            clAlloc SpectBuf;
            clAlloc IntSpectBuf;
            clCfgFile *CfgFile;
            clDSPOp DSP;
            clGtkUtils GtkUtils;
            clSockClie SockClie;
            clSockOp *SockOp;
            clSpectMsg SpectMsg;
            clFrameBuf FrameBuf;
            bool Build ();
            bool BuildTable1 ();
            bool BuildTable2 ();
            bool BuildTable3 ();
            bool BuildTableGram ();
            bool BuildTableSpect ();
            bool BuildDrawingPrims ();
            void FreeDrawingPrims ();
            bool ConnectSignals ();
            int GetGramHeight ();
            bool ParseServerStr (char *, int *, const char *);
            bool InitConnection (const char *, int);
            void GetSettings ();
            bool SendSettings ();
            void ReConfigDisplay ();
            void DrawSpectrogram ();
            void DrawSpectrum ();
            void PrintRealTime ();
            long CountBands (long);
            void SaveInfo (const char *);
        public:
            clSpectGUI (int *, char ***);
            ~clSpectGUI ();
            int Exec ();
            gint OnDelete (GtkWidget *, GdkEventAny *);
            void OnHideToggled (GtkToggleButton *, gpointer);
            gint OnConnectClick (GtkWidget *, gpointer);
            void OnFreezeToggled (GtkToggleButton *, gpointer);
            gint OnPaletteActivate (GtkWidget *, gpointer);
            gint OnMotionSgram (GtkWidget *, GdkEventMotion *);
            gint OnMotionSpect (GtkWidget *, GdkEventMotion *);
            gint OnExposeSgram (GtkWidget *, GdkEventExpose *);
            gint OnExposeSpect (GtkWidget *, GdkEventExpose *);
            gboolean OnConfigure (GtkWidget *, GdkEventConfigure *, gpointer);
            void OnSizeAllocate (GtkWidget *, GtkAllocation *, gpointer);
            void GdkInput (gpointer, gint, GdkInputCondition);
            void OnSaveClicks (GtkButton *, gpointer);
    };

#endif

