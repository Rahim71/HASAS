/*

    GUI for locating
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


#include <gtk/gtk.h>

#include <Alloc.hh>

#include "Config.h"
#include "CfgFile.hh"
#include "GtkUtils.hh"
#include "Messages.hh"
#include "Palette.hh"
#include "SockClie.hh"
#include "SockOp.hh"


#ifndef GUILOCATE_HH
    #define GUILOCATE_HH

    #define GUILOC_VER_MAJ          GLOBAL_VERSMAJ
    #define GUILOC_VER_MIN          GLOBAL_VERSMIN
    #define GUILOC_VER_PL           GLOBAL_VERSPL
    #define GUILOC_WSPACING         8
    #define GUILOC_ENTRY_WIDTH      80
    #define GUILOC_SERVER_MAXLEN    256
    #define GUILOC_LOCATE_BG        0x00000000
    #define GUILOC_LOCATE_FG        0x00ffffff

    #define GUILOC_PALETTE_ITEMS    9


    /**
        Available palettes
    */
    enum
    {
        GUILOC_PAL_BW = 0,
        GUILOC_PAL_HSV = 1,
        GUILOC_PAL_LIGHT = 2,
        GUILOC_PAL_TEMP = 3,
        GUILOC_PAL_DIR = 4,
        GUILOC_PAL_GREEN = 5,
        GUILOC_PAL_GREEN2 = 6,
        GUILOC_PAL_PUREGREEN = 7,
        GUILOC_PAL_WB = 8
    };


    /**
        GUI for locating
    */
    class clGUILocate
    {
            bool bRun;
            bool bConnected;
            int iPalette;
            int iMsgSize;
            stLocateHdr sHdr;
            stLocateRes sRes;
            /* glib types */
            gint iGdkInputTag;
            GList *glServer;
            /* --- === --- */
            /* Gdk types */
            GdkGC *ggcLocateBG;
            GdkGC *ggcLocateFG;
            GdkCursor *gcCrossHair;
            /* --- === --- */
            /* Gtk+ widgets */
            // Top level
            GtkWidget *gwWindow;
            GtkWidget *gwVBox;
            // Table 1
            GtkWidget *gwTable1;
            GtkWidget *gwLServer;
            GtkWidget *gwCServer;
            GtkWidget *gwBConnect;
            // Table 2
            GtkWidget *gwTable2;
            GtkWidget *gwLPalette;
            GtkWidget *gwOMPalette;
            GtkWidget *gwMPalette;
            GtkWidget *gwaMIPalette[GUILOC_PALETTE_ITEMS];
            // Locate
            GtkWidget *gwSWLocate;
            GtkWidget *gwDALocate;
            /* --- === --- */
            clAlloc ResMsg;
            clAlloc ResMatrix;
            clAlloc ResFrame;
            clCfgFile Cfg;
            clGtkUtils GtkUtils;
            clLocateMsg Msg;
            clPalette Pal;
            clSockClie SClient;
            clSockOp SOp;
            bool GetCfg ();
            bool Build ();
            bool BuildTable1 ();
            bool BuildTable2 ();
            bool BuildLocate ();
            bool ConnectSignals ();
            bool BuildDrawingPrims ();
            bool ConnectToServer (const char *, int);
            void DisplayResults ();
        public:
            clGUILocate ();
            ~clGUILocate ();
            int Main (int *, char ***);
            gboolean OnDelete (GtkWidget *, GdkEvent *, gpointer);
            void OnConnectClick (GtkButton *, gpointer);
            void OnPaletteActivate (GtkMenuItem *, gpointer);
            gboolean OnLocateExpose (GtkWidget *, GdkEventExpose *, gpointer);
            gboolean OnLocateMotion (GtkWidget *, GdkEventMotion *, gpointer);
            void OnGdkInput (gpointer, gint, GdkInputCondition);
    };

#endif

