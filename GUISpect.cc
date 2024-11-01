/*

    Transient spectrum GUI
    Copyright (C) 1999-2003 Jussi Laako

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
#include <time.h>
#include <errno.h>
#include <signal.h>
#ifdef USE_BACKING_STORE
    #include <X11/Xlib.h>
#endif
#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>
#ifdef USE_BACKING_STORE
    #include <gdk/gdkprivate.h>
#endif
#include "GUISpect.hh"


G_LOCK_DEFINE_STATIC(gmInputMutex);

static const char *cpWindowTxt = "Spectrogram";
static const char *cpLServerTxt = "Server";
static const char *cpaLChannelTxt[] = { "Channel", "Direction" };
static const char *cpBConnectTxt = "Connect";
static const char *cpBDisconnectTxt = "Disconnect";
static const char *cpCBFreezeTxt = "Freeze";
static const char *cpLTypeTxt = "Type";
static const char *cpaLTypeMenu[] = { "STFT", "(MR-STFT)", "(Gabor)",
    "(WVD)", "Hankel", "Autocorr.", "Cepstrum" };
static const char *cpLWindowTxt = "Window";
static const char *cpaLWindowMenu[] = { "Rectangle", "Bartlett",
    "Blackman", "Blackman-Harris", "Cosine tapered", "Exponential",
    "Flat top", "Generic cosine", "Hamming", "Hanning", "Kaiser",
    "Kaiser-Bessel", "Tukey" };
static const char *cpLWindowParamTxt = "Window param.";
static const char *cpLWindowLenTxt = "Window length";
static const char *cpaLWindowLenMenu[] = { "128", "256", "512", "1024",
    "2048", "4096", "8192", "16384", "32768", "65536" };
static const char *cpLLowFreqTxt = "Lower freq.";
static const char *cpLHighFreqTxt = "Higher freq.";
static const char *cpLGainTxt = "Gain dB";
static const char *cpLSlopeTxt = "Gain dB/oct";
static const char *cpLOverlapTxt = "Overlap (%)";
static const char *cpLLinearTxt = "Linear scale";
static const char *cpLNormalizeTxt = "Normalize";
//static const char *cpLApplyTxt = "Apply";
static const char *cpLRemoveNoiseTxt = "Remove noise";
static const char *cpaLRemoveNoiseMenu[] = { "None", "TPSW", "OTA",
    "Diff", "InvDiff" };
static const char *cpLAlphaTxt = "Alpha";
static const char *cpLMeanLengthTxt = "Mean length";
static const char *cpLGapLengthTxt = "Gap length";
static const char *cpLDynRangeTxt = "Dynamic range (dB)";
static const char *cpLPaletteTxt = "Palette";
static const char *cpaLPaletteMenu[] = { "BW", "HSV", "Light", "Temp",
    "Dir", "Green", "Green2", "PureGreen", "WB" };
static const char *cpBSaveTxt = "Save";
static const char *cpFSSaveTxt = "Save to TIFF file";

clSpectGUI *SpectGUI;


int main(int argc, char *argv[])
{
    int iTempRetVal;
    
    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    SpectGUI = new clSpectGUI(&argc, &argv);
    iTempRetVal = SpectGUI->Exec();
    delete SpectGUI;
    return iTempRetVal;
}


gint WrapOnDelete(GtkWidget *gwSender, GdkEventAny *geaEvent)
{
    return SpectGUI->OnDelete(gwSender, geaEvent);
}


void WrapOnHideToggled(GtkToggleButton *gtbSender, gpointer *gpData)
{
    SpectGUI->OnHideToggled(gtbSender, gpData);
}


gint WrapOnConnectClick(GtkWidget *gwSender, gpointer gpData)
{
    return SpectGUI->OnConnectClick(gwSender, gpData);
}


void WrapOnFreezeToggled(GtkToggleButton *gtbSender, gpointer *gpData)
{
    SpectGUI->OnFreezeToggled(gtbSender, gpData);
}


gint WrapOnPaletteActivate(GtkWidget *gwSender, gpointer gpData)
{
    return SpectGUI->OnPaletteActivate(gwSender, gpData);
}


gint WrapOnMotionSgram(GtkWidget *gwSender, GdkEventMotion *gemEvent)
{
    return SpectGUI->OnMotionSgram(gwSender, gemEvent);
}


gint WrapOnMotionSpect(GtkWidget *gwSender, GdkEventMotion *gemEvent)
{
    return SpectGUI->OnMotionSpect(gwSender, gemEvent);
}


gint WrapOnExposeSgram(GtkWidget *gwSender, GdkEventExpose *geeEvent)
{
    return SpectGUI->OnExposeSgram(gwSender, geeEvent);
}


gint WrapOnExposeSpect(GtkWidget *gwSender, GdkEventExpose *geeEvent)
{
    return SpectGUI->OnExposeSpect(gwSender, geeEvent);
}


gboolean WrapOnConfigure(GtkWidget *gwSender, GdkEventConfigure *gecEvent,
    gpointer gpData)
{
    return SpectGUI->OnConfigure(gwSender, gecEvent, gpData);
}


void WrapOnSizeAllocate(GtkWidget *gwSender, GtkAllocation *gaAllocation,
    gpointer gpData)
{
    SpectGUI->OnSizeAllocate(gwSender, gaAllocation, gpData);
}


void WrapGdkInput(gpointer gpData, gint giSource, 
    GdkInputCondition gicCondition)
{
    SpectGUI->GdkInput(gpData, giSource, gicCondition);
}


void WrapOnSaveClicks(GtkButton *gbSender, gpointer gpData)
{
    SpectGUI->OnSaveClicks(gbSender, gpData);
}


clSpectGUI::clSpectGUI(int *ipArgCount, char ***cpaArgVectP)
{
    bRun = true;
    bConnected = false;
    bFreezed = false;
    cpRcvMsgBuf = NULL;
    fpSpect = NULL;
    glServers = NULL;
    g_print("%s GUI v%i.%i.%i\n", cpWindowTxt, 
        SGUI_VER_MAJ, SGUI_VER_MIN, SGUI_VER_PL);
    g_print("Copyright (C) 1999-2002 Jussi Laako\n\n");
    g_print("Gtk+ version %i.%i.%i\n", gtk_major_version, gtk_minor_version,
        gtk_micro_version);
    g_print("Locale set to %s\n", gtk_set_locale());
    gtk_init(ipArgCount, cpaArgVectP);
    gdk_rgb_init();
    gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
    gtk_widget_set_default_visual(gdk_rgb_get_visual());
    CfgFile = new clCfgFile(SGUI_CFGFILE);
    if (!CfgFile->GetInt("Fit", &iFit))
        iFit = 0;
    if (!CfgFile->GetInt("Palette", &iPalette))
        iPalette = 0;
    if (!CfgFile->GetInt("TIFFCompression", &iTIFFCompression))
        iTIFFCompression = 0;
    if (!CfgFile->GetInt("JPEGQuality", &iJPEGQuality))
        iJPEGQuality = 100;
    if (!CfgFile->GetInt("BeamCount", &iBeamCount))
        iBeamCount = 0;
}


clSpectGUI::~clSpectGUI()
{
    if (bConnected) delete SockOp;
    delete CfgFile;
}


int clSpectGUI::Exec()
{
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


gint clSpectGUI::OnDelete(GtkWidget *gwSender, GdkEventAny *geaEvent)
{
    bRun = false;
    gtk_main_quit();
    return 0;
}


void clSpectGUI::OnHideToggled(GtkToggleButton *gtbSender, gpointer gpData)
{
    if (gtk_toggle_button_get_active(gtbSender))
    {
        gtk_widget_hide(gwTable1);
        gtk_widget_hide(gwTable2);
        gtk_widget_hide(gwTable3);
    }
    else
    {
        gtk_widget_show(gwTable1);
        gtk_widget_show(gwTable2);
        gtk_widget_show(gwTable3);
    }
}


gint clSpectGUI::OnConnectClick(GtkWidget *gwSender, gpointer gpData)
{
    char cpHost[SGUI_SERVER_MAXLEN + 1];
    int iPort;

    if (bConnected)
    {
        gdk_input_remove(giGdkTag);
        delete SockOp;
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


void clSpectGUI::OnFreezeToggled(GtkToggleButton *gtbSender, gpointer gpData)
{
    bFreezed = (gtk_toggle_button_get_active(gtbSender)) ? true : false;
}


gint clSpectGUI::OnPaletteActivate(GtkWidget *gwSender, gpointer gpData)
{
    int iPalType;

    if (gwSender == gwaMIPalette[SGUI_PAL_BW])
    {
        FrameBuf.PalGenBW();
    }
    else if (gwSender == gwaMIPalette[SGUI_PAL_HSV])
    {
        FrameBuf.PalGenHSV();
    }
    else if (gwSender == gwaMIPalette[SGUI_PAL_LIGHT])
    {
        FrameBuf.PalGenLight();
    }
    else if (gwSender == gwaMIPalette[SGUI_PAL_TEMP])
    {
        FrameBuf.PalGenTemp();
    }
    else if (gwSender == gwaMIPalette[SGUI_PAL_DIR])
    {
        FrameBuf.PalGenDir();
    }
    else if (gwSender == gwaMIPalette[SGUI_PAL_GREEN])
    {
        FrameBuf.PalGenGreen();
    }
    else if (gwSender == gwaMIPalette[SGUI_PAL_GREEN2])
    {
        FrameBuf.PalGenGreen2();
    }
    else if (gwSender == gwaMIPalette[SGUI_PAL_PUREGREEN])
    {
        FrameBuf.PalGenPureGreen();
    }
    else if (gwSender == gwaMIPalette[SGUI_PAL_WB])
    {
        FrameBuf.PalGenWB();
    }
    else
    {
        CfgFile->GetInt("Palette", &iPalType);
        switch (iPalType)
        {
            case SGUI_PAL_BW:
                FrameBuf.PalGenBW();
                break;
            case SGUI_PAL_HSV:
                FrameBuf.PalGenHSV();
                break;
            case SGUI_PAL_LIGHT:
                FrameBuf.PalGenLight();
                break;
            case SGUI_PAL_TEMP:
                FrameBuf.PalGenTemp();
                break;
            case SGUI_PAL_DIR:
                FrameBuf.PalGenDir();
                break;
            case SGUI_PAL_GREEN:
                FrameBuf.PalGenGreen();
                break;
            default:
                FrameBuf.PalGenBW();
        }
    }

    return 0;
}


gint clSpectGUI::OnMotionSgram(GtkWidget *gwSender, GdkEventMotion *gemEvent)
{
    float fBand;
    float fSpectTime;
    char cpConvStr[SGUI_CONV_MAXLEN];

    fBand = sSResult.iHighFreq - sSResult.iLowFreq;
    fSpectTime = sSResult.fLineTime;
    fGramX = (float) (iGramW - gemEvent->x) * fSpectTime;
    fGramY = fBand - (fBand / (float) iGramH * (float) gemEvent->y);
    PrintRealTime();
    g_snprintf(cpConvStr, SGUI_CONV_MAXLEN,
        "%.1f s, %.1f Hz / %.1f Hz, %.1f dB / %.1f dB %i clips / %s",
        fGramX, fGramY, fSpectX, fSpectY, sSResult.fPeakLevel, iClips,
        cpGramXTime);
    gtk_statusbar_pop(GTK_STATUSBAR(gwStatusBar), guCtxtSB);
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guCtxtSB, cpConvStr);

    return 0;
}


gint clSpectGUI::OnMotionSpect(GtkWidget *gwSender, GdkEventMotion *gemEvent)
{
    float fBand;
    char cpConvStr[SGUI_CONV_MAXLEN];

    fBand = sSResult.iHighFreq - sSResult.iLowFreq;
    fSpectX = fBand / (float) iSpectPoints * (float) gemEvent->x;
    fSpectY = -((float) sSRequest.fDynRange / (float) iSpectH * 
        (float) gemEvent->y);
    g_snprintf(cpConvStr, SGUI_CONV_MAXLEN,
        "%.1f s, %.1f Hz / %.1f Hz, %.1f dB / %.1f dB %i clips / %s",
        fGramX, fGramY, fSpectX, fSpectY, sSResult.fPeakLevel, iClips,
        cpGramXTime);
    gtk_statusbar_pop(GTK_STATUSBAR(gwStatusBar), guCtxtSB);
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guCtxtSB, cpConvStr);

    return 0;
}


gint clSpectGUI::OnExposeSgram(GtkWidget *gwSender, GdkEventExpose *geeEvent)
{
    int iWidth;
    int iHeight;

    if (!bRun) return FALSE;
    iWidth = ((geeEvent->area.x + geeEvent->area.width) > iGramW) ?
        iGramW - geeEvent->area.x : geeEvent->area.width;
    iHeight = ((geeEvent->area.y + geeEvent->area.height) > iGramH) ?
        iGramH - geeEvent->area.y : geeEvent->area.height;
    gdk_draw_rgb_32_image(geeEvent->window, ggcGramFG,
        geeEvent->area.x + 1, geeEvent->area.y,  // there should be no + 1...
        iWidth, iHeight,
        GDK_RGB_DITHER_NONE,
        FrameBuf.GetCurPtr(geeEvent->area.x, geeEvent->area.y),
        FrameBuf.GetRowStride());
    return TRUE;
}


gint clSpectGUI::OnExposeSpect(GtkWidget *gwSender, GdkEventExpose *geeEvent)
{
    int iLineCntr;
    int iMaxLine;
    int iLineLength;
    
    if (!bRun) return FALSE;
    gdk_window_clear_area(geeEvent->window,
        geeEvent->area.x, geeEvent->area.y,
        geeEvent->area.width, geeEvent->area.height);
    // Draw visible part of spectrum
    if (fpSpect == NULL) return FALSE;
    iMaxLine = 
        (iSpectPoints <= (geeEvent->area.x + geeEvent->area.width)) ?
        iSpectPoints : geeEvent->area.x + geeEvent->area.width;
    for (iLineCntr = geeEvent->area.x; iLineCntr < iMaxLine; iLineCntr++)
    {
        iLineLength = (int) (fpSpect[iLineCntr] * 
            (GDT) (geeEvent->area.height - 1) + 0.5);
        if (iLineLength < 0) iLineLength = 0;
        if (iLineLength >= geeEvent->area.height)
            iLineLength = geeEvent->area.height;
        gdk_draw_line(geeEvent->window, ggcSpectFG,
            iLineCntr, geeEvent->area.height - 1,
            iLineCntr, geeEvent->area.height - iLineLength);
    }
    return TRUE;
}


gboolean clSpectGUI::OnConfigure(GtkWidget *gwSender, 
    GdkEventConfigure *gecEvent, gpointer gpData)
{
    int iRealWidth;
    int iRealHeight;

    if (gwSender == gwDASpectrogram)
    {
        iRealWidth = gecEvent->width - 1;
        iRealHeight = gecEvent->height - 1;
        if (iGramW != iRealWidth || (iGramH != iRealHeight && iFit))
        {
            iGramW = iRealWidth;
            if (iFit)
                iGramH = iRealHeight;
            ReConfigDisplay();
        }
    }
    else if (gwSender == gwDASpectrum)
    {
        if (iSpectW <= 0) iSpectW = gecEvent->width;
        if (iSpectH != gecEvent->height)
        {
            iSpectH = gecEvent->height;
            ReConfigDisplay();
        }
    }
    return TRUE;
}


void clSpectGUI::OnSizeAllocate(GtkWidget *gwSender, 
    GtkAllocation *gaAllocation, gpointer gpData)
{
    int iRealWidth;
    int iRealHeight;

    if (gwSender == gwDASpectrogram)
    {
        iRealWidth = gaAllocation->width - 1;
        iRealHeight = gaAllocation->height - 1;
        if (iGramW != iRealWidth || (iGramH != iRealHeight && iFit))
        {
            iGramW = iRealWidth;
            if (iFit)
                iGramH = iRealHeight;
            ReConfigDisplay();
        }
    }
    else if (gwSender == gwDASpectrum)
    {
        if (iSpectW <= 0) iSpectW = gaAllocation->width;
        if (iSpectH != gaAllocation->height)
        {
            iSpectH = gaAllocation->height;
            ReConfigDisplay();
        }
    }
    else if (gwSender == gwScrolledW2)
    {
        if (!GTK_SCROLLED_WINDOW(gwSender)->hscrollbar_visible)
        {
            iRealHeight = gaAllocation->height - gwHRFreq->requisition.height;
        }
        else
        {
            iRealHeight = gaAllocation->height - gwHRFreq->requisition.height -
                GTK_SCROLLED_WINDOW(gwSender)->hscrollbar->requisition.height;
        }
        if (iSpectH != gaAllocation->height)
        {
            iSpectH = iRealHeight;
            ReConfigDisplay();
        }
    }
}


void clSpectGUI::GdkInput(gpointer gpData, gint giSource, 
    GdkInputCondition gicCondition)
{
    bool bNewData = false;
    char cpConvStr[SGUI_CONV_MAXLEN];
    stSpectRes sSResNew;

    if (!bRun) return;
    G_LOCK(gmInputMutex);
    while (SockOp->ReadSelect(0))
    {
        if (!bFreezed) bNewData = true;
        if (SockOp->ReadN(cpRcvMsgBuf, iRcvMsgSize) < iRcvMsgSize)
        {
            if (SockOp->GetErrno() == EPIPE || SockOp->GetErrno() == 0) 
                g_print("Server disconnected\n");
            else 
                g_print("Receive error: %s\n", strerror(SockOp->GetErrno()));
            gdk_input_remove(giGdkTag);
            delete SockOp;
            bConnected = false;
            G_UNLOCK(gmInputMutex);
            return;
        }
        if (bFreezed) continue;
        SpectMsg.GetResult(cpRcvMsgBuf, &sSResNew, fpSpect);
        if (sSResNew.lLength != sSResult.lLength) bReConfig = true;
        if (sSResNew.iLowFreq != sSResult.iLowFreq) bReConfig = true;
        if (sSResNew.iHighFreq != sSResult.iHighFreq) bReConfig = true;
        if (sSResNew.bLinear != sSResult.bLinear) bReConfig = true;
        memcpy(&sSResult, &sSResNew, sizeof(stSpectRes));
        if (bReConfig)
        {
            g_snprintf(cpConvStr, SGUI_CONV_MAXLEN, "%i", sSResult.iLowFreq);
            gtk_entry_set_text(GTK_ENTRY(gwELowFreq), cpConvStr);
            g_snprintf(cpConvStr, SGUI_CONV_MAXLEN, "%i", sSResult.iHighFreq);
            gtk_entry_set_text(GTK_ENTRY(gwEHighFreq), cpConvStr);
            ReConfigDisplay();
        }
        if (sSResult.fPeakLevel >= 0.0F) iClips++;
        DrawSpectrogram();
        if (bReConfig) break;
    }
    if (bNewData)
    {
        DrawSpectrum();
        PrintRealTime();
        g_snprintf(cpConvStr, SGUI_CONV_MAXLEN,
            "%.1f s, %.1f Hz / %.1f Hz, %.1f dB / %.1f dB %i clips / %s",
            fGramX, fGramY, fSpectX, fSpectY, sSResult.fPeakLevel, iClips,
            cpGramXTime);
        gtk_statusbar_pop(GTK_STATUSBAR(gwStatusBar), guCtxtSB);
        gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guCtxtSB, cpConvStr);
    }
    G_UNLOCK(gmInputMutex);
}


void clSpectGUI::OnSaveClicks (GtkButton *gbSender, gpointer gpData)
{
    int iAction = GPOINTER_TO_INT(gpData);
    int iCompressMode;
    const char *cpFileName;

    switch (iAction)
    {
        case 0:
            gtk_widget_hide(gwFSSave);
            break;
        case 1:
            gtk_widget_hide(gwFSSave);
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
            cpFileName = 
                gtk_file_selection_get_filename(GTK_FILE_SELECTION(gwFSSave));
            /*if (!FrameBuf.SaveToFile(cpFileName, iCompressMode, iJPEGQuality, 
                "Spectrogram",
                -(sSResult.fLineTime * iGramW), 0,
                sSResult.iLowFreq, sSResult.iHighFreq))*/
            if (!FrameBuf.SaveToFile(cpFileName, iCompressMode, iJPEGQuality, 
                "Spectrogram"))
            {
                g_warning("Saving to file failed!");
            }
            SaveInfo(cpFileName);
            break;
        case 2:
            gtk_widget_show(gwFSSave);
            break;
    }
}


