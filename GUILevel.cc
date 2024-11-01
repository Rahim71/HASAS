/*

    GUI for level server
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


#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <float.h>
#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>

#include "GUILevel.hh"


G_LOCK_DEFINE_STATIC(gmInputMutex);

static const char *cpWindowTxt = "Level";
// Table 1
static const char *cpLServerTxt = "Server";
static const char *cpaLChannelTxt[] = { "Channel", "Direction" };
static const char *cpBConnectTxt = "Connect";
// Table 2
static const char *cpLAlgorithmTxt = "Algorithm";
static const char *cpaLAlgorithmMenu[] = { "Peak", "RMS", "Mean", "Median",
    "StdDev" };
static const char *cpLIntegrationTimeTxt = "Integration time";
static const char *cpLLowFrequencyTxt = "Lower frequency";
static const char *cpLHighFrequencyTxt = "Higher frequency";
static const char *cpLDisplayLowTxt = "Display low limit";
static const char *cpLDisplayHighTxt = "Display high limit";

clGUILevel GUILevel;


int main (int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    return GUILevel.Main(&argc, &argv);
}


gboolean WrapOnDelete (GtkWidget *gwSender, GdkEvent *geEvent, 
    gpointer gpData)
{
    return GUILevel.OnDelete(gwSender, geEvent, gpData);
}


void WrapOnConnectClick (GtkButton *gbSender, gpointer gpData)
{
    GUILevel.OnConnectClick(gbSender, gpData);
}


gboolean WrapOnSofarExpose (GtkWidget *gwSender, GdkEventExpose *geeEvent,
    gpointer gpData)
{
    return GUILevel.OnSofarExpose(gwSender, geeEvent, gpData);
}


gboolean WrapOnSofarMotion (GtkWidget *gwSender, GdkEventMotion *gemEvent,
    gpointer gpData)
{
    return GUILevel.OnSofarMotion(gwSender, gemEvent, gpData);
}


gboolean WrapOnSofarConfigure (GtkWidget *gwSender, 
    GdkEventConfigure *gecEvent, gpointer gpData)
{
    return GUILevel.OnSofarConfigure(gwSender, gecEvent, gpData);
}


void WrapOnGdkInput (gpointer gpData, gint iSource, 
    GdkInputCondition gicCondition)
{
    GUILevel.OnGdkInput(gpData, iSource, gicCondition);
}


bool clGUILevel::GetCfg ()
{
    CfgFile.SetFileName(GUILEV_CFGFILE);
    CfgFile.GetFlt("DisplayLow", &fDisplayLow);
    CfgFile.GetFlt("DisplayHigh", &fDisplayHigh);
    CfgFile.GetFlt("IntegrationTime", &sRequest.fIntegrationTime);
    if (!CfgFile.GetInt("BeamCount", &iBeamCount))
        iBeamCount = 0;
    return true;
}


bool clGUILevel::Build ()
{
    gwWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gwWindow), cpWindowTxt);
    // shrink, grow, auto-shrink
    gtk_window_set_policy(GTK_WINDOW(gwWindow), TRUE, TRUE, FALSE);
    // homogenous, spacing
    gwVBox = gtk_vbox_new(FALSE, GUILEV_WSPACING);
    gtk_container_add(GTK_CONTAINER(gwWindow), gwVBox);
    gtk_widget_show(gwVBox);
    if (!BuildTable1()) return false;
    if (!BuildTable2()) return false;
    if (!BuildSofar()) return false;
    gwStatusBar = gtk_statusbar_new();
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwStatusBar, FALSE, FALSE, 0);
    gtk_widget_show(gwStatusBar);
    guSbCtxt = gtk_statusbar_get_context_id(GTK_STATUSBAR(gwStatusBar),
        "Status");
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guSbCtxt, "");
    if (!ConnectSignals()) return false;
    gtk_widget_show(gwWindow);
    if (!BuildDrawingPrims()) return false;
    gdk_window_set_cursor(gwDASofar->window, gcCrossHair);
    return true;
}


bool clGUILevel::BuildTable1 ()
{
    // rows, columns, homogenous
    gwTable1 = gtk_table_new(2, 3, FALSE);
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
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwLServer);
    gwCServer = gtk_combo_new();
    gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        GUILEV_SERVER_MAXLEN);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        "127.0.0.1:30001");
    // table, child, left attach, right attach, top attach, bottom attach,
    // x-options, y-options, x-padding, y-padding
    gtk_table_attach(GTK_TABLE(gwTable1), gwCServer,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwCServer);
    GtkUtils.ComboListFromFile(gwCServer, &glServer, GUILEV_HOSTFILE);

    // - Label & SpinButton: Channel
    if (iBeamCount)
        gwLChannel = gtk_label_new(cpaLChannelTxt[1]);
    else
        gwLChannel = gtk_label_new(cpaLChannelTxt[0]);
    gtk_label_set_justify(GTK_LABEL(gwLChannel), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable1), gwLChannel,
        1, 2, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwLChannel);
    if (iBeamCount)
    {
        // value, lower limit, upper limit, step increment, page increment,
        // page size
        goAChannel = gtk_adjustment_new(0.0, -90.0, 90.0, 1.0, 1.0, 1.0);
        // adjustment, climb rate, digits
        gwSBChannel = gtk_spin_button_new(GTK_ADJUSTMENT(goAChannel), 1.0, 1);
    }
    else
    {
        // value, lower limit, upper limit, step increment, page increment,
        // page size
        goAChannel = gtk_adjustment_new(1.0, GUILEV_CH_LOWER, GUILEV_CH_UPPER, 
            1.0, 1.0, 1.0);
        // adjustment, climb rate, digits
        gwSBChannel = gtk_spin_button_new(GTK_ADJUSTMENT(goAChannel), 1.0, 0);
    }
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(gwSBChannel), TRUE);
    gtk_table_attach(GTK_TABLE(gwTable1), gwSBChannel,
        1, 2, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwSBChannel);

    // - Button: Connect
    gwBConnect = gtk_button_new_with_label(cpBConnectTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBConnect,
        2, 3, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwBConnect);
    
    return true;
}


bool clGUILevel::BuildTable2 ()
{
    char *cpConvBuf;

    // rows, columns, homogenous
    gwTable2 = gtk_table_new(2, 6, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable2, FALSE, FALSE, 0);
    gtk_widget_show(gwTable2);

    // Label & OptionMenu: Algorithm
    gwLAlgorithm = gtk_label_new(cpLAlgorithmTxt);
    gtk_label_set_justify(GTK_LABEL(gwLAlgorithm), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLAlgorithm,
        0, 1, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwLAlgorithm);
    gwOMAlgorithm = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwOMAlgorithm,
        0, 1, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwOMAlgorithm);
    GtkUtils.BuildOptionMenu(gwOMAlgorithm, &gwMAlgorithm, gwaMIAlgorithm,
        cpaLAlgorithmMenu, GUILEV_ALGORITHM_ITEMS);

    // Label & Entry: Integration time
    gwLIntegrationTime = gtk_label_new(cpLIntegrationTimeTxt);
    gtk_label_set_justify(GTK_LABEL(gwLIntegrationTime), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLIntegrationTime,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwLIntegrationTime);
    gwEIntegrationTime = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEIntegrationTime,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwEIntegrationTime);
    gtk_widget_set_usize(gwEIntegrationTime, GUILEV_ENTRY_WIDTH,
        gwEIntegrationTime->requisition.height);
    cpConvBuf = g_strdup_printf("%g", sRequest.fIntegrationTime);
    gtk_entry_set_text(GTK_ENTRY(gwEIntegrationTime), cpConvBuf);
    g_free(cpConvBuf);

    // Label & Entry: Lower frequency
    gwLLowFrequency = gtk_label_new(cpLLowFrequencyTxt);
    gtk_label_set_justify(GTK_LABEL(gwLLowFrequency), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLLowFrequency,
        2, 3, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwLLowFrequency);
    gwELowFrequency = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwELowFrequency,
        2, 3, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwELowFrequency);
    gtk_widget_set_usize(gwELowFrequency, GUILEV_ENTRY_WIDTH,
        gwELowFrequency->requisition.height);

    // Label & Entry: Higher frequency
    gwLHighFrequency = gtk_label_new(cpLHighFrequencyTxt);
    gtk_label_set_justify(GTK_LABEL(gwLHighFrequency), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLHighFrequency,
        3, 4, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwLHighFrequency);
    gwEHighFrequency = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEHighFrequency,
        3, 4, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwEHighFrequency);
    gtk_widget_set_usize(gwEHighFrequency, GUILEV_ENTRY_WIDTH,
        gwEHighFrequency->requisition.height);

    // Label & Entry: Display low limit
    gwLDisplayLow = gtk_label_new(cpLDisplayLowTxt);
    gtk_label_set_justify(GTK_LABEL(gwLDisplayLow), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLDisplayLow,
        4, 5, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwLDisplayLow);
    gwEDisplayLow = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEDisplayLow,
        4, 5, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwEDisplayLow);
    gtk_widget_set_usize(gwEDisplayLow, GUILEV_ENTRY_WIDTH,
        gwEDisplayLow->requisition.height);
    cpConvBuf = g_strdup_printf("%g", fDisplayLow);
    gtk_entry_set_text(GTK_ENTRY(gwEDisplayLow), cpConvBuf);
    g_free(cpConvBuf);

    // Label & Entry: Display high limit
    gwLDisplayHigh = gtk_label_new(cpLDisplayHighTxt);
    gtk_label_set_justify(GTK_LABEL(gwLDisplayHigh), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLDisplayHigh,
        5, 6, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwLDisplayHigh);
    gwEDisplayHigh = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEDisplayHigh,
        5, 6, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILEV_WSPACING / 2, 0);
    gtk_widget_show(gwEDisplayHigh);
    gtk_widget_set_usize(gwEDisplayHigh, GUILEV_ENTRY_WIDTH,
        gwEDisplayHigh->requisition.height);
    cpConvBuf = g_strdup_printf("%g", fDisplayHigh);
    gtk_entry_set_text(GTK_ENTRY(gwEDisplayHigh), cpConvBuf);
    g_free(cpConvBuf);

    return true;
}


bool clGUILevel::BuildSofar ()
{
    // rows, columns, homogenous
    gwTableSofar = gtk_table_new(2, 2, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTableSofar, TRUE, TRUE, 0);
    gtk_widget_show(gwTableSofar);

    // Horizontal Ruler: Time
    gwHRTime = gtk_hruler_new();
    gtk_table_attach(GTK_TABLE(gwTableSofar), gwHRTime,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        0, 0);
    gtk_widget_show(gwHRTime);
    gtk_ruler_set_metric(GTK_RULER(gwHRTime), GTK_PIXELS);
    // lower, upper, position, max size
    gtk_ruler_set_range(GTK_RULER(gwHRTime), -1.0, 0.0, 0.0, 1.0);

    // Vertical Ruler: Level
    gwVRLevel = gtk_vruler_new();
    gtk_table_attach(GTK_TABLE(gwTableSofar), gwVRLevel,
        0, 1, 1, 2,
        (GtkAttachOptions) 0,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        0, 0);
    gtk_widget_show(gwVRLevel);
    gtk_ruler_set_metric(GTK_RULER(gwVRLevel), GTK_PIXELS);
    // lower, upper, position, max size
    gtk_ruler_set_range(GTK_RULER(gwVRLevel), fDisplayHigh, fDisplayLow, 
        0.0, fabs(fDisplayHigh - fDisplayLow));

    gwDASofar = gtk_drawing_area_new();
    gtk_table_attach(GTK_TABLE(gwTableSofar), gwDASofar,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        0, 0);
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDASofar), 
        GUILEV_SOFAR_WIDTH, GUILEV_SOFAR_HEIGHT);
    gtk_widget_show(gwDASofar);
    
    return true;
}


bool clGUILevel::ConnectSignals ()
{
    gtk_signal_connect(GTK_OBJECT(gwWindow), "delete-event", 
        GTK_SIGNAL_FUNC(WrapOnDelete), NULL);

    gtk_signal_connect(GTK_OBJECT(gwBConnect), "clicked",
        GTK_SIGNAL_FUNC(WrapOnConnectClick), NULL);

    gtk_widget_add_events(gwDASofar, GDK_POINTER_MOTION_MASK);
    GtkUtils.ConnectMotionEvent(gwHRTime, gwDASofar);
    GtkUtils.ConnectMotionEvent(gwVRLevel, gwDASofar);
    gtk_signal_connect(GTK_OBJECT(gwDASofar), "expose-event", 
        GTK_SIGNAL_FUNC(WrapOnSofarExpose), NULL);
    gtk_signal_connect(GTK_OBJECT(gwDASofar), "motion-notify-event",
        GTK_SIGNAL_FUNC(WrapOnSofarMotion), NULL);
    gtk_signal_connect(GTK_OBJECT(gwDASofar), "configure-event",
        GTK_SIGNAL_FUNC(WrapOnSofarConfigure), NULL);

    return true;
}


bool clGUILevel::BuildDrawingPrims ()
{
    ggcSofarBG = gdk_gc_new(gwDASofar->window);
    gdk_rgb_gc_set_foreground(ggcSofarBG, GUILEV_LINE_BG);
    gdk_rgb_gc_set_background(ggcSofarBG, GUILEV_LINE_BG);
    gdk_gc_set_function(ggcSofarBG, GDK_COPY);
    gdk_gc_set_fill(ggcSofarBG, GDK_SOLID);

    ggcSofarFG = gdk_gc_new(gwDASofar->window);
    gdk_rgb_gc_set_foreground(ggcSofarFG, GUILEV_LINE_FG);
    gdk_rgb_gc_set_background(ggcSofarFG, GUILEV_LINE_BG);
    gdk_gc_set_function(ggcSofarFG, GDK_COPY);
    gdk_gc_set_fill(ggcSofarFG, GDK_SOLID);

    gcCrossHair = gdk_cursor_new(GDK_CROSSHAIR);

    return true;
}


bool clGUILevel::ConnectToServer (const char *cpHostAddr, int iHostPort)
{
    int iSockH;
    char cpReqProcName[GLOBAL_HEADER_LEN];
    
    iSockH = SClient.Connect(cpHostAddr, NULL, iHostPort);
    if (iSockH < 0) return false;
    SOp.SetHandle(iSockH);
    strcpy(cpReqProcName, GUILEV_REQ_PROC);
    if (SOp.WriteN(cpReqProcName, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    return true;
}


bool clGUILevel::SendSettings ()
{
    float fDirection;
    char cpRequestMsg[GLOBAL_HEADER_LEN];

    if (iBeamCount)
    {
        fDirection = gtk_spin_button_get_value_as_float(
            GTK_SPIN_BUTTON(gwSBChannel)) + 90.0f;
        sRequest.iChannel = (int)
            (fDirection / (180.0f / (float) (iBeamCount - 1)));
        g_print("Channel: %i\n", sRequest.iChannel);
    }
    else
    {
        sRequest.iChannel = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(gwSBChannel)) - 1;
    }
    sRequest.iAlgorithm = GtkUtils.OptionMenuGetActive(gwOMAlgorithm,
        gwaMIAlgorithm, GUILEV_ALGORITHM_ITEMS);
    sRequest.fIntegrationTime = strtod(
        gtk_entry_get_text(GTK_ENTRY(gwEIntegrationTime)), NULL);
    sRequest.fLowFrequency = strtod(
        gtk_entry_get_text(GTK_ENTRY(gwELowFrequency)), NULL);
    sRequest.fHighFrequency = strtod(
        gtk_entry_get_text(GTK_ENTRY(gwEHighFrequency)), NULL);
    Msg.SetRequest(cpRequestMsg, &sRequest);
    if (SOp.WriteN(cpRequestMsg, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    return true;
}


void clGUILevel::DisplayResults ()
{
    //long lXCntr;
    long lResultIdx;
    gint iPointY;
    float fScaler;

    fScaler = gwDASofar->allocation.height / fabs(fDisplayHigh - fDisplayLow);
    /*gdk_window_clear(gwDASofar->window);
    for (lXCntr = 0L; lXCntr < lResultCount; lXCntr++)
    {
        lResultIdx = lXCntr + lResultPos + 1;
        iPointY = (gint) (fabs(spResults[lResultIdx].fResult - fDisplayHigh) *
            fScaler + 0.5F);
        gdk_draw_line(gwDASofar->window, ggcSofarFG,
            lXCntr, iPointY,
            lXCntr, gwDASofar->allocation.height - 1);
    }*/
    gdk_window_copy_area(gwDASofar->window, ggcSofarFG,
        0, 0,
        gwDASofar->window,
        1, 0, gwDASofar->allocation.width - 1, gwDASofar->allocation.height);
    gdk_window_clear_area(gwDASofar->window, 
        gwDASofar->allocation.width - 1, 0,
        1, gwDASofar->allocation.height);
    lResultIdx = lResultCount + lResultPos;
    iPointY = (gint) (fabs(spResults[lResultIdx].fResult - fDisplayHigh) *
        fScaler + 0.5F);
    gdk_draw_line(gwDASofar->window, ggcSofarFG,
        gwDASofar->allocation.width - 1, iPointY,
        gwDASofar->allocation.width - 1, gwDASofar->allocation.height - 1);
}


