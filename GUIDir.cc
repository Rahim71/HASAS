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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>

#include "GUIDir.hh"


G_LOCK_DEFINE_STATIC(gmInputMutex);

static const char *cpWindowTxt = "Direction";
// Table 1
static const char *cpLServerTxt = "Server";
static const char *cpBConnectTxt = "Connect";
static const char *cpBDisconnectTxt = "Disconnect";
static const char *cpCBFreezeTxt = "Freeze";
// Table 2
static const char *cpLAlgorithmTxt = "Algorithm";
static const char *cpaLAlgorithmMenu[] = { "Beamforming", "Correlation",
    "Beamforming & Correlation", "Spectrum power/phase" };
static const char *cpLSoundSpeedTxt = "Sound speed";
static const char *cpLLowFrequencyLimitTxt = "Low frequency limit";
static const char *cpLIntegrationTimeTxt = "Integration time";
// Table 3
static const char *cpLScalingTxt = "Scaling";
static const char *cpaLScalingMenu[] = { "Linear", "Logarithmic",
    "Exponential", "Sine" };
static const char *cpLScalingExponentTxt = "Scaling exponent";
static const char *cpLRemoveNoiseTxt = "Remove noise";
static const char *cpaLRemoveNoiseMenu[] = { "None", "TPSW", "OTA",
    "Diff", "InvDiff" };
static const char *cpLAlphaTxt = "Alpha";
static const char *cpLMeanLengthTxt = "Mean length";
static const char *cpLGapLengthTxt = "Gap length";
static const char *cpCBNormalizeTxt = "Normalize";
static const char *cpCBNoFilterTxt = "No filter";
static const char *cpLPaletteTxt = "Palette";
static const char *cpaLPaletteMenu[] = { "BW", "HSV", "Light", "Temp",
    "Dir", "Green", "Green2", "PureGreen", "WB" };
// Table 4
static const char *cpLLeftDirectionTxt = "Left direction";
static const char *cpLRightDirectionTxt = "Right direction";
static const char *cpLSectorCountTxt = "Sector count";
static const char *cpLDirectionScaleTxt = "Direction scale";
static const char *cpCBSavingTxt = "Saving";
static const char *cpBSaveTxt = "Save";
static const char *cpFSSaveTxt = "Save to TIFF file";


clGUIDir *GUIDir;


int main (int argc, char *argv[])
{
    int iRetVal;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    GUIDir = new clGUIDir(&argc, &argv);
    iRetVal = GUIDir->Exec();
    delete GUIDir;
    return iRetVal;
}


gint WrapOnDelete (GtkWidget *gwSender, GdkEventAny *geaEvent)
{
    return GUIDir->OnDelete(gwSender, geaEvent);
}


void WrapOnHideToggled (GtkToggleButton *gtbSender, gpointer gpData)
{
    GUIDir->OnHideToggled(gtbSender, gpData);
}


gint WrapOnConnectClick (GtkWidget *gwSender, gpointer gpData)
{
    return GUIDir->OnConnectClick(gwSender, gpData);
}


void WrapOnFreezeToggled (GtkToggleButton *gtbSender, gpointer gpData)
{
    GUIDir->OnFreezeToggled(gtbSender, gpData);
}


gint WrapOnExposeWorm (GtkWidget *gwSender, GdkEventExpose *geeEvent)
{
    return GUIDir->OnExposeWorm(gwSender, geeEvent);
}


gint WrapOnMotionWorm (GtkWidget *gwSender, GdkEventMotion *gemEvent)
{
    return GUIDir->OnMotionWorm(gwSender, gemEvent);
}


gint WrapOnPaletteActivate (GtkWidget *gwSender, gpointer gpData)
{
    return GUIDir->OnPaletteActivate(gwSender, gpData);
}


void WrapOnSaveClicks (GtkWidget *gwSender, gpointer gpData)
{
    GUIDir->OnSaveClicks(gwSender, gpData);
}


void WrapOnGdkInput (gpointer gpData, gint giSource,
    GdkInputCondition gicCondition)
{
    GUIDir->OnGdkInput(gpData, giSource, gicCondition);
}


bool clGUIDir::Build ()
{
    // --- Main window
    gwWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gwWindow), cpWindowTxt);
    // shrink, grow, auto-shrink
    gtk_window_set_policy(GTK_WINDOW(gwWindow), FALSE, FALSE, TRUE);
    // gtk_widget_show(gwWindow);

    // --- Vertical box
    // homogenous, spacing
    gwVBox = gtk_vbox_new(FALSE, DGUI_PADDING);
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

    // --- Table worm
    if (!BuildTableWorm()) return false;

    // --- Status bar
    gwStatusBar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(gwVBox), gwStatusBar, FALSE, FALSE, 0);
    gtk_widget_show(gwStatusBar);
    guSbCtxt = gtk_statusbar_get_context_id(GTK_STATUSBAR(gwStatusBar),
        "status");
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guSbCtxt, "");
    
    // Delayed realization of main window
    gtk_widget_show(gwWindow);

    // Set ruler starting sizes
    gtk_widget_set_usize(gwHRDirection, 180 * iDirectionScale,
        gwHRDirection->allocation.height);
    gtk_widget_set_usize(gwVRTime, gwVRTime->allocation.width, iHistoryLines);

    // Build drawing primitives
    if (!BuildDrawingPrims()) return false;

    // Set cursors
    gdk_window_set_cursor(gwDAWorm->window, gcCrossHair);

    // Enable backing store
    GtkUtils.EnableBackingStore(gwDAWorm);

    // Set default widget
    //gtk_window_set_default(GTK_WINDOW(gwWindow), gwBConnect);
    
    return true;
}