bool clSpectGUI::Build()
{
    int iDefWidth;
    int iDefHeight;
    int iYCntr;
    int iXCntr;
    int iPixIdx;
    unsigned int *upPalPtr;
    unsigned int *upFBPtr;
    char cpConvStr[SGUI_CONV_MAXLEN];

    // - Main window
    gwWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gwWindow), cpWindowTxt);
    // shrink, grow, auto-shrink
    gtk_window_set_policy(GTK_WINDOW(gwWindow), TRUE, TRUE, FALSE);
    if (CfgFile->GetInt("Width", &iDefWidth) && 
        CfgFile->GetInt("Height", &iDefHeight))
    {
        if ((iDefWidth > 0 && iDefHeight > 0) &&
            (iDefWidth <= gdk_screen_width() && 
            iDefHeight <= gdk_screen_height()))
        {
            gtk_window_set_default_size(GTK_WINDOW(gwWindow),
                iDefWidth, iDefHeight);
        }
        else
        {
            gtk_window_set_default_size(GTK_WINDOW(gwWindow),
                gdk_screen_width() - SGUI_PADDING * 2, 
                gdk_screen_height() - SGUI_PADDING * 2);
        }
    }
    else
    {
        gtk_window_set_default_size(GTK_WINDOW(gwWindow),
            SGUI_DEF_WIDTH, SGUI_DEF_HEIGHT);
    }

    // --- Vertical box
    // homogenous, spacing
    gwVBox = gtk_vbox_new(FALSE, SGUI_PADDING);
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

    // --- VerticalPaned
    gwVPaned = gtk_vpaned_new();
    gtk_paned_set_gutter_size(GTK_PANED(gwVPaned), SGUI_PADDING * 2);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwVPaned, TRUE, TRUE, 0);
    gtk_widget_show(gwVPaned);

    // --- ScrolledWindow 1: Spectrogram (TableGram)
    gwScrolledW1 = gtk_scrolled_window_new(NULL, NULL);
    if (iFit)
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gwScrolledW1),
            GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    else
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gwScrolledW1),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    // paned, child, resize, shrink
    gtk_paned_pack1(GTK_PANED(gwVPaned), gwScrolledW1, TRUE, TRUE);
    gtk_widget_show(gwScrolledW1);

    // --- ScrolledWindow 2: Spectrum (TableSpect)
    gwScrolledW2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gwScrolledW2),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    // paned, child, resize, shrink
    gtk_paned_pack2(GTK_PANED(gwVPaned), gwScrolledW2, TRUE, TRUE);
    gtk_widget_show(gwScrolledW2);

    // --- Table Gram
    if (!BuildTableGram()) return false;

    // --- Table Spect
    if (!BuildTableSpect()) return false;

    gwStatusBar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(gwVBox), gwStatusBar, FALSE, FALSE, 0);
    gtk_widget_show(gwStatusBar);
    guCtxtSB = gtk_statusbar_get_context_id(GTK_STATUSBAR(gwStatusBar),
        "status");
    g_snprintf(cpConvStr, SGUI_CONV_MAXLEN, 
        "%.1f s, %.1f Hz / %.1f Hz, %.1f dB / %.1f dB",
        fGramX, fGramY, fSpectX, fSpectY, sSResult.fPeakLevel);
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guCtxtSB, cpConvStr);

    // Delayed realization of main window
    gtk_widget_show(gwWindow);

    #ifdef USE_BACKING_STORE
        GdkWindowPrivate *gwpWindow = 
            (GdkWindowPrivate *) gwDASpectrogram->window;
        XSetWindowAttributes xswaAttrib;
        xswaAttrib.backing_store = Always;
        XChangeWindowAttributes(gwpWindow->xdisplay, gwpWindow->xwindow,
            CWBackingStore, &xswaAttrib);
    #endif

    // Build drawing primitives
    if (!BuildDrawingPrims()) return false;

    // Set default position for paned adjustment
    gtk_paned_set_position(GTK_PANED(gwVPaned), 
        gwVPaned->allocation.height / 8 * 7);

    // Set cursors
    gdk_window_set_cursor(gwDASpectrogram->window, gcCrossHair);
    gdk_window_set_cursor(gwDASpectrum->window, gcCrossHair);

    // Draw specified palette to framebuffer and screen
    iGramW = gwDASpectrogram->allocation.width;
    iGramH = FrameBuf.GetNumColors();
    sSResult.lLength = iGramH * 2 - 1;
    FrameBuf.SetSize(iGramW, iGramH);
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDASpectrogram),
        iGramW, iGramH);
    upPalPtr = FrameBuf.GetPalPtr();
    upFBPtr = FrameBuf.GetFBPtr();
    for (iYCntr = 0; iYCntr < iGramH; iYCntr++)
    {
        for (iXCntr = 0; iXCntr < iGramW; iXCntr++)
        {
            iPixIdx = iYCntr * iGramW + iXCntr;
            upFBPtr[iPixIdx] = upFBPtr[iGramW * iGramH + iPixIdx] =
                upPalPtr[iYCntr];
        }
    }

    return true;
}


