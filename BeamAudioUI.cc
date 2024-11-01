/*

    User interface for beam audio server
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
#include <math.h>
#include <float.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gtk/gtk.h>

#include "BeamAudioUI.hh"


G_LOCK_DEFINE_STATIC(gmInputMutex);

clBeamAudioUI *BeamAudioUI;


int main (int argc, char *argv[])
{
    int iRetVal = 0;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    BeamAudioUI = new clBeamAudioUI(&argc, &argv);
    iRetVal = BeamAudioUI->Main();
    delete BeamAudioUI;
    return iRetVal;
}


gboolean WrapOnDelete (GtkWidget *gwSender, GdkEvent *geEvent, gpointer gpData)
{
    return BeamAudioUI->OnDelete(gwSender, geEvent, gpData);
}


void WrapOnConnectClick (GtkButton *gbButton, gpointer gpData)
{
    BeamAudioUI->OnConnectClick(gbButton, gpData);
}


void WrapOnValueChanged (GtkAdjustment *gaAdjustment, gpointer gpData)
{
    BeamAudioUI->OnValueChanged(gaAdjustment, gpData);
}


void WrapOnToggled (GtkToggleButton *gtbToggleButton, gpointer gpData)
{
    BeamAudioUI->OnToggled(gtbToggleButton, gpData);
}


void WrapOnGdkInput (gpointer gpData, gint giSource,
    GdkInputCondition gicCondition)
{
    BeamAudioUI->OnGdkInput(gpData, giSource, gicCondition);
}


void *WrapAudioOutThread (void *vpParam)
{
    return BeamAudioUI->AudioOutThread(vpParam);
}


void clBeamAudioUI::Build ()
{
    // - Main window
    gwWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gwWindow), cpWindowTxt);
    // shrink, grow, auto-shrink
    gtk_window_set_policy(GTK_WINDOW(gwWindow), TRUE, TRUE, FALSE);

    // - VerticalBox
    // homogenous, spacing
    gwVBox = gtk_vbox_new(FALSE, BAUI_PADDING);
    gtk_container_add(GTK_CONTAINER(gwWindow), gwVBox);
    gtk_widget_show(gwVBox);

    // - Table 1
    BuildTable1();

    // - Table 2
    BuildTable2();

    // - Label & HorizontalScale: Direction
    gwLDirection = gtk_label_new(cpLDirectionTxt);
    gtk_label_set_justify(GTK_LABEL(gwLDirection), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwLDirection, TRUE, FALSE, 0);
    gtk_widget_show(gwLDirection);
    goADirection = gtk_adjustment_new(0.0, 
        -(BAUI_DEF_DIR_RANGE / 2.0), BAUI_DEF_DIR_RANGE / 2.0, 
        1.0, 10.0, 1.0);
    gwHSDirection = gtk_hscale_new(GTK_ADJUSTMENT(goADirection));
    gtk_scale_set_draw_value(GTK_SCALE(gwHSDirection), TRUE);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwHSDirection, TRUE, FALSE, 0);
    gtk_widget_show(gwHSDirection);

    // - Label & HorizontabScale: Distance
    /*gwLDistance = gtk_label_new(cpLDistanceTxt);
    gtk_label_set_justify(GTK_LABEL(gwLDistance), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwLDistance, TRUE, FALSE, 0);
    gtk_widget_show(gwLDistance);
    // value, lower, upper, step_increment, page_increment, page_size
    goADistance = gtk_adjustment_new(0.0, 0.0, 1.0, 0.01, 0.01, 0.01);
    gwHSDistance = gtk_hscale_new(GTK_ADJUSTMENT(goADistance));
    gtk_scale_set_digits(GTK_SCALE(gwHSDistance), 2);
    gtk_scale_set_draw_value(GTK_SCALE(gwHSDistance), TRUE);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwHSDistance, TRUE, FALSE, 0);
    gtk_widget_show(gwHSDistance);*/

    // - StatusBar
    gwStatusBar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(gwVBox), gwStatusBar, FALSE, FALSE, 0);
    gtk_widget_show(gwStatusBar);
    guSbCtxt = gtk_statusbar_get_context_id(GTK_STATUSBAR(gwStatusBar),
        "status");
    gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guSbCtxt, "");

    // Delayed realization of main window
    gtk_widget_show(gwWindow);

    ConnectSignals();
}


