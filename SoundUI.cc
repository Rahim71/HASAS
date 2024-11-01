/*

    Sound user interface
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


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <signal.h>
#include <errno.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <gtk/gtk.h>

#include "SoundUI.hh"


static const char *cpWindowTxt = "Sound UI";
static const char *cpChannelPfx = "Channel";
// Table 1
static const char *cpLServerTxt = "Server";
static const char *cpLChannelTxt = "Channel";
static const char *cpBConnectTxt = "Connect";
// -
static const char *cpLInputLevelTxt = "Average peak input level";
static const char *cpLEqTxt = "Eq";
// Eq
static const char *cpLOutputLevelTxt = "Level";
static const char *cpBApplyCurveTxt = "Apply curve";

clSoundUI *SoundUI;


int main (int argc, char *argv[])
{
    int iRetVal;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    SoundUI = new clSoundUI(&argc, &argv);
    iRetVal = SoundUI->Exec();
    delete SoundUI;
    return iRetVal;
}


// Wrapper functions


gboolean WrapOnDeleteEvent (GtkWidget *gwSender, GdkEvent *geEvent,
    gpointer gpData)
{
    return SoundUI->OnDeleteEvent(gwSender, geEvent, gpData);
}


void WrapOnClickedEvent (GtkButton *gbButton, gpointer gpData)
{
    SoundUI->OnClickedEvent(gbButton, gpData);
}


gint WrapOnTimeoutEvent (gpointer gpData)
{
    return SoundUI->OnTimeoutEvent(gpData);
}


void WrapOnToggledEvent (GtkToggleButton *gtbToggleButton, gpointer gpData)
{
    SoundUI->OnToggledEvent(gtbToggleButton, gpData);
}


void WrapOnValueChangedEvent (GtkAdjustment *gaAdjustment, gpointer gpData)
{
    SoundUI->OnValueChangedEvent(gaAdjustment, gpData);
}


void WrapOnApplyCurveClicked (GtkButton *gbButton, gpointer gpData)
{
    SoundUI->OnApplyCurveClicked(gbButton, gpData);
}


void WrapOnMotionCurve (GtkWidget *gwSender, GdkEventMotion *gemEvent,
    gpointer gpData)
{
    SoundUI->OnMotionCurve(gwSender, gemEvent, gpData);
}


void *WrapSoundOutThread (void *vpData)
{
    return SoundUI->SoundOutThread(vpData);
}


void *WrapSoundInThread (void *vpData)
{
    return SoundUI->SoundInThread(vpData);
}


// clSoundUI


void clSoundUI::GetCfg ()
{
    int iALSA;
    int iLocalCh;
    int iLocalSR;
    long lLocalSC;

    Cfg.SetFileName(SUI_CFGFILE);
    if (!Cfg.GetStr("Device", cpDevice))
        strcpy(cpDevice, SUI_DEF_DEVICE);
    if (!Cfg.GetInt("DeviceBase", &iDeviceBase))
        iDeviceBase = -1;
    if (Cfg.GetInt("UseALSA", &iALSA))
    {
        bALSA = (iALSA) ? true : false;
    }
    else bALSA = false;
    if (!Cfg.GetInt("ALSACard", &iALSACard))
        iALSACard = 0;
    if (!Cfg.GetInt("ALSADevice", &iALSADevice))
        iALSADevice = 0;
    if (!Cfg.GetInt("ALSASubDevice", &iALSASubDevice))
        iALSASubDevice = 0;
    if (Cfg.GetInt("Channels", &iLocalCh))
        iChCount = iLocalCh;
    else
        iChCount = SUI_DEF_CHANNELS;
    if (Cfg.GetInt("SampleRate", &iLocalSR))
        iSampleRate = iLocalSR;
    else
        iSampleRate = SUI_DEF_SAMPLERATE;
    if (Cfg.GetInt("SampleCount", &lLocalSC))
        lSampleCount = lLocalSC;
    else
        lSampleCount = SUI_SAMPLECOUNT;
    if (!Cfg.GetInt("UpdateInterval", &iVuTimeout))
        iVuTimeout = SUI_VU_TIMEOUT;
    if (!Cfg.GetInt("DCBlock", &iDCBlock))
        iDCBlock = 0;
}


void clSoundUI::BuildGUI ()
{
    int iWidgetCntr;
    int iWidgetCount;
    int iLocalOctaves;
    long lSpectSize;

    gwWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gwWindow), cpWindowTxt);
    // allow_shrink, allow_grow, auto_shrink
    gtk_window_set_policy(GTK_WINDOW(gwWindow), TRUE, TRUE, FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(gwWindow), SUI_PADDING);

    // homogenous, spacing
    gwVBox = gtk_vbox_new(FALSE, SUI_PADDING);
    gtk_container_add(GTK_CONTAINER(gwWindow), gwVBox);
    gtk_widget_show(gwVBox);

    // homogenous, spacing
    gwHBox = gtk_hbox_new(TRUE, SUI_PADDING);
    // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwHBox, TRUE, TRUE, 0);
    gtk_widget_show(gwHBox);

    gwStatusbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(gwVBox), gwStatusbar, FALSE, FALSE, 0);
    guiStatusbarCtxt = gtk_statusbar_get_context_id(
        GTK_STATUSBAR(gwStatusbar), "status");
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusbar), guiStatusbarCtxt, "");
    gtk_widget_show(gwStatusbar);

    MutexData.Wait();
    iWidgetCount = iChCount;
    iLocalOctaves = iOctaveCount;
    lSpectSize = lSampleCount + 1L;
    MutexData.Release();
    for (iWidgetCntr = 0; iWidgetCntr < iWidgetCount; iWidgetCntr++)
    {
        SoundChGUI[iWidgetCntr] = new clSoundChGUI(gwHBox, iWidgetCntr + 1,
            iLocalOctaves, lSpectSize);
    }

    ConnectSignals();

    // Delayed realization of main window
    gtk_widget_show(gwWindow);
}


void clSoundUI::ConnectSignals ()
{
    int iWidgetCntr;
    int iWidgetCount;
    int iObjectCntr;
    int iObjectCount;
    int iObjectMask;

    gtk_signal_connect(GTK_OBJECT(gwWindow), "delete-event", 
        GTK_SIGNAL_FUNC(WrapOnDeleteEvent), NULL);
    MutexData.Wait();
    iWidgetCount = iChCount;
    iObjectCount = iOctaveCount;
    MutexData.Release();
    for (iWidgetCntr = 0; iWidgetCntr < iWidgetCount; iWidgetCntr++)
    {
        gtk_signal_connect(GTK_OBJECT(SoundChGUI[iWidgetCntr]->gwBConnect),
            "clicked", GTK_SIGNAL_FUNC(WrapOnClickedEvent), 
            GINT_TO_POINTER(iWidgetCntr));
        gtk_signal_connect(GTK_OBJECT(SoundChGUI[iWidgetCntr]->gwCBEq),
            "toggled", GTK_SIGNAL_FUNC(WrapOnToggledEvent),
            GINT_TO_POINTER(iWidgetCntr));
        gtk_signal_connect(GTK_OBJECT(SoundChGUI[iWidgetCntr]->goAOutputLevel),
            "value-changed", GTK_SIGNAL_FUNC(WrapOnValueChangedEvent),
            GINT_TO_POINTER(iWidgetCntr << 16));
        for (iObjectCntr = 0; iObjectCntr < iObjectCount; iObjectCntr++)
        {
            iObjectMask = (iWidgetCntr << 16);
            iObjectMask |= ((iObjectCntr + 1) & 0xffff);
            gtk_signal_connect(
                GTK_OBJECT(SoundChGUI[iWidgetCntr]->goaAEqLevel[iObjectCntr]),
                "value-changed", GTK_SIGNAL_FUNC(WrapOnValueChangedEvent),
                GINT_TO_POINTER(iObjectMask));
        }
        gtk_signal_connect(GTK_OBJECT(SoundChGUI[iWidgetCntr]->gwBApplyCurve),
            "clicked", GTK_SIGNAL_FUNC(WrapOnApplyCurveClicked),
            GINT_TO_POINTER(iWidgetCntr));
        gtk_signal_connect(GTK_OBJECT(SoundChGUI[iWidgetCntr]->gwCurveEq),
            "motion_notify_event", GTK_SIGNAL_FUNC(WrapOnMotionCurve), 
            GINT_TO_POINTER(iWidgetCntr));
    }
}


bool clSoundUI::ParseServerStr (char *cpHostRes, int *ipPortRes,
    const char *cpSourceStr)
{
    char cpTempStr[SUI_SERV_MAXLEN + 1];
    char *cpTempHost;
    char *cpTempPort;

    strncpy(cpTempStr, cpSourceStr, SUI_SERV_MAXLEN);
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


clSoundUI::clSoundUI (int *ArgC, char ***ArgV)
{
    int iInitCntr;
    
    bRun = true;
    bFirstTimeout = true;
    lDataRefCount = 0L;
    for (iInitCntr = 0; iInitCntr < SUI_MAX_CHANNELS; iInitCntr++)
    {
        bpConnected[iInitCntr] = false;
    }

    g_print("%s v%i.%i.%i\n", cpWindowTxt,
        SUI_VER_MAJ, SUI_VER_MIN, SUI_VER_PL);
    g_print("Copyright (C) 2000-2001 Jussi Laako\n\n");
    g_print("Gtk+ version %i.%i.%i\n", gtk_major_version, gtk_minor_version,
        gtk_micro_version);
    g_print("Locale set to %s\n", gtk_set_locale());
    gtk_init(ArgC, ArgV);
}


clSoundUI::~clSoundUI ()
{
}


int clSoundUI::Exec ()
{
    int iChBufSize;
    int iAllocCntr;
    double dNyquist;
    clDSPOp DSP;

    GetCfg();
    if (iDeviceBase < 0)
    {
        pthread_create(&ptidSoundOut, NULL, WrapSoundOutThread, NULL);
        SemStart1.Wait();
    }
    iChBufSize = lSampleCount * sizeof(GDT);
    for (iAllocCntr = 0; iAllocCntr < iChCount; iAllocCntr++)
    {
        ChData[iAllocCntr].Size(iChBufSize);
        EqCoeffs[iAllocCntr].Size((lSampleCount + 1L) * sizeof(GDT));
        ChData[iAllocCntr].Lock();
        EqCoeffs[iAllocCntr].Lock();
        DSP.Zero((GDT *) ChData[iAllocCntr], lSampleCount);
        DSP.Set((GDT *) EqCoeffs[iAllocCntr], (GDT) 1.0, lSampleCount + 1L);
        fpLevelCoeff[iAllocCntr] = (GDT) 1.0;
        GDT *fpNullPtr = NULL;
        bpEqEnabled[iAllocCntr] = false;
        Filters[iAllocCntr].Initialize((long) lSampleCount, fpNullPtr,
            SUI_FILTER_WINDOW);
    }
    MutexData.Wait();
    dNyquist = iSampleRate / 2.0;
    iOctaveCount = (int) (log(dNyquist) / log(2.0) + 0.5);
    if (iOctaveCount > SUI_EQ_MAXOCTS) iOctaveCount = SUI_EQ_MAXOCTS;
    MutexData.Release();
    if (iDeviceBase < 0) SemStart2.Post();
    BuildGUI();
    gtk_timeout_add(iVuTimeout, WrapOnTimeoutEvent, NULL);
    gtk_main();
    if (iDeviceBase < 0) pthread_join(ptidSoundOut, NULL);
    for (iAllocCntr = 0; iAllocCntr < iChCount; iAllocCntr++)
    {
        MutexData.Wait();
        if (bpConnected[iAllocCntr]) 
        {
            bpConnected[iAllocCntr] = false;
            CondData[iAllocCntr].Notify();
            MutexData.Release();
            pthread_join(ptidSoundIn[iAllocCntr], NULL);
        }
        else MutexData.Release();
    }
    return 0;
}


gboolean clSoundUI::OnDeleteEvent (GtkWidget *gwSender, GdkEvent *geEvent,
    gpointer gpData)
{
    MutexData.Wait();
    bRun = false;
    MutexData.Release();
    gtk_main_quit();
    return TRUE;
}


void clSoundUI::OnClickedEvent (GtkButton *gbButton, gpointer gpData)
{
    int iChannel = GPOINTER_TO_INT(gpData);
    int iPort;
    int iSockHandle;
    int iSourceChannel;
    int iThreadMask;
    char cpServer[SUI_SERV_MAXLEN];
    GtkWidget *gwServerEntry;

    // Stop running input thread
    MutexData.Wait();
    if (bpConnected[iChannel])
    {
        bpConnected[iChannel] = false;
        MutexData.Release();
        pthread_join(ptidSoundIn[iChannel], NULL);
    }
    else MutexData.Release();
    // Get server entry, source channel number and generated thread mask
    gwServerEntry = GTK_COMBO(SoundChGUI[iChannel]->gwCServer)->entry;
    iSourceChannel = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(SoundChGUI[iChannel]->gwSBChannel)) - 1;
    iThreadMask = ((iSourceChannel << 16) | iChannel);
    // Parse server string and try to connect specified sound server
    if (ParseServerStr(cpServer, &iPort, 
        gtk_entry_get_text(GTK_ENTRY(gwServerEntry))))
    {
        iSockHandle = SClient.Connect(cpServer, NULL, iPort);
        if (iSockHandle >= 0)
        {
            MutexData.Wait();
            bpConnected[iChannel] = true;
            ipSockH[iChannel] = iSockHandle;
            MutexData.Release();
            pthread_create(&ptidSoundIn[iChannel], NULL, WrapSoundInThread,
                GINT_TO_POINTER(iThreadMask));
        }
        else
        {
            g_print("Connect failed!\n");
        }
    }
    else
    {
        g_print("Invalid entry, format is <server>:<port>\n");
    }
}


gint clSoundUI::OnTimeoutEvent (gpointer gpData)
{
    bool bTypeSet = false;
    int iLocalChCount;
    int iChCntr;
    gfloat fThisInLevel;
    gfloat fThisOutLevel;

    MutexData.Wait();
    iLocalChCount = iChCount;
    MutexData.Release();
    for (iChCntr = 0; iChCntr < iLocalChCount; iChCntr++)
    {
        MutexLevel.Wait();
        fThisInLevel = fpInputLevel[iChCntr];
        fThisOutLevel = fpOutputLevel[iChCntr];
        MutexLevel.Release();
        gtk_progress_set_value(
            GTK_PROGRESS(SoundChGUI[iChCntr]->gwPBInputLevel), fThisInLevel);
        gtk_progress_set_value(
            GTK_PROGRESS(SoundChGUI[iChCntr]->gwPBOutputLevel), fThisOutLevel);
        if (bFirstTimeout && 
            GTK_WIDGET_REALIZED(SoundChGUI[iChCntr]->gwCurveEq))
        {
            gtk_curve_set_curve_type(GTK_CURVE(SoundChGUI[iChCntr]->gwCurveEq),
                GTK_CURVE_TYPE_LINEAR);
            bTypeSet = true;
        }
    }
    if (bTypeSet) bFirstTimeout = false;
    return TRUE;
}


void clSoundUI::OnToggledEvent (GtkToggleButton *gtbToggleButton, 
    gpointer gpData)
{
    bool bToggled;
    int iThisCh = GPOINTER_TO_INT(gpData);

    bToggled = 
        (gtk_toggle_button_get_active(gtbToggleButton)) ? true : false;
    MutexFilter[iThisCh].Wait();
    bpEqEnabled[iThisCh] = bToggled;
    MutexFilter[iThisCh].Release();
}


void clSoundUI::OnValueChangedEvent (GtkAdjustment *gaAdjustment, 
    gpointer gpData)
{
    int iThisCh;
    int iThisObj;
    long lCoeffCntr;
    long lCoeffCount;
    long lStartIdx;
    long lCenterIdx;
    long lEndIdx;
    long lDist;
    double dNyquist;
    double dResolution;
    GDT fValue;
    GDT fCoeff;
    GDT fPrevCoeff;
    GDT fNextCoeff;
    GDT fDiff;
    GDT *fpCoeffPtr;

    iThisCh = (GPOINTER_TO_INT(gpData) >> 16);
    iThisObj = (GPOINTER_TO_INT(gpData) & 0xffff);
    fValue = -(gaAdjustment->value);
    fpCoeffPtr = EqCoeffs[iThisCh];

    if (iThisObj == 0)
    {
        MutexFilter[iThisCh].Wait();
        fpLevelCoeff[iThisCh] = (GDT) pow(10.0, fValue / 20.0);
        MutexFilter[iThisCh].Release();
        return;
    }
    
    MutexData.Wait();
    lCoeffCount = lSampleCount + 1L;
    dNyquist = iSampleRate / 2.0;
    MutexData.Release();
    dResolution = dNyquist / lCoeffCount;
    lStartIdx = (long) (pow(2.0, iThisObj - 1) / dResolution + 0.5);
    lCenterIdx = (long) (pow(2.0, iThisObj) / dResolution + 0.5);
    lEndIdx = (long) (pow(2.0, iThisObj + 1) / dResolution + 0.5);
    if (lStartIdx < 0) lStartIdx = 1L;
    if (lCenterIdx < 0) lCenterIdx = 1L;
    else if (lCenterIdx > lCoeffCount) lCenterIdx = lCoeffCount - 1L;
    if (lEndIdx > lCoeffCount) lEndIdx = lCoeffCount - 1L;
    fPrevCoeff = fpCoeffPtr[lStartIdx - 1L];
    fNextCoeff = fpCoeffPtr[lEndIdx];
    fCoeff = pow(10.0, fValue / 20.0);
    for (lCoeffCntr = lStartIdx; lCoeffCntr < lEndIdx; lCoeffCntr++)
    {
        if (lCoeffCntr < lCenterIdx)
        {
            fDiff = fCoeff - fPrevCoeff;
            lDist = lCenterIdx - lStartIdx;
            fpCoeffPtr[lCoeffCntr] = 
                fPrevCoeff + fDiff / lDist * (lCoeffCntr - lStartIdx + 1);
        }
        else
        {
            fDiff = fCoeff - fNextCoeff;
            lDist = lEndIdx - lCenterIdx;
            fpCoeffPtr[lCoeffCntr] =
                fNextCoeff + fDiff / lDist * (lEndIdx - lCoeffCntr);
        }
    }
    if (iDCBlock > 0) fpCoeffPtr[0] = (GDT) 0.0;
    MutexFilter[iThisCh].Wait();
    Filters[iThisCh].SetCoeffs(fpCoeffPtr);
    MutexFilter[iThisCh].Release();

    #ifdef __GNUG__
    gfloat fpCurveValues[lCoeffCount];
    #else
    gfloat *fpCurveValues;
    clDSPAlloc CurveValues;
    fpCurveValues = (gfloat *) CurveValues.Size(lCoeffCount * sizeof(gfloat));
    #endif
    
    for (lCoeffCntr = 0L; lCoeffCntr < lCoeffCount; lCoeffCntr++)
    {
        fpCurveValues[lCoeffCntr] = 20.0 * log10(fpCoeffPtr[lCoeffCntr]);
    }
    gtk_curve_set_vector(GTK_CURVE(SoundChGUI[iThisCh]->gwCurveEq),
        lCoeffCount, fpCurveValues);
    gtk_curve_set_curve_type(GTK_CURVE(SoundChGUI[iThisCh]->gwCurveEq),
        GTK_CURVE_TYPE_LINEAR);
}


void clSoundUI::OnApplyCurveClicked (GtkButton *gbButton, gpointer gpData)
{
    int iThisCh = GPOINTER_TO_INT(gpData);
    long lCoeffCntr;
    MutexData.Wait();
    long lCoeffCount = lSampleCount + 1L;
    MutexData.Release();
    #ifdef __GNUG__
    gfloat fpCurveValues[lCoeffCount];
    #else
    clDSPAlloc CurveValues;
    gfloat *fpCurveValues = (gfloat *) 
        CurveValues.Size(lCoeffCount * sizeof(gfloat));
    #endif
    GDT *fpCoeffPtr = EqCoeffs[iThisCh];

    gtk_curve_get_vector(GTK_CURVE(SoundChGUI[iThisCh]->gwCurveEq),
        lCoeffCount, fpCurveValues);
    for (lCoeffCntr = 0L; lCoeffCntr < lCoeffCount; lCoeffCntr++)
    {
        fpCoeffPtr[lCoeffCntr] = 
            (GDT) pow(10.0, fpCurveValues[lCoeffCntr] / 20.0);
    }
    if (iDCBlock > 0) fpCoeffPtr[0] = (GDT) 0.0;
    MutexFilter[iThisCh].Wait();
    Filters[iThisCh].SetCoeffs(fpCoeffPtr);
    MutexFilter[iThisCh].Release();
}


void clSoundUI::OnMotionCurve (GtkWidget *gwSender, GdkEventMotion *gemEvent,
    gpointer gpData)
{
    int iThisCh = GPOINTER_TO_INT(gpData);
    char cpStatusTxt[SUI_CONV_LEN];
    int iWidth;
    int iHeight;
    gfloat fResolution;
    gfloat fFreqScale;
    gfloat fHzValue;
    gfloat fdBValue;

    iWidth = SoundChGUI[iThisCh]->gwCurveEq->allocation.width;
    iHeight = SoundChGUI[iThisCh]->gwCurveEq->allocation.height;
    MutexData.Wait();
    fResolution = iSampleRate / 2.0 / (lSampleCount + 1L);
    fFreqScale = (gfloat) (lSampleCount + 1L) / (gfloat) iWidth;
    MutexData.Release();
    fHzValue = gemEvent->x * fFreqScale * fResolution;
    fdBValue = (iHeight - gemEvent->y) * (SUI_EQ_RANGE * 2.0 / iHeight) -
        SUI_EQ_RANGE;
    g_snprintf(cpStatusTxt, SUI_CONV_LEN, "%.1f Hz  %.2f dB", 
        fHzValue, fdBValue);
    gtk_statusbar_pop(GTK_STATUSBAR(gwStatusbar), guiStatusbarCtxt);
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusbar), guiStatusbarCtxt,
        cpStatusTxt);
}


void *clSoundUI::SoundOutThread (void *vpData)
{
    bool bLocalRun = true;
    #ifndef __QNX__
    int iFormat = SUI_SND_FORMAT;
    #endif
    int iLocalCh;
    int iLocalSR;

    MutexData.Wait();
    iLocalCh = iChCount;
    iLocalSR = iSampleRate;
    MutexData.Release();
    if (!bALSA)
    {
        #ifndef __QNX__
        if (Audio.Open(cpDevice, &iFormat, &iLocalSR, &iLocalCh, AUDIO_WRITE))
        {
            g_print("Audio device open; fs %i ch %i fmt %xh\n", iLocalSR,
                iLocalCh, iFormat);
            MutexData.Wait();
            iChCount = iLocalCh;
            iSampleRate = iLocalSR;
            MutexData.Release();
        }
        else
        {
            g_print("Unable to open audio device, reason: %s\n",
                strerror(Audio.GetError()));
            bLocalRun = false;
        }
        #else
        g_warning("OSS is not supported under QNX");
        #endif
    }
    else
    {
        #ifndef BSDSYS
        if (AudioA.CardOpen(iALSACard))
        {
            #ifdef USE_ALSA05
            if (AudioA.PcmOpen(iALSADevice, iALSASubDevice, AA_MODE_PLAY))
            #else
            sprintf(cpDevice, "plughw:%i,%i", iALSACard, iALSADevice);
            if (AudioA.PcmOpen(cpDevice, AA_MODE_PLAY))
            #endif
            {
                #ifdef USE_ALSA05
                if (AudioA.PcmSetSetup(iLocalCh, iLocalSR, SUI_SND_BITS,
                    SUI_SND_QUEUESIZE, false))
                #else
                if (AudioA.PcmSetSetup(iLocalCh, iLocalSR, SUI_SND_BITS,
                    SUI_SND_QUEUESIZE, 2))
                #endif
                {
                    if (AudioA.PcmPrepare())
                    {
                        iLocalCh = AudioA.PcmGetChannels();
                        iLocalSR = AudioA.PcmGetSampleRate();
                        g_print("Audio device open; fs %i ch %i wl %i\n", 
                            iLocalSR, iLocalCh, 
                            AudioA.PcmGetBits());
                        MutexData.Wait();
                        iChCount = iLocalCh;
                        iSampleRate = iLocalSR;
                        MutexData.Release();
                    }
                    else
                    {
                        g_print("ALSA PCM prepare failed\n");
                        bLocalRun = false;
                    }
                }
                else
                {
                    g_print("ALSA PCM setup failed\n");
                    g_print("%s\n", AudioA.PcmGetStatusStr(AudioA.PcmGetStatus()));
                    bLocalRun = false;
                }
            }
            else
            {
                g_print("ALSA PCM open failed\n");
                bLocalRun = false;
            }
        }
        else
        {
            g_print("ALSA card open failed\n");
            bLocalRun = false;
        }
        #else
        g_warning("ALSA not supported under BSD systems");
        #endif        
    }
    SemStart1.Post();
    SemStart2.Wait();

    long lChMask;
    long lChCntr;
    long lSampleCntr;
    long lLocalSC;
    volatile long lLocalRefCount;
    long lOutDataCount;
    long lOutBufSize;
    GDT *fpChBuf;
    GDT *fpOutBuf;
    SUI_SND_DATATYPE *ipOutBuf;
    // Priority stuff
    #ifndef BSDSYS
    int iSchedPolicy;
    uid_t uidCurrent;
    struct sched_param sSchedParam;
    #endif
    // -
    clDSPAlloc OutBuf1;
    clDSPAlloc OutBuf2;
    clDSPOp DSP;

    MutexData.Wait();
    lLocalSC = lSampleCount;
    MutexData.Release();
    lOutDataCount = lLocalSC * iLocalCh;
    fpOutBuf = (GDT *) OutBuf1.Size(lOutDataCount * sizeof(GDT));
    lOutBufSize = lLocalSC * iLocalCh * sizeof(SUI_SND_DATATYPE);
    ipOutBuf = (SUI_SND_DATATYPE *) OutBuf2.Size(lOutBufSize);
    OutBuf1.Lock();
    OutBuf2.Lock();
    #ifndef BSDSYS
    uidCurrent = getuid();
    setuid(0);
    pthread_getschedparam(ptidSoundOut, &iSchedPolicy, &sSchedParam);
    sSchedParam.sched_priority = sched_get_priority_min(SCHED_FIFO) + 2;
    if (pthread_setschedparam(ptidSoundOut, SCHED_FIFO, &sSchedParam) != 0)
    {
        g_print("SoundOut unable to set scheduling policy\n");
    }
    setuid(uidCurrent);
    /*sSchedParam.sched_priority = 2;
    pthread_setschedparam(ptidSoundOut, SCHED_OTHER, &sSchedParam);*/
    #endif
    g_print("SoundOut thread running\n");
    DSP.Zero(fpOutBuf, lOutDataCount);
    #ifndef BSDSYS
    #ifdef USE_ALSA05
    if (bALSA) AudioA.PcmGo();
    #else
    if (bALSA) AudioA.PcmStart();
    #endif
    #endif
    while (bLocalRun)
    {
        for (lChCntr = 0; lChCntr < iLocalCh; lChCntr++)
        {
            lChMask = (1L << lChCntr);
            MutexData.Wait();
            lLocalRefCount = lDataRefCount;
            MutexData.Release();
            // --- This one is nasty piece of code, we shouldn't need this
            //     but Linux is far from realtime OS, so we can try to
            //     give it some extra time to meet the deadline...
            if (!(lLocalRefCount & lChMask))
            {
                usleep((unsigned long) (lLocalSC / 2 / iLocalSR * 1000000));
                MutexData.Wait();
                lLocalRefCount = lDataRefCount;
                MutexData.Release();
            }
            // ---
            if ((lLocalRefCount & lChMask) == lChMask)
            {
                MutexChData[lChCntr].Wait();
                fpChBuf = ChData[lChCntr];
                for (lSampleCntr = 0; lSampleCntr < lLocalSC; lSampleCntr++)
                {
                    fpOutBuf[lSampleCntr * iLocalCh + lChCntr] = 
                        fpChBuf[lSampleCntr];
                }
                MutexData.Wait();
                lDataRefCount &= ~(lChMask);
                MutexData.Release();
                CondData[lChCntr].Notify();
                MutexChData[lChCntr].Release();
            }
            else
            {
                for (lSampleCntr = 0; lSampleCntr < lLocalSC; lSampleCntr++)
                {
                    fpOutBuf[lSampleCntr * iLocalCh + lChCntr] = (GDT) 0.0;
                }
                MutexChData[lChCntr].Wait();
                CondData[lChCntr].Notify();
                MutexChData[lChCntr].Release();
            }
        }
        DSP.Convert(ipOutBuf, fpOutBuf, lOutDataCount, false);
        if (!bALSA)
        {
            #ifndef __QNX__
            if (Audio.Write(ipOutBuf, lOutBufSize) < lOutBufSize)
                g_warning("Write to PCM device failed!");
            #endif
        }
        else
        {
            #ifndef BSDSYS
            /*if (AudioA.PcmWrite(ipOutBuf, lOutBufSize) < lOutBufSize)
                g_warning("Write to PCM device failed!");*/
            AudioA.PcmWrite(ipOutBuf, lOutBufSize);
            #endif
        }
        MutexData.Wait();
        bLocalRun = bRun;
        MutexData.Release();
    }
    if (!bALSA)
    {
        #ifndef __QNX__
        Audio.Reset();
        Audio.Close();
        #endif
    }
    else
    {
        #ifndef BSDSYS
        AudioA.PcmClose();
        AudioA.CardClose();
        #endif
    }
    g_print("SoundOut thread ending\n");
    return NULL;
}


