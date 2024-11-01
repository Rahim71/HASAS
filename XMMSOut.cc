/*

    XMMS output plugin streamdist replacement
    Copyright (C) 2003 Jussi Laako

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
#include <float.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include "XMMSOut.hh"


extern "C"
{
OutputPlugin *get_oplugin_info (void);
void hasas_init (void);
void hasas_about (void);
void hasas_configure (void);
void hasas_get_volume (int *, int *);
void hasas_set_volume (int, int);
int hasas_open_audio (AFormat, int, int);
void hasas_write_audio (void *, int);
void hasas_close_audio (void);
void hasas_flush (int);
void hasas_pause (short);
int hasas_buffer_free (void);
int hasas_buffer_playing (void);
int hasas_output_time (void);
int hasas_written_time (void);
}


OutputPlugin hasas_op = {
    NULL,
    NULL,
    NULL,
    hasas_init,
    hasas_about,
    hasas_configure,
    NULL,
    NULL,
    hasas_open_audio,
    hasas_write_audio,
    hasas_close_audio,
    hasas_flush,
    hasas_pause,
    hasas_buffer_free,
    hasas_buffer_playing,
    hasas_output_time,
    hasas_written_time
};
clXMMSOut XMMSOut;
clDynThreads<clXMMSOut> XMMSOutThreads(XMMSOut);
static const int iDefBufferSize = 65536;
static const char *cpDefLocalSocket = "streamdist.socket";
/* UI component texts */
static const char *cpWinTitleTxt = "HASAS streamdist settings";
static const char *cpLBufSizeTxt = "Buffer size";
static const char *cpLLocalSocketTxt = "Local socket";
static const char *cpBOkTxt = "OK";
static const char *cpBCancelTxt = "Cancel";


extern "C"
{


OutputPlugin *get_oplugin_info()
{
    hasas_op.description = g_strdup_printf("%s %i.%i.%i", 
        XMMSOUT_DESCRIPTION, 
        GLOBAL_VERSMAJ, GLOBAL_VERSMIN, GLOBAL_VERSPL);
    return &hasas_op;
}


void hasas_init ()
{
    XMMSOut.Init();
}


void hasas_about ()
{
    XMMSOut.About();
}


void hasas_configure ()
{
    XMMSOut.Configure();
}


int hasas_open_audio (AFormat fmt, int rate, int nch)
{
    return XMMSOut.OpenAudio(fmt, rate, nch);
}


void hasas_write_audio (void *ptr, int length)
{
    XMMSOut.WriteAudio(ptr, length);
}


void hasas_close_audio ()
{
    XMMSOut.CloseAudio();
}


void hasas_flush (int time)
{
    XMMSOut.Flush(time);
}


void hasas_pause (short paused)
{
    XMMSOut.Pause(paused);
}


int hasas_buffer_free ()
{
    return XMMSOut.BufferFree();
}


int hasas_buffer_playing ()
{
    return XMMSOut.BufferPlaying();
}


int hasas_output_time ()
{
    return XMMSOut.OutputTime();
}


int hasas_written_time ()
{
    return XMMSOut.WrittenTime();
}


void WrapOnAboutButton (GtkButton *gbSender, gpointer gpData)
{
    XMMSOut.OnAboutButton(gbSender, gpData);
}


void WrapOnButtonClick (GtkButton *gbSender, gpointer gpData)
{
    XMMSOut.OnButtonClick(gbSender, gpData);
}

}


inline double clXMMSOut::GetTime ()
{
    double dTime;
    struct timeval sTime;
    
    gettimeofday(&sTime, NULL);
    dTime = (double) sTime.tv_sec + (double) sTime.tv_usec / 1.0e6;
    return dTime;
}


inline void clXMMSOut::Convert8s8u (void *vpData, int iCount)
{
    int iSampleCntr;
    signed char *cpSrcData = (signed char *) vpData;
    unsigned char *cpDestData = (unsigned char *) vpData;

    for (iSampleCntr = 0; iSampleCntr < iCount; iSampleCntr++)
    {
        cpDestData[iSampleCntr] = (unsigned char)
            ((int) cpSrcData[iSampleCntr] + 0x7f);
    }
}


inline void clXMMSOut::Convert16u16s (void *vpData, int iCount)
{
    int iSampleCntr;
    unsigned short *uipSrcData = (unsigned short *) vpData;
    signed short *ipDestData = (signed short *) vpData;

    for (iSampleCntr = 0; iSampleCntr < iCount; iSampleCntr++)
    {
        ipDestData[iSampleCntr] = (signed short)
            ((int) uipSrcData[iSampleCntr] - 0x7fff);
    }
}


