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


#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <float.h>

#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>

#include "GUILocate.hh"


G_LOCK_DEFINE_STATIC(gmInputMutex);

static const char *cpWindowTxt = "Locate";
// Table 1
static const char *cpLServerTxt = "Server";
static const char *cpBConnectTxt = "Connect";
// Table 2
static const char *cpLPaletteTxt = "Palette";
static const char *cpaLPaletteMenu[] = { "BW", "HSV", "Light", "Temp",
    "Dir", "Green", "Green2", "PureGreen", "WB" };

clGUILocate GUILocate;


int main (int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    return GUILocate.Main(&argc, &argv);
}


gboolean WrapOnDelete (GtkWidget *gwSender, GdkEvent *geEvent,
    gpointer gpData)
{
    return GUILocate.OnDelete(gwSender, geEvent, gpData);
}


void WrapOnConnectClick (GtkButton *gbSender, gpointer gpData)
{
    GUILocate.OnConnectClick(gbSender, gpData);
}


void WrapOnPaletteActivate (GtkMenuItem *gmiSender, gpointer gpData)
{
    GUILocate.OnPaletteActivate(gmiSender, gpData);
}


gboolean WrapOnLocateExpose (GtkWidget *gwSender, GdkEventExpose *geeEvent,
    gpointer gpData)
{
    return GUILocate.OnLocateExpose(gwSender, geeEvent, gpData);
}


gboolean WrapOnLocateMotion (GtkWidget *gwSender, GdkEventMotion *gemEvent,
    gpointer gpData)
{
    return GUILocate.OnLocateMotion(gwSender, gemEvent, gpData);
}


void WrapOnGdkInput (gpointer gpData, gint iSource,
    GdkInputCondition gicCondition)
{
    GUILocate.OnGdkInput(gpData, iSource, gicCondition);
}


bool clGUILocate::GetCfg ()
{
    Cfg.SetFileName(GUILOC_CFGFILE);
    if (Cfg.GetInt("Palette", &iPalette))
    {
        OnPaletteActivate(NULL, GINT_TO_POINTER(iPalette));
    }
    else
    {
        iPalette = 0;
        Pal.GenBW();
    }
    return true;
}


bool clGUILocate::Build ()
{
    gwWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gwWindow), cpWindowTxt);
    // shrink, grow, auto-shrink
    gtk_window_set_policy(GTK_WINDOW(gwWindow), TRUE, TRUE, FALSE);
    // homogenous, spacing
    gwVBox = gtk_vbox_new(FALSE, GUILOC_WSPACING);
    gtk_container_add(GTK_CONTAINER(gwWindow), gwVBox);
    gtk_widget_show(gwVBox);
    if (!BuildTable1()) return false;
    if (!BuildTable2()) return false;
    if (!BuildLocate()) return false;
    if (!ConnectSignals()) return false;
    gtk_widget_show(gwWindow);
    if (!BuildDrawingPrims()) return false;
    gdk_window_set_cursor(gwDALocate->window, gcCrossHair);
    return true;
}


bool clGUILocate::BuildTable1 ()
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
        GUILOC_WSPACING / 2, 0);
    gtk_widget_show(gwLServer);
    gwCServer = gtk_combo_new();
    gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        GUILOC_SERVER_MAXLEN);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        "127.0.0.1:30002");
    // table, child, left attach, right attach, top attach, bottom attach,
    // x-options, y-options, x-padding, y-padding
    gtk_table_attach(GTK_TABLE(gwTable1), gwCServer,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        GUILOC_WSPACING / 2, 0);
    gtk_widget_show(gwCServer);
    GtkUtils.ComboListFromFile(gwCServer, &glServer, GUILOC_HOSTFILE);

    // - Button: Connect
    gwBConnect = gtk_button_new_with_label(cpBConnectTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBConnect,
        1, 2, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        GUILOC_WSPACING / 2, 0);
    gtk_widget_show(gwBConnect);

    return true;
}


bool clGUILocate::BuildTable2 ()
{
    // rows, columns, homogenous
    gwTable2 = gtk_table_new(2, 1, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable2, FALSE, FALSE, 0);
    gtk_widget_show(gwTable2);

    // Label & OptionMenu: Palette
    gwLPalette = gtk_label_new(cpLPaletteTxt);
    gtk_label_set_justify(GTK_LABEL(gwLPalette), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLPalette,
        0, 1, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        GUILOC_WSPACING / 2, 0);
    gtk_widget_show(gwLPalette);
    gwOMPalette = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwOMPalette,
        0, 1, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        GUILOC_WSPACING / 2, 0);
    gtk_widget_show(gwOMPalette);
    GtkUtils.BuildOptionMenu(gwOMPalette, &gwMPalette, gwaMIPalette,
        cpaLPaletteMenu, GUILOC_PALETTE_ITEMS);
    gtk_option_menu_set_history(GTK_OPTION_MENU(gwOMPalette),
        (guint) iPalette);
        
    return true;
}