void *clSoundUI::SoundInThread (void *vpData)
{
    bool bLocalRun = true;
    int iSrcCh = (GPOINTER_TO_INT(vpData) >> 16);
    int iDestCh = (GPOINTER_TO_INT(vpData) & 0xffff);
    int iThisIdx = iDestCh;
    int iLocalChCount;
    long lLocalSC;
    int iMsgSize;
    char *cpMsgBuf;
    char cpHdrBuf[GLOBAL_HEADER_LEN];
    stSoundStart sSoundHdr;
    clAlloc MsgBuf;
    clSoundMsg SoundMsg;
    clSockOp SOp;

    MutexData.Wait();
    iLocalChCount = iChCount;
    lLocalSC = lSampleCount;
    SOp.SetHandle(ipSockH[iThisIdx]);
    MutexData.Release();
    g_print("SoundIn[%i] thread receiving first message...\n", iThisIdx);
    if (SOp.ReadSelect(SUI_FIRST_TIMEOUT))
    {
        if (SOp.ReadN(cpHdrBuf, GLOBAL_HEADER_LEN) == GLOBAL_HEADER_LEN)
        {
            SoundMsg.GetStart(cpHdrBuf, &sSoundHdr);
        }
        else bLocalRun = false;
    }
    iMsgSize = lLocalSC * sSoundHdr.iChannels * sizeof(GDT);
    cpMsgBuf = (char *) MsgBuf.Size(iMsgSize);
    MsgBuf.Lock();

    bool bWait;
    #ifndef __QNX__
    int iLocalFmt = SUI_SND_FORMAT;
    #endif
    int iLocalSR = (int) (sSoundHdr.dSampleRate + 0.5);
    int iLocalCh = iChCount;
    long lExtCount = lLocalSC * sSoundHdr.iChannels;
    long lOutCount = lLocalSC;
    long lChMask = (1L << iThisIdx);
    #ifndef __QNX__
    long lOutBufSize = 0L;
    long lSampleCntr;
    char cpDeviceName[_POSIX_PATH_MAX];
    #endif
    gfloat fInLevel;
    gfloat fOutLevel;
    GDT *fpInBuf;
    GDT *fpOutData;
    #ifndef __QNX__
    SUI_SND_DATATYPE *ipOutData = NULL;
    SUI_SND_DATATYPE *ipOutBuf = NULL;
    audio_buf_info sAudioBufInfo;
    #endif
    // Priority stuff
    #ifndef BSDSYS
    int iSchedPolicy;
    uid_t uidCurrent;
    struct sched_param sSchedParam;
    #endif
    // -
    clDSPAlloc InBuf;
    clDSPAlloc OutData1;
    clDSPAlloc OutData2;
    clDSPAlloc OutBuf;
    #ifndef __QNX__
    clAudio Audio;
    #endif
    clDSPOp DSP;
    
    if (iDeviceBase >= 0)
    {
        #ifndef __QNX__
        g_snprintf(cpDeviceName, _POSIX_PATH_MAX, "%s%i",
            cpDevice, iDeviceBase + iDestCh);
        g_print("SoundIn[%i] open device %s : %i fs\n",
            iThisIdx, cpDeviceName, iLocalSR);
        if (!Audio.Open(cpDeviceName, &iLocalFmt, &iLocalSR, &iLocalCh, 
            AUDIO_WRITE))
        {
            g_print("Fatal: failed to open device %s in SoundIn[%i]\n",
                cpDeviceName, iThisIdx);
            return NULL;
        }
        g_print("SoundIn[%i] device opened at %i fs\n", iThisIdx, iLocalSR);
        lOutBufSize = lOutCount * iLocalCh * sizeof(SUI_SND_DATATYPE);
        ipOutData = (SUI_SND_DATATYPE *) OutData2.Size(lOutCount * sizeof(SUI_SND_DATATYPE));
        ipOutBuf = (SUI_SND_DATATYPE *) OutBuf.Size(lOutBufSize);
        OutData2.Lock();
        OutBuf.Lock();
        memset(ipOutBuf, 0x00, lOutBufSize);
        #else
        g_warning("This functionality is not supported under QNX");
        #endif
    }
    SOp.SetRecvBufSize(iMsgSize * 2);
    fpInBuf = (GDT *) InBuf.Size(lExtCount * sizeof(GDT));
    fpOutData = (GDT *) OutData1.Size(lOutCount * sizeof(GDT));
    InBuf.Lock();
    OutData1.Lock();
    #ifndef BSDSYS
    uidCurrent = getuid();
    setuid(0);
    pthread_getschedparam(ptidSoundOut, &iSchedPolicy, &sSchedParam);
    sSchedParam.sched_priority = sched_get_priority_min(SCHED_FIFO) + 1;
    if (pthread_setschedparam(ptidSoundOut, SCHED_FIFO, &sSchedParam) != 0)
    {
        g_print("SoundIn[%i] unable to set scheduling policy\n", iThisIdx);
    }
    setuid(uidCurrent);
    /*sSchedParam.sched_priority = 1;
    pthread_setschedparam(ptidSoundOut, SCHED_OTHER, &sSchedParam);*/
    #endif
    g_print("SoundIn[%i] thread running\n", iThisIdx);
    #ifndef __QNX__
    if (iDeviceBase >= 0)
    {
        g_print("SoundIn[%i] prefilling output buffer...\n", iThisIdx);
        do {
            Audio.Write(ipOutBuf, lOutBufSize);
            Audio.GetOutBufInfo(&sAudioBufInfo);
        } while (sAudioBufInfo.bytes >= lOutBufSize);
    }
    #endif
    // Wait for one message size
    // This is not very clean, but we shouldn't create multilayer buffering
    // for latency's sake!
    float fTime = 1.0f / (iLocalSR * iLocalCh);
    usleep((unsigned long) (lExtCount * fTime * 1000000.0f + 0.5f));
    while (bLocalRun)
    {
        if (SOp.ReadSelect(SUI_IN_TIMEOUT))
        {
            if (SOp.ReadN(cpMsgBuf, iMsgSize) == iMsgSize)
            {
                SoundMsg.GetData(cpMsgBuf, fpInBuf, lExtCount);
                DSP.Extract(fpOutData, fpInBuf, iSrcCh, sSoundHdr.iChannels,
                    lExtCount);
                fInLevel = DSP.PeakLevel(fpOutData, lOutCount);
                MutexLevel.Wait();
                fpInputLevel[iThisIdx] = 
                    (fpInputLevel[iThisIdx] + fInLevel) * (gfloat) 0.5;
                MutexLevel.Release();
                MutexFilter[iThisIdx].Wait();
                DSP.Mul(fpOutData, fpLevelCoeff[iThisIdx], lOutCount);
                if (bpEqEnabled[iThisIdx]) 
                    Filters[iThisIdx].Process(fpOutData);
                MutexFilter[iThisIdx].Release();
                fOutLevel = DSP.PeakLevel(fpOutData, lOutCount);
                MutexLevel.Wait();
                fpOutputLevel[iThisIdx] =
                    (fpOutputLevel[iThisIdx] + fOutLevel) * 
                    (gfloat) 0.5;
                MutexLevel.Release();
                if (iDeviceBase < 0)
                {
                    MutexData.Wait();
                    bWait = ((lDataRefCount & lChMask) == lChMask) ?
                        true : false;
                    MutexData.Release();
                    MutexChData[iThisIdx].Wait();
                    if (bWait)
                    {
                        CondData[iThisIdx].Wait(
                            MutexChData[iThisIdx].GetPtr());
                    }
                    DSP.Copy((GDT *) ChData[iThisIdx], fpOutData, lOutCount);
                    MutexData.Wait();
                    lDataRefCount |= lChMask;
                    bLocalRun = bpConnected[iThisIdx];
                    MutexData.Release();
                    MutexChData[iThisIdx].Release();
                }
                #ifndef __QNX__
                else
                {
                    DSP.Convert(ipOutData, fpOutData, lOutCount, false);
                    for (lSampleCntr = 0L; 
                        lSampleCntr < lOutCount; 
                        lSampleCntr++)
                    {
                        ipOutBuf[lSampleCntr * iLocalCh + iDestCh] =
                            ipOutData[lSampleCntr];
                    }
                    Audio.Write(ipOutBuf, lOutBufSize);
                    MutexData.Wait();
                    bLocalRun = bpConnected[iThisIdx];
                    MutexData.Release();
                }
                #endif
            }
            else bLocalRun = false;
        }
        else
        {
            if (SOp.GetErrno() != 0) bLocalRun = false;
        }
        MutexData.Wait();
        bLocalRun = bpConnected[iThisIdx];
        MutexData.Release();
    }
    SOp.Close();
    MutexData.Wait();
    bpConnected[iThisIdx] = false;
    MutexData.Release();
    #ifndef __QNX__
    if (iDeviceBase >= 0)
    {
        Audio.Reset();
        Audio.Close();
    }
    #endif
    g_print("SoundIn[%i] thread ending\n", iThisIdx);
    return NULL;
}


