/*

    GUI for level server, header
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


#include <gtk/gtk.h>

#include <Alloc.hh>

#include "Config.h"
#include "CfgFile.hh"
#include "GtkUtils.hh"
#include "Messages.hh"
#include "SockClie.hh"
#include "SockOp.hh"


#ifndef GUILEVEL_HH
    #define GUILEVEL_HH

    #define GUILEV_VER_MAJ          GLOBAL_VERSMAJ
    #define GUILEV_VER_MIN          GLOBAL_VERSMIN
    #define GUILEV_VER_PL           GLOBAL_VERSPL
    #define GUILEV_WSPACING         8
    #define GUILEV_ENTRY_WIDTH      80
    #define GUILEV_SOFAR_WIDTH      600
    #define GUILEV_SOFAR_HEIGHT     100
    #define GUILEV_SERVER_MAXLEN    256
    #define GUILEV_CH_LOWER         1
    #define GUILEV_CH_UPPER         16
    #define GUILEV_LINE_BG          0x00ffffff
    #define GUILEV_LINE_FG          0x000000ff

    #define GUILEV_ALGORITHM_ITEMS  5


    /**
        GUI for level server
    */
    class clGUILevel
    {
            bool bRun;
            bool bConnected;
            bool bFirstResult;
            int iBeamCount;
            long lResultPos;
            long lResultCount;
            float fDisplayLow;
            float fDisplayHigh;
            char cpResultBuf[GLOBAL_HEADER_LEN];
            stLevelReq sRequest;
            stpLevelRes spResults;
            /* glib types */
            guint guSbCtxt;
            gint iGdkInputTag;
            GList *glServer;
            /* --- === --- */
            /* Gdk types */
            GdkGC *ggcSofarBG;
            GdkGC *ggcSofarFG;
            GdkCursor *gcCrossHair;
            /* --- === --- */
            /* Gtk+ widgets */
            // Top level
            GtkWidget *gwWindow;
            GtkWidget *gwVBox;
            GtkWidget *gwStatusBar;
            // Table 1
            GtkWidget *gwTable1;
            GtkWidget *gwLServer;
            GtkWidget *gwCServer;
            GtkWidget *gwLChannel;
            GtkObject *goAChannel;
            GtkWidget *gwSBChannel;
            GtkWidget *gwBConnect;
            // Table 2
            GtkWidget *gwTable2;
            GtkWidget *gwLAlgorithm;
            GtkWidget *gwOMAlgorithm;
            GtkWidget *gwMAlgorithm;
            GtkWidget *gwaMIAlgorithm[GUILEV_ALGORITHM_ITEMS];
            GtkWidget *gwLIntegrationTime;
            GtkWidget *gwEIntegrationTime;
            GtkWidget *gwLLowFrequency;
            GtkWidget *gwELowFrequency;
            GtkWidget *gwLHighFrequency;
            GtkWidget *gwEHighFrequency;
            GtkWidget *gwLDisplayLow;
            GtkWidget *gwEDisplayLow;
            GtkWidget *gwLDisplayHigh;
            GtkWidget *gwEDisplayHigh;
            // Sofar
            GtkWidget *gwTableSofar;
            GtkWidget *gwHRTime;
            GtkWidget *gwVRLevel;
            GtkWidget *gwDASofar;
            /* --- === --- */
            //clAlloc ResultsBuf;
            clCfgFile CfgFile;
            clGtkUtils GtkUtils;
            clSockClie SClient;
            clSockOp SOp;
            clLevelMsg Msg;
            bool GetCfg ();
            bool Build ();
            bool BuildTable1 ();
            bool BuildTable2 ();
            bool BuildSofar ();
            bool ConnectSignals ();
            bool BuildDrawingPrims ();
            bool ConnectToServer (const char *, int);
            bool SendSettings ();
            void DisplayResults ();
            void InitializeDisplay ();
        public:
            clGUILevel ();
            ~clGUILevel ();
            int Main (int *, char ***);
            gboolean OnDelete (GtkWidget *, GdkEvent *, gpointer);
            void OnConnectClick (GtkButton *, gpointer);
            gboolean OnSofarExpose (GtkWidget *, GdkEventExpose *, gpointer);
            gboolean OnSofarMotion (GtkWidget *, GdkEventMotion *, gpointer);
            gboolean OnSofarConfigure (GtkWidget *, GdkEventConfigure *, 
                gpointer);
            void OnGdkInput (gpointer, gint, GdkInputCondition);
    };

#endif

