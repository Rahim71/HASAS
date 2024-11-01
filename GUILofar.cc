/*

    LOFAR/DEMON GUI
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>

#include "GUILofar.hh"


G_LOCK_DEFINE_STATIC(gmInputMutex);

static const char *cpWindowTxt = "LOFAR / DEMON";
// Table 1
static const char *cpLServerTxt = "Server";
static const char *cpaLChannelTxt[] = { "Channel", "Direction" };
static const char *cpBConnectTxt = "Connect";
static const char *cpBDisconnectTxt = "Disconnect";
static const char *cpCBFreezeTxt = "Freeze";
// Table 2
static const char *cpLTypeTxt = "Type";
static const char *cpaLTypeMenu[] = { "FFT", "Cepstrum", "Autocorr." };
static const char *cpLWindowTxt = "Window";
static const char *cpaLWindowMenu[] = { "Rectangle", "Bartlett",
    "Blackman", "Blackman-Harris", "Cosine tapered", "Exponential",
    "Flat-top", "Hamming", "Hanning", "Kaiser", "Kaiser-Bessel", "Tukey" };
static const char *cpLWinParamTxt = "Window param.";
static const char *cpLWinLengthTxt = "Window length";
static const char *cpaLWinLengthMenu[] = { "1024", "2048", "4096", "8192",
    "16384", "32768", "65536" };
static const char *cpLLowerFreqTxt = "Lower freq.";
static const char *cpLHigherFreqTxt = "Higher freq.";
static const char *cpLOverlapTxt = "Overlap %";
// Table 3
static const char *cpLRemoveNoiseTxt = "Remove noise";
static const char *cpaLRemoveNoiseMenu[] = { "None", "TPSW", "OTA",
    "Diff", "InvDiff", "StdDev" };
static const char *cpLAlphaTxt = "Alpha";
static const char *cpLMeanLengthTxt = "Mean length";
static const char *cpLGapLengthTxt = "Gap length";
static const char *cpLAverageCountTxt = "Average count";
static const char *cpLClipTxt = "Clip";
static const char *cpaLClipMenu[] = { "None", "Low", "Both", "Mean" ,
    "Median", "10 dB", "20 dB", "50%", "75%", "Offset", "Offset 2",
    "Offset 3", "Sliding" };
static const char *cpCBLinearTxt = "Linear";
static const char *cpCBDemonTxt = "DEMON";
static const char *cpLPaletteTxt = "Palette";
static const char *cpaLPaletteMenu[] = { "BW", "HSV", "Light", "Temp",
    "Dir", "Green", "Green2", "Green3", "Green4", "PureGreen", "WB" };
static const char *cpCBAverageTxt = "ContAvg";
static const char *cpCBSavingTxt = "Saving";
static const char *cpBSaveTxt = "Save";
static const char *cpFSSaveTxt = "Save to TIFF file";
// Table 4
static const char *cpLClipValueTxt = "Clip value";

clGUILofar *GUILofar;


int main(int argc, char *argv[])
{
    int iRetVal;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    GUILofar = new clGUILofar(&argc, &argv);
    iRetVal = GUILofar->Exec();
    delete GUILofar;
    return iRetVal;
}


gint WrapOnDelete (GtkWidget *gwSender, GdkEventAny *geaEvent)
{
    return GUILofar->OnDelete(gwSender, geaEvent);
}


void WrapOnHideToggled (GtkToggleButton *gtbSender, gpointer gpData)
{
    GUILofar->OnHideToggled(gtbSender, gpData);
}


gint WrapOnConnectClick (GtkWidget *gwSender, gpointer gpData)
{
    return GUILofar->OnConnectClick(gwSender, gpData);
}


void WrapOnFreezeToggled (GtkToggleButton *gtbSender, gpointer gpData)
{
    GUILofar->OnFreezeToggled(gtbSender, gpData);
}


gint WrapOnExposeLofar (GtkWidget *gwSender, GdkEventExpose *geeEvent,
    gpointer gpData)
{
    return GUILofar->OnExposeLofar(gwSender, geeEvent, gpData);
}


gint WrapOnConfigureLofar (GtkWidget *gwSender, GdkEventConfigure *gecEvent,
    gpointer gpData)
{
    return GUILofar->OnConfigureLofar(gwSender, gecEvent, gpData);
}


gint WrapOnExposeLine (GtkWidget *gwSender, GdkEventExpose *geeEvent,
    gpointer gpData)
{
    return GUILofar->OnExposeLine(gwSender, geeEvent, gpData);
}


gint WrapOnExposeCursor (GtkWidget *gwSender, GdkEventExpose *geeEvent,
    gpointer gpData)
{
    return GUILofar->OnExposeCursor(gwSender, geeEvent, gpData);
}


gint WrapOnMotionLofar (GtkWidget *gwSender, GdkEventMotion *gemEvent,
    gpointer gpData)
{
    return GUILofar->OnMotionLofar(gwSender, gemEvent, gpData);
}


gint WrapOnPaletteActivate (GtkWidget *gwSender, gpointer gpData)
{
    return GUILofar->OnPaletteActivate(gwSender, gpData);
}


void WrapOnAverageToggled (GtkToggleButton *gtbSender, gpointer gpData)
{
    GUILofar->OnAverageToggled(gtbSender, gpData);
}


void WrapOnClipValueChanged (GtkAdjustment *gaSender, gpointer gpData)
{
    GUILofar->OnClipValueChanged(gaSender, gpData);
}


void WrapOnSaveClicks (GtkWidget *gwSender, gpointer gpData)
{
    GUILofar->OnSaveClicks(gwSender, gpData);
}


void WrapOnGdkInput (gpointer gpData, gint giSource,
    GdkInputCondition gicCondition)
{
    GUILofar->OnGdkInput(gpData, giSource, gicCondition);
}


bool clGUILofar::Build ()
{
    // --- Main window
    gwWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gwWindow), cpWindowTxt);
    // shrink, grow, auto-shrink
    gtk_window_set_policy(GTK_WINDOW(gwWindow), TRUE, TRUE, FALSE);
    #if (GTK_MAJOR_VERSION == 1)
    gtk_window_set_default_size(GTK_WINDOW(gwWindow), 
        gdk_screen_width() - LGUI_PADDING * 2,
        gwWindow->requisition.height);
    #endif
    // Realization of main window is delayed for later time
    // gtk_widget_show(gwWindow);

    // --- Vertical box
    // homogenous, spacing
    gwVBox = gtk_vbox_new(FALSE, LGUI_PADDING);
    gtk_container_add(GTK_CONTAINER(gwWindow), gwVBox);
    gtk_widget_show(gwVBox);

    // --- Hide button
    gwCBHide = gtk_check_button_new();
    gtk_box_pack_start(GTK_BOX(gwVBox), gwCBHide, FALSE, FALSE, 0);
    gtk_widget_show(gwCBHide);

    // --- Table 1
    if (!BuildTable1()) return false;

    // --- Table 2
    if (!BuildTable2()) return false;

    // --- Table 3
    if (!BuildTable3()) return false;

    // --- Table 4
    if (!BuildTable4()) return false;

    // --- Table Lofar
    if (!BuildTableLofar()) return false;

    // --- Status bar
    gwStatusBar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(gwVBox), gwStatusBar, FALSE, FALSE, 0);
    gtk_widget_show(gwStatusBar);
    guSbCtxt = gtk_statusbar_get_context_id(GTK_STATUSBAR(gwStatusBar),
        "status");
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guSbCtxt, "");

    // Delayed realization of main window
    gtk_widget_show(gwWindow);
    #if (GTK_MAJOR_VERSION > 1)
    gtk_window_resize(GTK_WINDOW(gwWindow), 
        gdk_screen_width() - LGUI_PADDING * 2,
        gwWindow->requisition.height);
    #endif

    // Build drawing primitives
    if (!BuildDrawingPrims()) return false;

    // Set cursors
    gdk_window_set_cursor(gwDALofar->window, gcCrossHair);
    gdk_window_set_cursor(gwDALine->window, gcCrossHair);

    // Enable backing store
    //GtkUtils.EnableBackingStore(gwDALofar);

    // Set default widget
    //gtk_window_set_default(GTK_WINDOW(gwWindow), gwBConnect);

    return true;
}


bool clGUILofar::BuildTable1 ()
{
    // rows, columns, homogenous
    gwTable1 = gtk_table_new(2, 5, FALSE);
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
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLServer);
    gwCServer = gtk_combo_new();
    gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        LGUI_SERVER_MAXLEN);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        "127.0.0.1:30001");
    // table, child, left attach, right attach, top attach, bottom attach,
    // x-options, y-options, x-padding, y-padding
    gtk_table_attach(GTK_TABLE(gwTable1), gwCServer,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwCServer);
    GtkUtils.ComboListFromFile(gwCServer, &glServer, LGUI_HOSTFILE);

    // - Label & SpinButton: Channel
    if (iBeamCount)
        gwLChannel = gtk_label_new(cpaLChannelTxt[1]);
    else
        gwLChannel = gtk_label_new(cpaLChannelTxt[0]);
    gtk_label_set_justify(GTK_LABEL(gwLChannel), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable1), gwLChannel,
        1, 2, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
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
        goAChannel = gtk_adjustment_new(1.0, LGUI_CH_LOWER, LGUI_CH_UPPER, 1.0,
            1.0, 1.0);
        // adjustment, climb rate, digits
        gwSBChannel = gtk_spin_button_new(GTK_ADJUSTMENT(goAChannel), 1.0, 0);
    }
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(gwSBChannel), TRUE);
    gtk_table_attach(GTK_TABLE(gwTable1), gwSBChannel,
        1, 2, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwSBChannel);

    // - Button: Connect
    gwBConnect = gtk_button_new_with_label(cpBConnectTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBConnect,
        2, 3, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwBConnect);

    // - Button: Disconnect
    gwBDisconnect = gtk_button_new_with_label(cpBDisconnectTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBDisconnect,
        3, 4, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwBDisconnect);

    // - CheckButton: Freeze
    gwCBFreeze = gtk_check_button_new_with_label(cpCBFreezeTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwCBFreeze,
        4, 5, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwCBFreeze);

    return true;
}


bool clGUILofar::BuildTable2 ()
{
    char cpConvBuf[LGUI_CONV_BUF_SIZE];

    // rows, columns, homogenous
    gwTable2 = gtk_table_new(2, 7, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable2, FALSE, FALSE, 0);
    gtk_widget_show(gwTable2);

    // Label & OptionMenu: Type
    gwLType = gtk_label_new(cpLTypeTxt);
    gtk_label_set_justify(GTK_LABEL(gwLType), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLType,
        0, 1, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLType);
    gwOMType = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwOMType,
        0, 1, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMType);
    GtkUtils.BuildOptionMenu(gwOMType, &gwMType, gwaMIType,
        cpaLTypeMenu, LGUI_TYPE_ITEMS);

    // Label & OptionMenu: Window
    gwLWindow = gtk_label_new(cpLWindowTxt);
    gtk_label_set_justify(GTK_LABEL(gwLWindow), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLWindow,
        1, 2, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLWindow);
    gwOMWindow = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwOMWindow,
        1, 2, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMWindow);
    GtkUtils.BuildOptionMenu(gwOMWindow, &gwMWindow, gwaMIWindow,
        cpaLWindowMenu, LGUI_WINDOW_ITEMS);

    // Label & Entry: Window parameter
    gwLWinParam = gtk_label_new(cpLWinParamTxt);
    gtk_label_set_justify(GTK_LABEL(gwLWinParam), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLWinParam,
        2, 3, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLWinParam);
    gwEWinParam = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEWinParam,
        2, 3, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwEWinParam);
    gtk_widget_set_usize(gwEWinParam, LGUI_ENTRY_WIDTH,
        gwEWinParam->requisition.height);

    // Label & OptionMenu: Window length
    gwLWinLength = gtk_label_new(cpLWinLengthTxt);
    gtk_label_set_justify(GTK_LABEL(gwLWinLength), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLWinLength,
        3, 4, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLWinLength);
    gwOMWinLength = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwOMWinLength,
        3, 4, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMWinLength);
    GtkUtils.BuildOptionMenu(gwOMWinLength, &gwMWinLength, gwaMIWinLength,
        cpaLWinLengthMenu, LGUI_WIN_LENGTH_ITEMS);
    gtk_option_menu_set_history(GTK_OPTION_MENU(gwOMWinLength),
        ((guint) (log(sLofarRq.lWinLength) / log(2.0) + 0.5) - 10));

    // Label & Entry: Lower frequency
    gwLLowerFreq = gtk_label_new(cpLLowerFreqTxt);
    gtk_label_set_justify(GTK_LABEL(gwLLowerFreq), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLLowerFreq,
        4, 5, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLLowerFreq);
    gwELowerFreq = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwELowerFreq,
        4, 5, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwELowerFreq);
    gtk_widget_set_usize(gwELowerFreq, LGUI_ENTRY_WIDTH,
        gwELowerFreq->requisition.height);
    g_snprintf(cpConvBuf, LGUI_CONV_BUF_SIZE, "%g", sLofarRq.fLowFreq);
    gtk_entry_set_text(GTK_ENTRY(gwELowerFreq), cpConvBuf);

    // Label & Entry: Higher frequency
    gwLHigherFreq = gtk_label_new(cpLHigherFreqTxt);
    gtk_label_set_justify(GTK_LABEL(gwLHigherFreq), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLHigherFreq,
        5, 6, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLHigherFreq);
    gwEHigherFreq = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEHigherFreq,
        5, 6, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwEHigherFreq);
    gtk_widget_set_usize(gwEHigherFreq, LGUI_ENTRY_WIDTH,
        gwEHigherFreq->requisition.height);
    g_snprintf(cpConvBuf, LGUI_CONV_BUF_SIZE, "%g", sLofarRq.fHighFreq);
    gtk_entry_set_text(GTK_ENTRY(gwEHigherFreq), cpConvBuf);

    // Label & Entry: Overlap
    gwLOverlap = gtk_label_new(cpLOverlapTxt);
    gtk_label_set_justify(GTK_LABEL(gwLOverlap), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLOverlap,
        6, 7, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLOverlap);
    gwEOverlap = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEOverlap,
        6, 7, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwEOverlap);
    gtk_widget_set_usize(gwEOverlap, LGUI_ENTRY_WIDTH,
        gwEOverlap->requisition.height);

    return true;
}


bool clGUILofar::BuildTable3 ()
{
    char cpConvBuf[LGUI_CONV_BUF_SIZE];

    // rows, columns, homogenous
    gwTable3 = gtk_table_new(2, 10, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable3, FALSE, FALSE, 0);
    gtk_widget_show(gwTable3);
    
    // Label & OptionMenu: Remove noise
    gwLRemoveNoise = gtk_label_new(cpLRemoveNoiseTxt);
    gtk_label_set_justify(GTK_LABEL(gwLRemoveNoise), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLRemoveNoise,
        0, 1, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLRemoveNoise);
    gwOMRemoveNoise = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwOMRemoveNoise,
        0, 1, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMRemoveNoise);
    GtkUtils.BuildOptionMenu(gwOMRemoveNoise, &gwMRemoveNoise, 
        gwaMIRemoveNoise, cpaLRemoveNoiseMenu, LGUI_REMOVE_NOISE_ITEMS);
    gtk_option_menu_set_history(GTK_OPTION_MENU(gwOMRemoveNoise),
        sLofarRq.iRemoveNoise);

    // Label & Entry: Alpha
    gwLAlpha = gtk_label_new(cpLAlphaTxt);
    gtk_label_set_justify(GTK_LABEL(gwLAlpha), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLAlpha,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLAlpha);
    gwEAlpha = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEAlpha,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwEAlpha);
    gtk_widget_set_usize(gwEAlpha, LGUI_ENTRY_WIDTH,
        gwEAlpha->requisition.height);
    g_snprintf(cpConvBuf, LGUI_CONV_BUF_SIZE, "%g", sLofarRq.fAlpha);
    gtk_entry_set_text(GTK_ENTRY(gwEAlpha), cpConvBuf);

    // Label & Entry: Mean length
    gwLMeanLength = gtk_label_new(cpLMeanLengthTxt);
    gtk_label_set_justify(GTK_LABEL(gwLMeanLength), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLMeanLength,
        2, 3, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLMeanLength);
    gwEMeanLength = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEMeanLength,
        2, 3, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwEMeanLength);
    gtk_widget_set_usize(gwEMeanLength, LGUI_ENTRY_WIDTH,
        gwEMeanLength->requisition.height);
    g_snprintf(cpConvBuf, LGUI_CONV_BUF_SIZE, "%li", sLofarRq.lMeanLength);
    gtk_entry_set_text(GTK_ENTRY(gwEMeanLength), cpConvBuf);

    // Label & Entry: Gap length
    gwLGapLength = gtk_label_new(cpLGapLengthTxt);
    gtk_label_set_justify(GTK_LABEL(gwLGapLength), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLGapLength,
        3, 4, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLGapLength);
    gwEGapLength = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEGapLength,
        3, 4, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwEGapLength);
    gtk_widget_set_usize(gwEGapLength, LGUI_ENTRY_WIDTH,
        gwEGapLength->requisition.height);
    g_snprintf(cpConvBuf, LGUI_CONV_BUF_SIZE, "%li", sLofarRq.lGapLength);
    gtk_entry_set_text(GTK_ENTRY(gwEGapLength), cpConvBuf);

    // Label & Entry: Average count
    gwLAverageCount = gtk_label_new(cpLAverageCountTxt);
    gtk_label_set_justify(GTK_LABEL(gwLAverageCount), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLAverageCount,
        4, 5, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLAverageCount);
    gwEAverageCount = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEAverageCount,
        4, 5, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwEAverageCount);
    gtk_widget_set_usize(gwEAverageCount, LGUI_ENTRY_WIDTH,
        gwEGapLength->requisition.height);
    g_snprintf(cpConvBuf, LGUI_CONV_BUF_SIZE, "%li", sLofarRq.lAvgCount);
    gtk_entry_set_text(GTK_ENTRY(gwEAverageCount), cpConvBuf);

    // Label & OptionMenu: Clip
    gwLClip = gtk_label_new(cpLClipTxt);
    gtk_label_set_justify(GTK_LABEL(gwLClip), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLClip,
        5, 6, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLClip);
    gwOMClip = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwOMClip,
        5, 6, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMClip);
    GtkUtils.BuildOptionMenu(gwOMClip, &gwMClip, gwaMIClip, cpaLClipMenu,
        LGUI_CLIP_ITEMS);
    gtk_option_menu_set_history(GTK_OPTION_MENU(gwOMClip), sLofarRq.iClip);

    // CheckButton: Linear
    gwCBLinear = gtk_check_button_new_with_label(cpCBLinearTxt);
    gtk_table_attach(GTK_TABLE(gwTable3), gwCBLinear,
        6, 7, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gwCBLinear), TRUE);
    gtk_widget_show(gwCBLinear);

    // CheckButton: DEMON
    gwCBDemon = gtk_check_button_new_with_label(cpCBDemonTxt);
    gtk_table_attach(GTK_TABLE(gwTable3), gwCBDemon,
        6, 7, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    //gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gwCBDemon), TRUE);
    gtk_widget_show(gwCBDemon);

    // Label & OptionMenu: Palette
    gwLPalette = gtk_label_new(cpLPaletteTxt);
    gtk_label_set_justify(GTK_LABEL(gwLPalette), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLPalette,
        7, 8, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLPalette);
    gwOMPalette = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwOMPalette,
        7, 8, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMPalette);
    GtkUtils.BuildOptionMenu(gwOMPalette, &gwMPalette, gwaMIPalette,
        cpaLPaletteMenu, LGUI_PALETTE_ITEMS);
    gtk_option_menu_set_history(GTK_OPTION_MENU(gwOMPalette), 
        (guint) iPalette);

    // CheckButton: Average
    gwCBAverage = gtk_check_button_new_with_label(cpCBAverageTxt);
    gtk_table_attach(GTK_TABLE(gwTable3), gwCBAverage,
        8, 9, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    //gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gwCBAverage), TRUE);
    gtk_widget_show(gwCBAverage);

    // CheckButton: Saving
    gwCBSaving = gtk_check_button_new_with_label(cpCBSavingTxt);
    gtk_table_attach(GTK_TABLE(gwTable3), gwCBSaving,
        9, 10, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwCBSaving);

    // Button: Save
    gwBSave = gtk_button_new_with_label(cpBSaveTxt);
    gtk_table_attach(GTK_TABLE(gwTable3), gwBSave,
        9, 10, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwBSave);

    // FileSelection: Save
    gwFSSave = gtk_file_selection_new(cpFSSaveTxt);

    return true;
}


bool clGUILofar::BuildTable4 ()
{
    // rows, columns, homogenous
    gwTable4 = gtk_table_new(2, 1, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable4, FALSE, FALSE, 0);
    gtk_widget_show(gwTable4);

    // Label & Horizontal Scale: Clip
    gwLClipValue = gtk_label_new(cpLClipValueTxt);
    gtk_table_attach(GTK_TABLE(gwTable4), gwLClipValue,
        0, 1, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwLClipValue);
    // there is a stupid bug in scale/adjustment which leaves maximum value
    // one step away from specified maximum
    goAClipValue = gtk_adjustment_new(1, 0, 1.01, 0.01, 0.1, 0.01);
    gwHSClipValue = gtk_hscale_new(GTK_ADJUSTMENT(goAClipValue));
    gtk_scale_set_digits(GTK_SCALE(gwHSClipValue), 2);
    gtk_table_attach(GTK_TABLE(gwTable4), gwHSClipValue,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        LGUI_PADDING / 2, 0);
    gtk_widget_show(gwHSClipValue);

    return true;
}


bool clGUILofar::BuildTableLofar ()
{
    // rows, columns, homogenous
    gwTableLofar = gtk_table_new(4, 1, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTableLofar, TRUE, TRUE, 0);
    gtk_widget_show(gwTableLofar);

    // Label: Top time
    gwLTopTime = gtk_label_new("00:00:00");
    gtk_label_set_justify(GTK_LABEL(gwLTopTime), GTK_JUSTIFY_RIGHT);
    gtk_table_attach(GTK_TABLE(gwTableLofar), gwLTopTime,
        0, 1, 0, 1,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) 0,
        0, 0);
    gtk_widget_show(gwLTopTime);

    // Label: Bottom time
    gwLBottomTime = gtk_label_new("00:00:00");
    gtk_label_set_justify(GTK_LABEL(gwLBottomTime), GTK_JUSTIFY_RIGHT);
    gtk_table_attach(GTK_TABLE(gwTableLofar), gwLBottomTime,
        0, 1, 2, 3,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) 0,
        0, 0);
    gtk_widget_show(gwLBottomTime);

    // ScrolledWindow: Lofar
    gwSWLofar = gtk_scrolled_window_new(NULL, NULL);
    if (iFit)
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gwSWLofar),
            GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    else
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gwSWLofar),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_table_attach(GTK_TABLE(gwTableLofar), gwSWLofar,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_SHRINK|GTK_EXPAND),
        (GtkAttachOptions) (GTK_FILL|GTK_SHRINK|GTK_EXPAND),
        0, 0);
    gtk_widget_show(gwSWLofar);

    // rows, columns, homogenous
    gwTableLofar2 = gtk_table_new(3, 2, FALSE);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(gwSWLofar),
        gwTableLofar2);
    gtk_widget_show(gwTableLofar2);

    // HorizontalRuler: Frequency
    gwHRFrequency = gtk_hruler_new();
    gtk_ruler_set_metric(GTK_RULER(gwHRFrequency), GTK_PIXELS);
    // ruler, lower, upper, position, max size
    gtk_ruler_set_range(GTK_RULER(gwHRFrequency), 
        0.0, (gfloat) sLofarRq.lWinLength, 0.0, (gfloat) sLofarRq.lWinLength);
    gtk_table_attach(GTK_TABLE(gwTableLofar2), gwHRFrequency,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) 0,
        0, 0);
    gtk_widget_show(gwHRFrequency);

    // DrawingArea: Line spectrum
    gwDALine = gtk_drawing_area_new();
    // drawing area, width, height
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDALine), 1000, 
        LGUI_LINESPECT_HEIGHT);
    gtk_table_attach(GTK_TABLE(gwTableLofar2), gwDALine,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (GTK_FILL),
        0, 0);
    gtk_widget_show(gwDALine);

    // DrawingArea: Cursor
    gwDACursor = gtk_drawing_area_new();
    // drawing area, width, height
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDACursor), 1000,
        LGUI_CURSOR_HEIGHT);
    gtk_table_attach(GTK_TABLE(gwTableLofar2), gwDACursor,
        1, 2, 2, 3,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (GTK_FILL),
        0, 0);
    gtk_widget_show(gwDACursor);

    // VerticalRuler: Time
    gwVRTime = gtk_vruler_new();
    gtk_ruler_set_metric(GTK_RULER(gwVRTime), GTK_PIXELS);
    // ruler, lower, upper, position, max size
    gtk_ruler_set_range(GTK_RULER(gwVRTime), 0.0, -1.0, 0.0, 0.0);
    gtk_table_attach(GTK_TABLE(gwTableLofar2), gwVRTime,
        0, 1, 3, 4,
        (GtkAttachOptions) 0,
        (GtkAttachOptions) (GTK_FILL|GTK_SHRINK|GTK_EXPAND),
        0, 0);
    gtk_widget_show(gwVRTime);

    // DrawingArea: Lofar
    gwDALofar = gtk_drawing_area_new();
    // drawing area, width, height
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDALofar), 1000, 100);
    gtk_table_attach(GTK_TABLE(gwTableLofar2), gwDALofar,
        1, 2, 3, 4,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (GTK_FILL),
        0, 0);
    gtk_widget_show(gwDALofar);

    return true;
}


bool clGUILofar::BuildDrawingPrims ()
{
    int iPalType;

    // Create graphics contexts
    ggcLofarBG = gdk_gc_new(gwDALofar->window);
    gdk_rgb_gc_set_foreground(ggcLofarBG, LGUI_LOFAR_BG);
    gdk_rgb_gc_set_background(ggcLofarBG, LGUI_LOFAR_BG);
    gdk_gc_set_function(ggcLofarBG, GDK_COPY);
    gdk_gc_set_fill(ggcLofarBG, GDK_SOLID);

    ggcLofarFG = gdk_gc_new(gwDALofar->window);
    gdk_rgb_gc_set_foreground(ggcLofarFG, LGUI_LOFAR_FG);
    gdk_rgb_gc_set_background(ggcLofarFG, LGUI_LOFAR_BG);
    gdk_gc_set_function(ggcLofarFG, GDK_COPY);
    gdk_gc_set_fill(ggcLofarFG, GDK_SOLID);

    ggcLineBG = gdk_gc_new(gwDALine->window);
    gdk_rgb_gc_set_foreground(ggcLineBG, LGUI_LINE_BG);
    gdk_rgb_gc_set_background(ggcLineBG, LGUI_LINE_BG);
    gdk_gc_set_function(ggcLineBG, GDK_COPY);
    gdk_gc_set_fill(ggcLineBG, GDK_SOLID);

    ggcLineFG = gdk_gc_new(gwDALine->window);
    gdk_rgb_gc_set_foreground(ggcLineFG, LGUI_LINE_FG);
    gdk_rgb_gc_set_background(ggcLineFG, LGUI_LINE_BG);
    gdk_gc_set_function(ggcLineFG, GDK_COPY);
    gdk_gc_set_fill(ggcLineFG, GDK_SOLID);

    ggcCursorBG = gdk_gc_new(gwDACursor->window);
    gdk_rgb_gc_set_foreground(ggcCursorBG, LGUI_CURSOR_BG);
    gdk_rgb_gc_set_background(ggcCursorBG, LGUI_CURSOR_BG);
    gdk_gc_set_function(ggcCursorBG, GDK_COPY);
    gdk_gc_set_fill(ggcCursorBG, GDK_SOLID);

    ggcCursorFG = gdk_gc_new(gwDACursor->window);
    gdk_rgb_gc_set_foreground(ggcCursorFG, LGUI_CURSOR_FG);
    gdk_rgb_gc_set_background(ggcCursorFG, LGUI_CURSOR_BG);
    gdk_gc_set_function(ggcCursorFG, GDK_COPY);
    gdk_gc_set_fill(ggcCursorFG, GDK_SOLID);

    // Create cursors
    gcCrossHair = gdk_cursor_new(GDK_CROSSHAIR);

    // Create specified palette for direction display
    Cfg.GetInt("Palette", &iPalType);
    SetPalette(iPalType);

    return true;
}


void clGUILofar::FreeDrawingPrims ()
{
    gdk_cursor_destroy(gcCrossHair);
    gdk_gc_destroy(ggcLofarBG);
    gdk_gc_destroy(ggcLofarFG);
    gdk_gc_destroy(ggcLineBG);
    gdk_gc_destroy(ggcLineFG);
    gdk_gc_destroy(ggcCursorBG);
    gdk_gc_destroy(ggcCursorFG);
}


bool clGUILofar::ConnectSignals ()
{
    int iWidgetCntr;

    gtk_signal_connect(GTK_OBJECT(gwWindow), "delete_event",
        GTK_SIGNAL_FUNC(WrapOnDelete), NULL);

    gtk_signal_connect(GTK_OBJECT(gwCBHide), "toggled",
        GTK_SIGNAL_FUNC(WrapOnHideToggled), NULL);

    //gtk_widget_add_events(gwDALofar, GDK_ALL_EVENTS_MASK);
    gtk_widget_add_events(gwDALofar, 
        (GDK_POINTER_MOTION_MASK|GDK_BUTTON_MOTION_MASK|
        GDK_BUTTON1_MOTION_MASK|GDK_BUTTON2_MOTION_MASK|GDK_BUTTON3_MOTION_MASK|
        GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|
        GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK));
    GtkUtils.ConnectMotionEvent(gwHRFrequency, gwDALofar);
    GtkUtils.ConnectMotionEvent(gwVRTime, gwDALofar);
    gtk_signal_connect(GTK_OBJECT(gwDALofar), "expose_event",
        GTK_SIGNAL_FUNC(WrapOnExposeLofar), NULL);
    gtk_signal_connect(GTK_OBJECT(gwDALofar), "configure_event",
        GTK_SIGNAL_FUNC(WrapOnConfigureLofar), NULL);
    gtk_signal_connect(GTK_OBJECT(gwDALofar), "motion_notify_event",
        GTK_SIGNAL_FUNC(WrapOnMotionLofar), NULL);

    gtk_widget_add_events(gwDALine, GDK_POINTER_MOTION_MASK);
    GtkUtils.ConnectMotionEvent(gwHRFrequency, gwDALine);
    gtk_signal_connect(GTK_OBJECT(gwDALine), "expose_event",
        GTK_SIGNAL_FUNC(WrapOnExposeLine), NULL);

    gtk_signal_connect(GTK_OBJECT(gwDACursor), "expose_event",
        GTK_SIGNAL_FUNC(WrapOnExposeCursor), NULL);

    gtk_signal_connect(GTK_OBJECT(gwBConnect), "clicked",
        GTK_SIGNAL_FUNC(WrapOnConnectClick), NULL);
    gtk_signal_connect(GTK_OBJECT(gwBDisconnect), "clicked",
        GTK_SIGNAL_FUNC(WrapOnConnectClick), NULL);
    gtk_signal_connect(GTK_OBJECT(gwCBFreeze), "toggled",
        GTK_SIGNAL_FUNC(WrapOnFreezeToggled), NULL);

    for (iWidgetCntr = 0; iWidgetCntr < LGUI_PALETTE_ITEMS; iWidgetCntr++)
    {
        gtk_signal_connect(GTK_OBJECT(gwaMIPalette[iWidgetCntr]), "activate",
            GTK_SIGNAL_FUNC(WrapOnPaletteActivate), (gpointer) iWidgetCntr);
    }

    gtk_signal_connect(GTK_OBJECT(gwCBAverage), "toggled",
        GTK_SIGNAL_FUNC(WrapOnAverageToggled), NULL);

    gtk_signal_connect(GTK_OBJECT(gwCBSaving), "toggled",
        GTK_SIGNAL_FUNC(WrapOnSaveClicks), GINT_TO_POINTER(3));
    gtk_signal_connect(GTK_OBJECT(gwBSave), "clicked",
        GTK_SIGNAL_FUNC(WrapOnSaveClicks), GINT_TO_POINTER(2));
    gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(gwFSSave)->ok_button),
        "clicked", GTK_SIGNAL_FUNC(WrapOnSaveClicks), GINT_TO_POINTER(1));
    gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(gwFSSave)->cancel_button),
        "clicked", GTK_SIGNAL_FUNC(WrapOnSaveClicks), GINT_TO_POINTER(0));

    gtk_signal_connect(GTK_OBJECT(goAClipValue), "value-changed",
        GTK_SIGNAL_FUNC(WrapOnClipValueChanged), NULL);

    return true;
}


bool clGUILofar::ParseServerStr (char *cpHostRes, int *ipPortRes,
    const char *cpSourceStr)
{
    char cpTempStr[LGUI_SERVER_MAXLEN + 1];
    char *cpTempHost;
    char *cpTempPort;

    strncpy(cpTempStr, cpSourceStr, LGUI_SERVER_MAXLEN);
    cpTempHost = strtok(cpTempStr, ": \t\n");
    if (cpTempHost != NULL)
    {
        strcpy(cpHostRes, cpTempHost);
        cpTempPort = strtok(NULL, " \t\n");
        if (cpTempPort != NULL)
        {
            *ipPortRes = atoi(cpTempPort);
            if (*ipPortRes > 0) return true;
        }
    }
    return false;
}


bool clGUILofar::InitConnection (const char *cpServerHost, int iServerPort)
{
    int iSockH;
    char cpReqProcName[GLOBAL_HEADER_LEN];

    iSockH = SClient.Connect(cpServerHost, NULL, iServerPort);
    if (iSockH < 0) 
    {
        g_print("Failed to connect host!\n");
        return false;
    }
    g_print("Connection established - sending process request...\n");
    bConnected = true;
    SOp.SetHandle(iSockH);
    strcpy(cpReqProcName, LGUI_REQ_PROC);
    if (SOp.WriteN(cpReqProcName, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
    {
        g_print("Failed to send process request!\n");
        return false;
    }
    g_print("Sending settings...\n");
    if (!SendSettings())
    {
        g_print("Failed to send settings!\n");
        return false;
    }
    lSpectSize = sLofarRq.lWinLength / 2L + 1L;
    SpectData.Size(sizeof(GDT) * lSpectSize);
    AvgSpectData.Size(sizeof(GDT) * lSpectSize);
    bConfigured = false;
    iClips = 0;
    giGdkTag = gdk_input_add(iSockH, GDK_INPUT_READ, WrapOnGdkInput, NULL);
    return true;
}


bool clGUILofar::SendSettings ()
{
    float fDirection;
    char cpMsgBuf[GLOBAL_HEADER_LEN];

    if (iBeamCount)
    {
        fDirection = gtk_spin_button_get_value_as_float(
            GTK_SPIN_BUTTON(gwSBChannel)) + 90.0f;
        sLofarRq.iChannel = (int) 
            (fDirection / (180.0f / (float) (iBeamCount - 1)));
        g_print("Channel: %i\n", sLofarRq.iChannel);
    }
    else
    {
        sLofarRq.iChannel = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(gwSBChannel)) - 1;
    }
    sLofarRq.iType = GtkUtils.OptionMenuGetActive(gwOMType, gwaMIType, 
        LGUI_TYPE_ITEMS);
    sLofarRq.iWindow = GtkUtils.OptionMenuGetActive(gwOMWindow, gwaMIWindow,
        LGUI_WINDOW_ITEMS);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEWinParam)), "%g", 
        &sLofarRq.fWinParameter);
    sLofarRq.lWinLength = (long) (pow(2.0, 
        10 + GtkUtils.OptionMenuGetActive(gwOMWinLength, gwaMIWinLength,
        LGUI_WIN_LENGTH_ITEMS)) + 0.5);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwELowerFreq)), "%g",
        &sLofarRq.fLowFreq);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEHigherFreq)), "%g",
        &sLofarRq.fHighFreq);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEOverlap)), "%i",
        &sLofarRq.iOverlap);
    sLofarRq.bLinear = (gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(gwCBLinear))) ? true : false;
    sLofarRq.iRemoveNoise = GtkUtils.OptionMenuGetActive(gwOMRemoveNoise,
        gwaMIRemoveNoise, LGUI_REMOVE_NOISE_ITEMS);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEAlpha)), "%g",
        &sLofarRq.fAlpha);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEMeanLength)), "%li",
        &sLofarRq.lMeanLength);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEGapLength)), "%li",
        &sLofarRq.lGapLength);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEAverageCount)), "%li",
        &sLofarRq.lAvgCount);
    if (sLofarRq.lAvgCount <= 0) sLofarRq.lAvgCount = 1;
    sLofarRq.bDemon = (gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(gwCBDemon))) ? true : false;
    sLofarRq.iClip = GtkUtils.OptionMenuGetActive(gwOMClip, gwaMIClip,
        LGUI_CLIP_ITEMS);

    LofarMsg.SetRequest(cpMsgBuf, &sLofarRq);
    if (SOp.WriteN(cpMsgBuf, GLOBAL_HEADER_LEN) != GLOBAL_HEADER_LEN)
        return false;
    
    return true;
}


void clGUILofar::PrintStatus ()
{
    time_t ttCursorTime;
    float fFreqRes;
    float fFreq;
    float fTime;
    float fCursorC;
    float fCursorD;
    char cpTimeBuf[LGUI_CONV_BUF_SIZE];
    char cpConvBuf[LGUI_CONV_BUF_SIZE];

    if (iFit)
    {
        if (iLofarWidth != 0)
        {
            if (!sLofarRq.bDemon)
            {
                fFreqRes = (sLofarResHdr.fHighFreq - sLofarResHdr.fLowFreq) /
                    ((float) iLofarWidth);
            }
            else
            {
                fFreqRes = sLofarResHdr.fDemonBand / 
                    (float) iLofarWidth;
            }
        }
        else fFreqRes = 0.0f;
    }
    else
    {
        if (sLofarResHdr.lSpectLength != 0)
        {
            if (!sLofarRq.bDemon)
            {
                fFreqRes = (sLofarResHdr.fHighFreq - sLofarResHdr.fLowFreq) /
                    ((float) sLofarResHdr.lSpectLength);
            }
            else
            {
                fFreqRes = sLofarResHdr.fDemonBand / 
                    (float) sLofarResHdr.lSpectLength;
            }
        }
        else fFreqRes = 0.0f;
    }
    fFreq = ((float) iCursorX) * fFreqRes;
    fTime = sLofarResHdr.fLineTime * (float) iCursorY;
    fCursorC = ((float) sLCursor.iPosition) * fFreqRes;
    fCursorD = ((float) sLCursor.iDistance) * fFreqRes;
    ttCursorTime = (time_t) 
        ((float) sLofarResHdr.sTimeStamp.tv_sec - fTime + 0.5f);
    strftime(cpTimeBuf, LGUI_CONV_BUF_SIZE, "%H:%M:%S", 
        localtime(&ttCursorTime));
    g_snprintf(cpConvBuf, LGUI_CONV_BUF_SIZE, 
        "%.2f Hz  %.2f s  (%s)  /  %.2f dB  %i clips, c = %.2f Hz  d = %.2f Hz",
        fFreq, fTime, cpTimeBuf, sLofarResHdr.fPeakLevel, iClips,
        fCursorC, fCursorD);
    gtk_statusbar_pop(GTK_STATUSBAR(gwStatusBar), guSbCtxt);
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guSbCtxt, cpConvBuf);
}


void clGUILofar::SetPalette (int iPalType)
{
    switch (iPalType)
    {
        case LGUI_PAL_BW:
            FBLofar.PalGenBW();
            break;
        case LGUI_PAL_HSV:
            FBLofar.PalGenHSV();
            break;
        case LGUI_PAL_LIGHT:
            FBLofar.PalGenLight();
            break;
        case LGUI_PAL_TEMP:
            FBLofar.PalGenTemp();
            break;
        case LGUI_PAL_DIR:
            FBLofar.PalGenDir();
            break;
        case LGUI_PAL_GREEN:
            FBLofar.PalGenGreen();
            break;
        case LGUI_PAL_GREEN2:
            FBLofar.PalGenGreen2();
            break;
        case LGUI_PAL_GREEN3:
            FBLofar.PalGenGreen3();
            break;
        case LGUI_PAL_GREEN4:
            FBLofar.PalGenGreen4();
            break;
        case LGUI_PAL_PUREGREEN:
            FBLofar.PalGenPureGreen();
            break;
        case LGUI_PAL_WB:
            FBLofar.PalGenWB();
            break;
        default:
            FBLofar.PalGenBW();
    }
}


void clGUILofar::Configure ()
{
    char cpConvBuf[LGUI_CONV_BUF_SIZE];
    
    lSpectSize = sLofarResHdr.lSpectLength;
    if (iFit)
        iLofarWidth = 
            gwSWLofar->allocation.width - gwVRTime->allocation.width - 2;
    else
        iLofarWidth = (int) lSpectSize;
    FBLofar.SetSize(iLofarWidth, iLofarHeight);
    //FBLofar.Clear();
    // Without following, window size is reset at gtk_drawing_area_size!
    gtk_widget_set_usize(gwWindow, gwWindow->allocation.width,
        gwWindow->allocation.height);
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDALofar), iLofarWidth, 
        iLofarHeight);
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDALine), iLofarWidth,
        gwDALine->requisition.height);
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDACursor), iLofarWidth,
        gwDACursor->requisition.height);
    if (!sLofarRq.bDemon)
    {
        gtk_ruler_set_range(GTK_RULER(gwHRFrequency), 
            sLofarResHdr.fLowFreq, sLofarResHdr.fHighFreq, 
            sLofarResHdr.fLowFreq, sLofarResHdr.fHighFreq);
    }
    else
    {
        gtk_ruler_set_range(GTK_RULER(gwHRFrequency),
            0.0, sLofarResHdr.fDemonBand,
            0.0, sLofarResHdr.fDemonBand);
    }
    gtk_ruler_set_range(GTK_RULER(gwVRTime), 
        0.0, iLofarHeight * sLofarResHdr.fLineTime / 60.0f, 
        0.0, 0.0);
    g_snprintf(cpConvBuf, LGUI_CONV_BUF_SIZE, "%g", sLofarResHdr.fLowFreq);
    gtk_entry_set_text(GTK_ENTRY(gwELowerFreq), cpConvBuf);
    g_snprintf(cpConvBuf, LGUI_CONV_BUF_SIZE, "%g", 
        sLofarResHdr.fHighFreq);
    gtk_entry_set_text(GTK_ENTRY(gwEHigherFreq), cpConvBuf);
    bConfigured = true;
}


void clGUILofar::ConfigureHeight ()
{
    iLofarHeight = gwDALofar->allocation.height;
    FBLofar.SetSize(iLofarWidth, iLofarHeight);
    gtk_ruler_set_range(GTK_RULER(gwVRTime),
        0.0, iLofarHeight * sLofarResHdr.fLineTime / 60.0f,
        0.0, 0.0);
}


void clGUILofar::DrawCursor ()
{
    int iLineCntr;
    int iLineX;

    gdk_draw_rectangle(gwDACursor->window, ggcCursorBG, TRUE,
        0, 0, gwDACursor->allocation.width, gwDACursor->allocation.height);
    switch (sLCursor.iType)
    {
        case LGUI_CURSOR_11:
            for (iLineCntr = -5; iLineCntr <= 5; iLineCntr++)
            {
                iLineX = sLCursor.iPosition + iLineCntr * sLCursor.iDistance;
                gdk_draw_line(gwDACursor->window, ggcCursorFG,
                    iLineX, 0, iLineX, gwDACursor->allocation.height - 1);
            }
            break;
        case LGUI_CURSOR_INF:
            if (sLCursor.iDistance <= 0) break;
            iLineX = 0;
            do {
                gdk_draw_line(gwDACursor->window, ggcCursorFG,
                    iLineX, 0, iLineX, gwDACursor->allocation.height - 1);
                iLineX += sLCursor.iDistance;
            } while (iLineX < gwDACursor->allocation.width);
            break;
        default:
            g_warning("Uknown cursor type %i", sLCursor.iType);
    }
}


void clGUILofar::SaveInfo (const char *cpFileName, time_t ttStartTime)
{
    char *cpFullFileName;
    char cpTime[20];
    FILE *fileInfo;

    cpFullFileName = g_strdup_printf("%s.inf", cpFileName);
    fileInfo = fopen(cpFullFileName, FB_TIFF_MODE);
    g_free(cpFullFileName);
    if (fileInfo == NULL) return;
    strftime(cpTime, 20, "%Y/%m/%d %H:%M:%S", localtime(&ttStartTime));
    fprintf(fileInfo, "Time: %s\n", cpTime);
    fprintf(fileInfo, "LowFreq: %f\n", sLofarResHdr.fLowFreq);
    fprintf(fileInfo, "HighFreq: %f\n", sLofarResHdr.fHighFreq);
    if (sLofarRq.bDemon)
        fprintf(fileInfo, "DemonBand: %f\n", sLofarResHdr.fDemonBand);
    fprintf(fileInfo, "LineTime: %f\n", sLofarResHdr.fLineTime);
    fclose(fileInfo);
}


void clGUILofar::StartNewImgFile ()
{
    char *cpCompleteFileName;

    FBLofar.StopSaveToFile();
    cpCompleteFileName = g_strdup_printf("%s%02i.tif", 
        strImgFileName.c_str(), iImgCount);
    if (!FBLofar.StartSaveToFile(cpCompleteFileName, iCompressMode, 
        iJPEGQuality, "LOFAR/DEMON", FB_TIFF_CONT_VERTICAL))
    {
        g_warning("Saving to file failed!");
        gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(gwCBSaving));
        bSaving = false;
    }
    SaveInfo(cpCompleteFileName, time(NULL));
    g_free(cpCompleteFileName);
}


clGUILofar::clGUILofar (int *ipArgC, char ***cpapArgV)
{
    bRun = true;
    bConnected = false;
    bFreezed = false;
    bConfigured = false;
    bAveraged = false;
    bCursorDrag = false;
    fClip = 1;
    glServer = NULL;
    memset(&sLCursor, 0x00, sizeof(stLofarCursor));
    g_print("%s GUI v%i.%i.%i\n", cpWindowTxt,
        LGUI_VER_MAJ, LGUI_VER_MIN, LGUI_VER_PL);
    g_print("Copyright (C) 2000-2002 Jussi Laako\n\n");
    g_print("Gtk+ version %i.%i.%i\n", gtk_major_version, gtk_minor_version,
        gtk_micro_version);
    g_print("Locale set to %s\n", gtk_set_locale());
    gtk_init(ipArgC, cpapArgV);
    gdk_rgb_init();
    gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
    gtk_widget_set_default_visual(gdk_rgb_get_visual());
    Cfg.SetFileName(LGUI_CFGFILE);
    if (!Cfg.GetInt("WindowLength", &sLofarRq.lWinLength))
        sLofarRq.lWinLength = LGUI_DEF_WIN_LENGTH;
    if (!Cfg.GetFlt("LowFrequency", &sLofarRq.fLowFreq))
        sLofarRq.fLowFreq = LGUI_DEF_LOW_FREQ;
    if (!Cfg.GetFlt("HighFrequency", &sLofarRq.fHighFreq))
        sLofarRq.fHighFreq = LGUI_DEF_HIGH_FREQ;
    if (!Cfg.GetInt("RemoveNoise", &sLofarRq.iRemoveNoise))
        sLofarRq.iRemoveNoise = LGUI_DEF_REMOVE_NOISE;
    if (!Cfg.GetFlt("Alpha", &sLofarRq.fAlpha))
        sLofarRq.fAlpha = LGUI_DEF_ALPHA;
    if (!Cfg.GetInt("MeanLength", &sLofarRq.lMeanLength))
        sLofarRq.lMeanLength = LGUI_DEF_MEAN_LENGTH;
    if (!Cfg.GetInt("GapLength", &sLofarRq.lGapLength))
        sLofarRq.lGapLength = LGUI_DEF_GAP_LENGTH;
    if (!Cfg.GetInt("AverageCount", &sLofarRq.lAvgCount))
        sLofarRq.lAvgCount = 1;
    if (!Cfg.GetInt("Clip", &sLofarRq.iClip))
        sLofarRq.iClip = MSG_LOFAR_CLIP_NONE;
    if (!Cfg.GetInt("Fit", &iFit))
        iFit = 0;
    if (!Cfg.GetInt("Palette", &iPalette))
        iPalette = 0;
    if (!iFit)
        iLofarWidth = sLofarRq.lWinLength;
    if (!Cfg.GetInt("Height", &iLofarHeight))
        iLofarHeight = LGUI_DEF_HEIGHT;
    if (!Cfg.GetInt("TIFFCompression", &iTIFFCompression))
        iTIFFCompression = 0;
    if (!Cfg.GetInt("JPEGQuality", &iJPEGQuality))
        iJPEGQuality = 100;
    if (!Cfg.GetInt("ContSaveScans", &iContSaveScans))
        iContSaveScans = 1024;
    if (!Cfg.GetInt("BeamCount", &iBeamCount))
        iBeamCount = 0;
    switch (iTIFFCompression)
    {
        case 0:
            iCompressMode = FB_TIFF_COMPRESS_NONE;
            break;
        case 1:
            iCompressMode = FB_TIFF_COMPRESS_RLE;
            break;
        case 2:
            iCompressMode = FB_TIFF_COMPRESS_LZW;
            break;
        case 3:
            iCompressMode = FB_TIFF_COMPRESS_JPEG;
            break;
        case 4:
            iCompressMode = FB_TIFF_COMPRESS_DEFLATE;
            break;
        default:
            iCompressMode = FB_TIFF_COMPRESS_NONE;
    }
}


clGUILofar::~clGUILofar ()
{
}


int clGUILofar::Exec ()
{
    FBLofar.SetSize(iLofarWidth, iLofarHeight);
    FBLofar.Clear();
    if (!Build())
    {
        g_print("User interface creation failed\n");
        return 1;
    }
    if (!ConnectSignals())
    {
        g_print("User interface signal connection failed\n");
        return 1;
    }
    gtk_main();
    FreeDrawingPrims();
    return 0;
}


gint clGUILofar::OnDelete (GtkWidget *gwSender, GdkEventAny *geaEvent)
{
    bRun = false;
    if (bSaving)
    {
        FBLofar.StopSaveToFile();
    }
    gtk_main_quit();
    return 0;
}


void clGUILofar::OnHideToggled (GtkToggleButton *gtbSender, gpointer gpData)
{
    if (gtk_toggle_button_get_active(gtbSender))
    {
        gtk_widget_hide(gwTable1);
        gtk_widget_hide(gwTable2);
        gtk_widget_hide(gwTable3);
        gtk_widget_hide(gwTable4);
    }
    else
    {
        gtk_widget_show(gwTable1);
        gtk_widget_show(gwTable2);
        gtk_widget_show(gwTable3);
        gtk_widget_show(gwTable4);
    }
}


gint clGUILofar::OnConnectClick (GtkWidget *gwSender, gpointer gpData)
{
    char cpHost[LGUI_SERVER_MAXLEN + 1];
    int iPort;

    if (bConnected)
    {
        gdk_input_remove(giGdkTag);
        SOp.Shutdown(2);
        SOp.Close();
        bConnected = false;
    }
    if (gwSender == gwBConnect)
    {
        if (ParseServerStr(cpHost, &iPort,
            gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(gwCServer)->entry))))
        {
            g_print("Connecting to host %s port %i...\n",
                cpHost, iPort);
            if (InitConnection(cpHost, iPort))
            {
                g_print("OK\n\n");
            }
            else
            {
                g_print("Failed\n\n");
            }
        }
        else
        {
            g_print("Incorrect server entry! Format: <host>:<port>\n");
        }
    }

    return 0;
}


void clGUILofar::OnFreezeToggled (GtkToggleButton *gtbSender, gpointer gpData)
{
    bFreezed = (gtk_toggle_button_get_active(gtbSender)) ? true : false;
}


gint clGUILofar::OnExposeLofar (GtkWidget *gwSender, GdkEventExpose *geeEvent,
    gpointer gpData)
{
    int iExposeX;
    int iExposeY;
    int iExposeWidth;
    int iExposeHeight;

    iExposeX = (geeEvent->area.x >= iLofarWidth) ? 0 : geeEvent->area.x;
    iExposeY = (geeEvent->area.y >= iLofarHeight) ? 0 : geeEvent->area.y;
    iExposeWidth = (geeEvent->area.width > (iLofarWidth - iExposeX)) ?
        (iLofarWidth - iExposeX) : geeEvent->area.width;
    iExposeHeight = (geeEvent->area.height > (iLofarHeight - iExposeY)) ?
        (iLofarHeight - iExposeY) : geeEvent->area.height;
    gdk_draw_rgb_32_image(gwDALofar->window, ggcLofarFG,
        iExposeX, iExposeY, iExposeWidth, iExposeHeight,
        GDK_RGB_DITHER_NONE, FBLofar.GetCurPtr(iExposeX, iExposeY),
        FBLofar.GetRowStride());
    return 0;
}


gint clGUILofar::OnConfigureLofar (GtkWidget *gwSender, 
    GdkEventConfigure *gecEvent, gpointer gpData)
{
    //g_print("configure: %ix%i\n", gecEvent->width, gecEvent->height);
    return 0;
}


gint clGUILofar::OnExposeLine (GtkWidget *gwSender, GdkEventExpose *geeEvent,
    gpointer gpData)
{
    int iExposeX;
    int iExposeY;
    int iExposeWidth;
    int iExposeHeight;
    int iWinHeight;
    int iLineCntr;
    int iLineY1;
    GDT *fpSpectData = SpectData;

    if (fpSpectData == NULL) return 0;
    iWinHeight = gwDALine->allocation.height;
    iExposeX = (geeEvent->area.x >= lSpectSize) ? 0 : geeEvent->area.x;
    iExposeY = (geeEvent->area.y >= iWinHeight - 1) ? 0 : geeEvent->area.y;
    iExposeWidth = (geeEvent->area.width > (lSpectSize - iExposeX)) ?
        (lSpectSize - iExposeX) : geeEvent->area.width;
    iExposeHeight = (geeEvent->area.height > (iWinHeight - iExposeY)) ?
        (iWinHeight - iExposeY) : geeEvent->area.height;
    for (iLineCntr = iExposeX; iLineCntr < iExposeWidth; iLineCntr++)
    {
        iLineY1 = (int) 
            ((iWinHeight - iWinHeight * fpSpectData[iLineCntr]) + 0.5);
        gdk_draw_line(gwDALine->window, ggcLineFG, 
            iLineCntr, iLineY1, 
            iLineCntr, iWinHeight - 1);
    }
    return 0;
}


gint clGUILofar::OnExposeCursor (GtkWidget *gwSender, GdkEventExpose *geeEvent,
    gpointer gpData)
{
    DrawCursor();
    return 0;
}


gint clGUILofar::OnMotionLofar (GtkWidget *gwSender, GdkEventMotion *gemEvent,
    gpointer gpData)
{
    iCursorX = (int) gemEvent->x;
    iCursorY = (int) gemEvent->y;
    /*g_print("->x = %g, ->y = %g, ->state = %u\n", 
        gemEvent->x,
        gemEvent->y,
        gemEvent->state);*/
    /*if (gemEvent->state & GDK_SHIFT_MASK)
        g_print("Shift\n");
    if (gemEvent->state & GDK_CONTROL_MASK)
        g_print("Ctrl\n");*/
    if (gemEvent->state & GDK_BUTTON1_MASK)
    {
        if (!bCursorDrag)
        {
            sLCursor.iType = LGUI_CURSOR_11;
            sLCursor.iPosition = (int) gemEvent->x;
            bCursorDrag = true;
        }
        sLCursor.iDistance = abs(((int) gemEvent->x) - sLCursor.iPosition);
        DrawCursor();
    }
    else if (gemEvent->state & GDK_BUTTON2_MASK)
    {
        if (!bCursorDrag)
        {
            sLCursor.iType = LGUI_CURSOR_INF;
            sLCursor.iPosition = (int) gemEvent->x;
            bCursorDrag = true;
        }
        sLCursor.iDistance = abs(((int) gemEvent->x) - sLCursor.iPosition);
        DrawCursor();
    }
    else if (gemEvent->state & GDK_BUTTON3_MASK)
    {
        sLCursor.iPosition = (int) gemEvent->x;
        DrawCursor();
    }
    else bCursorDrag = false;
    PrintStatus();
    return 0;
}