void clGUILevel::InitializeDisplay ()
{
    gfloat fTimeRange;
    char *cpConvBuf;

    fDisplayLow = strtod(gtk_entry_get_text(GTK_ENTRY(gwEDisplayLow)), NULL);
    fDisplayHigh = strtod(gtk_entry_get_text(GTK_ENTRY(gwEDisplayHigh)), NULL);
    fTimeRange = gwDASofar->allocation.width * 
        spResults[lResultPos].fIntegrationTime;
    gtk_ruler_set_range(GTK_RULER(gwHRTime), 
        -fTimeRange, 0.0, 
        0.0, fTimeRange);
    gtk_ruler_set_range(GTK_RULER(gwVRLevel),
        fDisplayHigh, fDisplayLow,
        0.0, fabs(fDisplayHigh - fDisplayLow));
    cpConvBuf = g_strdup_printf("%g", spResults[lResultPos].fIntegrationTime);
    gtk_entry_set_text(GTK_ENTRY(gwEIntegrationTime), cpConvBuf);
    g_free(cpConvBuf);
}


clGUILevel::clGUILevel ()
{
    bRun = true;
    bConnected = false;
    lResultPos = 0L;
    lResultCount = 0L;
    spResults = NULL;
}


clGUILevel::~clGUILevel ()
{
    if (spResults != NULL) g_free(spResults);
}