bool clSpectGUI::BuildTable1()
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
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLServer);
    gwCServer = gtk_combo_new();
    gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        SGUI_SERVER_MAXLEN);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        "127.0.0.1:30001");
    // table, child, left attach, right attach, top attach, bottom attach,
    // x-options, y-options, x-padding, y-padding
    gtk_table_attach(GTK_TABLE(gwTable1), gwCServer,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwCServer);
    GtkUtils.ComboListFromFile(gwCServer, &glServers, SGUI_HOSTFILE);

    // - Label & SpinButton: Channel
    if (iBeamCount)
        gwLChannel = gtk_label_new(cpaLChannelTxt[1]);
    else
        gwLChannel = gtk_label_new(cpaLChannelTxt[0]);
    gtk_label_set_justify(GTK_LABEL(gwLChannel), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable1), gwLChannel,
        1, 2, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
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
        goAChannel = gtk_adjustment_new(1.0, SGUI_CH_LOWER, SGUI_CH_UPPER, 1.0,
            1.0, 1.0);
        // adjustment, climb rate, digits
        gwSBChannel = gtk_spin_button_new(GTK_ADJUSTMENT(goAChannel), 1.0, 0);
    }
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(gwSBChannel), TRUE);
    gtk_table_attach(GTK_TABLE(gwTable1), gwSBChannel,
        1, 2, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwSBChannel);

    // - Button: Connect
    gwBConnect = gtk_button_new_with_label(cpBConnectTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBConnect,
        2, 3, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwBConnect);

    // - Button: Disconnect
    gwBDisconnect = gtk_button_new_with_label(cpBDisconnectTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBDisconnect,
        3, 4, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwBDisconnect);

    // - CheckButton: Freeze
    gwCBFreeze = gtk_check_button_new_with_label(cpCBFreezeTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwCBFreeze,
        4, 5, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwCBFreeze);

    return true;
}