inline void clXMMSOut::EndianConvert (unsigned short *uipData, int iCount)
{
    int iSampleCntr;
    
    for (iSampleCntr = 0; iSampleCntr < iCount; iSampleCntr++)
    {
        uipData[iSampleCntr] = GUINT16_SWAP_LE_BE(uipData[iSampleCntr]);
    }
}


inline void clXMMSOut::CopyChannel (GDT *fpDest, const GDT *fpSrc, 
    int iChannel)
{
    int iSampleCntr;
    int iSampleCount;

    iSampleCount = iFragmentSize / sHdr.iChannels;
    for (iSampleCntr = 0; iSampleCntr < iSampleCount; iSampleCntr++)
    {
        fpDest[iSampleCntr] = fpSrc[iSampleCntr * sHdr.iChannels + iChannel];
    }
}


clXMMSOut::clXMMSOut ()
{
    bRun = false;
    bPause = false;
    iMainThreadH = -1;
    iAudioBufSize = 0;
    iFragmentSize = 0;
    dStartTime = 0.0;
    cpLocalSocket = NULL;
    gwWinConfig = NULL;
}


clXMMSOut::~clXMMSOut ()
{
    CloseAudio();
}


void clXMMSOut::Init ()
{
    int iBufSize;
    ConfigFile *cfCfgFile;

    cfCfgFile = xmms_cfg_open_default_file();
    if (cfCfgFile == NULL)
        cfCfgFile = xmms_cfg_new();
    if (!xmms_cfg_read_int(cfCfgFile, "hasas",
        "buffersize", &iBufSize));
        iBufSize = iDefBufferSize;
    iAudioBufSize = iBufSize;
    if (cpLocalSocket)
        g_free(cpLocalSocket);
    if (!xmms_cfg_read_string(cfCfgFile, "hasas", 
        "localsocket", &cpLocalSocket))
        cpLocalSocket = g_strdup(cpDefLocalSocket);
    xmms_cfg_free(cfCfgFile);

    iFragmentSize = iAudioBufSize / sizeof(GDT);
    AudioBuf.Size(iFragmentSize * sizeof(GDT));
    if (!SServ.Bind(cpLocalSocket))
        g_warning("hasas_op: failed to bind local socket %s", cpLocalSocket);
}


void clXMMSOut::About ()
{
    cpMessageTxt = g_strdup_printf(
        "%s %i.%i.%i\nCopyright (C) 2003 Jussi Laako", 
        XMMSOUT_DESCRIPTION, 
        GLOBAL_VERSMAJ, GLOBAL_VERSMIN, GLOBAL_VERSPL);
    gwMessageBox = xmms_show_message("About plugin", cpMessageTxt, "OK", TRUE,
        GTK_SIGNAL_FUNC(WrapOnAboutButton), NULL);
}


void clXMMSOut::Configure ()
{
    gchar *cpEntryTxt;

    if (gwWinConfig)
    {
        gdk_window_raise(gwWinConfig->window);
        return;
    }

    gwWinConfig = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gwWinConfig), cpWinTitleTxt);
    gtk_window_set_policy(GTK_WINDOW(gwWinConfig), FALSE, FALSE, FALSE);
    
    gwVBox = gtk_vbox_new(FALSE, 4);
    gtk_container_add(GTK_CONTAINER(gwWinConfig), gwVBox);
    gtk_widget_show(gwVBox);
    
    gwLBufSize = gtk_label_new(cpLBufSizeTxt);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwLBufSize, FALSE, FALSE, 0);
    gtk_widget_show(gwLBufSize);
    
    gwEBufSize = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(gwVBox), gwEBufSize, FALSE, FALSE, 0);
    gtk_widget_show(gwEBufSize);
    cpEntryTxt = g_strdup_printf("%i", iAudioBufSize);
    gtk_entry_set_text(GTK_ENTRY(gwEBufSize), cpEntryTxt);
    g_free(cpEntryTxt);
    
    gwLLocalSocket = gtk_label_new(cpLLocalSocketTxt);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwLLocalSocket, FALSE, FALSE, 0);
    gtk_widget_show(gwLLocalSocket);
    
    gwELocalSocket = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(gwVBox), gwELocalSocket, FALSE, FALSE, 0);
    gtk_widget_show(gwELocalSocket);
    cpEntryTxt = g_strdup_printf("%s", cpLocalSocket);
    gtk_entry_set_text(GTK_ENTRY(gwELocalSocket), cpEntryTxt);
    g_free(cpEntryTxt);
    
    gwHBox = gtk_hbox_new(TRUE, 4);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwHBox, FALSE, FALSE, 0);
    gtk_widget_show(gwHBox);
    
    gwBOk = gtk_button_new_with_label(cpBOkTxt);
    gtk_box_pack_start(GTK_BOX(gwHBox), gwBOk, TRUE, TRUE, 0);
    gtk_widget_show(gwBOk);
    
    gwBCancel = gtk_button_new_with_label(cpBCancelTxt);
    gtk_box_pack_start(GTK_BOX(gwHBox), gwBCancel, TRUE, TRUE, 0);
    gtk_widget_show(gwBCancel);
    
    gtk_signal_connect(GTK_OBJECT(gwBOk), "clicked",
        GTK_SIGNAL_FUNC(WrapOnButtonClick), NULL);
    gtk_signal_connect(GTK_OBJECT(gwBCancel), "clicked",
        GTK_SIGNAL_FUNC(WrapOnButtonClick), NULL);

    gtk_widget_show(gwWinConfig);
}