bool clGUIDir::ConnectSignals ()
{
    int iWidgetCntr;
    
    gtk_signal_connect(GTK_OBJECT(gwWindow), "delete_event",
        GTK_SIGNAL_FUNC(WrapOnDelete), NULL);

    gtk_signal_connect(GTK_OBJECT(gwCBHide), "toggled",
        GTK_SIGNAL_FUNC(WrapOnHideToggled), NULL);

    gtk_widget_add_events(gwDAWorm, GDK_POINTER_MOTION_MASK);
    GtkUtils.ConnectMotionEvent(gwHRDirection, gwDAWorm);
    GtkUtils.ConnectMotionEvent(gwVRTime, gwDAWorm);
    gtk_signal_connect(GTK_OBJECT(gwDAWorm), "expose_event",
        GTK_SIGNAL_FUNC(WrapOnExposeWorm), NULL);
    gtk_signal_connect(GTK_OBJECT(gwDAWorm), "motion_notify_event",
        GTK_SIGNAL_FUNC(WrapOnMotionWorm), NULL);

    gtk_signal_connect(GTK_OBJECT(gwBConnect), "clicked",
        GTK_SIGNAL_FUNC(WrapOnConnectClick), NULL);
    gtk_signal_connect(GTK_OBJECT(gwBDisconnect), "clicked",
        GTK_SIGNAL_FUNC(WrapOnConnectClick), NULL);
    gtk_signal_connect(GTK_OBJECT(gwCBFreeze), "toggled",
        GTK_SIGNAL_FUNC(WrapOnFreezeToggled), NULL);

    for (iWidgetCntr = 0; iWidgetCntr < DGUI_PALETTE_ITEMS; iWidgetCntr++)
    {
        gtk_signal_connect(GTK_OBJECT(gwaMIPalette[iWidgetCntr]), "activate",
            GTK_SIGNAL_FUNC(WrapOnPaletteActivate), NULL);
    }

    gtk_signal_connect(GTK_OBJECT(gwCBSaving), "toggled",
        GTK_SIGNAL_FUNC(WrapOnSaveClicks), GINT_TO_POINTER(3));
    gtk_signal_connect(GTK_OBJECT(gwBSave), "clicked",
        GTK_SIGNAL_FUNC(WrapOnSaveClicks), GINT_TO_POINTER(2));
    gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(gwFSSave)->ok_button),
        "clicked", GTK_SIGNAL_FUNC(WrapOnSaveClicks), GINT_TO_POINTER(1));
    gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(gwFSSave)->cancel_button),
        "clicked", GTK_SIGNAL_FUNC(WrapOnSaveClicks), GINT_TO_POINTER(0));

    return true;
}


bool clGUIDir::BuildTable1 ()
{
    // rows, columns, homogenous
    gwTable1 = gtk_table_new(2, 4, FALSE);
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
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLServer);
    gwCServer = gtk_combo_new();
    gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        DGUI_SERVER_MAXLEN);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        "127.0.0.1:30001");
    // table, child, left attach, right attach, top attach, bottom attach,
    // x-options, y-options, x-padding, y-padding
    gtk_table_attach(GTK_TABLE(gwTable1), gwCServer,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwCServer);
    GtkUtils.ComboListFromFile(gwCServer, &glServers, DGUI_HOSTFILE);

    // - Button: Connect
    gwBConnect = gtk_button_new_with_label(cpBConnectTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBConnect,
        2, 3, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwBConnect);

    // - Button: Disconnect
    gwBDisconnect = gtk_button_new_with_label(cpBDisconnectTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBDisconnect,
        3, 4, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwBDisconnect);

    // - CheckButton: Freeze
    gwCBFreeze = gtk_check_button_new_with_label(cpCBFreezeTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwCBFreeze,
        4, 5, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwCBFreeze);

    return true;
}


bool clGUIDir::BuildTable2 ()
{
    char cpConvBuf[DGUI_CONV_BUF_SIZE];

    // rows, columns, homogenous
    gwTable2 = gtk_table_new(2, 4, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable2, FALSE, FALSE, 0);
    gtk_widget_show(gwTable2);

    // Label & OptionMenu: Algorithm
    gwLAlgorithm = gtk_label_new(cpLAlgorithmTxt);
    gtk_label_set_justify(GTK_LABEL(gwLAlgorithm), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLAlgorithm,
        0, 1, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLAlgorithm);
    gwOMAlgorithm = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwOMAlgorithm,
        0, 1, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMAlgorithm);
    GtkUtils.BuildOptionMenu(gwOMAlgorithm, &gwMAlgorithm, gwaMIAlgorithm,
        cpaLAlgorithmMenu, DGUI_ALGORITHM_ITEMS);

    // Label & Entry: Soundspeed
    gwLSoundSpeed = gtk_label_new(cpLSoundSpeedTxt);
    gtk_label_set_justify(GTK_LABEL(gwLSoundSpeed), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLSoundSpeed,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK), 
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLSoundSpeed);
    gwESoundSpeed = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwESoundSpeed,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwESoundSpeed);
    gtk_widget_set_usize(gwESoundSpeed, DGUI_ENTRY_WIDTH,
        gwESoundSpeed->requisition.height);
    g_snprintf(cpConvBuf, DGUI_CONV_BUF_SIZE, "%.1f", fSoundSpeed);
    gtk_entry_set_text(GTK_ENTRY(gwESoundSpeed), cpConvBuf);

    // Label & Entry: Low frequency limit
    gwLLowFrequencyLimit = gtk_label_new(cpLLowFrequencyLimitTxt);
    gtk_label_set_justify(GTK_LABEL(gwLLowFrequencyLimit), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLLowFrequencyLimit,
        2, 3, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLLowFrequencyLimit);
    gwELowFrequencyLimit = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwELowFrequencyLimit,
        2, 3, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwELowFrequencyLimit);
    gtk_widget_set_usize(gwELowFrequencyLimit, DGUI_ENTRY_WIDTH,
        gwELowFrequencyLimit->requisition.height);
    gtk_entry_set_text(GTK_ENTRY(gwELowFrequencyLimit), "0.0");

    // Label & Entry: Integration time
    gwLIntegrationTime = gtk_label_new(cpLIntegrationTimeTxt);
    gtk_label_set_justify(GTK_LABEL(gwLIntegrationTime), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLIntegrationTime,
        3, 4, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLIntegrationTime);
    gwEIntegrationTime = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEIntegrationTime,
        3, 4, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwEIntegrationTime);
    gtk_widget_set_usize(gwEIntegrationTime, DGUI_ENTRY_WIDTH,
        gwEIntegrationTime->requisition.height);
    gtk_entry_set_text(GTK_ENTRY(gwEIntegrationTime), "1.0");

    return true;
}