void clBeamAudioUI::BuildTable1 ()
{
    // rows, columns, homogenous
    gwTable1 = gtk_table_new(2, 2, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable1, TRUE, FALSE, 0);
    gtk_widget_show(gwTable1);

    // - Label & Combo: Server
    gwLServer = gtk_label_new(cpLServerTxt);
    gtk_label_set_justify(GTK_LABEL(gwLServer), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable1), gwLServer,
        0, 1, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        BAUI_PADDING / 2, 0);
    gtk_widget_show(gwLServer);
    gwCServer = gtk_combo_new();
    gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        BAUI_SERVER_MAXLEN);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(gwCServer)->entry),
        "127.0.0.1:30001");
    // table, child, left attach, right attach, top attach, bottom attach,
    // x-options, y-options, x-padding, y-padding
    gtk_table_attach(GTK_TABLE(gwTable1), gwCServer,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        BAUI_PADDING / 2, 0);
    gtk_widget_show(gwCServer);
    GtkUtils.ComboListFromFile(gwCServer, &glServer, BAUI_HOSTFILE);

    // - Button: Connect
    gwBConnect = gtk_button_new_with_label(cpBConnectTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBConnect,
        1, 2, 1, 2,
        (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) 0,
        BAUI_PADDING / 2, 0);
    gtk_widget_show(gwBConnect);
}


void clBeamAudioUI::BuildTable2 ()
{
    // rows, columns, homogenous
    gwTable2 = gtk_table_new(2, 3, FALSE);
    // box, child, expand, fill, padding
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable2, TRUE, FALSE, 0);
    gtk_widget_show(gwTable2);

    // - Label & Entry: Sound speed
    gwLSoundSpeed = gtk_label_new(cpLSoundSpeedTxt);
    gtk_label_set_justify(GTK_LABEL(gwLSoundSpeed), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLSoundSpeed,
        0, 1, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        BAUI_PADDING / 2, 0);
    gtk_widget_show(gwLSoundSpeed);
    gwESoundSpeed = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable2), gwESoundSpeed,
        0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND|GTK_SHRINK),
        (GtkAttachOptions) 0,
        BAUI_PADDING / 2, 0);
    gtk_widget_show(gwESoundSpeed);
    gtk_entry_set_text(GTK_ENTRY(gwESoundSpeed), BAUI_DEF_SOUNDSPEED);

    // - CheckButton: High frequencies
    gwCBHighFreq = gtk_check_button_new_with_label(cpCBHighFreqTxt);
    gtk_table_attach(GTK_TABLE(gwTable2), gwCBHighFreq,
        1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
        (GtkAttachOptions) 0,
        BAUI_PADDING / 2, 0);
    gtk_widget_show(gwCBHighFreq);

    // - CheckButton: Dither
    gwCBDither = gtk_check_button_new_with_label(cpCBDitherTxt);
    gtk_table_attach(GTK_TABLE(gwTable2), gwCBDither,
        1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
        (GtkAttachOptions) 0,
        BAUI_PADDING / 2, 0);
    gtk_widget_show(gwCBDither);

    // - CheckButton: 3D
    gwCB3DAudio = gtk_check_button_new_with_label(cpCB3DAudioTxt);
    gtk_table_attach(GTK_TABLE(gwTable2), gwCB3DAudio,
        2, 3, 0, 1,
        (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
        (GtkAttachOptions) 0,
        BAUI_PADDING / 2, 0);
    gtk_widget_show(gwCB3DAudio);
}