int clGUILevel::Main (int *ipArgC, char ***cppArgV)
{
    g_print("%s GUI v%i.%i.%i\n", cpWindowTxt,
        GUILEV_VER_MAJ, GUILEV_VER_MIN, GUILEV_VER_PL);
    g_print("Copyright (C) 2000-2002 Jussi Laako\n\n");
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


gboolean clGUILevel::OnDelete (GtkWidget *gwSender, GdkEvent *geEvent,
    gpointer gpData)
{
    bRun = false;
    gtk_main_quit();
    return TRUE;
}


void clGUILevel::OnConnectClick (GtkButton *gbSender, gpointer gpData)
{
    char **cpaServerAddr;

    if (bConnected)
    {
        gdk_input_remove(iGdkInputTag);
        SOp.Close();
    }
    cpaServerAddr = g_strsplit(
        gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(gwCServer)->entry)),
        ":", 2);
    if (cpaServerAddr[0] != NULL && cpaServerAddr[1] != NULL)
    {
        g_print("\nConnecting to %s port %s...\n", cpaServerAddr[0],
            cpaServerAddr[1]);
        if (ConnectToServer(cpaServerAddr[0], atoi(cpaServerAddr[1])))
        {
            bConnected = true;
            g_print("Connected, sending settings...\n");
            if (SendSettings())
            {
                g_print("OK\n");
                bFirstResult = true;
                iGdkInputTag = gdk_input_add(SOp.GetHandle(), GDK_INPUT_READ,
                    WrapOnGdkInput, NULL);
            }
            else
            {
                g_print("Failed to send settings!\n");
            }
        }
        else
        {
            g_print("Failed to connect to specified server!\n");
        }
    }
    else
    {
        g_print(
            "\nIncorrect server address format. Format: <server>:<port>\n");
    }
    g_strfreev(cpaServerAddr);
}