bool clGUIDir::BuildTable3 ()
{
    // rows, columns, homogenous
    gwTable3 = gtk_table_new(2, 8, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable3, FALSE, FALSE, 0);
    gtk_widget_show(gwTable3);

    // Label & OptionMenu: Scaling
    gwLScaling = gtk_label_new(cpLScalingTxt);
    gtk_label_set_justify(GTK_LABEL(gwLScaling), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLScaling,
        0, 1, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLScaling);
    gwOMScaling = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwOMScaling,
        0, 1, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMScaling);
    GtkUtils.BuildOptionMenu(gwOMScaling, &gwMScaling, gwaMIScaling,
        cpaLScalingMenu, DGUI_SCALING_ITEMS);

    // Label & Entry: Scaling exponent
    gwLScalingExponent = gtk_label_new(cpLScalingExponentTxt);
    gtk_label_set_justify(GTK_LABEL(gwLScalingExponent), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLScalingExponent,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLScalingExponent);
    gwEScalingExponent = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEScalingExponent,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwEScalingExponent);
    gtk_widget_set_usize(gwEScalingExponent, DGUI_ENTRY_WIDTH,
        gwEScalingExponent->requisition.height);
    gtk_entry_set_text(GTK_ENTRY(gwEScalingExponent), "2.0");

    // Label & OptionMenu: Remove noise
    gwLRemoveNoise = gtk_label_new(cpLRemoveNoiseTxt);
    gtk_label_set_justify(GTK_LABEL(gwLRemoveNoise), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLRemoveNoise,
        2, 3, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLRemoveNoise);
    gwOMRemoveNoise = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwOMRemoveNoise,
        2, 3, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMRemoveNoise);
    GtkUtils.BuildOptionMenu(gwOMRemoveNoise, &gwMRemoveNoise, 
        gwaMIRemoveNoise, cpaLRemoveNoiseMenu, DGUI_REMOVE_NOISE_ITEMS);

    // Label & Entry: Alpha
    gwLAlpha = gtk_label_new(cpLAlphaTxt);
    gtk_label_set_justify(GTK_LABEL(gwLAlpha), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLAlpha,
        3, 4, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLAlpha);
    gwEAlpha = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEAlpha,
        3, 4, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwEAlpha);
    gtk_widget_set_usize(gwEAlpha, DGUI_ENTRY_WIDTH,
        gwEAlpha->requisition.height);
    gtk_entry_set_text(GTK_ENTRY(gwEAlpha), "2.0");  // 0.9

    // Label & Entry: Mean length
    gwLMeanLength = gtk_label_new(cpLMeanLengthTxt);
    gtk_label_set_justify(GTK_LABEL(gwLMeanLength), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLMeanLength,
        4, 5, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLMeanLength);
    gwEMeanLength = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEMeanLength,
        4, 5, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwEMeanLength);
    gtk_widget_set_usize(gwEMeanLength, DGUI_ENTRY_WIDTH,
        gwEMeanLength->requisition.height);
    gtk_entry_set_text(GTK_ENTRY(gwEMeanLength), "10");  // 18

    // Label & Entry: Gap length
    gwLGapLength = gtk_label_new(cpLGapLengthTxt);
    gtk_label_set_justify(GTK_LABEL(gwLGapLength), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLGapLength,
        5, 6, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLGapLength);
    gwEGapLength = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEGapLength,
        5, 6, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwEGapLength);
    gtk_widget_set_usize(gwEGapLength, DGUI_ENTRY_WIDTH, 
        gwEGapLength->requisition.height);
    gtk_entry_set_text(GTK_ENTRY(gwEGapLength), "3");

    // CheckButton: No filter
    gwCBNoFilter = gtk_check_button_new_with_label(cpCBNoFilterTxt);
    gtk_table_attach(GTK_TABLE(gwTable3), gwCBNoFilter,
        6, 7, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwCBNoFilter);

    // CheckButton: Normalize
    gwCBNormalize = gtk_check_button_new_with_label(cpCBNormalizeTxt);
    gtk_table_attach(GTK_TABLE(gwTable3), gwCBNormalize,
        6, 7, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gwCBNormalize), TRUE);
    gtk_widget_show(gwCBNormalize);

    // Label & OptionMenu: Palette
    gwLPalette = gtk_label_new(cpLPaletteTxt);
    gtk_label_set_justify(GTK_LABEL(gwLPalette), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLPalette,
        7, 8, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLPalette);
    gwOMPalette = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwOMPalette,
        7, 8, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMPalette);
    GtkUtils.BuildOptionMenu(gwOMPalette, &gwMPalette, gwaMIPalette,
        cpaLPaletteMenu, DGUI_PALETTE_ITEMS);
    gtk_option_menu_set_history(GTK_OPTION_MENU(gwOMPalette), iPalette);

    return true;
}


