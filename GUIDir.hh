/*

    Direction finding GUI
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


#ifndef GUIDIR_HH
    #define GUIDIR_HH

    #define DGUI_VER_MAJ            GLOBAL_VERSMAJ
    #define DGUI_VER_MIN            GLOBAL_VERSMIN
    #define DGUI_VER_PL             GLOBAL_VERSPL
    #define DGUI_PADDING            8
    #define DGUI_ENTRY_WIDTH        80
    #define DGUI_SERVER_MAXLEN      255
    #define DGUI_WORM_BG            0x00ffffff
    #define DGUI_WORM_FG            0x00000000
    #define DGUI_CONV_BUF_SIZE      255

    #define DGUI_ALGORITHM_ITEMS    4
    #define DGUI_SCALING_ITEMS      4
    #define DGUI_REMOVE_NOISE_ITEMS 5
    #define DGUI_PALETTE_ITEMS      9


    /**
        Available palettes
    */
    enum
    {
        DGUI_PAL_BW = 0,
        DGUI_PAL_HSV = 1,
        DGUI_PAL_LIGHT = 2,
        DGUI_PAL_TEMP = 3,
        DGUI_PAL_DIR = 4,
        DGUI_PAL_GREEN = 5,
        DGUI_PAL_GREEN2 = 6,
        DGUI_PAL_PUREGREEN = 7,
        DGUI_PAL_WB = 8
    };


    /**
        Direction finding GUI
    */
    class clGUIDir
    {
            /* Variables */
            volatile bool bRun;
            volatile bool bConnected;
            volatile bool bFreezed;
            bool bSaving;
            int iSockH;
            int iDirectionScale;
            int iHistoryLines;
            int iPalette;
            int iClips;
            int iWormWidth;
            int iWormHeight;
            int iCursorX;
            int iCursorY;
            int iTIFFCompression;
            int iJPEGQuality;
            int iCompressMode;
            int iContSaveScans;
            int iScanCount;
            int iImgCount;
            long lResultMsgBufSize;
            long lResultCount;
            float fSoundSpeed;
            float fLeftDirDeg;
            float fRightDirDeg;
            GDT fIntegrationTime;
            stDirReq sDirRq;
            stDirRes sResultHeader;
            /* glib types */
            GList *glServers;
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
            GtkWidget *gwBConnect;
            GtkWidget *gwBDisconnect;
            GtkWidget *gwCBFreeze;
            // Table 2
            GtkWidget *gwTable2;
            GtkWidget *gwLAlgorithm;
            GtkWidget *gwOMAlgorithm;
            GtkWidget *gwMAlgorithm;
            GtkWidget *gwaMIAlgorithm[DGUI_ALGORITHM_ITEMS];
            GtkWidget *gwLSoundSpeed;
            GtkWidget *gwESoundSpeed;
            GtkWidget *gwLLowFrequencyLimit;
            GtkWidget *gwELowFrequencyLimit;
            GtkWidget *gwLIntegrationTime;
            GtkWidget *gwEIntegrationTime;
            // Table 3
            GtkWidget *gwTable3;
            GtkWidget *gwLScaling;
            GtkWidget *gwOMScaling;
            GtkWidget *gwMScaling;
            GtkWidget *gwaMIScaling[DGUI_SCALING_ITEMS];
            GtkWidget *gwLScalingExponent;
            GtkWidget *gwEScalingExponent;
            GtkWidget *gwLRemoveNoise;
            GtkWidget *gwOMRemoveNoise;
            GtkWidget *gwMRemoveNoise;
            GtkWidget *gwaMIRemoveNoise[DGUI_REMOVE_NOISE_ITEMS];
            GtkWidget *gwLAlpha;
            GtkWidget *gwEAlpha;
            GtkWidget *gwLMeanLength;
            GtkWidget *gwEMeanLength;
            GtkWidget *gwLGapLength;
            GtkWidget *gwEGapLength;
            GtkWidget *gwCBNoFilter;
            GtkWidget *gwCBNormalize;
            GtkWidget *gwLPalette;
            GtkWidget *gwOMPalette;
            GtkWidget *gwMPalette;
            GtkWidget *gwaMIPalette[DGUI_PALETTE_ITEMS];
            // Table 4
            GtkWidget *gwTable4;
            GtkWidget *gwLLeftDirection;
            GtkWidget *gwELeftDirection;
            GtkWidget *gwLRightDirection;
            GtkWidget *gwERightDirection;
            GtkWidget *gwLSectorCount;
            GtkWidget *gwESectorCount;
            GtkWidget *gwLDirectionScale;
            GtkWidget *gwEDirectionScale;
            GtkWidget *gwCBSaving;
            GtkWidget *gwBSave;
            GtkWidget *gwFSSave;
            // Table worm
            GtkWidget *gwTableWorm;
            GtkWidget *gwHRDirection;
            GtkWidget *gwVRTime;
            GtkWidget *gwDAWorm;
            // Drawing primitives and cursors
            GdkGC *ggcWormBG;
            GdkGC *ggcWormFG;
            GdkCursor *gcCrossHair;
            /* Classes */
            std::string strImgFileName;
            clAlloc Results;
            clAlloc ResultMsgBuf;
            clAlloc ScaledResults;
            clCfgFile *Cfg;
            clFrameBuf FBDir;
            clGtkUtils GtkUtils;
            clSockClie SClient;
            clSockOp *SOp;
            clDirMsg DirMsg;
            clDSPOp DSP;
            /* Methods */
            bool Build ();
            bool ConnectSignals ();
            bool BuildTable1 ();
            bool BuildTable2 ();
            bool BuildTable3 ();
            bool BuildTable4 ();
            bool BuildTableWorm ();
            bool BuildDrawingPrims ();
            void FreeDrawingPrims ();
            bool ParseServerStr (char *, int *, const char *);
            bool InitConnection (const char *, int);
            void GetSettings ();
            bool SendSettings ();
            void PrintStatus ();
            void SaveInfo (const char *, time_t);
            void StartNewImgFile ();
        public:
            clGUIDir (int *, char ***);
            ~clGUIDir ();
            int Exec ();
            // Event handlers
            gint OnDelete (GtkWidget *, GdkEventAny *);
            void OnHideToggled (GtkToggleButton *, gpointer);
            gint OnConnectClick (GtkWidget *, gpointer gpData);
            void OnFreezeToggled (GtkToggleButton *, gpointer);
            gint OnExposeWorm (GtkWidget *, GdkEventExpose *);
            gint OnMotionWorm (GtkWidget *, GdkEventMotion *);
            gint OnPaletteActivate (GtkWidget *, gpointer);
            void OnSaveClicks (GtkWidget *, gpointer);
            void OnGdkInput (gpointer, gint, GdkInputCondition);
    };

#endif