bool clGUILocate::BuildLocate ()
{
    gwSWLocate = gtk_scrolled_window_new(NULL, NULL);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwSWLocate, TRUE, TRUE, 0);
    // hpolicy, vpolicy
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gwSWLocate),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_show(gwSWLocate);

    gwDALocate = gtk_drawing_area_new();
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(gwSWLocate),
        gwDALocate);
    gtk_widget_show(gwDALocate);

    return true;
}


bool clGUILocate::ConnectSignals ()
{
    int iWidgetCntr;

    gtk_signal_connect(GTK_OBJECT(gwWindow), "delete-event",
        GTK_SIGNAL_FUNC(WrapOnDelete), NULL);

    gtk_signal_connect(GTK_OBJECT(gwBConnect), "clicked",
        GTK_SIGNAL_FUNC(WrapOnConnectClick), NULL);

    gtk_widget_add_events(gwDALocate, GDK_POINTER_MOTION_MASK);
    gtk_signal_connect(GTK_OBJECT(gwDALocate), "expose-event",
        GTK_SIGNAL_FUNC(WrapOnLocateExpose), NULL);
    gtk_signal_connect(GTK_OBJECT(gwDALocate), "motion-notify-event",
        GTK_SIGNAL_FUNC(WrapOnLocateMotion), NULL);

    for (iWidgetCntr = 0; iWidgetCntr < GUILOC_PALETTE_ITEMS; iWidgetCntr++)
    {
        gtk_signal_connect(GTK_OBJECT(gwaMIPalette[iWidgetCntr]), "activate",
            GTK_SIGNAL_FUNC(WrapOnPaletteActivate), 
            GINT_TO_POINTER(iWidgetCntr));
    }

    return true;
}


bool clGUILocate::BuildDrawingPrims ()
{
    ggcLocateBG = gdk_gc_new(gwDALocate->window);
    gdk_rgb_gc_set_foreground(ggcLocateBG, GUILOC_LOCATE_BG);
    gdk_rgb_gc_set_background(ggcLocateBG, GUILOC_LOCATE_BG);
    gdk_gc_set_function(ggcLocateBG, GDK_COPY);
    gdk_gc_set_fill(ggcLocateBG, GDK_SOLID);

    ggcLocateFG = gdk_gc_new(gwDALocate->window);
    gdk_rgb_gc_set_foreground(ggcLocateFG, GUILOC_LOCATE_FG);
    gdk_rgb_gc_set_background(ggcLocateFG, GUILOC_LOCATE_BG);
    gdk_gc_set_function(ggcLocateFG, GDK_COPY);
    gdk_gc_set_fill(ggcLocateFG, GDK_SOLID);

    gcCrossHair = gdk_cursor_new(GDK_CROSSHAIR);

    return true;
}