void clBeamAudioUI::ConnectSignals ()
{
    gtk_signal_connect(GTK_OBJECT(gwWindow), "delete-event",
        GTK_SIGNAL_FUNC(WrapOnDelete), NULL);
        
    gtk_signal_connect(GTK_OBJECT(gwBConnect), "clicked",
        GTK_SIGNAL_FUNC(WrapOnConnectClick), NULL);

    gtk_signal_connect(goADirection, "value-changed",
        GTK_SIGNAL_FUNC(WrapOnValueChanged), NULL);

    /*gtk_signal_connect(goADistance, "value-changed",
        GTK_SIGNAL_FUNC(WrapOnValueChanged), NULL);*/

    gtk_signal_connect(GTK_OBJECT(gwCBHighFreq), "toggled",
        GTK_SIGNAL_FUNC(WrapOnToggled), NULL);

    gtk_signal_connect(GTK_OBJECT(gwCBDither), "toggled",
        GTK_SIGNAL_FUNC(WrapOnToggled), NULL);

    gtk_signal_connect(GTK_OBJECT(gwCB3DAudio), "toggled",
        GTK_SIGNAL_FUNC(WrapOnToggled), NULL);
}


bool clBeamAudioUI::ParseServerStr (char *cpHostAddr, int *ipHostPort, 
    const char *cpSourceStr)
{
    char cpTempStr[BAUI_SERVER_MAXLEN + 1];
    char *cpTempHost;
    char *cpTempPort;

    strncpy(cpTempStr, cpSourceStr, BAUI_SERVER_MAXLEN);
    cpTempHost = strtok(cpTempStr, ": \t\n");
    if (cpTempHost != NULL)
    {
        strcpy(cpHostAddr, cpTempHost);
        cpTempPort = strtok(NULL, " \t\n");
        if (cpTempPort != NULL)
        {
            *ipHostPort = atoi(cpTempPort);
            if (*ipHostPort > 0) return true;
        }
    }
    return false;
}


bool clBeamAudioUI::InitConnection (const char *cpHostAddr, int iHostPort)
{
    int iSockH;
    char cpReqProcName[GLOBAL_HEADER_LEN];

    iSockH = SClient.Connect(cpHostAddr, NULL, iHostPort);
    if (iSockH < 0)
    {
        g_print("Failed to connect host!\n");
        return false;
    }
    g_print("Connection established - sending process request...\n");
    bConnected = true;
    SOp.SetHandle(iSockH);
    strcpy(cpReqProcName, BAUI_REQ_PROC);
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
    lClips = 0l;
    giGdkTag = gdk_input_add(iSockH, GDK_INPUT_READ, WrapOnGdkInput, NULL);
    return true;
}