gboolean clGUILevel::OnSofarExpose (GtkWidget *gwSender, 
    GdkEventExpose *geeEvent, gpointer gpData)
{
    long lXCntr;
    long lResultIdx;
    gint iPointY;
    float fScaler;

    fScaler = gwSender->allocation.height / fabs(fDisplayHigh - fDisplayLow);
    for (lXCntr = geeEvent->area.x; 
        lXCntr < (geeEvent->area.x + geeEvent->area.width); 
        lXCntr++)
    {
        lResultIdx = lXCntr + lResultPos + 1;
        iPointY = (gint) (fabs(spResults[lResultIdx].fResult - fDisplayHigh) *
            fScaler + 0.5F);
        gdk_draw_line(gwSender->window, ggcSofarFG,
            lXCntr, iPointY,
            lXCntr, gwSender->allocation.height - 1);
    }
    return TRUE;
}


gboolean clGUILevel::OnSofarMotion (GtkWidget *gwSender,
    GdkEventMotion *gemEvent, gpointer gpData)
{
    char *cpConvBuf;
    float fXPos;
    float fYPos;

    fXPos = (gwSender->allocation.width - gemEvent->x + 1) *
        spResults[lResultPos].fIntegrationTime;
    fYPos = -(fabs(fDisplayHigh - fDisplayLow) / gwSender->allocation.height *
        gemEvent->y - fDisplayHigh);
    gtk_statusbar_pop(GTK_STATUSBAR(gwStatusBar), guSbCtxt);
    cpConvBuf = g_strdup_printf("%g %g = %g", 
        fXPos, fYPos,
        (spResults[(long) gemEvent->x + lResultPos].fResult));
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guSbCtxt, cpConvBuf);
    g_free(cpConvBuf);
    return TRUE;
}