bool clGUIDir::BuildTable4 ()
{
    char cpConvBuf[DGUI_CONV_BUF_SIZE];

    // rows, columns, homogenous
    gwTable4 = gtk_table_new(2, 5, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable4, FALSE, FALSE, 0);
    gtk_widget_show(gwTable4);

    // Label & Entry: Left direction
    gwLLeftDirection = gtk_label_new(cpLLeftDirectionTxt);
    gtk_label_set_justify(GTK_LABEL(gwLLeftDirection), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable4), gwLLeftDirection,
        0, 1, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLLeftDirection);
    gwELeftDirection = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable4), gwELeftDirection,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwELeftDirection);
    gtk_widget_set_usize(gwELeftDirection, DGUI_ENTRY_WIDTH,
        gwELeftDirection->requisition.height);
    gtk_entry_set_text(GTK_ENTRY(gwELeftDirection), "-90.0");

    // Label & Entry: Right direction
    gwLRightDirection = gtk_label_new(cpLRightDirectionTxt);
    gtk_label_set_justify(GTK_LABEL(gwLRightDirection), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable4), gwLRightDirection,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLRightDirection);
    gwERightDirection = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable4), gwERightDirection,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwERightDirection);
    gtk_widget_set_usize(gwERightDirection, DGUI_ENTRY_WIDTH,
        gwERightDirection->requisition.height);
    gtk_entry_set_text(GTK_ENTRY(gwERightDirection), "90.0");

    // Label & Entry: Sector count
    gwLSectorCount = gtk_label_new(cpLSectorCountTxt);
    gtk_label_set_justify(GTK_LABEL(gwLSectorCount), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable4), gwLSectorCount,
        2, 3, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLSectorCount);
    gwESectorCount = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable4), gwESectorCount,
        2, 3, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwESectorCount);
    gtk_widget_set_usize(gwESectorCount, DGUI_ENTRY_WIDTH,
        gwESectorCount->requisition.height);
    gtk_entry_set_text(GTK_ENTRY(gwESectorCount), "180");

    // Label & Entry: Direction scale factor
    gwLDirectionScale = gtk_label_new(cpLDirectionScaleTxt);
    gtk_label_set_justify(GTK_LABEL(gwLDirectionScale), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable4), gwLDirectionScale,
        3, 4, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwLDirectionScale);
    gwEDirectionScale = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable4), gwEDirectionScale,
        3, 4, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwEDirectionScale);
    gtk_widget_set_usize(gwEDirectionScale, DGUI_ENTRY_WIDTH,
        gwEDirectionScale->requisition.height);
    g_snprintf(cpConvBuf, DGUI_CONV_BUF_SIZE, "%i", iDirectionScale);
    gtk_entry_set_text(GTK_ENTRY(gwEDirectionScale), cpConvBuf);

    gwCBSaving = gtk_check_button_new_with_label(cpCBSavingTxt);
    gtk_table_attach(GTK_TABLE(gwTable4), gwCBSaving,
        4, 5, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwCBSaving);

    // Button: Save
    gwBSave = gtk_button_new_with_label(cpBSaveTxt);
    gtk_table_attach(GTK_TABLE(gwTable4), gwBSave,
        4, 5, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        DGUI_PADDING / 2, 0);
    gtk_widget_show(gwBSave);

    // FileSelection: Save
    gwFSSave = gtk_file_selection_new(cpFSSaveTxt);

    return true;
}


bool clGUIDir::BuildTableWorm ()
{
    // rows, columns, homogenous
    gwTableWorm = gtk_table_new(2, 2, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTableWorm, FALSE, FALSE, 0);
    gtk_widget_show(gwTableWorm);

    // HorizontalRuler: Direction
    gwHRDirection = gtk_hruler_new();
    gtk_ruler_set_metric(GTK_RULER(gwHRDirection), GTK_PIXELS);
    // ruler, lower, upper, position, max size
    gtk_ruler_set_range(GTK_RULER(gwHRDirection), -90.0, 90.0, 0.0, 180.0);
    gtk_table_attach(GTK_TABLE(gwTableWorm), gwHRDirection,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK),  // GTK_FILL
        (GtkAttachOptions) 0,
        0, 0);
    gtk_widget_show(gwHRDirection);

    // VerticalRuler: Time
    gwVRTime = gtk_vruler_new();
    gtk_ruler_set_metric(GTK_RULER(gwVRTime), GTK_PIXELS);
    // ruler, lower, upper, position, max size
    gtk_ruler_set_range(GTK_RULER(gwVRTime), 0.0, 
        (gfloat) iHistoryLines / (gfloat) 60.0 , 0.0, 1.0);
    gtk_table_attach(GTK_TABLE(gwTableWorm), gwVRTime,
        0, 1, 1, 2,
        (GtkAttachOptions) 0,
        (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK),  // GTK_FILL
        0, 0);
    gtk_widget_show(gwVRTime);

    // DrawingArea: Direction display (Worm)
    gwDAWorm = gtk_drawing_area_new();
    // drawing area, width, height
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDAWorm), 180 * iDirectionScale, 
        iHistoryLines);
    gtk_table_attach(GTK_TABLE(gwTableWorm), gwDAWorm,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK),  // GTK_FILL
        (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK),  // GTK_FILL
        0, 0);
    gtk_widget_show(gwDAWorm);

    return true;
}