bool clBeamAudioUI::SendSettings ()
{
    char cpMsgBuf[GLOBAL_HEADER_LEN];
    stBeamAudioFirst sFirst;

    sRequest.fDirection = GTK_ADJUSTMENT(goADirection)->value;
    sscanf(gtk_entry_get_text(GTK_ENTRY(gwESoundSpeed)), "%f",
        &sRequest.fSoundSpeed);
    sRequest.bHighFreq = 
        (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gwCBHighFreq))) ?
        true : false;
    Msg.SetRequest(cpMsgBuf, &sRequest);
    if (SOp.WriteN(cpMsgBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    if (!SOp.ReadSelect(BAUI_1ST_TIMEOUT))
        return false;
    if (SOp.ReadN(cpMsgBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
        return false;
    Msg.GetFirst(cpMsgBuf, &sFirst);
    AllocateBuffers(&sFirst);
    lBufIdx = 0l;
    pthread_create(&ptidAudio, NULL, WrapAudioOutThread, NULL);
    return true;
}


void clBeamAudioUI::AllocateBuffers (const stpBeamAudioFirst spFirst)
{
    long lBufCntr;
    long lSampleCntr;
    char *cpDitherRand;

    iSampleRate = spFirst->iSampleRate;
    lFragSize = spFirst->lBufLength;
    lDataCount = lFragSize * 2;
    lMsgSize = GLOBAL_HEADER_LEN + lFragSize * sizeof(GDT);
    MessageBuf.Size(lMsgSize);
    InAudioBuf.Size(lFragSize * sizeof(GDT));
    fpInAudio = InAudioBuf;
    SrcAudioBuf.Size(lDataCount * sizeof(GDT));
    fpSrcAudio = SrcAudioBuf;
    DitherBuf.Size(lDataCount * sizeof(BAUI_INT_DATATYPE));
    DitherRandBuf.Size(lFragSize * sizeof(char));
    for (lBufCntr = 0l; lBufCntr < BAUI_AUDIO_BUFCOUNT; lBufCntr++)
    {
        OutAudioBuf[lBufCntr].Size(
            lDataCount * sizeof(BAUI_SND_DATATYPE));
        ipOutAudio[lBufCntr] = OutAudioBuf[lBufCntr];
        memset(ipOutAudio[lBufCntr], 0x00, 
            spFirst->lBufLength * sizeof(BAUI_SND_DATATYPE));
    }
    Audio3D.Initialize(lFragSize);
    g_print("Incoming fragment size %li (%.3f ms)\n", 
        lFragSize, (float) lFragSize / (float) iSampleRate * 1000.0f);
    g_print("Buffering %i fragments (%.3f ms)\n",
        BAUI_AUDIO_BUFCOUNT,
        (float) lFragSize / (float) iSampleRate * 1000.0f * 
        BAUI_AUDIO_BUFCOUNT);
    // Initialize random buffer for dither
    cpDitherRand = DitherRandBuf;
    if (iRandH >= 0)
    {
        if (read(iRandH, cpDitherRand, lFragSize) < lFragSize)
        {
            g_print("Read from /dev/urandom failed!?\n");
        }
    }
    else
    {
        for (lSampleCntr = 0; lSampleCntr < lFragSize; lSampleCntr++)
        {
            cpDitherRand[lSampleCntr] = (char) (random() & 0xff);
        }
    }
}


#if (!defined(BSDSYS) && !defined(__QNX__))
#ifdef USE_ALSA05
bool clBeamAudioUI::InitAudio (clAudio &Audio, clAudioA &AudioA)
#else
bool clBeamAudioUI::InitAudio (clAudio &Audio, clAudioA2 &AudioA)
#endif
#elif defined(BSDSYS)
bool clBeamAudioUI::InitAudio (clAudio &Audio)
#elif defined(__QNX__)
bool clBeamAudioUI::InitAudio (clAudioA &AudioA)
#endif
{
    int iAudioSR = iSampleRate;
    int iAudioCh = 2;

    if (!bALSA)
    {
        #ifndef __QNX__
        int iAudioFormat = BAUI_SND_FORMAT;
        char cpDevice[_POSIX_PATH_MAX + 1];

        if (!Cfg.GetStr("AudioDevice", cpDevice))
            strcpy(cpDevice, "/dev/dsp");
        g_print("Open device %s fs %i ch %i fmt %xh\n", cpDevice, iAudioSR,
            iAudioCh, iAudioFormat);
        Audio.Open(cpDevice, &iAudioFormat, &iAudioSR, &iAudioCh, 
            AUDIO_WRITE);
        g_print("Device opened fs %i ch %i fmt %xh\n",
            iAudioSR, iAudioCh, iAudioFormat);
        #else
        g_warning("OSS not supported under QNX");
        #endif
    }
    else
    {
        #ifndef BSDSYS
        int iALSACard;
        int iALSADevice;
        int iALSASubDevice;
        int iAudioBits = BAUI_SND_BITS;
        #ifndef USE_ALSA05
        char cpDeviceId[10];
        #endif

        if (!Cfg.GetInt("ALSACard", &iALSACard))
            iALSACard = 0;
        if (!Cfg.GetInt("ALSADevice", &iALSADevice))
            iALSADevice = 0;
        if (!Cfg.GetInt("ALSASubDevice", &iALSASubDevice))
            iALSASubDevice = 0;
        g_print("Open device %i:%i:%i fs %i ch %i wl %i\n", 
            iALSACard, iALSADevice, iALSASubDevice, 
            iAudioSR, iAudioCh, iAudioBits);
        if (!AudioA.CardOpen(iALSACard))
        {
            g_print("ALSA card open failed!\n");
            return false;
        }
        #ifdef USE_ALSA05
        if (!AudioA.PcmOpen(iALSADevice, iALSASubDevice, AA_MODE_PLAY))
        #else
        sprintf(cpDeviceId, "plughw:%i,%i", iALSACard, iALSADevice);
        if (!AudioA.PcmOpen(cpDeviceId, AA_MODE_PLAY))
        #endif
        {
            g_print("ALSA PCM device open failed!\n");
            return false;
        }
        #ifdef USE_ALSA05
        if (!AudioA.PcmSetSetup(iAudioCh, iAudioSR, iAudioBits, 
            lFragSize * sizeof(BAUI_SND_DATATYPE), false))
        #else
        if (!AudioA.PcmSetSetup(iAudioCh, iAudioSR, iAudioBits, 
            lFragSize * sizeof(BAUI_SND_DATATYPE), 2))
        #endif
        {
            g_print("ALSA PCM setup failed!\n");
            return false;
        }
        g_print("Device opened fs %i ch %i wl %i frag %i bytes\n",
            AudioA.PcmGetSampleRate(),
            AudioA.PcmGetChannels(),
            AudioA.PcmGetBits(),
            AudioA.PcmGetFragmentSize());
        if (!AudioA.PcmPrepare())
        {
            g_print("ALSA PCM prepare failed!\n");
            return false;
        }
        #else
        g_warning("ALSA not supported under BSD systems");
        #endif
    }
    return true;
}


void clBeamAudioUI::UpdateSettings ()
{
    char cpMsgBuf[GLOBAL_HEADER_LEN];
    
    if (bConnected)
    {
        Msg.SetRequest(cpMsgBuf, &sRequest);
        if (SOp.WriteSelect(0))
        {
            if (SOp.WriteN(cpMsgBuf, GLOBAL_HEADER_LEN) < GLOBAL_HEADER_LEN)
            {
                g_print("SockOp::WriteN() error: %s\n",
                    strerror(SOp.GetErrno()));
            }
        }
        else
        {
            g_print("Server busy...\n");
        }
    }
}


void clBeamAudioUI::ConvertMS ()
{
    long lSrcCntr;
    long lDestCntr;

    lDestCntr = 0;
    for (lSrcCntr = 0; lSrcCntr < lFragSize; lSrcCntr++)
    {
        fpSrcAudio[lDestCntr++] = fpInAudio[lSrcCntr];
        fpSrcAudio[lDestCntr++] = fpInAudio[lSrcCntr];
    }
}


void clBeamAudioUI::Process3D ()
{
    long lBlockSize;
    long lBlockCount;
    long lBlockCntr;

    lBlockSize = Audio3D.GetWindowSize();
    lBlockCount = lFragSize / lBlockSize;
    for (lBlockCntr = 0; lBlockCntr < lBlockCount; lBlockCntr++)
    {
        Audio3D.Process(&fpSrcAudio[lBlockCntr * lBlockSize * 2]);
    }
}


void clBeamAudioUI::Dither ()
{
    long lDithCntr;
    long lSampleCntr;
    BAUI_INT_DATATYPE *ipDither;
    char *cpDitherRand;

    ipDither = DitherBuf;
    cpDitherRand = DitherRandBuf;
    lSampleCntr = 0;
    for (lDithCntr = 0; lDithCntr < lFragSize; lDithCntr++)
    {
        ipDither[lSampleCntr] = 
            DSP.Round(fpSrcAudio[lSampleCntr] * 0x007fff00);
        ipDither[lSampleCntr] += cpDitherRand[lDithCntr];
        lSampleCntr++;
        ipDither[lSampleCntr] =
            DSP.Round(fpSrcAudio[lSampleCntr] * 0x007fff00);
        ipDither[lSampleCntr] += cpDitherRand[lDithCntr];
        lSampleCntr++;
    }
}


void clBeamAudioUI::ConvertFromDither ()
{
    long lSampleCntr;
    BAUI_INT_DATATYPE *ipConvSrc;
    BAUI_SND_DATATYPE *ipConvDest;

    ipConvSrc = DitherBuf;
    ipConvDest = ipOutAudio[lBufIdx];
    for (lSampleCntr = 0; lSampleCntr < lDataCount; lSampleCntr++)
    {
        ipConvDest[lSampleCntr] = (BAUI_SND_DATATYPE) 
            (ipConvSrc[lSampleCntr] >> 8);
    }
}


clBeamAudioUI::clBeamAudioUI (int *ipArgCount, char ***cpapArgVect)
{
    int iALSA;

    bConnected = false;
    bDither = false;
    b3DAudio = false;
    bALSA = false;
    iPrevHeading = 0;

    g_print("BeamAudio UI v%i.%i.%i\n", BAUI_VER_MAJ, BAUI_VER_MIN,
        BAUI_VER_PL);
    g_print("Copyright (C) 2000-2001 Jussi Laako\n\n");
    
    g_print("Gtk+ version %i.%i.%i\n", gtk_major_version, gtk_minor_version,
        gtk_micro_version);
    g_print("Locale set to %s\n\n", gtk_set_locale());
    gtk_init(ipArgCount, cpapArgVect);

    Cfg.SetFileName(BAUI_CFGFILE);
    if (Cfg.GetInt("UseALSA", &iALSA))
    {
        bALSA = (iALSA) ? true : false;
    }
    else bALSA = false;
    
    Build();
    
    iRandH = open("/dev/urandom", O_RDONLY);
    if (iRandH < 0)
    {
        g_print("Unable to open /dev/urandom for reading, using random()\n");
    }
}


clBeamAudioUI::~clBeamAudioUI ()
{
    if (iRandH >= 0) close(iRandH);
}


int clBeamAudioUI::Main ()
{
    gtk_main();
    return 0;
}


gboolean clBeamAudioUI::OnDelete (GtkWidget *gwSender, GdkEvent *geEvent,
    gpointer gpData)
{
    if (bConnected) gdk_input_remove(giGdkTag);
    gtk_main_quit();
    return TRUE;
}


void clBeamAudioUI::OnConnectClick (GtkButton *gbButton, gpointer gpData)
{
    int iHostPort;
    char cpHostAddr[BAUI_SERVER_MAXLEN + 1];

    if (ParseServerStr(cpHostAddr, &iHostPort,
        gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(gwCServer)->entry))))
    {
        g_print("Connecting to host %s port %i...\n", cpHostAddr, iHostPort);
        if (bConnected)
        {
            bConnected = false;
            gdk_input_remove(giGdkTag);
            SOp.Close();
            pthread_join(ptidAudio, NULL);
        }
        if (InitConnection(cpHostAddr, iHostPort))
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


void clBeamAudioUI::OnValueChanged (GtkAdjustment *gaAdjustment, 
    gpointer gpData)
{
    if (gaAdjustment == GTK_ADJUSTMENT(goADirection))
    {
        sRequest.fDirection = DSP.DegToRad(gaAdjustment->value);
        UpdateSettings();
    }
    /*else if (gaAdjustment == GTK_ADJUSTMENT(goADistance))
    {
        Audio3D.SetAngles(iPrevHeading, 0, gaAdjustment->value);
    }*/
}


void clBeamAudioUI::OnToggled (GtkToggleButton *gtbToggleButton, 
    gpointer gpData)
{
    if (gtbToggleButton == GTK_TOGGLE_BUTTON(gwCBHighFreq))
    {
        sRequest.bHighFreq = 
            (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gwCBHighFreq))) ?
            true : false;
        UpdateSettings();
    }
    else if (gtbToggleButton == GTK_TOGGLE_BUTTON(gwCBDither))
    {
        bDither =
            (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gwCBDither))) ?
            true : false;
    }
    else if (gtbToggleButton == GTK_TOGGLE_BUTTON(gwCB3DAudio))
    {
        b3DAudio =
            (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gwCB3DAudio))) ?
            true : false;
    }
}