bool clGUILocate::ConnectToServer (const char *cpHostAddr, int iHostPort)
{
    int iSockH;
    char cpHeader[GLOBAL_HEADER_LEN];

    iSockH = SClient.Connect(cpHostAddr, NULL, iHostPort);
    if (iSockH < 0) return false;
    SOp.SetHandle(iSockH);
    if (SOp.ReadN(cpHeader, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
    {
        g_print("Header receive failed!\n");
        return false;
    }
    Msg.GetHeader(cpHeader, &sHdr);
    return true;
}


void clGUILocate::DisplayResults ()
{
    int iValue;
    int iMaxValue;
    long lDataCntr;
    long lDataCount = sRes.lPointCount;
    unsigned int *ipRes = ResFrame;
    GDT *fpRes = ResMatrix;
    GtkAdjustment *gaHScrollBar;
    GtkAdjustment *gaVScrollBar;

    iMaxValue = Pal.Size() - 1;
    for (lDataCntr = 0; lDataCntr < lDataCount; lDataCntr++)
    {
        iValue = (int) (fpRes[lDataCntr] * iMaxValue + (GDT) 0.5);
        if (iValue < 0) iValue = 0;
        else if (iValue > iMaxValue) iValue = iMaxValue;
        ipRes[lDataCntr] = Pal[iValue];
    }
    gaHScrollBar = gtk_scrolled_window_get_hadjustment(
        GTK_SCROLLED_WINDOW(gwSWLocate));
    gaVScrollBar = gtk_scrolled_window_get_vadjustment(
        GTK_SCROLLED_WINDOW(gwSWLocate));
    if (gaHScrollBar == NULL || gaVScrollBar == NULL)
    {
        gdk_draw_rgb_32_image(gwDALocate->window, ggcLocateFG,
            0, 0, sHdr.iWidth, sHdr.iHeight,
            GDK_RGB_DITHER_NONE,
            ResFrame,
            sHdr.iWidth * sizeof(unsigned int));
    }
    else
    {
        int iPosX = (int) gaHScrollBar->value;
        int iPosY = (int) gaVScrollBar->value;
        int iPosW = (int) gaHScrollBar->page_size;
        int iPosH = (int) gaVScrollBar->page_size;

        if (iPosX >= sHdr.iWidth || iPosY >= sHdr.iHeight) return;
        if (iPosX + iPosW > sHdr.iWidth) iPosW = sHdr.iWidth - iPosX;
        if (iPosY + iPosH > sHdr.iHeight) iPosH = sHdr.iHeight - iPosY;

        gdk_draw_rgb_32_image(gwDALocate->window, ggcLocateFG,
            iPosX, iPosY, iPosW, iPosH,
            GDK_RGB_DITHER_NONE,
            (guchar *) &ipRes[iPosY * sHdr.iWidth + iPosX],
            sHdr.iWidth * sizeof(unsigned int));
    }
}


clGUILocate::clGUILocate ()
{
    bRun = true;
    bConnected = false;
}


clGUILocate::~clGUILocate ()
{
}


int clGUILocate::Main (int *ipArgC, char ***cppArgV)
{
    g_print("%s GUI v%i.%i.%i\n", cpWindowTxt,
        GUILOC_VER_MAJ, GUILOC_VER_MIN, GUILOC_VER_PL);
    g_print("Copyright (C) 2000-2001 Jussi Laako\n\n");
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


gboolean clGUILocate::OnDelete (GtkWidget *gwSender, GdkEvent *geEvent,
    gpointer gpData)
{
    bRun = false;
    gtk_main_quit();
    return TRUE;
}


void clGUILocate::OnConnectClick (GtkButton *gbSender, gpointer gpData)
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
            g_print("OK\n");
            iMsgSize = GLOBAL_HEADER_LEN + 
                sHdr.iWidth * sHdr.iHeight * sizeof(GDT);
            ResMsg.Size(iMsgSize);
            ResMatrix.Size(sHdr.iWidth * sHdr.iHeight * sizeof(GDT));
            ResFrame.Size(sHdr.iWidth * sHdr.iHeight * sizeof(unsigned int));
            gtk_drawing_area_size(GTK_DRAWING_AREA(gwDALocate),
                sHdr.iWidth, sHdr.iHeight);
            iGdkInputTag = gdk_input_add(SOp.GetHandle(), GDK_INPUT_READ,
                WrapOnGdkInput, NULL);
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


void clGUILocate::OnPaletteActivate (GtkMenuItem *gmiSender, gpointer gpData)
{
    iPalette = GPOINTER_TO_INT(gpData);
    switch (iPalette)
    {
        case GUILOC_PAL_BW:
            Pal.GenBW();
            break;
        case GUILOC_PAL_HSV:
            Pal.GenHSV();
            break;
        case GUILOC_PAL_LIGHT:
            Pal.GenLight();
            break;
        case GUILOC_PAL_TEMP:
            Pal.GenTemp();
            break;
        case GUILOC_PAL_DIR:
            Pal.GenDir();
            break;
        case GUILOC_PAL_GREEN:
            Pal.GenGreen();
            break;
        case GUILOC_PAL_GREEN2:
            Pal.GenGreen2();
            break;
        case GUILOC_PAL_PUREGREEN:
            Pal.GenPureGreen();
            break;
        case GUILOC_PAL_WB:
            Pal.GenWB();
            break;
        default:
            g_print("Error: Unknown palette\n");
    }
}


gboolean clGUILocate::OnLocateExpose (GtkWidget *gwSender, 
    GdkEventExpose *geeEvent, gpointer gpData)
{
    int iAreaW;
    int iAreaH;
    unsigned int *ipFrame = ResFrame;

    if (geeEvent->area.x >= sHdr.iWidth ||
        geeEvent->area.y >= sHdr.iHeight) return TRUE;

    if ((geeEvent->area.x + geeEvent->area.width) <= sHdr.iWidth) 
        iAreaW = geeEvent->area.width;
    else
        iAreaW = sHdr.iWidth - geeEvent->area.x;

    if ((geeEvent->area.y + geeEvent->area.height) <= sHdr.iHeight)
        iAreaH = geeEvent->area.height;
    else
        iAreaH = sHdr.iHeight - geeEvent->area.y;

    gdk_draw_rgb_32_image(gwDALocate->window, ggcLocateFG,
        geeEvent->area.x, geeEvent->area.y, 
        iAreaW, iAreaH, 
        GDK_RGB_DITHER_NONE,
        (guchar *) &ipFrame[geeEvent->area.y * sHdr.iWidth + geeEvent->area.x],
        sHdr.iWidth * sizeof(unsigned int));
    return TRUE;
}


gboolean clGUILocate::OnLocateMotion (GtkWidget *gwSender, 
    GdkEventMotion *gemEvent, gpointer gpData)
{
    return TRUE;
}


void clGUILocate::OnGdkInput (gpointer gpData, gint iSource,
    GdkInputCondition gicCondition)
{
    if (!bRun) return;
    G_LOCK(gmInputMutex);
    while (SOp.ReadSelect(0))
    {
        if (SOp.ReadN(ResMsg, iMsgSize) < iMsgSize)
        {
            gdk_input_remove(iGdkInputTag);
            SOp.Close();
            bConnected = false;
            g_print(
                "Error occurred while receiving data, dropped connection.");
            break;
        }
        Msg.GetResult(ResMsg, &sRes, (GDT *) ResMatrix);
        DisplayResults();
    }
    G_UNLOCK(gmInputMutex);
}