bool clGUIDir::BuildDrawingPrims ()
{
    // Create graphics contexts
    ggcWormBG = gdk_gc_new(gwDAWorm->window);
    gdk_rgb_gc_set_foreground(ggcWormBG, DGUI_WORM_BG);
    gdk_rgb_gc_set_background(ggcWormBG, DGUI_WORM_BG);
    gdk_gc_set_function(ggcWormBG, GDK_COPY);
    gdk_gc_set_fill(ggcWormBG, GDK_SOLID);

    ggcWormFG = gdk_gc_new(gwDAWorm->window);
    gdk_rgb_gc_set_foreground(ggcWormFG, DGUI_WORM_FG);
    gdk_rgb_gc_set_background(ggcWormFG, DGUI_WORM_BG);
    gdk_gc_set_function(ggcWormFG, GDK_COPY);
    gdk_gc_set_fill(ggcWormFG, GDK_SOLID);

    // Create cursors
    gcCrossHair = gdk_cursor_new(GDK_CROSSHAIR);

    // Create specified palette for direction display
    switch (iPalette)
    {
        case DGUI_PAL_BW:
            FBDir.PalGenBW();
            break;
        case DGUI_PAL_HSV:
            FBDir.PalGenHSV();
            break;
        case DGUI_PAL_LIGHT:
            FBDir.PalGenLight();
            break;
        case DGUI_PAL_TEMP:
            FBDir.PalGenTemp();
            break;
        case DGUI_PAL_DIR:
            FBDir.PalGenDir();
            break;
        case DGUI_PAL_GREEN:
            FBDir.PalGenGreen();
            break;
        case DGUI_PAL_GREEN2:
            FBDir.PalGenGreen2();
            break;
        case DGUI_PAL_PUREGREEN:
            FBDir.PalGenPureGreen();
            break;
        case DGUI_PAL_WB:
            FBDir.PalGenWB();
            break;
        default:
            FBDir.PalGenDir();
    }

    return true;
}


void clGUIDir::FreeDrawingPrims ()
{
    gdk_cursor_destroy(gcCrossHair);
    gdk_gc_destroy(ggcWormBG);
    gdk_gc_destroy(ggcWormFG);
}


bool clGUIDir::ParseServerStr (char *cpHostRes, int *ipPortRes,
    const char *cpSourceStr)
{
    char cpTempStr[DGUI_SERVER_MAXLEN + 1];
    char *cpTempHost;
    char *cpTempPort;

    strncpy(cpTempStr, cpSourceStr, DGUI_SERVER_MAXLEN);
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


bool clGUIDir::InitConnection (const char *cpServerHost, int iServerPort)
{
    char cpReqProcName[GLOBAL_HEADER_LEN];

    GetSettings();
    iSockH = SClient.Connect(cpServerHost, NULL, iServerPort);
    if (iSockH < 0) return false;
    g_print("Connection established - sending process request...\n");
    bConnected = true;
    SOp = new clSockOp(iSockH);
    switch (sDirRq.iAlgorithm)
    {
        case MSG_DIR_ALG_BEAM:
        case MSG_DIR_ALG_CORR:
        case (MSG_DIR_ALG_BEAM | MSG_DIR_ALG_CORR):
            strcpy(cpReqProcName, DGUI_REQ_PROC);
            break;
        case MSG_DIR_ALG_SPECT:
            strcpy(cpReqProcName, DGUI_REQ_PROC2);
            break;
        default:
            g_print("Unknown algorithm or combination!? This is bug.\n");
    }
    if (SOp->WriteN(cpReqProcName, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    g_print("Sending settings...\n");
    if (!SendSettings()) return false;
    iWormWidth = lResultCount * iDirectionScale;
    iWormHeight = iHistoryLines;
    FBDir.SetSize(iWormWidth, iWormHeight);
    //FBDir.Clear();
    lResultMsgBufSize = GLOBAL_HEADER_LEN + lResultCount * sizeof(GDT);
    ResultMsgBuf.Size(lResultMsgBufSize);
    Results.Size(lResultCount * sizeof(GDT));
    ScaledResults.Size(iWormWidth * sizeof(GDT));
    iClips = 0;
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDAWorm), iWormWidth, iWormHeight);
    gtk_widget_set_usize(gwHRDirection, iWormWidth,
        gwHRDirection->allocation.height);
    gtk_widget_set_usize(gwVRTime, gwVRTime->allocation.width, iWormHeight);
    gdk_draw_rgb_32_image(gwDAWorm->window, ggcWormFG,
        0, 0, iWormWidth, iWormHeight,
        GDK_RGB_DITHER_NONE, FBDir.GetCurPtr(0,0), FBDir.GetRowStride());
    giGdkTag = gdk_input_add(iSockH, GDK_INPUT_READ, WrapOnGdkInput, NULL);
    return true;
}


void clGUIDir::GetSettings ()
{
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEDirectionScale)), "%i",
        &iDirectionScale);
    switch (GtkUtils.OptionMenuGetActive(gwOMAlgorithm, gwaMIAlgorithm, 
        DGUI_ALGORITHM_ITEMS))
    {
        case 0:
            sDirRq.iAlgorithm = MSG_DIR_ALG_BEAM;
            break;
        case 1:
            sDirRq.iAlgorithm = MSG_DIR_ALG_CORR;
            break;
        case 2:
            sDirRq.iAlgorithm = (MSG_DIR_ALG_BEAM | MSG_DIR_ALG_CORR);
            break;
        case 3:
            sDirRq.iAlgorithm = MSG_DIR_ALG_SPECT;
            break;
        default:
            sDirRq.iAlgorithm = MSG_DIR_ALG_BEAM;
    }
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwESoundSpeed)), "%f",
        &sDirRq.fSoundSpeed);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwELowFrequencyLimit)), "%f",
        &sDirRq.fLowFreqLimit);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEIntegrationTime)), "%f",
        &fIntegrationTime);
    sDirRq.fIntegrationTime = fIntegrationTime;
    gtk_ruler_set_range(GTK_RULER(gwVRTime), 0.0, 
        fIntegrationTime / 60.0 * iHistoryLines, 0.0, 
        fIntegrationTime / 60.0 * iHistoryLines);
    sDirRq.iScaling = GtkUtils.OptionMenuGetActive(gwOMScaling,
        gwaMIScaling, DGUI_SCALING_ITEMS);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEScalingExponent)), "%f",
        &sDirRq.fScalingExp);
    sDirRq.iRemoveNoise = GtkUtils.OptionMenuGetActive(gwOMRemoveNoise,
        gwaMIRemoveNoise, DGUI_REMOVE_NOISE_ITEMS);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEAlpha)), "%f",
        &sDirRq.fAlpha);
    sDirRq.lMeanLength = atol(gtk_entry_get_text(GTK_ENTRY(gwEMeanLength)));
    sDirRq.lGapLength = atol(gtk_entry_get_text(GTK_ENTRY(gwEGapLength)));
    sDirRq.bNormalize = 
        (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gwCBNormalize))) ?
        true : false;
    sDirRq.bDisableFilter = 
        (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gwCBNoFilter))) ?
        true : false;
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwELeftDirection)), "%f",
        &fLeftDirDeg);
    sDirRq.fLeftDir = DSP.DegToRad(fLeftDirDeg);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwERightDirection)), "%f",
        &fRightDirDeg);
    sDirRq.fRightDir = DSP.DegToRad(fRightDirDeg);
    gtk_ruler_set_range(GTK_RULER(gwHRDirection), fLeftDirDeg, fRightDirDeg,
        0.0, fRightDirDeg - fLeftDirDeg);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwESectorCount)), "%li",
        &sDirRq.lSectorCount);
    lResultCount = sDirRq.lSectorCount;
}