void clBeamAudioUI::OnGdkInput (gpointer gpData, gint giSource,
    GdkInputCondition gicCondition)
{
    int iHeading;
    char cpStatusTxt[BAUI_CONV_BUF_LEN + 1];
    stBeamAudioRes sResHdr;

    if (!bConnected) return;
    G_LOCK(gmInputMutex);
    if (SOp.ReadSelect(0))
    {
        if (SOp.ReadN(MessageBuf, lMsgSize) == lMsgSize)
        {
            Msg.GetResult(MessageBuf, &sResHdr, fpInAudio);
            ConvertMS();
            iHeading = DSP.Round(DSP.RadToDeg(sResHdr.fDirection));
            if (iPrevHeading != iHeading)
            {
                /*Audio3D.SetAngles(iHeading, 0, 
                    GTK_ADJUSTMENT(goADistance)->value);*/
                Audio3D.SetAngles(iHeading, 0, 0);
                iPrevHeading = iHeading;
            }
            if (b3DAudio) Process3D();
            if (bDither) Dither();
            OutBufMutex[lBufIdx].Wait();
            if (!bDither)
            {
                DSP.Convert(ipOutAudio[lBufIdx], fpSrcAudio, lDataCount, 
                    false);
            }
            else
            {
                ConvertFromDither();
            }
            OutBufMutex[lBufIdx].Release();
            lBufIdx++;
            if (lBufIdx >= BAUI_AUDIO_BUFCOUNT) lBufIdx = 0l;
            if (sResHdr.fPeakLevel == 0.0f) lClips++;
            g_snprintf(cpStatusTxt, BAUI_CONV_BUF_LEN, 
                "%.1f deg  %.1f dB  %li clips",
                DSP.RadToDeg(sResHdr.fDirection), sResHdr.fPeakLevel, lClips);
            gtk_statusbar_pop(GTK_STATUSBAR(gwStatusBar), guSbCtxt);
            gtk_statusbar_push(GTK_STATUSBAR(gwStatusBar), guSbCtxt,
                cpStatusTxt);
        }
        else
        {
            g_print("Receive error: %s\n", strerror(SOp.GetErrno()));
        }
    }
    G_UNLOCK(gmInputMutex);
}


