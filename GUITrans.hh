/*

    GUI for transient analysis
    Copyright (C) 2001 Jussi Laako

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

#include "Config.h"
#include "CfgFile.hh"
#include "GtkUtils.hh"


#ifndef GUITRANS_HH
    #define GUITRANS_HH

    #define GUITRANS_VER_MAJ        GLOBAL_VERSMAJ
    #define GUITRANS_VER_MIN        GLOBAL_VERSMIN
    #define GUITRANS_VER_PL         GLOBAL_VERSPL
    #define GUITRANS_WSPACING       8
    #define GUITRANS_ENTRY_WIDTH    80
    #define GUITRANS_SERVER_MAXLEN  256
    #define GUITRANS_LOCATE_BG      0x00000000
    #define GUITRANS_LOCATE_FG      0x00ffffff

    #define GUITRANS_PALETTE_ITEMS    9


    /**
        Available palettes
    */
    enum
    {
        GUITRANS_PAL_BW = 0,
        GUITRANS_PAL_HSV = 1,
        GUITRANS_PAL_LIGHT = 2,
        GUITRANS_PAL_TEMP = 3,
        GUITRANS_PAL_DIR = 4,
        GUITRANS_PAL_GREEN = 5,
        GUITRANS_PAL_GREEN2 = 6,
        GUITRANS_PAL_PUREGREEN = 7,
        GUITRANS_PAL_WB = 8
    };


    class clGUITransient
    {
            bool bRun;
            bool bConnected;
            /* glib types */
            gint iGdkInputTag;
            GList *glServer;
            /* --- === --- */
            /* Gdk types */
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
            /* --- === --- */
            clCfgFile Cfg;
            clGtkUtils GtkUtils;
            bool GetCfg ();
            bool Build ();
            bool BuildTable1 ();
            bool BuildDrawingPrims ();
            bool ConnectSignals ();
        public:
            clGUITransient ();
            ~clGUITransient ();
            int Main (int *, char ***);
            gboolean OnDelete (GtkWidget *, GdkEvent *, gpointer);
            void OnConnectClick (GtkButton *, gpointer);
    };


#endif