bool clSpectGUI::BuildTable2()
{
    // rows, columns, homogenous
    gwTable2 = gtk_table_new(2, 10, FALSE);  // columns = 11 if BApply
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable2, FALSE, FALSE, 0);
    gtk_widget_show(gwTable2);
    
    // - Label & OptionMenu: Type
    gwLType = gtk_label_new(cpLTypeTxt);
    gtk_label_set_justify(GTK_LABEL(gwLType), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLType,
        0, 1, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLType);
    gwOMType = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwOMType,
        0, 1, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMType);
    GtkUtils.BuildOptionMenu(gwOMType, &gwMType, gwaMIType, cpaLTypeMenu, 
        SGUI_TYPE_ITEMS);

    // - Label & OptionMenu: Window
    gwLWindow = gtk_label_new(cpLWindowTxt);
    gtk_label_set_justify(GTK_LABEL(gwLWindow), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLWindow,
        1, 2, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLWindow);
    gwOMWindow = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwOMWindow,
        1, 2, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMWindow);
    GtkUtils.BuildOptionMenu(gwOMWindow, &gwMWindow, gwaMIWindow, 
        cpaLWindowMenu, SGUI_WINDOW_ITEMS);

    // - Label & Entry: Window parameter
    gwLWindowParam = gtk_label_new(cpLWindowParamTxt);
    gtk_label_set_justify(GTK_LABEL(gwLWindowParam), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLWindowParam,
        2, 3, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK), 
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLWindowParam);
    gwEWindowParam = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEWindowParam,
        2, 3, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwEWindowParam);

    // - Label & OptionMenu: Window length
    gwLWindowLen = gtk_label_new(cpLWindowLenTxt);
    gtk_label_set_justify(GTK_LABEL(gwLWindowLen), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLWindowLen,
        3, 4, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLWindowLen);
    gwOMWindowLen = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwOMWindowLen,
        3, 4, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMWindowLen);
    GtkUtils.BuildOptionMenu(gwOMWindowLen, &gwMWindowLen, gwaMIWindowLen,
        cpaLWindowLenMenu, SGUI_WINLEN_ITEMS);

    // - Label & Entry: Lower frequency
    gwLLowFreq = gtk_label_new(cpLLowFreqTxt);
    gtk_label_set_justify(GTK_LABEL(gwLLowFreq), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLLowFreq,
        4, 5, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK), 
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLLowFreq);
    gwELowFreq = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwELowFreq,
        4, 5, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwELowFreq);

    // - Label & Entry: Higher frequency
    gwLHighFreq = gtk_label_new(cpLHighFreqTxt);
    gtk_label_set_justify(GTK_LABEL(gwLHighFreq), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLHighFreq,
        5, 6, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLHighFreq);
    gwEHighFreq = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEHighFreq,
        5, 6, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwEHighFreq);

    // - Label & Entry: Gain
    gwLGain = gtk_label_new(cpLGainTxt);
    gtk_label_set_justify(GTK_LABEL(gwLGain), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLGain,
        6, 7, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLGain);
    gwEGain = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEGain,
        6, 7, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwEGain);

    // - Label & Entry: Slope
    gwLSlope = gtk_label_new(cpLSlopeTxt);
    gtk_label_set_justify(GTK_LABEL(gwLSlope), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLSlope,
        7, 8, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLSlope);
    gwESlope = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwESlope,
        7, 8, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwESlope);

    // - Label & Entry: Overlap
    gwLOverlap = gtk_label_new(cpLOverlapTxt);
    gtk_label_set_justify(GTK_LABEL(gwLOverlap), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLOverlap,
        8, 9, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK), 
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLOverlap);
    gwEOverlap = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwEOverlap,
        8, 9, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK), 
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwEOverlap);

    // - CheckButton: Linear scale
    gwCBLinear = gtk_check_button_new_with_label(cpLLinearTxt);
    gtk_table_attach(GTK_TABLE(gwTable2), gwCBLinear,
        9, 10, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwCBLinear);

    // - CheckButton: Normalize display
    gwCBNormalize = gtk_check_button_new_with_label(cpLNormalizeTxt);
    gtk_table_attach(GTK_TABLE(gwTable2), gwCBNormalize,
        9, 10, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwCBNormalize);

    return true;
}