gint clGUILofar::OnPaletteActivate (GtkWidget *gwSender, gpointer gpData)
{
    iPalette = GtkUtils.OptionMenuGetActive(gwOMPalette, gwaMIPalette, 
        LGUI_PALETTE_ITEMS);
    SetPalette(iPalette);
    return 0;
}


void clGUILofar::OnAverageToggled (GtkToggleButton *gtbSender, gpointer gpData)
{
    bAveraged = (gtk_toggle_button_get_active(gtbSender)) ? true : false;
}


void clGUILofar::OnClipValueChanged (GtkAdjustment *gaSender, gpointer gpData)
{
    fClip = gaSender->value;
}


void clGUILofar::OnSaveClicks (GtkWidget *gwSender, gpointer gpData)
{
    int iAction = GPOINTER_TO_INT(gpData);
    const char *cpFileName;
    char *cpParseFileName;
    char *cpParsePtr;
    char *cpCompleteFileName;
    double dMinX;
    double dMaxX;
    /*GtkButton *gbSender = (GtkButton *) gwSender;
    GtkToggleButton *gtbSender = (GtkToggleButton *) gwSender;*/

    bSaving = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gwCBSaving)) ? 
        true : false;
    switch (iAction)
    {
        case 0:
            gtk_widget_hide(gwFSSave);
            if (bSaving)
            {
                gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(gwCBSaving));
            }
            break;
        case 1:
            gtk_widget_hide(gwFSSave);
            cpFileName = 
                gtk_file_selection_get_filename(GTK_FILE_SELECTION(gwFSSave));
            if (cpFileName == NULL) break;
            cpParseFileName = g_strdup(cpFileName);
            cpParsePtr = strtok(cpParseFileName, ".");
            if (cpParsePtr != NULL)
            {
                strImgFileName = cpParsePtr;
            }
            g_free(cpParseFileName);
            if (cpParsePtr == NULL) break;
            if (!bSaving)
            {
                if (!sLofarRq.bDemon)
                {
                    dMinX = sLofarResHdr.fLowFreq;
                    dMaxX = sLofarResHdr.fHighFreq;
                }
                else
                {
                    dMinX = 0;
                    dMaxX = sLofarResHdr.fDemonBand;
                }
                cpCompleteFileName = g_strdup_printf("%s.tif", 
                    strImgFileName.c_str());
                /*if (!FBLofar.SaveToFile(cpCompleteFileName, iCompressMode, 
                    iJPEGQuality, "LOFAR/DEMON",
                    dMinX, dMaxX,
                    -(sLofarResHdr.fLineTime * iLofarHeight), 0))*/
                if (!FBLofar.SaveToFile(cpCompleteFileName, iCompressMode, 
                    iJPEGQuality, "LOFAR/DEMON"))
                {
                    g_warning("Saving to file failed!");
                }
                SaveInfo(cpCompleteFileName, time(NULL) - (time_t) 
                    (FBLofar.GetHeight() * sLofarResHdr.fLineTime + 0.5f));
            }
            else
            {
                iScanCount = 0;
                iImgCount = 0;
                cpCompleteFileName = g_strdup_printf("%s%02i.tif",
                    strImgFileName.c_str(), iImgCount);
                if (!FBLofar.StartSaveToFile(cpCompleteFileName, 
                    iCompressMode, iJPEGQuality, "LOFAR/DEMON", 
                    FB_TIFF_CONT_VERTICAL))
                {
                    g_warning("Saving to file failed!");
                    gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(gwCBSaving));
                }
                SaveInfo(cpCompleteFileName, time(NULL));
            }
            g_free(cpCompleteFileName);
            break;
        case 2:
            if (!bSaving)
            {
                gtk_widget_show(gwFSSave);
            }
            break;
        case 3:
            if (bSaving)
            {
                gtk_widget_show(gwFSSave);
            }
            else
            {
                FBLofar.StopSaveToFile();
            }
            break;
    }
}