int clXMMSOut::OpenAudio (AFormat eAudioFormatP, int iSampleRate, 
    int iChannels)
{
    eAudioFormat = eAudioFormatP;

    if (iMainThreadH >= 0)
        return 0;

    sHdr.iChannels = iChannels;
    sHdr.dSampleRate = iSampleRate;
    iWriteTime = 0;
    iPlayTime = 0;
    uiTotalTickCount = 0;
    dStartTime = 0.0;
    bRun = true;
    iMainThreadH = XMMSOutThreads.Create(&clXMMSOut::MainThread, NULL);

    return 1;
}


void clXMMSOut::WriteAudio (void *vpData, int iSize) // iLength?
{
    int iSampleCount = 0;
    int iTickCount;
    double dFragTime;
    double dWaitTime;
    clAlloc LocalAudioBuf;

    if (dStartTime == 0.0)
        dStartTime = GetTime();

    switch (eAudioFormat)
    {
        case FMT_S8:
            Convert8s8u(vpData, iSize / sizeof(char));
        case FMT_U8:
            iSampleCount = iSize / sizeof(unsigned char);
            LocalAudioBuf.Size(iSampleCount * sizeof(GDT));
            DSP.Convert((GDT *) LocalAudioBuf, (unsigned char *) vpData, 
                iSampleCount);
            break;
        case FMT_U16_LE:
#           if (G_BYTE_ORDER == G_BIG_ENDIAN)
            EndianConvert((unsigned short *) vpData, 
                iSize / sizeof(unsigned short));
#           endif
        case FMT_U16_BE:
#           if (G_BYTE_ORDER == G_LITTLE_ENDIAN)
            EndianConvert((unsigned short *) vpData, 
                iSize / sizeof(unsigned short));
#           endif
        case FMT_U16_NE:
            Convert16u16s(vpData, iSize / sizeof(short));
            iSampleCount = iSize / sizeof(signed short);
            LocalAudioBuf.Size(iSampleCount * sizeof(GDT));
            DSP.Convert((GDT *) LocalAudioBuf, (signed short *) vpData,
                iSampleCount, false);
            break;
        case FMT_S16_LE:
#           if (G_BYTE_ORDER == G_BIG_ENDIAN)
            EndianConvert((unsigned short *) vpData, 
                iSize / sizeof(unsigned short));
#           endif
        case FMT_S16_BE:
#           if (G_BYTE_ORDER == G_LITTLE_ENDIAN)
            EndianConvert((unsigned short *) vpData, 
                iSize / sizeof(unsigned short));
#           endif
        case FMT_S16_NE:
            iSampleCount = iSize / sizeof(signed short);
            LocalAudioBuf.Size(iSampleCount * sizeof(GDT));
            DSP.Convert((GDT *) LocalAudioBuf, (signed short *) vpData,
                iSampleCount, false);
            break;
    }

    iTickCount = iSampleCount / sHdr.iChannels;
    iWriteTime = (int) ((double) (uiTotalTickCount + iTickCount) / 
        sHdr.dSampleRate * 1000.0 + 0.5);
    dFragTime = (double) iTickCount / sHdr.dSampleRate;
//g_print("hasas_op: frag time %f\n", dFragTime);
    dWaitTime = dStartTime + 
        (uiTotalTickCount / sHdr.dSampleRate + dFragTime) - 
        GetTime();
//g_print("hasas_op: wait time %f\n", dWaitTime);
    if (dWaitTime > 0.0)
        xmms_usleep((gint) (dWaitTime * 1e6));  // truncated because wait time is always a bit longer
    uiTotalTickCount += iTickCount;
    iPlayTime = iWriteTime;

    if (!bPause)
    {
        MtxAudio.Wait();
        iAudioBufSize = LocalAudioBuf.GetSize();
        iFragmentSize = iSampleCount;
        AudioBuf.Size(LocalAudioBuf.GetSize());
        DSP.Copy((GDT *) AudioBuf, (GDT *) LocalAudioBuf, iSampleCount);
        MtxAudio.Release();
        CndAudio.NotifyAll();
    }
}