bool clSpectGUI::BuildTable3()
{
    // rows, columns, homogenous
    gwTable3 = gtk_table_new(2, 7, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable3, FALSE, FALSE, 0);
    gtk_widget_show(gwTable3);

    // - Label & OptionMenu: Remove noise
    gwLRemoveNoise = gtk_label_new(cpLRemoveNoiseTxt);
    gtk_label_set_justify(GTK_LABEL(gwLRemoveNoise), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLRemoveNoise,
        0, 1, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLRemoveNoise);
    gwOMRemoveNoise = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwOMRemoveNoise,
        0, 1, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMRemoveNoise);
    GtkUtils.BuildOptionMenu(gwOMRemoveNoise, &gwMRemoveNoise, 
        gwaMIRemoveNoise, cpaLRemoveNoiseMenu, SGUI_REMOVE_NOISE_ITEMS);

    // - Label & Entry: Alpha
    gwLAlpha = gtk_label_new(cpLAlphaTxt);
    gtk_label_set_justify(GTK_LABEL(gwLAlpha), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLAlpha,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLAlpha);
    gwEAlpha = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEAlpha,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwEAlpha);
    gtk_entry_set_text(GTK_ENTRY(gwEAlpha), "2.0");

    // - Label & Entry: Mean length
    gwLMeanLength = gtk_label_new(cpLMeanLengthTxt);
    gtk_label_set_justify(GTK_LABEL(gwLMeanLength), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLMeanLength,
        2, 3, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLMeanLength);
    gwEMeanLength = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEMeanLength,
        2, 3, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwEMeanLength);
    gtk_entry_set_text(GTK_ENTRY(gwEMeanLength), "10");

    // - Label & Entry: Gap length
    gwLGapLength = gtk_label_new(cpLGapLengthTxt);
    gtk_label_set_justify(GTK_LABEL(gwLGapLength), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLGapLength,
        3, 4, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLGapLength);
    gwEGapLength = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEGapLength,
        3, 4, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwEGapLength);
    gtk_entry_set_text(GTK_ENTRY(gwEGapLength), "3");

    // - Label & Entry: Dynamic range
    gwLDynRange = gtk_label_new(cpLDynRangeTxt);
    gtk_label_set_justify(GTK_LABEL(gwLDynRange), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLDynRange,
        4, 5, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLDynRange);
    gwEDynRange = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwEDynRange,
        4, 5, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwEDynRange);
    gtk_entry_set_text(GTK_ENTRY(gwEDynRange), "96.3296");

    // - Label & OptionMenu: Palette
    gwLPalette = gtk_label_new(cpLPaletteTxt);
    gtk_label_set_justify(GTK_LABEL(gwLPalette), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable3), gwLPalette,
        5, 6, 0, 1,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwLPalette);
    gwOMPalette = gtk_option_menu_new();
    gtk_table_attach(GTK_TABLE(gwTable3), gwOMPalette,
        5, 6, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwOMPalette);
    GtkUtils.BuildOptionMenu(gwOMPalette, &gwMPalette, gwaMIPalette, 
        cpaLPaletteMenu, SGUI_PALETTE_ITEMS);
    gtk_option_menu_set_history(GTK_OPTION_MENU(gwOMPalette), iPalette);

    // - Button: Save
    gwBSave = gtk_button_new_with_label(cpBSaveTxt);
    gtk_table_attach(GTK_TABLE(gwTable3), gwBSave,
        6, 7, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        SGUI_PADDING / 2, 0);
    gtk_widget_show(gwBSave);

    // - FileSelection: Save
    gwFSSave = gtk_file_selection_new(cpFSSaveTxt);

    return true;
}


bool clSpectGUI::BuildTableGram()
{
    // rows, columns, homogenous
    gwTableGram = gtk_table_new(2, 2, FALSE);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(gwScrolledW1),
        gwTableGram);
    gtk_widget_show(gwTableGram);

    // - HorizontalRuler: Time
    gwHRTime = gtk_hruler_new();
    gtk_ruler_set_metric(GTK_RULER(gwHRTime), GTK_PIXELS);
    // ruler, lower, upper, position, max size
    gtk_ruler_set_range(GTK_RULER(gwHRTime), -1.0, 0.0, 0.0, 0.0);
    gtk_table_attach(GTK_TABLE(gwTableGram), gwHRTime,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK), 
        (GtkAttachOptions) 0,
        0, 0);
    gtk_widget_show(gwHRTime);

    // - VerticalRuler: Frequency
    gwVRFreq = gtk_vruler_new();
    gtk_ruler_set_metric(GTK_RULER(gwVRFreq), GTK_PIXELS);
    gtk_ruler_set_range(GTK_RULER(gwVRFreq), 22.05, 0.0, 0.0, 22.05);
    gtk_table_attach(GTK_TABLE(gwTableGram), gwVRFreq,
        0, 1, 1, 2,
        (GtkAttachOptions) 0, 
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        0, 0);
    gtk_widget_show(gwVRFreq);

    // - DrawingArea: Spectrogram
    gwDASpectrogram = gtk_drawing_area_new();
    gtk_table_attach(GTK_TABLE(gwTableGram), gwDASpectrogram,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        0, 0);
    gtk_widget_show(gwDASpectrogram);

    return true;
}