bool clGUIDir::SendSettings ()
{
    char cpMsgBuf[GLOBAL_HEADER_LEN];

    DirMsg.SetRequest(cpMsgBuf, &sDirRq);
    if (SOp->WriteN(cpMsgBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
    {
        g_print("Error (%i) sending settings\n\n", SOp->GetErrno());
        return false;
    }
    g_print("Settings sent\n");
    return true;
}


void clGUIDir::PrintStatus ()
{
    float fCursorDirection;
    time_t ttCurrentTime;
    time_t ttCursorTime;
    char cpTimeConv[DGUI_CONV_BUF_SIZE];
    char cpStatusBarTxt[DGUI_CONV_BUF_SIZE];

    if (sResultHeader.lSectorCount > 0 && iDirectionScale > 0)
    {
        fCursorDirection = ((fRightDirDeg - fLeftDirDeg) / 
            (sResultHeader.lSectorCount * iDirectionScale)) * iCursorX +
            fLeftDirDeg;
    }
    else
    {
        fCursorDirection = 0.0f;
    }
    time(&ttCurrentTime);
    ttCursorTime = (time_t) 
        (ttCurrentTime - iCursorY * fIntegrationTime + 0.5f);
    strftime(cpTimeConv, DGUI_CONV_BUF_SIZE,
        "%H:%M:%S", localtime(&ttCursorTime));
    g_snprintf(cpStatusBarTxt, DGUI_CONV_BUF_SIZE,
        "%.1f deg / %s / %.1f dB / %i clips", fCursorDirection, cpTimeConv,
        sResultHeader.fPeakLevel, iClips);
    gtk_statusbar_pop(GTK_STATUSBAR(gwStatusBar), guSbCtxt);
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guSbCtxt, cpStatusBarTxt);
}


void clGUIDir::SaveInfo (const char *cpFileName, time_t ttStartTime)
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
    fprintf(fileInfo, "LeftDir: %f\n", sDirRq.fLeftDir);
    fprintf(fileInfo, "RightDir: %f\n", sDirRq.fRightDir);
    fprintf(fileInfo, "IntTime: %f\n", sResultHeader.fIntegrationTime);
    fprintf(fileInfo, "HighFreq: %f\n", sResultHeader.fHighFreqLimit);
    fprintf(fileInfo, "Sectors: %li\n", sResultHeader.lSectorCount);
    fclose(fileInfo);
}


void clGUIDir::StartNewImgFile ()
{
    char *cpCompleteFileName;

    FBDir.StopSaveToFile();
    cpCompleteFileName = g_strdup_printf("%s%02i.tif",
        strImgFileName.c_str(), iImgCount);
    if (!FBDir.StartSaveToFile(cpCompleteFileName, iCompressMode,
        iJPEGQuality, "Direction", FB_TIFF_CONT_VERTICAL))
    {
        g_warning("Saving to file failed!");
        gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(gwCBSaving));
        bSaving = false;
    }
    SaveInfo(cpCompleteFileName, time(NULL));
    g_free(cpCompleteFileName);
}