gboolean clGUILevel::OnSofarConfigure (GtkWidget *gwSender,
    GdkEventConfigure *gecEvent, gpointer gpData)
{
    G_LOCK(gmInputMutex);
    lResultCount = gecEvent->width;
    spResults = (stpLevelRes) 
        g_realloc(spResults, lResultCount * sizeof(stLevelRes) * 2);
    G_UNLOCK(gmInputMutex);
    return TRUE;
}


void clGUILevel::OnGdkInput (gpointer gpData, gint iSource,
    GdkInputCondition gicCondition)
{
    if (!bRun) return;
    G_LOCK(gmInputMutex);
    while (SOp.ReadSelect(0))
    {
        if (SOp.ReadN(cpResultBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        {
            gdk_input_remove(iGdkInputTag);
            SOp.Close();
            bConnected = false;
            g_print(
                "Error occurred while receiving data, dropped connection.\n");
            break;
        }
        lResultPos++;
        if (lResultPos >= lResultCount) lResultPos = 0L;
        Msg.GetResult(cpResultBuf, &spResults[lResultPos]);
        memcpy(&spResults[lResultPos + lResultCount], &spResults[lResultPos],
            sizeof(stLevelRes));
        if (bFirstResult)
        {
            InitializeDisplay();
            bFirstResult = false;
        }
        DisplayResults();
    }
    G_UNLOCK(gmInputMutex);
}