bool clSpectGUI::BuildTableSpect()
{
    // rows, columns, homogenous
    gwTableSpect = gtk_table_new(2, 2, FALSE);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(gwScrolledW2),
        gwTableSpect);
    gtk_widget_show(gwTableSpect);

    // - HorizontalRuler: Frequency
    gwHRFreq = gtk_hruler_new();
    gtk_ruler_set_metric(GTK_RULER(gwHRFreq), GTK_PIXELS);
    // ruler, lower, upper, position, max size
    gtk_ruler_set_range(GTK_RULER(gwHRFreq), 0.0, 22.05, 0.0, 22.05);
    gtk_table_attach(GTK_TABLE(gwTableSpect), gwHRFreq,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        0, 0);
    gtk_widget_show(gwHRFreq);

    // - VerticalRuler: Level
    gwVRLevel = gtk_vruler_new();
    gtk_ruler_set_metric(GTK_RULER(gwVRLevel), GTK_PIXELS);
    gtk_ruler_set_range(GTK_RULER(gwVRLevel), 0.0, -96.0, 0.0, 0.0);
    gtk_table_attach(GTK_TABLE(gwTableSpect), gwVRLevel,
        0, 1, 1, 2,
        (GtkAttachOptions) 0,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        0, 0);
    gtk_widget_show(gwVRLevel);

    // - DrawingArea: Spectrum
    gwDASpectrum = gtk_drawing_area_new();
    gtk_table_attach(GTK_TABLE(gwTableSpect), gwDASpectrum,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        0, 0);
    gtk_widget_show(gwDASpectrum);

    return true;
}


bool clSpectGUI::BuildDrawingPrims()
{
    int iPalType;

    // Create graphics contexts
    ggcGramBG = gdk_gc_new(gwDASpectrogram->window);
    gdk_rgb_gc_set_foreground(ggcGramBG, SGUI_GRAM_BG);
    gdk_rgb_gc_set_background(ggcGramBG, SGUI_GRAM_BG);
    gdk_gc_set_function(ggcGramBG, GDK_COPY);
    gdk_gc_set_fill(ggcGramBG, GDK_SOLID);

    ggcGramFG = gdk_gc_new(gwDASpectrogram->window);
    gdk_rgb_gc_set_foreground(ggcGramFG, SGUI_GRAM_FG);
    gdk_rgb_gc_set_background(ggcGramFG, SGUI_GRAM_BG);
    gdk_gc_set_function(ggcGramFG, GDK_COPY);
    gdk_gc_set_fill(ggcGramFG, GDK_SOLID);

    ggcSpectBG = gdk_gc_new(gwDASpectrum->window);
    gdk_rgb_gc_set_foreground(ggcSpectBG, SGUI_SPECT_BG);
    gdk_rgb_gc_set_background(ggcSpectBG, SGUI_SPECT_BG);
    gdk_gc_set_function(ggcSpectBG, GDK_COPY);
    gdk_gc_set_fill(ggcSpectBG, GDK_SOLID);

    ggcSpectFG = gdk_gc_new(gwDASpectrum->window);
    gdk_rgb_gc_set_foreground(ggcSpectFG, SGUI_SPECT_FG);
    gdk_rgb_gc_set_background(ggcSpectFG, SGUI_SPECT_BG);
    gdk_gc_set_function(ggcSpectFG, GDK_COPY);
    gdk_gc_set_fill(ggcSpectFG, GDK_SOLID);

    // Create cursors
    gcCrossHair = gdk_cursor_new(GDK_CROSSHAIR);

    // Create specified palette for spectrogram
    CfgFile->GetInt("Palette", &iPalType);
    switch (iPalType)
    {
        case SGUI_PAL_BW:
            FrameBuf.PalGenBW();
            break;
        case SGUI_PAL_HSV:
            FrameBuf.PalGenHSV();
            break;
        case SGUI_PAL_LIGHT:
            FrameBuf.PalGenLight();
            break;
        case SGUI_PAL_TEMP:
            FrameBuf.PalGenTemp();
            break;
        case SGUI_PAL_DIR:
            FrameBuf.PalGenDir();
            break;
        case SGUI_PAL_GREEN:
            FrameBuf.PalGenGreen();
            break;
        default:
            FrameBuf.PalGenBW();
    }

    return true;
}


void clSpectGUI::FreeDrawingPrims()
{
    gdk_cursor_destroy(gcCrossHair);
    gdk_gc_destroy(ggcGramBG);
    gdk_gc_destroy(ggcGramFG);
    gdk_gc_destroy(ggcSpectBG);
    gdk_gc_destroy(ggcSpectFG);
}


bool clSpectGUI::ConnectSignals()
{
    int iWidgetCntr;
    
    gtk_widget_add_events(gwDASpectrogram, GDK_POINTER_MOTION_MASK);
    GtkUtils.ConnectMotionEvent(gwHRTime, gwDASpectrogram);
    GtkUtils.ConnectMotionEvent(gwVRFreq, gwDASpectrogram);
    gtk_widget_add_events(gwDASpectrum, GDK_POINTER_MOTION_MASK);
    GtkUtils.ConnectMotionEvent(gwHRFreq, gwDASpectrum);
    GtkUtils.ConnectMotionEvent(gwVRLevel, gwDASpectrum);
    gtk_signal_connect(GTK_OBJECT(gwWindow), "delete_event",
        GTK_SIGNAL_FUNC(WrapOnDelete), NULL);
    gtk_signal_connect(GTK_OBJECT(gwCBHide), "toggled",
        GTK_SIGNAL_FUNC(WrapOnHideToggled), NULL);
    gtk_signal_connect(GTK_OBJECT(gwBConnect), "clicked",
        GTK_SIGNAL_FUNC(WrapOnConnectClick), NULL);
    gtk_signal_connect(GTK_OBJECT(gwBDisconnect), "clicked",
        GTK_SIGNAL_FUNC(WrapOnConnectClick), NULL);
    gtk_signal_connect(GTK_OBJECT(gwCBFreeze), "toggled",
        GTK_SIGNAL_FUNC(WrapOnFreezeToggled), NULL);
    for (iWidgetCntr = 0; iWidgetCntr < SGUI_PALETTE_ITEMS; iWidgetCntr++)
    {
        gtk_signal_connect(GTK_OBJECT(gwaMIPalette[iWidgetCntr]), "activate",
            GTK_SIGNAL_FUNC(WrapOnPaletteActivate), NULL);
    }
    gtk_signal_connect(GTK_OBJECT(gwDASpectrogram), "motion_notify_event",
        GTK_SIGNAL_FUNC(WrapOnMotionSgram), NULL);
    gtk_signal_connect(GTK_OBJECT(gwDASpectrum), "motion_notify_event",
        GTK_SIGNAL_FUNC(WrapOnMotionSpect), NULL);
    gtk_signal_connect(GTK_OBJECT(gwDASpectrogram), "expose_event",
        GTK_SIGNAL_FUNC(WrapOnExposeSgram), NULL);
    gtk_signal_connect(GTK_OBJECT(gwDASpectrum), "expose_event",
        GTK_SIGNAL_FUNC(WrapOnExposeSpect), NULL);
    gtk_signal_connect(GTK_OBJECT(gwDASpectrogram), "size-allocate",
        GTK_SIGNAL_FUNC(WrapOnSizeAllocate), NULL);

    gtk_signal_connect(GTK_OBJECT(gwScrolledW2), "size-allocate",
        GTK_SIGNAL_FUNC(WrapOnSizeAllocate), NULL);

    gtk_signal_connect(GTK_OBJECT(gwBSave), "clicked",
        GTK_SIGNAL_FUNC(WrapOnSaveClicks), GINT_TO_POINTER(2));
    gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(gwFSSave)->ok_button),
        "clicked", GTK_SIGNAL_FUNC(WrapOnSaveClicks), GINT_TO_POINTER(1));
    gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(gwFSSave)->cancel_button),
        "clicked", GTK_SIGNAL_FUNC(WrapOnSaveClicks), GINT_TO_POINTER(0));

    return true;
}


