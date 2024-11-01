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


#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <float.h>

#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>

#include "GUITrans.hh"


static const char *cpWindowTxt = "Locate";
// Table 1
static const char *cpLServerTxt = "Server";
static const char *cpBConnectTxt = "Connect";


clGUITransient GUITransient;


int main (int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    return GUITransient.Main(&argc, &argv);
}


gboolean WrapOnDelete (GtkWidget *gwSender, GdkEvent *geEvent,
    gpointer gpData)
{
    return GUITransient.OnDelete(gwSender, geEvent, gpData);
}


void WrapOnConnectClick (GtkButton *gbSender, gpointer gpData)
{
    GUITransient.OnConnectClick(gbSender, gpData);
}


bool clGUITransient::GetCfg ()
{
    Cfg.SetFileName(GUITRANS_CFGFILE);
    return true;
}


bool clGUITransient::Build ()
{
    gwWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gwWindow), cpWindowTxt);
    // shrink, grow, auto-shrink
    gtk_window_set_policy(GTK_WINDOW(gwWindow), TRUE, TRUE, FALSE);
    // homogenous, spacing
    gwVBox = gtk_vbox_new(FALSE, GUITRANS_WSPACING);
    gtk_container_add(GTK_CONTAINER(gwWindow), gwVBox);
    gtk_widget_show(gwVBox);
    if (!BuildTable1()) return false;
    if (!ConnectSignals()) return false;
    gtk_widget_show(gwWindow);
    if (!BuildDrawingPrims()) return false;
    //gdk_window_set_cursor(gwDALocate->window, gcCrossHair);
    return true;
}


bool clGUITransient::BuildTable1 ()
{
    // rows, columns, homogenous
    gwTable1 = gtk_table_new(2, 2, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable1, FALSE, FALSE, 0);
    gtk_widget_show(gwTable1);

    // - Label & Combo: Server
    gwLServer = gtk_label_new(cpLServerTxt);
    gtk_label_set_justify(GTK_LABEL(gwLServer), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable1), gwLServer,
        0, 1, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUITRANS_WSPACING / 2, 0);
    gtk_widget_show(gwLServer);
    gwCServer = gtk_combo_new();
    gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        GUITRANS_SERVER_MAXLEN);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        "127.0.0.1:30001");
    // table, child, left attach, right attach, top attach, bottom attach,
    // x-options, y-options, x-padding, y-padding
    gtk_table_attach(GTK_TABLE(gwTable1), gwCServer,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUITRANS_WSPACING / 2, 0);
    gtk_widget_show(gwCServer);
    GtkUtils.ComboListFromFile(gwCServer, &glServer, GUITRANS_HOSTFILE);

    // - Button: Connect
    gwBConnect = gtk_button_new_with_label(cpBConnectTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBConnect,
        1, 2, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        GUITRANS_WSPACING / 2, 0);
    gtk_widget_show(gwBConnect);

    return true;
}


bool clGUITransient::ConnectSignals ()
{
    gtk_signal_connect(GTK_OBJECT(gwWindow), "delete-event",
        GTK_SIGNAL_FUNC(WrapOnDelete), NULL);

    gtk_signal_connect(GTK_OBJECT(gwBConnect), "clicked",
        GTK_SIGNAL_FUNC(WrapOnConnectClick), NULL);

    return true;
}


bool clGUITransient::BuildDrawingPrims ()
{
    return true;
}


clGUITransient::clGUITransient ()
{
    bRun = true;
    bConnected = false;
}


clGUITransient::~clGUITransient ()
{
}


int clGUITransient::Main (int *ipArgC, char ***cppArgV)
{
    g_print("%s GUI v%i.%i.%i\n", cpWindowTxt,
        GUITRANS_VER_MAJ, GUITRANS_VER_MIN, GUITRANS_VER_PL);
    g_print("Copyright (C) 2001 Jussi Laako\n\n");
    g_print("Gtk+ version %i.%i.%i\n", gtk_major_version, gtk_minor_version,
        gtk_micro_version);
    g_print("Locale set to %s\n", gtk_set_locale());
    gtk_init(ipArgC, cppArgV);
    gdk_rgb_init();
    gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
    gtk_widget_set_default_visual(gdk_rgb_get_visual());
    if (!GetCfg()) return 1;
    if (!Build()) return 1;
    gtk_main();
    return 0;
}


gboolean clGUITransient::OnDelete (GtkWidget *gwSender, GdkEvent *geEvent,
    gpointer gpData)
{
    bRun = false;
    gtk_main_quit();
    return TRUE;
}


void clGUITransient::OnConnectClick (GtkButton *gbSender, gpointer gpData)
{
}