// clSoundChGUI


clSoundChGUI::clSoundChGUI (GtkWidget *gwAttachBox, int iChannelNo, 
    int iOctaveCount, long lFilterSize)
{
    int iOctaveCntr;
    char cpConvBuf[SUI_CONV_LEN];
    gfloat fFreq;

    g_snprintf(cpConvBuf, SUI_CONV_LEN, "%s %i", cpChannelPfx, iChannelNo);
    gwFrame = gtk_frame_new(cpConvBuf);
    // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwAttachBox), gwFrame, TRUE, TRUE, 0);
    gtk_widget_show(gwFrame);
    // homogenous, spacing
    gwVBox = gtk_vbox_new(FALSE, SUI_PADDING);
    gtk_container_add(GTK_CONTAINER(gwFrame), gwVBox);
    gtk_widget_show(gwVBox);

    // Table 1
    // rows, columns, homogenous
    gwTable1 = gtk_table_new(2, 3, FALSE);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable1, FALSE, FALSE, 0);
    gtk_widget_show(gwTable1);

    gwLServer = gtk_label_new(cpLServerTxt);
    gtk_label_set_justify(GTK_LABEL(gwLServer), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable1), gwLServer,
        0, 1, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_SHRINK|GTK_EXPAND),
        (GtkAttachOptions) 0,
        SUI_PADDING / 2, 0);
    gtk_widget_show(gwLServer);
    gwCServer = gtk_combo_new();
    gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO(gwCServer)->entry), 
        SUI_SERV_MAXLEN);
    gtk_table_attach(GTK_TABLE(gwTable1), gwCServer,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_SHRINK|GTK_EXPAND),
        (GtkAttachOptions) 0,
        SUI_PADDING / 2, 0);
    gtk_widget_show(gwCServer);
    glServers = NULL;
    GtkUtils.ComboListFromFile(gwCServer, &glServers, SUI_HOSTFILE);

    gwLChannel = gtk_label_new(cpLChannelTxt);
    gtk_label_set_justify(GTK_LABEL(gwLChannel), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable1), gwLChannel,
        1, 2, 0, 1,
        (GtkAttachOptions) 0,
        (GtkAttachOptions) 0,
        SUI_PADDING / 2, 0);
    gtk_widget_show(gwLChannel);
    // value, lower, higher, step, page, page size
    goAChannel = gtk_adjustment_new(SUI_CH_VALUE, SUI_CH_LOWER, SUI_CH_HIGHER,
        1.0, 1.0, 1.0);
    // climb rate, digits
    gwSBChannel = gtk_spin_button_new(GTK_ADJUSTMENT(goAChannel),
        1.0, 0);
    gtk_table_attach(GTK_TABLE(gwTable1), gwSBChannel,
        1, 2, 1, 2,
        (GtkAttachOptions) 0,
        (GtkAttachOptions) 0,
        SUI_PADDING / 2, 0);
    gtk_widget_show(gwSBChannel);

    gwBConnect = gtk_button_new_with_label(cpBConnectTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBConnect,
        2, 3, 1, 2,
        (GtkAttachOptions) 0,
        (GtkAttachOptions) 0,
        SUI_PADDING / 2, 0);
    gtk_widget_show(gwBConnect);

    gwLInputLevel = gtk_label_new(cpLInputLevelTxt);
    gtk_label_set_justify(GTK_LABEL(gwLInputLevel), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwLInputLevel, FALSE, FALSE, 0);
    gtk_widget_show(gwLInputLevel);
    gwPBInputLevel = gtk_progress_bar_new();
    gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(gwPBInputLevel),
        GTK_PROGRESS_LEFT_TO_RIGHT);
    gtk_progress_set_show_text(GTK_PROGRESS(gwPBInputLevel), TRUE);
    gtk_progress_set_text_alignment(GTK_PROGRESS(gwPBInputLevel),
        0.0, 0.5);
    gtk_progress_set_format_string(GTK_PROGRESS(gwPBInputLevel),
        "%v dB");
    //gtk_progress_set_activity_mode(GTK_PROGRESS(gwPBInputLevel), TRUE);
    gtk_progress_configure(GTK_PROGRESS(gwPBInputLevel),
        SUI_IN_VALUE, SUI_IN_LOWER, SUI_IN_HIGHER);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwPBInputLevel, FALSE, FALSE, 0);
    gtk_widget_show(gwPBInputLevel);

    gwCBEq = gtk_check_button_new_with_label(cpLEqTxt);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwCBEq, FALSE, FALSE, 0);
    gtk_widget_show(gwCBEq);
    gwTableEq = gtk_table_new(2, iOctaveCount, FALSE);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTableEq, TRUE, TRUE, 0);
    gtk_widget_show(gwTableEq);
    gwLOutputLevel = gtk_label_new(cpLOutputLevelTxt);
    gtk_table_attach(GTK_TABLE(gwTableEq), gwLOutputLevel,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND),
        (GtkAttachOptions) 0,
        SUI_PADDING / 2, 0);
    gtk_widget_show(gwLOutputLevel);
    goAOutputLevel = gtk_adjustment_new(0.0, -(SUI_OUT_RANGE), SUI_OUT_RANGE,
        SUI_OUT_STEP, 1.0, 1.0);
    gwVSOutputLevel = gtk_vscale_new(GTK_ADJUSTMENT(goAOutputLevel));
    gtk_scale_set_draw_value(GTK_SCALE(gwVSOutputLevel), FALSE);
    gtk_table_attach(GTK_TABLE(gwTableEq), gwVSOutputLevel,
        0, 1, 0, 1,
        (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND),
        (GtkAttachOptions) (GTK_FILL|GTK_SHRINK|GTK_EXPAND),
        SUI_PADDING / 2, 0);
    gtk_widget_show(gwVSOutputLevel);
    gwPBOutputLevel = gtk_progress_bar_new();
    gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(gwPBOutputLevel),
        GTK_PROGRESS_BOTTOM_TO_TOP);
    gtk_progress_set_show_text(GTK_PROGRESS(gwPBOutputLevel), TRUE);
    gtk_progress_set_text_alignment(GTK_PROGRESS(gwPBOutputLevel),
        0.5, 1.0);
    gtk_progress_set_format_string(GTK_PROGRESS(gwPBOutputLevel),
        "%v dB");
    //gtk_progress_set_activity_mode(GTK_PROGRESS(gwPBOutputLevel), TRUE);
    gtk_progress_configure(GTK_PROGRESS(gwPBOutputLevel),
        SUI_OUT_VALUE, SUI_OUT_LOWER, SUI_OUT_HIGHER);
    gtk_table_attach(GTK_TABLE(gwTableEq), gwPBOutputLevel,
        1, 2, 0, 2,
        (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND),
        (GtkAttachOptions) (GTK_FILL|GTK_SHRINK|GTK_EXPAND),
        SUI_PADDING / 2, 0);
    gtk_widget_show(gwPBOutputLevel);
    for (iOctaveCntr = 0; iOctaveCntr < iOctaveCount; iOctaveCntr++)
    {
        fFreq = pow(2.0, iOctaveCntr + 1);
        if (fFreq < 1000.0)
            g_snprintf(cpConvBuf, SUI_CONV_LEN, "%.0f", fFreq);
        else
            g_snprintf(cpConvBuf, SUI_CONV_LEN, "%.1fk", fFreq / 1000.0);
        gwaLEqLevel[iOctaveCntr] = gtk_label_new(cpConvBuf);
        gtk_table_attach(GTK_TABLE(gwTableEq), gwaLEqLevel[iOctaveCntr],
            2 + iOctaveCntr, 3 + iOctaveCntr, 1, 2,
            (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND),
            (GtkAttachOptions) 0,
            SUI_PADDING / 2, 0);
        gtk_widget_show(gwaLEqLevel[iOctaveCntr]);
        goaAEqLevel[iOctaveCntr] = gtk_adjustment_new(
            0.0, -(SUI_EQ_RANGE), SUI_EQ_RANGE, SUI_EQ_STEP, 1.0, 1.0);
        gwaVSEqLevel[iOctaveCntr] = gtk_vscale_new(
            GTK_ADJUSTMENT(goaAEqLevel[iOctaveCntr]));
        gtk_scale_set_draw_value(GTK_SCALE(gwaVSEqLevel[iOctaveCntr]), FALSE);
        gtk_table_attach(GTK_TABLE(gwTableEq), gwaVSEqLevel[iOctaveCntr],
            2 + iOctaveCntr, 3 + iOctaveCntr, 0, 1,
            (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND),
            (GtkAttachOptions) (GTK_FILL|GTK_SHRINK|GTK_EXPAND),
            SUI_PADDING / 2, 0);
        gtk_widget_show(gwaVSEqLevel[iOctaveCntr]);
    }

    // Curve: Eq
    gwCurveEq = gtk_curve_new();
    gtk_box_pack_start(GTK_BOX(gwVBox), gwCurveEq, TRUE, TRUE, 0);
    gtk_curve_set_range(GTK_CURVE(gwCurveEq), 
        0.0, (gfloat) lFilterSize,
        -SUI_EQ_RANGE, SUI_EQ_RANGE);
        
    #ifdef __GNUG__
    GDT fpInitVect[lFilterSize];
    #else
    clDSPAlloc InitVect;
    GDT *fpInitVect = (GDT *) InitVect.Size(lFilterSize * sizeof(GDT));
    #endif
    clDSPOp DSP;
    
    DSP.Set(fpInitVect, 0.0, lFilterSize);
    gtk_curve_set_vector(GTK_CURVE(gwCurveEq), lFilterSize, fpInitVect);
    gtk_widget_set_usize(gwCurveEq, gwCurveEq->requisition.width,
        gwCurveEq->requisition.width);
    gtk_widget_show(gwCurveEq);

    // Button: Apply curve
    gwBApplyCurve = gtk_button_new_with_label(cpBApplyCurveTxt);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwBApplyCurve, FALSE, FALSE, 0);
    gtk_widget_show(gwBApplyCurve);
}


clSoundChGUI::~clSoundChGUI ()
{
}