int clSpectGUI::GetGramHeight ()
{
    return (gwScrolledW1->allocation.height - gwHRTime->allocation.height - 2);
}


bool clSpectGUI::ParseServerStr(char *cpHostRes, int *ipPortRes,
    const char *cpSourceStr)
{
    char cpTempStr[SGUI_SERVER_MAXLEN + 1];
    char *cpTempHost;
    char *cpTempPort;

    strncpy(cpTempStr, cpSourceStr, SGUI_SERVER_MAXLEN);
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


bool clSpectGUI::InitConnection(const char *cpServerHost, int iServerPort)
{
    char cpReqProcName[GLOBAL_HEADER_LEN];
    
    GetSettings();
    iSockH = SockClie.Connect(cpServerHost, NULL, iServerPort);
    if (iSockH < 0) return false;
    g_print("Connection established - sending process request...\n");
    bConnected = true;
    SockOp = new clSockOp(iSockH);
    strcpy(cpReqProcName, SGUI_REQ_PROC);
    if (SockOp->WriteN(cpReqProcName, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN) 
        return false;
    g_print("Sending initial settings...\n");
    if (!SendSettings()) return false;
    giGdkTag = gdk_input_add(iSockH, GDK_INPUT_READ, WrapGdkInput, NULL);
    iClips = 0;
    return true;
}


void clSpectGUI::GetSettings()
{
    float fDirection;
    int iItem;
    size_t iSpectSize;

    if (iBeamCount)
    {
        fDirection = gtk_spin_button_get_value_as_float(
            GTK_SPIN_BUTTON(gwSBChannel)) + 90.0f;
        sSRequest.iChannel = (int)
            (fDirection / (180.0f / (float) (iBeamCount - 1)));
        g_print("Channel: %i\n", sSRequest.iChannel);
    }
    else
    {
        sSRequest.iChannel = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(gwSBChannel)) - 1;
    }
    sSRequest.iType = GtkUtils.OptionMenuGetActive(gwOMType,
        gwaMIType, SGUI_TYPE_ITEMS);
    sSRequest.iWindow = GtkUtils.OptionMenuGetActive(gwOMWindow,
        gwaMIWindow, SGUI_WINDOW_ITEMS);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEWindowParam)), "%g",
        &sSRequest.fWinParam);
    iItem = GtkUtils.OptionMenuGetActive(gwOMWindowLen, gwaMIWindowLen,
        SGUI_WINLEN_ITEMS);
    if (iItem >= 0) sSRequest.lLength = atol(cpaLWindowLenMenu[iItem]);
    else sSRequest.lLength = atol(cpaLWindowLenMenu[0]);
    sSRequest.iLowFreq = atoi(gtk_entry_get_text(GTK_ENTRY(gwELowFreq)));
    sSRequest.iHighFreq = atoi(gtk_entry_get_text(GTK_ENTRY(gwEHighFreq)));
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEGain)), "%g",
        &sSRequest.fGain);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwESlope)), "%g",
        &sSRequest.fSlope);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEOverlap)), "%d",
        &sSRequest.iOverlap);
    sSRequest.bLinear = 
        (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gwCBLinear)) == TRUE) ?
        true : false;
    sSRequest.bNormalize = (gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(gwCBNormalize)) == TRUE) ?
        true : false;
    sSRequest.iRemoveNoise = GtkUtils.OptionMenuGetActive(gwOMRemoveNoise,
        gwaMIRemoveNoise, SGUI_REMOVE_NOISE_ITEMS);
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEAlpha)), "%g", &sSRequest.fAlpha);
    sSRequest.lMeanLength = atol(gtk_entry_get_text(GTK_ENTRY(gwEMeanLength)));
    sSRequest.lGapLength = atol(gtk_entry_get_text(GTK_ENTRY(gwEGapLength)));
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwEDynRange)), "%g", 
        &sSRequest.fDynRange);

    // Memory allocation for requested spectrum size
    switch (sSRequest.iType)
    {
        case MSG_SPECT_TYPE_FFT:
            iSpectSize = (sSRequest.lLength / 2 + 1) * sizeof(GDT);
            iRcvMsgSize = GLOBAL_HEADER_LEN + iSpectSize;
            break;
        /*case MSG_SPECT_TYPE_MRFFT:
            iSpectSize = (sSRequest.lLength / 2 + 1) * 
                sizeof(GDT) * CountBands(sSRequest.lFreqDiv);
            iRcvMsgSize = GLOBAL_HEADER_LEN + iSpectSize;
            break;*/
        case MSG_SPECT_TYPE_GABOR:
            iSpectSize = 0;
            iRcvMsgSize = GLOBAL_HEADER_LEN;
            break;
        case MSG_SPECT_TYPE_WVD:
            iSpectSize = (sSRequest.lLength / 2 + 1) * sizeof(GDT);
            iRcvMsgSize = GLOBAL_HEADER_LEN + iSpectSize;
            break;
        case MSG_SPECT_TYPE_HANKEL:
            iSpectSize = (sSRequest.lLength / 2 + 1) * sizeof(GDT);
            iRcvMsgSize = GLOBAL_HEADER_LEN + iSpectSize;
            break;
        case MSG_SPECT_TYPE_AUTOCORR:
            iSpectSize = (sSRequest.lLength / 2) * sizeof(GDT);
            iRcvMsgSize = GLOBAL_HEADER_LEN + iSpectSize;
            break;
        case MSG_SPECT_TYPE_CEPSTRUM:
            iSpectSize = (sSRequest.lLength / 2) * sizeof(GDT);
            iRcvMsgSize = GLOBAL_HEADER_LEN + iSpectSize;
            break;
        default:
            iSpectSize = 0;
    }
    cpRcvMsgBuf = (char *) RcvMsgBuf.Size(iRcvMsgSize);
    fpSpect = (GDT *) SpectBuf.Size(iSpectSize);
}