clGUIDir::clGUIDir (int *ipArgC, char ***cpapArgV)
{
    bRun = true;
    bConnected = false;
    bFreezed = false;
    glServers = NULL;
    g_print("%s GUI v%i.%i.%i\n", cpWindowTxt,
        DGUI_VER_MAJ, DGUI_VER_MIN, DGUI_VER_PL);
    g_print("Copyright (C) 2000-2001 Jussi Laako\n\n");
    g_print("Gtk+ version %i.%i.%i\n", gtk_major_version, gtk_minor_version,
        gtk_micro_version);
    g_print("Locale set to %s\n", gtk_set_locale());
    gtk_init(ipArgC, cpapArgV);
    gdk_rgb_init();
    gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
    gtk_widget_set_default_visual(gdk_rgb_get_visual());
    Cfg = new clCfgFile(DGUI_CFGFILE);
    if (!Cfg->GetInt("Palette", &iPalette))
        iPalette = DGUI_PAL_DIR;
}


clGUIDir::~clGUIDir ()
{
    delete Cfg;
}


int clGUIDir::Exec ()
{
    if (!Cfg->GetInt("HistoryLines", &iHistoryLines))
        iHistoryLines = DGUI_DEF_LINES;
    if (!Cfg->GetInt("DirectionScale", &iDirectionScale))
        iDirectionScale = 1;
    if (!Cfg->GetInt("Palette", &iPalette))
        iPalette = 0;
    if (!Cfg->GetFlt("SoundSpeed", &fSoundSpeed))
        fSoundSpeed = DGUI_DEF_SOUNDSPEED;
    if (!Cfg->GetInt("TIFFCompression", &iTIFFCompression))
        iTIFFCompression = 0;
    if (!Cfg->GetInt("JPEGQuality", &iJPEGQuality))
        iJPEGQuality = 100;
    if (!Cfg->GetInt("ContSaveScans", &iContSaveScans))
        iContSaveScans = 1024;
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
    iWormWidth = 180 * iDirectionScale;
    iWormHeight = iHistoryLines;
    FBDir.SetSize(iWormWidth, iWormHeight);
    FBDir.Clear();
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


gint clGUIDir::OnDelete (GtkWidget *gwSender, GdkEventAny *geaEvent)
{
    bRun = false;
    if (bSaving)
    {
        FBDir.StopSaveToFile();
    }
    gtk_main_quit();
    return 0;
}


void clGUIDir::OnHideToggled (GtkToggleButton *gtbSender, gpointer gpData)
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


gint clGUIDir::OnConnectClick (GtkWidget *gwSender, gpointer gpData)
{
    char cpHost[DGUI_SERVER_MAXLEN + 1];
    int iPort;

    if (bConnected)
    {
        gdk_input_remove(giGdkTag);
        SOp->Shutdown(2);
        delete SOp;
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


void clGUIDir::OnFreezeToggled (GtkToggleButton *gtbSender, gpointer gpData)
{
    bFreezed = (gtk_toggle_button_get_active(gtbSender)) ? true : false;
}


gint clGUIDir::OnExposeWorm(GtkWidget *gwSender, GdkEventExpose *geeEvent)
{
    int iExposeX;
    int iExposeY;
    int iExposeWidth;
    int iExposeHeight;

    iExposeX = (geeEvent->area.x >= iWormWidth) ? 0 : geeEvent->area.x;
    iExposeY = (geeEvent->area.y >= iWormHeight) ? 0 : geeEvent->area.y;
    iExposeWidth = (geeEvent->area.width > iWormWidth) ? 
        iWormWidth : geeEvent->area.width;
    iExposeHeight = (geeEvent->area.height > iWormHeight) ?
        iWormHeight : geeEvent->area.height;
    gdk_draw_rgb_32_image(gwDAWorm->window, ggcWormFG,
        iExposeX, iExposeY, iExposeWidth, iExposeHeight,
        GDK_RGB_DITHER_NONE, FBDir.GetCurPtr(iExposeX, iExposeY),
        FBDir.GetRowStride());
    return 0;
}


gint clGUIDir::OnMotionWorm (GtkWidget *gwSender, GdkEventMotion *gemEvent)
{
    iCursorX = (int) gemEvent->x;
    iCursorY = (int) gemEvent->y;
    PrintStatus();
    return 0;
}


gint clGUIDir::OnPaletteActivate (GtkWidget *gwSender, gpointer gpData)
{
    int iPalType;

    if (gwSender == gwaMIPalette[DGUI_PAL_BW])
    {
        FBDir.PalGenBW();
    }
    else if (gwSender == gwaMIPalette[DGUI_PAL_HSV])
    {
        FBDir.PalGenHSV();
    }
    else if (gwSender == gwaMIPalette[DGUI_PAL_LIGHT])
    {
        FBDir.PalGenLight();
    }
    else if (gwSender == gwaMIPalette[DGUI_PAL_TEMP])
    {
        FBDir.PalGenTemp();
    }
    else if (gwSender == gwaMIPalette[DGUI_PAL_DIR])
    {
        FBDir.PalGenDir();
    }
    else if (gwSender == gwaMIPalette[DGUI_PAL_GREEN])
    {
        FBDir.PalGenGreen();
    }
    else if (gwSender == gwaMIPalette[DGUI_PAL_GREEN2])
    {
        FBDir.PalGenGreen2();
    }
    else if (gwSender == gwaMIPalette[DGUI_PAL_PUREGREEN])
    {
        FBDir.PalGenPureGreen();
    }
    else if (gwSender == gwaMIPalette[DGUI_PAL_WB])
    {
        FBDir.PalGenWB();
    }
    else
    {
        Cfg->GetInt("Palette", &iPalType);
        switch (iPalType)
        {
            case DGUI_PAL_BW:
                FBDir.PalGenBW();
                break;
            case DGUI_PAL_HSV:
                FBDir.PalGenHSV();
                break;
            case DGUI_PAL_LIGHT:
                FBDir.PalGenLight();
                break;
            case DGUI_PAL_TEMP:
                FBDir.PalGenTemp();
                break;
            case DGUI_PAL_DIR:
                FBDir.PalGenDir();
                break;
            case DGUI_PAL_GREEN:
                FBDir.PalGenGreen();
                break;
            default:
                FBDir.PalGenBW();
        }
    }

    return 0;
}


void clGUIDir::OnSaveClicks (GtkWidget *gwSender, gpointer gpData)
{
    int iAction = GPOINTER_TO_INT(gpData);
    const char *cpFileName;
    char *cpParseFileName;
    char *cpParsePtr;
    char *cpCompleteFileName;

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
                cpCompleteFileName = g_strdup_printf("%s.tif",
                    strImgFileName.c_str());
                /*if (!FBDir.SaveToFile(cpCompleteFileName, iCompressMode, 
                    iJPEGQuality, "Direction", 
                    sDirRq.fLeftDir, sDirRq.fRightDir,
                    -(iWormHeight * sResultHeader.fIntegrationTime), 0))*/
                if (!FBDir.SaveToFile(cpCompleteFileName, iCompressMode, 
                    iJPEGQuality, "Direction"))
                {
                    g_warning("Saving to file failed!");
                }
                SaveInfo(cpCompleteFileName, time(NULL) - (time_t)
                    (FBDir.GetHeight() * sResultHeader.fIntegrationTime + 0.5f));
            }
            else
            {
                iScanCount = 0;
                iImgCount = 0;
                cpCompleteFileName = g_strdup_printf("%s%02i.tif",
                    strImgFileName.c_str(), iImgCount);
                if (!FBDir.StartSaveToFile(cpCompleteFileName, iCompressMode,
                    iJPEGQuality, "Direction", FB_TIFF_CONT_VERTICAL))
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
                FBDir.StopSaveToFile();
            }
            break;
    }
}


void clGUIDir::OnGdkInput (gpointer gpData, gint giSource,
    GdkInputCondition gicCondition)
{
    int iReadRes;
    char cpConvBuf[DGUI_CONV_BUF_SIZE];
    GDT *fpResults = Results;
    GDT *fpScaledResults = ScaledResults;
    
    if (!bRun) return;
    G_LOCK(gmInputMutex);
    iReadRes = SOp->ReadN(ResultMsgBuf, lResultMsgBufSize);
    if (iReadRes == lResultMsgBufSize)
    {
        if (bFreezed)
        {
            G_UNLOCK(gmInputMutex);
            return;
        }
        DirMsg.GetResult(ResultMsgBuf, &sResultHeader, fpResults);
        if (sResultHeader.lSectorCount > lResultCount)
        {
            g_print("Fatal error: more sectors than requested!\n\n");
            gdk_input_remove(giGdkTag);
            gtk_main_quit();
            exit(2);
        }
        //g_print("Peak level: %f dB\n", sResultHeader.fPeakLevel);
        if (sResultHeader.fPeakLevel >= 0.0f) iClips++;
        DSP.InterpolateAvg(fpScaledResults, fpResults, iDirectionScale,
            lResultCount);
        FBDir.DrawLine(fpScaledResults);
        gdk_window_copy_area(gwDAWorm->window, ggcWormFG,
            0, 1,
            gwDAWorm->window,
            0, 0, iWormWidth, iWormHeight - 1);
        gdk_draw_rgb_32_image(gwDAWorm->window, ggcWormFG,
            0, 0, iWormWidth, 1,
            GDK_RGB_DITHER_NONE,
            FBDir.GetCurPtr(0,0), FBDir.GetRowStride());
        PrintStatus();
        if (fIntegrationTime != sResultHeader.fIntegrationTime)
        {
            fIntegrationTime = sResultHeader.fIntegrationTime;
            gtk_ruler_set_range(GTK_RULER(gwVRTime), 0.0,
                fIntegrationTime / 60.0 * iHistoryLines, 0.0,
                fIntegrationTime / 60.0 * iHistoryLines);
            g_snprintf(cpConvBuf, DGUI_CONV_BUF_SIZE, "%g",
                fIntegrationTime);
            gtk_entry_set_text(GTK_ENTRY(gwEIntegrationTime), cpConvBuf);
        }
    }
    else if (iReadRes > 0)
    {
        g_print("Error %i while reading result data; disconnecting\n\n",
            SOp->GetErrno());
        gdk_input_remove(giGdkTag);
        SOp->Shutdown(2);
        delete SOp;
        bConnected = false;
    }
    else
    {
        g_print("Warning: clGUIDir::OnGdkInput() called without data,\n");
        g_print("         server disconnected?\n");
        gdk_input_remove(giGdkTag);
        delete SOp;
        bConnected = false;
    }
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
    G_UNLOCK(gmInputMutex);
}