void clGUILofar::OnGdkInput (gpointer gpData, gint giSource,
    GdkInputCondition gicCondition)
{
    int iResMsgBufSize = GLOBAL_HEADER_LEN + sizeof(GDT) * lSpectSize;
    int iLineHeight;
    int iLineCntr;
    int iLineY1;
    long lAvgCntr;
    time_t ttBottomTime;
    char cpConvBuf[LGUI_CONV_BUF_SIZE];
    #ifdef __GNUG__
    char cpResMsgBuf[iResMsgBufSize];
    #else
    clAlloc ResMsgBuf;
    char *cpResMsgBuf = (char *) ResMsgBuf.Size(iResMsgBufSize * sizeof(char));
    #endif
    clAlloc LineData;
    GDT *fpSpectData = SpectData;
    GDT *fpAvgSpectData = AvgSpectData;
    GDT *fpLineData;

    if (!bRun) return;
    G_LOCK(gmInputMutex);
    while (SOp.ReadSelect(0))
    {
        if (SOp.ReadN(cpResMsgBuf, iResMsgBufSize) == iResMsgBufSize)
        {
            if (bFreezed) continue;
            LofarMsg.GetResult(cpResMsgBuf, &sLofarResHdr, fpSpectData);
            if (fClip < 1)
            {
                DSP.Clip(fpSpectData, fClip, lSpectSize);
                DSP.Mul(fpSpectData, 1 / fClip, lSpectSize);
            }
            if (!bConfigured) Configure();
            if (sLofarResHdr.fPeakLevel >= 0.0f) iClips++;
            if (gwDALofar->allocation.height != iLofarHeight)
                ConfigureHeight();
            fpLineData = (GDT *) LineData.Size(iLofarWidth * sizeof(GDT));
            if (!bAveraged)
            {
                switch (iFit)
                {
                    case LGUI_FIT_NEIGHBOR:
                        DSP.Resample(fpLineData, iLofarWidth,
                            fpSpectData, lSpectSize);
                        FBLofar.DrawLine(fpLineData);
                        break;
                    case LGUI_FIT_AVERAGE:
                        DSP.ResampleAvg(fpLineData, iLofarWidth,
                            fpSpectData, lSpectSize);
                        FBLofar.DrawLine(fpLineData);
                        break;
                    default:
                        FBLofar.DrawLine(fpSpectData);
                }
            }
            else
            {
                for (lAvgCntr = 0L; lAvgCntr < lSpectSize; lAvgCntr++)
                {
                    fpAvgSpectData[lAvgCntr] += fpSpectData[lAvgCntr];
                    fpAvgSpectData[lAvgCntr] *= (GDT) 0.5;
                }
                switch (iFit)
                {
                    case LGUI_FIT_NEIGHBOR:
                        DSP.Resample(fpLineData, iLofarWidth,
                            fpAvgSpectData, lSpectSize);
                        FBLofar.DrawLine(fpLineData);
                        break;
                    case LGUI_FIT_AVERAGE:
                        DSP.ResampleAvg(fpLineData, iLofarWidth,
                            fpAvgSpectData, lSpectSize);
                        FBLofar.DrawLine(fpLineData);
                        break;
                    default:
                        FBLofar.DrawLine(fpAvgSpectData);
                }
            }
            gdk_window_copy_area(gwDALofar->window, ggcLofarFG,
                0, 1,
                gwDALofar->window,
                0, 0, iLofarWidth, iLofarHeight - 1);
            gdk_draw_rgb_32_image(gwDALofar->window, ggcLofarFG,
                0, 0, iLofarWidth, 1,
                GDK_RGB_DITHER_NONE,
                FBLofar.GetCurPtr(0,0), FBLofar.GetRowStride());
            gdk_window_clear(gwDALine->window);
            iLineHeight = gwDALine->allocation.height;
            if (iFit)
            {
                for (iLineCntr = 0; iLineCntr < iLofarWidth; iLineCntr++)
                {
                    iLineY1 = (int) 
                        ((iLineHeight - iLineHeight * fpLineData[iLineCntr]) +
                        0.5);
                    gdk_draw_line(gwDALine->window, ggcLineFG,
                        iLineCntr, iLineY1, iLineCntr, iLineHeight - 1);
                }
            }
            else
            {
                for (iLineCntr = 0; iLineCntr < lSpectSize; iLineCntr++)
                {
                    iLineY1 = (int) 
                        ((iLineHeight - iLineHeight * fpSpectData[iLineCntr]) +
                        0.5);
                    gdk_draw_line(gwDALine->window, ggcLineFG,
                        iLineCntr, iLineY1, iLineCntr, iLineHeight - 1);
                }
            }
            PrintStatus();
            strftime(cpConvBuf, LGUI_CONV_BUF_SIZE, "%H:%M:%S", 
                localtime((time_t *) &sLofarResHdr.sTimeStamp.tv_sec));
            gtk_label_set_text(GTK_LABEL(gwLTopTime), cpConvBuf);
            ttBottomTime = (time_t) (sLofarResHdr.sTimeStamp.tv_sec -
                iLofarHeight * sLofarResHdr.fLineTime + 0.5f);
            strftime(cpConvBuf, LGUI_CONV_BUF_SIZE, "%H:%M:%S",
                localtime(&ttBottomTime));
            gtk_label_set_text(GTK_LABEL(gwLBottomTime), cpConvBuf);
            if (bSaving)
            {
                iScanCount++;
                if (iScanCount >= iContSaveScans)
                {
                    iImgCount++;
                    StartNewImgFile();
                    iScanCount = 0;
                }
            }
        }
        else
        {
            g_print("clSockOp::ReadN() error: %s\n", strerror(SOp.GetErrno()));
            gdk_input_remove(giGdkTag);
            SOp.Shutdown(2);
            SOp.Close();
            bConnected = false;
            break;
        }
    }
    G_UNLOCK(gmInputMutex);
}

