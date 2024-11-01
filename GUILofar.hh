/*

    LOFAR/DEMON GUI, header
    Copyright (C) 2000-2003 Jussi Laako

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


#include <ctime>
#include <string>

#include <Alloc.hh>
#include <dsp/DSPOp.hh>

#include "Config.h"
#include "CfgFile.hh"
#include "FrameBuf.hh"
#include "GtkUtils.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"


#ifndef GUILOFAR_HH
    #define GUILOFAR_HH

    #define LGUI_VER_MAJ            GLOBAL_VERSMAJ
    #define LGUI_VER_MIN            GLOBAL_VERSMIN
    #define LGUI_VER_PL             GLOBAL_VERSPL
    #define LGUI_PADDING            8
    #define LGUI_ENTRY_WIDTH        80
    #define LGUI_LINESPECT_HEIGHT   40
    #define LGUI_CURSOR_HEIGHT      16
    #define LGUI_SERVER_MAXLEN      255
    #define LGUI_CH_LOWER           1
    #define LGUI_CH_UPPER           16
    #define LGUI_LOFAR_BG           0x00ffffff
    #define LGUI_LOFAR_FG           0x00000000
    #define LGUI_LINE_BG            0x00ffffff
    #define LGUI_LINE_FG            0x000000ff
    #define LGUI_CURSOR_BG          0x00ffffff
    #define LGUI_CURSOR_FG          0x00ff0000
    #define LGUI_CONV_BUF_SIZE      255

    #define LGUI_TYPE_ITEMS         3
    #define LGUI_WINDOW_ITEMS       12
    #define LGUI_WIN_LENGTH_ITEMS   7
    #define LGUI_REMOVE_NOISE_ITEMS 6
    #define LGUI_CLIP_ITEMS         13
    #define LGUI_PALETTE_ITEMS      11


    /**
        Palettes available
    */
    enum
    {
        LGUI_PAL_BW = 0,
        LGUI_PAL_HSV = 1,
        LGUI_PAL_LIGHT = 2,
        LGUI_PAL_TEMP = 3,
        LGUI_PAL_DIR = 4,
        LGUI_PAL_GREEN = 5,
        LGUI_PAL_GREEN2 = 6,
        LGUI_PAL_GREEN3 = 7,
        LGUI_PAL_GREEN4 = 8,
        LGUI_PAL_PUREGREEN = 9,
        LGUI_PAL_WB = 10
    };

    /**
        Fit algorithms available
    */
    enum
    {
        LGUI_FIT_NONE = 0,
        LGUI_FIT_NEIGHBOR = 1,
        LGUI_FIT_AVERAGE = 2
    };

    /**
        Cursor types
    */
    enum
    {
        LGUI_CURSOR_11 = 0,  ///< 11 spike type
        LGUI_CURSOR_INF = 1  ///< Inifinite spike count
    };


    /**
        Lofar cursor
    */
    typedef struct _stLofarCursor
    {
        int iType;  ///< Cursor type
        int iPosition;  ///< Position
        int iDistance;  ///< Spike spacing
    } stLofarCursor, *stpLofarCursor;


    /**
        LOFAR/DEMON GUI
    */
    class clGUILofar
    {
            bool bRun;
            bool bConnected;
            bool bFreezed;
            bool bConfigured;
            bool bAveraged;
            bool bCursorDrag;
            bool bSaving;
            int iResMsgBufSize;
            int iFit;
            int iPalette;
            int iLofarWidth;
            int iLofarHeight;
            int iCursorX;
            int iCursorY;
            int iClips;
            int iTIFFCompression;
            int iJPEGQuality;
            int iContSaveScans;
            int iCompressMode;
            int iScanCount;
            int iImgCount;
            int iBeamCount;
            long lSpectSize;
            GDT fClip;
            stLofarReq sLofarRq;
            stLofarRes sLofarResHdr;
            stLofarCursor sLCursor;
            /* glib types */
            GList *glServer;
            /* gtk+ types */
            gint giGdkTag;
            guint guSbCtxt;
            // Top level
            GtkWidget *gwWindow;
            GtkWidget *gwVBox;
            GtkWidget *gwCBHide;
            GtkWidget *gwStatusBar;
            // Table 1
            GtkWidget *gwTable1;
            GtkWidget *gwLServer;
            GtkWidget *gwCServer;
            GtkWidget *gwLChannel;
            GtkObject *goAChannel;
            GtkWidget *gwSBChannel;
            GtkWidget *gwBConnect;
            GtkWidget *gwBDisconnect;
            GtkWidget *gwCBFreeze;
            // Table 2
            GtkWidget *gwTable2;
            GtkWidget *gwLType;
            GtkWidget *gwOMType;
            GtkWidget *gwMType;
            GtkWidget *gwaMIType[LGUI_TYPE_ITEMS];
            GtkWidget *gwLWindow;
            GtkWidget *gwOMWindow;
            GtkWidget *gwMWindow;
            GtkWidget *gwaMIWindow[LGUI_WINDOW_ITEMS];
            GtkWidget *gwLWinParam;
            GtkWidget *gwEWinParam;
            GtkWidget *gwLWinLength;
            GtkWidget *gwOMWinLength;
            GtkWidget *gwMWinLength;
            GtkWidget *gwaMIWinLength[LGUI_WIN_LENGTH_ITEMS];
            GtkWidget *gwLLowerFreq;
            GtkWidget *gwELowerFreq;
            GtkWidget *gwLHigherFreq;
            GtkWidget *gwEHigherFreq;
            GtkWidget *gwLOverlap;
            GtkWidget *gwEOverlap;
            // Table 3
            GtkWidget *gwTable3;
            GtkWidget *gwLRemoveNoise;
            GtkWidget *gwOMRemoveNoise;
            GtkWidget *gwMRemoveNoise;
            GtkWidget *gwaMIRemoveNoise[LGUI_REMOVE_NOISE_ITEMS];
            GtkWidget *gwLAlpha;
            GtkWidget *gwEAlpha;
            GtkWidget *gwLMeanLength;
            GtkWidget *gwEMeanLength;
            GtkWidget *gwLGapLength;
            GtkWidget *gwEGapLength;
            GtkWidget *gwLAverageCount;
            GtkWidget *gwEAverageCount;
            GtkWidget *gwLClip;
            GtkWidget *gwOMClip;
            GtkWidget *gwMClip;
            GtkWidget *gwaMIClip[LGUI_CLIP_ITEMS];
            GtkWidget *gwCBLinear;
            GtkWidget *gwCBDemon;
            GtkWidget *gwLPalette;
            GtkWidget *gwOMPalette;
            GtkWidget *gwMPalette;
            GtkWidget *gwaMIPalette[LGUI_PALETTE_ITEMS];
            GtkWidget *gwCBAverage;
            GtkWidget *gwCBSaving;
            GtkWidget *gwBSave;
            GtkWidget *gwFSSave;
            // Table 4
            GtkWidget *gwTable4;
            GtkWidget *gwLClipValue;
            GtkObject *goAClipValue;
            GtkWidget *gwHSClipValue;
            // Table lofar
            GtkWidget *gwTableLofar;
            GtkWidget *gwLTopTime;
            GtkWidget *gwLBottomTime;
            GtkWidget *gwSWLofar;
            GtkWidget *gwTableLofar2;
            GtkWidget *gwHRFrequency;
            GtkWidget *gwVRTime;
            GtkWidget *gwDALine;
            GtkWidget *gwDACursor;
            GtkWidget *gwDALofar;
            // Drawing primitives and cursors
            GdkGC *ggcLofarBG;
            GdkGC *ggcLofarFG;
            GdkGC *ggcLineBG;
            GdkGC *ggcLineFG;
            GdkGC *ggcCursorBG;
            GdkGC *ggcCursorFG;
            GdkCursor *gcCrossHair;
            // Classes
            std::string strImgFileName;
            clAlloc SpectData;
            clAlloc AvgSpectData;
            clCfgFile Cfg;
            clDSPOp DSP;
            clFrameBuf FBLofar;
            clGtkUtils GtkUtils;
            clLofarMsg LofarMsg;
            clSockClie SClient;
            clSockOp SOp;
            /* Methods */
            bool Build ();
            bool BuildTable1 ();
            bool BuildTable2 ();
            bool BuildTable3 ();
            bool BuildTable4 ();
            bool BuildTableLofar ();
            bool BuildDrawingPrims ();
            void FreeDrawingPrims ();
            bool ConnectSignals ();
            bool ParseServerStr (char *, int *, const char *);
            bool InitConnection (const char *, int);
            bool SendSettings ();
            void PrintStatus ();
            void SetPalette (int);
            void Configure ();
            void ConfigureHeight ();
            void DrawCursor ();
            void SaveInfo (const char *, time_t);
            void StartNewImgFile ();
        public:
            clGUILofar (int *, char ***);
            ~clGUILofar ();
            int Exec ();
            gint OnDelete (GtkWidget *, GdkEventAny *);
            void OnHideToggled (GtkToggleButton *, gpointer);
            gint OnConnectClick (GtkWidget *, gpointer);
            void OnFreezeToggled (GtkToggleButton *, gpointer);
            gint OnExposeLofar (GtkWidget *, GdkEventExpose *, gpointer);
            gint OnConfigureLofar (GtkWidget *, GdkEventConfigure *, gpointer);
            gint OnExposeLine (GtkWidget *, GdkEventExpose *, gpointer);
            gint OnExposeCursor (GtkWidget *, GdkEventExpose *, gpointer);
            gint OnMotionLofar (GtkWidget *, GdkEventMotion *, gpointer);
            gint OnPaletteActivate (GtkWidget *, gpointer);
            void OnAverageToggled (GtkToggleButton *, gpointer);
            void OnClipValueChanged (GtkAdjustment *, gpointer);
            void OnSaveClicks (GtkWidget *, gpointer);
            void OnGdkInput (gpointer, gint, GdkInputCondition);
    };

#endif