bool clSpectGUI::SendSettings()
{
    char cpMsgBuf[GLOBAL_HEADER_LEN];

    SpectMsg.SetRequest(cpMsgBuf, &sSRequest);
    if (SockOp->WriteN(cpMsgBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    bReConfig = true;
    return true;
}


void clSpectGUI::ReConfigDisplay()
{
    gfloat fSpectTime;
    gfloat fMaxFreq;
    gfloat fMinFreq;
    gfloat fLowKHz;
    gfloat fHighKHz;
    gfloat fWinWidth;
    gfloat fWinHeight;
    
    iSpectPoints = sSResult.lLength;
    if (!iFit) iGramH = iSpectPoints;
    //iGramH = GetGramHeight();
    FrameBuf.SetSize(iGramW, iGramH);
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDASpectrogram),
        iGramW, iGramH);
    gtk_drawing_area_size(GTK_DRAWING_AREA(gwDASpectrum),
        iSpectPoints, iSpectH);
    fSpectTime = sSResult.fLineTime * iGramW;
    fLowKHz = (gfloat) sSResult.iLowFreq / (gfloat) 1000.0;
    fHighKHz = (gfloat) sSResult.iHighFreq / (gfloat) 1000.0;
    gtk_ruler_set_range(GTK_RULER(gwHRTime), 
        fSpectTime, 0.0,
        0.0, fSpectTime);
    if (gwHRFreq->allocation.width > iSpectPoints)
    {
        fWinWidth = (gfloat) gwHRFreq->allocation.width;
        fMaxFreq = fLowKHz + (fHighKHz - fLowKHz) * 
            (fWinWidth / (gfloat) iSpectPoints);
        gtk_ruler_set_range(GTK_RULER(gwHRFreq),
            fLowKHz, fMaxFreq,
            fLowKHz, fMaxFreq);
    }
    else
    {
        gtk_ruler_set_range(GTK_RULER(gwHRFreq),
            fLowKHz, fHighKHz,
            fLowKHz, fHighKHz);
    }
    if (sSResult.bLinear)
    {
        gtk_ruler_set_range(GTK_RULER(gwVRLevel),
            1.0, 0.0,
            0.0, 1.0);
    }
    else
    {
        gtk_ruler_set_range(GTK_RULER(gwVRLevel),
            0.0, -sSRequest.fDynRange,
            0.0, 0.0);
    }
    if (gwVRFreq->allocation.height > iGramH)
    {
        fWinHeight = (gfloat) gwVRFreq->allocation.height;
        fMinFreq = fLowKHz - (fWinHeight - (gfloat) iGramH) * 
            ((fHighKHz - fLowKHz) / (gfloat) iGramH);
        gtk_ruler_set_range(GTK_RULER(gwVRFreq),
            fHighKHz, fMinFreq,
            fLowKHz, fHighKHz);
    }
    else
    {
        gtk_ruler_set_range(GTK_RULER(gwVRFreq),
            fHighKHz, fLowKHz,
            fLowKHz, fHighKHz);
    }
    bReConfig = false;
}


void clSpectGUI::DrawSpectrogram()
{
    int iYPos;
    int iViewHeight;
    GtkAdjustment *gaVScrollBar;
    GdkWindow *gwGramWin;

    if (iFit)
    {
        fpIntSpect = (GDT *)
            IntSpectBuf.Size(iGramH * sizeof(GDT));
        switch (iFit)
        {
            case SGUI_FIT_NEIGHBOR:
                DSP.Resample(fpIntSpect, iGramH, fpSpect, iSpectPoints);
                break;
            case SGUI_FIT_AVERAGE:
                DSP.ResampleAvg(fpIntSpect, iGramH, fpSpect, iSpectPoints);
                break;
            default:
                g_warning("Invalid fill algorithm!");
        }
        FrameBuf.DrawColumn(fpIntSpect);
    }
    else
    {
        FrameBuf.DrawColumn(fpSpect);
    }

    gaVScrollBar = gtk_scrolled_window_get_vadjustment(
        GTK_SCROLLED_WINDOW(gwScrolledW1));
    gwGramWin = gwDASpectrogram->window;
    iYPos = (int) gaVScrollBar->value - gwHRTime->allocation.height;
    if (iYPos < 0) iYPos = 0;
    iViewHeight = (iYPos < 0) ?
        ((int) gaVScrollBar->page_size + iYPos) : 
        (int) gaVScrollBar->page_size;
    if (iViewHeight > iGramH) iViewHeight = iGramH;
    if (iYPos < 0) iYPos = 0;
    if (iYPos > iGramH) return;
    #ifndef USE_BACKING_STORE
        gdk_window_copy_area(gwGramWin, ggcGramFG,
            0, iYPos,
            gwGramWin,
            1, iYPos, 
            iGramW, iViewHeight);  // here should be iGramW - 1
        gdk_draw_rgb_32_image(gwGramWin, ggcGramFG,
            iGramW, iYPos,  // and here iGramW - 1
            1, iViewHeight,
            GDK_RGB_DITHER_NONE,
            FrameBuf.GetCurPtr(iGramW - 1, iYPos),
            FrameBuf.GetRowStride());
    #else
        gdk_window_copy_area(gwGramWin, ggcGramFG,
            0, 0,
            gwGramWin,
            1, 0,
            iGramW - 1, iGramH);
        gdk_draw_rgb_32_image(gwGramWin, ggcGramFG,
            iGramW - 1, 0,
            1, iGramH,
            GDK_RGB_DITHER_NONE,
            FrameBuf.GetCurPtr(iGramW - 1, 0),
            FrameBuf.GetRowStride());
    #endif
}


void clSpectGUI::DrawSpectrum()
{
    int iMaxLine;
    int iLineCntr;
    int iLineLength;
    int iXPos;
    int iViewWidth;
    GtkAdjustment *gaHScrollBar;
    GdkWindow *gwSpectWin;
    
    gaHScrollBar = gtk_scrolled_window_get_hadjustment(
        GTK_SCROLLED_WINDOW(gwScrolledW2));
    gwSpectWin = gwDASpectrum->window;
    iXPos = (int) gaHScrollBar->value;
    iViewWidth = (int) gaHScrollBar->page_size;
    gdk_window_clear(gwSpectWin);
    iMaxLine = (iSpectPoints <= (iXPos + iViewWidth)) ?
        iSpectPoints : iXPos + iViewWidth;
    for (iLineCntr = iXPos; iLineCntr < iMaxLine; iLineCntr++)
    {
        iLineLength = (int) (fpSpect[iLineCntr] * (GDT) (iSpectH - 1) + 0.5);
        if (iLineLength < 0) iLineLength = 0;
        if (iLineLength >= iSpectH) iLineLength = iSpectH - 1;
        gdk_draw_line(gwSpectWin, ggcSpectFG,
            iLineCntr, iSpectH - 1, iLineCntr, iSpectH - iLineLength);
    }
}


void clSpectGUI::PrintRealTime()
{
    time_t ttCursorTime;
    struct tm *spCursorTime;

    ttCursorTime = sSResult.sTimeStamp.tv_sec - (time_t) (fGramX + 0.5F);
    spCursorTime = localtime(&ttCursorTime);
    strftime(cpGramXTime, 20, "%d.%m.%Y %H:%M:%S", spCursorTime);
}


long clSpectGUI::CountBands(long lDivisor)
{
    float fNyquist;
    float fFreq;
    long lDivCntr = 0L;

    fNyquist = 44100.0F / 2.0F;
    fFreq = fNyquist;
    while (fFreq > SPECT_BAND_LIMIT)
    {
        lDivCntr++;
        fFreq /= (float) lDivisor;
    }
    return (lDivCntr - 1);
}


void clSpectGUI::SaveInfo (const char *cpFileName)
{
    time_t ttTime;
    char *cpFullFileName;
    char cpTime[24];
    FILE *fileInfo;

    cpFullFileName = g_strdup_printf("%s.inf", cpFileName);
    fileInfo = fopen(cpFullFileName, FB_TIFF_MODE);
    g_free(cpFullFileName);
    if (fileInfo == NULL) return;
    time(&ttTime);
    strftime(cpTime, 23, "%Y/%m/%d %H:%M:%S", localtime(&ttTime));
    fprintf(fileInfo, "Time: %s\n", cpTime);
    fprintf(fileInfo, "LowFreq: %i\n", sSResult.iLowFreq);
    fprintf(fileInfo, "HighFreq: %i\n", sSResult.iHighFreq);
    fprintf(fileInfo, "LineTime: %f\n", sSResult.fLineTime);
    fclose(fileInfo);
}