void *clBeamAudioUI::AudioOutThread (void *vpParam)
{
    long lOutIdx = 0l;
    long lOutBufSize = lDataCount * sizeof(BAUI_SND_DATATYPE);
    #ifndef __QNX__
    struct audio_buf_info sAudioBufInfo;
    #endif
    clAlloc ZeroBuf;
    #ifndef __QNX__
    clAudio Audio;
    #endif
    #ifndef BSDSYS
    #ifdef USE_ALSA05
    clAudioA AudioA;
    #else
    clAudioA2 AudioA;
    #endif
    #endif

    ZeroBuf.Size(lOutBufSize);
    memset(ZeroBuf, 0x00, lOutBufSize);
    #if (!defined(BSDSYS) && !defined(__QNX__))
    InitAudio(Audio, AudioA);
    #elif defined(BSDSYS)
    InitAudio(Audio);
    #elif defined(__QNX__)
    InitAudio(AudioA);
    #endif
    while (lBufIdx < (BAUI_AUDIO_BUFCOUNT / 2) && bConnected) sched_yield();
    if (!bALSA)
    {
        #ifndef __QNX__
        do
        {
            Audio.Write(ZeroBuf, lOutBufSize);
            Audio.GetOutBufInfo(&sAudioBufInfo);
        } while (sAudioBufInfo.bytes > lOutBufSize && bConnected);
        #endif
    }
    else
    {
        #ifndef BSDSYS
        #ifdef USE_ALSA05
        AudioA.PcmGo();
        #else
        AudioA.PcmStart();
        #endif
        /*do
        {
            if (AudioA.PcmWrite(ZeroBuf, lOutBufSize) < lOutBufSize)
            {
                //g_print("Error writing to PCM device\n");
                break;
            }
        } while (AudioA.PcmGetBufFree() > lOutBufSize && bConnected);*/
        /*g_message("Status: %s", 
            AudioA.PcmGetStatusStr(AudioA.PcmGetStatus()));*/
        #endif
    }
    while (bConnected)
    {
        OutBufMutex[lOutIdx].Wait();
        if (!bALSA)
        {
            #ifndef __QNX__
            if (Audio.Write(ipOutAudio[lOutIdx], lOutBufSize) < lOutBufSize)
                g_warning("Error writing to PCM device");
            #endif
        }
        else
        {
            #ifndef BSDSYS
            int iErrorCode;

            iErrorCode = AudioA.PcmWrite(ipOutAudio[lOutIdx], lOutBufSize);
            if (iErrorCode < 0)
            {
                /*g_warning("Error writing to PCM device: %s",
                    snd_strerror(iErrorCode));*/
            }
            #endif
        }
        OutBufMutex[lOutIdx].Release();
        lOutIdx++;
        if (lOutIdx >= BAUI_AUDIO_BUFCOUNT) lOutIdx = 0l;
    }
    if (!bALSA)
    {
        #ifndef __QNX__
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
    return NULL;
}