void clXMMSOut::CloseAudio ()
{
    if (iMainThreadH < 0)
        return;

    Stop();
    XMMSOutThreads.Wait(iMainThreadH);
    iMainThreadH = -1;
}


void clXMMSOut::Flush (int iTime)
{
    dStartTime = GetTime();
    iWriteTime = iTime;
    iPlayTime = iTime;
    uiTotalTickCount = 0;
}


void clXMMSOut::Pause (short iPaused)
{
    bPause = (iPaused) ? true : false;
}


int clXMMSOut::BufferFree ()
{
    return iAudioBufSize;
}


int clXMMSOut::BufferPlaying ()
{
    //return (bRun) ? true : false;
    return 0;
}


int clXMMSOut::OutputTime ()
{
    return iPlayTime;
}


int clXMMSOut::WrittenTime ()
{
    return iWriteTime;
}


void *clXMMSOut::MainThread (void *vpParam)
{
    int iSockH;

    while (bRun)
    {
        iSockH = SServ.WaitForConnect(100);
        if (iSockH >= 0)
        {
//g_print("hasas_op: incoming connection\n");
            XMMSOutThreads.Create(&clXMMSOut::ServeClientThread,
                (void *) iSockH, true);
        }
    }

    return NULL;
}


void *clXMMSOut::ServeClientThread (void *vpParam)
{
    stRawDataReq sReq;
    sigset_t sigsetThis;
    clAlloc LocalBuf;
    clSockOp SOp((int) vpParam);

//g_print("hasas_op: in client thread\n");
    sigemptyset(&sigsetThis);
    sigaddset(&sigsetThis, SIGPIPE);
    sigaddset(&sigsetThis, SIGINT);
    sigaddset(&sigsetThis, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &sigsetThis, NULL);

    if (SOp.WriteN(&sHdr, sizeof(sHdr)) < (int) sizeof(sHdr))
        return NULL;
    if (SOp.ReadN(&sReq, sizeof(sReq)) < (int) sizeof(sReq))
        return NULL;
    if (sReq.iChannel >= sHdr.iChannels)
        return NULL;
    while (bRun)
    {
        MtxAudio.Wait();
        CndAudio.Wait(MtxAudio.GetPtr());
        if (sReq.iChannel < 0)
        {
            LocalBuf.Size(iAudioBufSize);
            DSP.Copy((GDT *) LocalBuf, (GDT *) AudioBuf, iFragmentSize);
        }
        else
        {
            LocalBuf.Size(iAudioBufSize / sHdr.iChannels);
            CopyChannel(LocalBuf, AudioBuf, sReq.iChannel);
        }
        MtxAudio.Release();

        if (SOp.WriteN(LocalBuf, LocalBuf.GetSize()) < LocalBuf.GetSize())
            break;
    }
//g_print("hasas_op: client disconnect or stop\n");
    
    return NULL;
}


void clXMMSOut::OnAboutButton (GtkButton *gbSender, gpointer gpData)
{
    gtk_widget_destroy(gwMessageBox);
    g_free(cpMessageTxt);
}


void clXMMSOut::OnButtonClick (GtkButton *gbSender, gpointer gpData)
{
    ConfigFile *cfCfgFile;

    if (GTK_BUTTON(gwBOk) == gbSender)
    {
        cfCfgFile = xmms_cfg_open_default_file();
        if (cfCfgFile == NULL)
            cfCfgFile = xmms_cfg_new();
        xmms_cfg_write_int(cfCfgFile, "hasas", "buffersize",
            atoi(gtk_entry_get_text(GTK_ENTRY(gwEBufSize))));
        xmms_cfg_write_string(cfCfgFile, "hasas", "localsocket",
            (gchar *) gtk_entry_get_text(GTK_ENTRY(gwELocalSocket)));
        xmms_cfg_write_default_file(cfCfgFile);
        xmms_cfg_free(cfCfgFile);
        
        SServ.Close();
        Init();
    }
    else if (GTK_BUTTON(gwBCancel) == gbSender)
    {
    }
    if (gwWinConfig)
    {
        gtk_widget_destroy(gwWinConfig);
        gwWinConfig = NULL;
    }
}
