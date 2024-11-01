/*

    File replay server
    Copyright (C) 2002 Jussi Laako

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
#include <errno.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <sys/time.h>

#include <gtk/gtk.h>

#include <DynThreads.hh>

#include "FileSrv.hh"


static const char *cpWindowTxt = "File server";
static const char *cpFileSelectTxt = "Select file";
// Table1
static const char *cpFileTxt = "Filename";
static const char *cpBrowseTxt = "Browse...";
// Table2
static const char *cpPlayStopTxt[] = { "Play", "Stop" };
static const char *cpPositionTxt = "Position";


clFileSrv FileSrv;
clDynThreads<clFileSrv> FileSrvThreads(FileSrv);


int main (int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    return FileSrv.Main(&argc, &argv);
}


gboolean WrapOnDelete (GtkWidget *gwSender, GdkEvent *geEvent, gpointer gpData)
{
    return FileSrv.OnDelete(gwSender, geEvent, gpData);
}


void WrapOnFileSelectOkClick (GtkButton *gbSender, gpointer gpData)
{
    FileSrv.OnFileSelectOkClick(gbSender, gpData);
}


void WrapOnFileSelectCancelClick (GtkButton *gbSender, gpointer gpData)
{
    FileSrv.OnFileSelectCancelClick(gbSender, gpData);
}


void WrapOnBrowseClick (GtkButton *gbSender, gpointer gpData)
{
    FileSrv.OnBrowseClick(gbSender, gpData);
}


void WrapOnPlayStopToggle (GtkToggleButton *gtbSender, gpointer gpData)
{
    FileSrv.OnPlayStopToggle(gtbSender, gpData);
}


void WrapOnPositionChange (GtkAdjustment *gaSender, gpointer gpData)
{
    FileSrv.OnPositionChange(gaSender, gpData);
}


#ifdef USE_FLAC

FLAC__StreamDecoderWriteStatus WrapFLACWriteCB (
    const FLAC__FileDecoder *flacDecoderInst, const FLAC__Frame *flacFrame,
    const FLAC__int32 * const flacBuffer[], void *vpUserData)
{
    clFileSrv *FileSrvInst = (clFileSrv *) vpUserData;
    
    return FileSrvInst->FLACWriteCB(flacDecoderInst, flacFrame, flacBuffer);
}


void WrapFLACMetadataCB (const FLAC__FileDecoder *flacDecoderInst,
    const FLAC__StreamMetadata *flacMetadata, void *vpUserData)
{
    clFileSrv *FileSrvInst = (clFileSrv *) vpUserData;
    
    FileSrvInst->FLACMetadataCB(flacDecoderInst, flacMetadata);
}


void WrapFLACErrorCB (const FLAC__FileDecoder *flacDecoderInst,
    FLAC__StreamDecoderErrorStatus flacError, void *vpUserData)
{
    clFileSrv *FileSrvInst = (clFileSrv *) vpUserData;

    FileSrvInst->FLACErrorCB(flacDecoderInst, flacError);
}

#endif


bool clFileSrv::Build ()
{
    gwWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gwWindow), cpWindowTxt);
    // shrink, grow, auto-shrink
    gtk_window_set_policy(GTK_WINDOW(gwWindow), TRUE, TRUE, FALSE);
    // homogenous, spacing
    gwVBox = gtk_vbox_new(FALSE, FS_WSPACING);
    gtk_container_add(GTK_CONTAINER(gwWindow), gwVBox);
    gtk_widget_show(gwVBox);

    gwFileSelect = gtk_file_selection_new(cpFileSelectTxt);

    if (!BuildTable1()) return false;
    if (!BuildTable2()) return false;

    if (!ConnectSignals()) return false;

    gtk_widget_show(gwWindow);

    return true;
}


bool clFileSrv::BuildTable1 ()
{
    gwTable1 = gtk_table_new(2, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable1, FALSE, FALSE, 0);
    gtk_widget_show(gwTable1);

    // Label & Entry: filename
    gwLFile = gtk_label_new(cpFileTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwLFile,
        0, 1,
        0, 1,
        (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL),
        (GtkAttachOptions) 0,
        FS_WSPACING / 2, FS_WSPACING / 2);
    gtk_label_set_justify(GTK_LABEL(gwLFile), GTK_JUSTIFY_LEFT);
    gtk_widget_show(gwLFile);
    gwEFile = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(gwTable1), gwEFile,
        0, 1,
        1, 2,
        (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL),
        (GtkAttachOptions) 0,
        FS_WSPACING / 2, FS_WSPACING / 2);
    gtk_widget_show(gwEFile);

    // Button: browse
    gwBBrowse = gtk_button_new_with_label(cpBrowseTxt);
    gtk_table_attach(GTK_TABLE(gwTable1), gwBBrowse,
        1, 2,
        1, 2,
        (GtkAttachOptions) 0,
        (GtkAttachOptions) 0,
        FS_WSPACING / 2, FS_WSPACING / 2);
    gtk_widget_show(gwBBrowse);

    return true;
}


bool clFileSrv::BuildTable2 ()
{
    gwTable2 = gtk_table_new(2, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(gwVBox), gwTable2, FALSE, FALSE, 0);
    gtk_widget_show(gwTable2);

    // ToggleButton: play / stop
    gwTBPlayStop = gtk_toggle_button_new_with_label(cpPlayStopTxt[0]);
    gtk_table_attach(GTK_TABLE(gwTable2), gwTBPlayStop,
        0, 1,
        1, 2,
        (GtkAttachOptions) 0,
        (GtkAttachOptions) 0,
        FS_WSPACING / 2, FS_WSPACING / 2);
    gtk_widget_show(gwTBPlayStop);
    
    // Label & Adjustment & HScale: position
    gwLPosition = gtk_label_new(cpPositionTxt);
    gtk_table_attach(GTK_TABLE(gwTable2), gwLPosition,
        1, 2,
        0, 1,
        (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL),
        (GtkAttachOptions) 0,
        FS_WSPACING / 2, FS_WSPACING / 2);
    gtk_label_set_justify(GTK_LABEL(gwLPosition), GTK_JUSTIFY_LEFT);
    gtk_widget_show(gwLPosition);
    gaPosition = gtk_adjustment_new(0, 0, 1, 
        1.0 / 60.0, 1.0 / 60.0, 1.0 / 60.0);
    gwHSPosition = gtk_hscale_new(GTK_ADJUSTMENT(gaPosition));
    gtk_table_attach(GTK_TABLE(gwTable2), gwHSPosition,
        1, 2,
        1, 2,
        (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL),
        (GtkAttachOptions) 0,
        FS_WSPACING / 2, FS_WSPACING / 2);
    gtk_scale_set_digits(GTK_SCALE(gwHSPosition), 2);
    gtk_scale_set_draw_value(GTK_SCALE(gwHSPosition), TRUE);
    gtk_widget_show(gwHSPosition);

    return true;
}


bool clFileSrv::ConnectSignals ()
{
    //int iWidgetCntr;

    gtk_signal_connect(GTK_OBJECT(gwWindow), "delete-event",
        GTK_SIGNAL_FUNC(WrapOnDelete), NULL);

    gtk_signal_connect(
        GTK_OBJECT(GTK_FILE_SELECTION(gwFileSelect)->ok_button),
        "clicked",
        GTK_SIGNAL_FUNC(WrapOnFileSelectOkClick), NULL);
    gtk_signal_connect(
        GTK_OBJECT(GTK_FILE_SELECTION(gwFileSelect)->cancel_button),
        "clicked",
        GTK_SIGNAL_FUNC(WrapOnFileSelectCancelClick), NULL);
    gtk_signal_connect(GTK_OBJECT(gwBBrowse), "clicked",
        GTK_SIGNAL_FUNC(WrapOnBrowseClick), NULL);
    gtk_signal_connect(GTK_OBJECT(gwTBPlayStop), "toggled",
        GTK_SIGNAL_FUNC(WrapOnPlayStopToggle), NULL);

    /*gtk_signal_connect(gaPosition, "value-changed",
        GTK_SIGNAL_FUNC(WrapOnPositionChange), NULL);*/

    return true;
}


double clFileSrv::GetTime ()
{
    double dTime;
#   if (defined(linux) || defined(BSDSYS))
    struct timespec sTimeSpec;
    
    clock_gettime(CLOCK_REALTIME, &sTimeSpec);
    dTime = (double) (sTimeSpec.tv_sec - lEpoch);
    dTime += (double) sTimeSpec.tv_nsec / 1000000000.0;
#   else
    struct timeval sTimeVal;
    
    gettimeofday(&sTimeVal, NULL);
    dTime = (double) (sTimeVal.tv_sec - lEpoch);
    dTime += (double) sTimeVal.tv_usec / 1000000.0;
#   endif
    return dTime;
}


void clFileSrv::ShortSleep (long lSleepTime)
{
    if (lSleepTime <= 0) return;
#   if (defined(linux) || defined(BSDSYS))
    struct timeval sTimeout;
    
    sTimeout.tv_sec = lSleepTime / 1000000;
    sTimeout.tv_usec = lSleepTime % 1000000;
    select(0, NULL, NULL, NULL, &sTimeout);
#   else
    int iEC = 0;
    struct timespec sTimeSpec;

    sTimeSpec.tv_sec = lSleepTime / 1000000;
    sTiemSpec.tv_nsec = lSleepTime % 1000000 * 1000;
    do
    {
        if (nanosleep(&sTimeSpec, &sTimeSpec) < 0)
            iEC = errno;
    } while (iEC == EINTR);
#   endif
}


#ifdef USE_FLAC

bool clFileSrv::OpenFLAC (const char *cpFilename)
{
    flacDecoder = FLAC__file_decoder_new();

    FLAC__file_decoder_set_filename(flacDecoder, cpFilename);
    FLAC__file_decoder_set_write_callback(flacDecoder, WrapFLACWriteCB);
    FLAC__file_decoder_set_metadata_callback(flacDecoder, WrapFLACMetadataCB);
    FLAC__file_decoder_set_error_callback(flacDecoder, WrapFLACErrorCB);
    FLAC__file_decoder_set_client_data(flacDecoder, (void *) this);
    
    if (FLAC__file_decoder_init(flacDecoder) != FLAC__FILE_DECODER_OK)
    {
        CloseFLAC();
        return false;
    }

    if (!FLAC__file_decoder_process_until_end_of_metadata(flacDecoder))
    {
        g_print("FLAC: warning, unable to process metadata!\n");
    }

    return true;
}


void clFileSrv::CloseFLAC ()
{
    if (flacDecoder != NULL)
    {
        if (FLAC__file_decoder_get_state(flacDecoder) !=
            FLAC__FILE_DECODER_UNINITIALIZED)
            FLAC__file_decoder_finish(flacDecoder);
        FLAC__file_decoder_delete(flacDecoder);
        flacDecoder = NULL;
    }
}

#endif


clFileSrv::clFileSrv ()
{
    bRun = true;
    sndfileFile = NULL;
    lEpoch = time(NULL);
#   ifdef USE_FLAC
    flacDecoder = NULL;
#   endif
}


clFileSrv::~clFileSrv ()
{
}


int clFileSrv::Main (int *ipArgC, char ***cppaArgV)
{
    int iReaderH;
    int iServerH;
    int iFragSize;
    int iServerPort;

    g_print("%s v%i.%i.%i\n", cpWindowTxt,
        FS_VER_MAJ, FS_VER_MIN, FS_VER_PL);
    g_print("Copyright (C) 2002 Jussi Laako\n\n");
    g_print("Gtk+ version %i.%i.%i\n", gtk_major_version, gtk_minor_version,
        gtk_micro_version);
    g_print("Locale set to %s\n", gtk_set_locale());
    gtk_init(ipArgC, cppaArgV);

    Cfg.SetFileName(FS_CFGFILE);
    if (!Cfg.GetInt("FragSize", &iFragSize))
        iFragSize = FS_FRAG_SIZE_DEFAULT;
    if (!Cfg.GetInt("Port", &iServerPort))
        iServerPort = FS_DEFAULT_PORT;

    if (!Build()) return 1;

    AudioBlock.Size(iFragSize);
    AudioBlock.Lock();
    iFragSize /= sizeof(GDT);

    iReaderH = FileSrvThreads.Create(&clFileSrv::ReaderThread, 
        (void *) iFragSize);
    iServerH = FileSrvThreads.Create(&clFileSrv::ServerThread, 
        (void *) iServerPort);

    gtk_main();

    FileSrvThreads.Wait(iServerH);
    FileSrvThreads.Wait(iReaderH);

    return 0;
}


gboolean clFileSrv::OnDelete (GtkWidget *gwSender, GdkEvent *geEvent,
    gpointer gpData)
{
    bRun = false;
    gtk_main_quit();
    return TRUE;
}


void clFileSrv::OnFileSelectOkClick (GtkButton *gbSender, gpointer gpData)
{
    const gchar *cpFilename;

    cpFilename = 
        gtk_file_selection_get_filename(GTK_FILE_SELECTION(gwFileSelect));
    if (access(cpFilename, F_OK) != 0)
    {
        gtk_file_selection_complete(GTK_FILE_SELECTION(gwFileSelect), 
            cpFilename);
        return;
    }
    gtk_entry_set_text(GTK_ENTRY(gwEFile), cpFilename);
    gtk_widget_hide(gwFileSelect);
}


void clFileSrv::OnFileSelectCancelClick (GtkButton *gbSender, gpointer gpData)
{
    gtk_widget_hide(gwFileSelect);
}


void clFileSrv::OnBrowseClick (GtkButton *gbSender, gpointer gpData)
{
    gtk_widget_show(gwFileSelect);
}


void clFileSrv::OnPlayStopToggle (GtkToggleButton *gtbSender, gpointer gpData)
{
    if (gtk_toggle_button_get_active(gtbSender))
    {
        const gchar *cpFilename = gtk_entry_get_text(GTK_ENTRY(gwEFile));

        MtxAudio.Wait();
        if (sndfileFile != NULL)
            sf_close(sndfileFile);
        if (strlen(cpFilename) > 4)
        {
            if (strcmp(&cpFilename[strlen(cpFilename) - 4], "flac") == 0)
            {
#               ifdef USE_FLAC
                CloseFLAC();
                if (!OpenFLAC(cpFilename))
                    g_print("FLAC: open failed\n");
#               else
                g_print("FLAC files are not supported by this compile!\n");
#               endif                
            }
            else
            {
                sndfileFile = sf_open(cpFilename, SFM_READ, &sFileInfo);
                if (sndfileFile != NULL)
                {
                    long lSamples = sf_seek(sndfileFile, 0, SEEK_END);
                    sf_seek(sndfileFile, 0, SEEK_SET);
                    double dLength = (double) lSamples /
                        (double) sFileInfo.samplerate;
                    dLength /= 60.0;
                    g_print("File length %.2f minutes\n", dLength);
                    GTK_ADJUSTMENT(gaPosition)->upper = dLength;
                    gtk_adjustment_changed(GTK_ADJUSTMENT(gaPosition));
                    gtk_adjustment_set_value(GTK_ADJUSTMENT(gaPosition), 0);
                }
                else g_print("sf_open_read() failed\n");
                if (!sf_command(sndfileFile, SFC_SET_NORM_DOUBLE, NULL, 
                    SF_TRUE))
                {
                    g_print("sf_command(SFC_SET_NORM_DOUBLE) failed\n");
                }
            }
        }
        MtxAudio.Release();
    }
    else
    {
        MtxAudio.Wait();
        if (sndfileFile != NULL)
            sf_close(sndfileFile);
        sndfileFile = NULL;
#       ifdef USE_FLAC
        CloseFLAC();
#       endif
        MtxAudio.Release();
    }
}


void clFileSrv::OnPositionChange (GtkAdjustment *gaSender, gpointer gpData)
{
}


void *clFileSrv::ReaderThread (void *vpParam)
{
    int iFragSize = (int) vpParam;
    int iBlockCntr;
    int iSampleCntr;
    long lSleepTime;
    double dStartTime;
    double dFrameTime;
    double *dpFileBlock;
    GDT *fpAudioBlock;
    clAlloc FileBlock;

    g_print("Reader running\n");
    dpFileBlock = (double *) FileBlock.Size(iFragSize * sizeof(double));
    FileBlock.Lock();
    fpAudioBlock = AudioBlock;

    #ifndef BSDSYS
    FileSrvThreads.SetSched(FileSrvThreads.Self(), 
        SCHED_FIFO, FS_INTHREAD_PRIORITY);
    #endif

    iBlockCntr = 0;
    dStartTime = GetTime();
    while (bRun)
    {
        MtxAudio.Wait();
#       ifdef USE_FLAC
        if (sndfileFile == NULL && flacDecoder == NULL)
#       else
        if (sndfileFile == NULL)
#       endif
        {
            MtxAudio.Release();
            ShortSleep(100);
            iBlockCntr = 0;
            dStartTime = GetTime();
            continue;
        }
        if (sndfileFile != NULL)
        {
            if (sf_read_double(sndfileFile, dpFileBlock, 
                (sf_count_t) iFragSize) < (sf_count_t) iFragSize)
            {
                g_print("At EOF, starting over\n");
                sf_seek(sndfileFile, 0, SEEK_SET);
                iBlockCntr = 0;
                dStartTime = GetTime();
                MtxAudio.Release();
                continue;
            }
        }
#       ifdef USE_FLAC
        if (flacDecoder != NULL)
        {
            while (!StreamBuf.Get(dpFileBlock, iFragSize))
            {
                if (!FLAC__file_decoder_process_single(flacDecoder))
                {
                    g_print("FLAC: process single failed, EOF?\n");
                    CloseFLAC();
                    MtxAudio.Release();
                    continue;
                }
            }
        }
#       endif
        dFrameTime = dStartTime + (double) iBlockCntr * 
            ((double) (iFragSize / sFileInfo.channels) / 
            (double) sFileInfo.samplerate);
        MtxAudio.Release();
        // we truncate here, because system sleeps this time _minimum_
        lSleepTime = (long) ((dFrameTime - GetTime()) * 1000000.0);
        /*printf("sleep time %li  time delta %f us (%f - %f)\n", 
            lSleepTime, (dFrameTime - GetTime()) * 1000000.0,
            dFrameTime, dStartTime);*/
        ShortSleep(lSleepTime);
        MtxAudio.Wait();
        gettimeofday(&sTimeStamp, NULL);
        for (iSampleCntr = 0; iSampleCntr < iFragSize; iSampleCntr++)
            fpAudioBlock[iSampleCntr] = (GDT) dpFileBlock[iSampleCntr];
        CndReady.NotifyAll();
        MtxAudio.Release();
        iBlockCntr++;

        if (iBlockCntr % 100 == 0)
        {
            gtk_adjustment_set_value(GTK_ADJUSTMENT(gaPosition), 
                (dFrameTime - dStartTime) / 60.0);
        }
    }
    // wake up all serveclients
    CndReady.NotifyAll();
    g_print("Reader stopping\n");
    
    return NULL;
}


void *clFileSrv::ServerThread (void *vpParam)
{
    int iPort;
    int iSockH;
    clSockServ SServ;

    g_print("Server running\n");
    iPort = (int) vpParam;
    SServ.Bind((unsigned short) iPort);
    while (bRun)
    {
        iSockH = SServ.WaitForConnect(FS_ACCEPT_TIMEOUT);
        if (iSockH >= 0)
        {
            FileSrvThreads.Create(&clFileSrv::ServeClientThread,
                (void *) iSockH);
        }
    }
    g_print("Server stopping\n");
    
    return NULL;
}


void *clFileSrv::ServeClientThread (void *vpParam)
{
    long lBlockSize;
    struct timeval sLTimeStamp;
    stSoundStart sHdr;
    clAlloc LAudioBlock;
    clAlloc MsgBuf;
    clSockOp SOp((int) vpParam);
    clSoundMsg Msg;

    g_print("Client connected\n");

    MtxAudio.Wait();
    LAudioBlock.Size(AudioBlock.GetSize());
    lBlockSize = AudioBlock.GetSize() / sizeof(GDT);
    sHdr.iChannels = sFileInfo.channels;
    sHdr.dSampleRate = sFileInfo.samplerate;
    sHdr.iFragmentSize = lBlockSize;
    sHdr.iCompress = MSG_SOUND_COMPRESS_NONE;
    MtxAudio.Release();
    
    MsgBuf.Size(GLOBAL_HEADER_LEN + LAudioBlock.GetSize());
    Msg.SetStart(MsgBuf, &sHdr);
    if (SOp.WriteN(MsgBuf, GLOBAL_HEADER_LEN) <= 0)
    {
        g_print("Error sending header!\n");
        return NULL;
    }

    if (!SOp.DisableNagle())
    {
        g_warning("Failed to disable TCP Nagle algorithm.");
    }
    if (!SOp.SetTypeOfService(IPTOS_LOWDELAY))
    {
        g_warning("Unable to set type-of-service flag.");
    }

    #ifndef BSDSYS
    FileSrvThreads.SetSched(FileSrvThreads.Self(), 
        SCHED_FIFO, FS_OUTTHREAD_PRIORITY);
    #endif

    while (bRun)
    {
        MtxAudio.Wait();
        CndReady.Wait(MtxAudio.GetPtr());

        memcpy(&sLTimeStamp, &sTimeStamp, sizeof(sTimeStamp));
        memcpy(LAudioBlock, AudioBlock, AudioBlock.GetSize());

        MtxAudio.Release();

        //Msg.SetData(MsgBuf, &sLTimeStamp, (GDT *) LAudioBlock, lBlockSize);
        Msg.SetData(MsgBuf, (GDT *) LAudioBlock, lBlockSize);
        //if (SOp.WriteN(MsgBuf, MsgBuf.GetSize()) <= 0) break;
        if (SOp.WriteN(MsgBuf, lBlockSize * sizeof(GDT)) <= 0) break;
    }
    g_print("Client disconnected\n");
    
    return NULL;
}


#ifdef USE_FLAC

FLAC__StreamDecoderWriteStatus clFileSrv::FLACWriteCB (
    const FLAC__FileDecoder *flacDecoderInst, const FLAC__Frame *flacFrame,
    const FLAC__int32 * const flacBuffer[])
{
    int iBits;
    int iSamples;
    int iSampleCntr;
    int iChannels;
    int iChannelCntr;
    double dScale;
    double *dpConvBuf;

    iBits = flacFrame->header.bits_per_sample;
    iSamples = flacFrame->header.blocksize;
    iChannels = flacFrame->header.channels;
    sFileInfo.samplerate = flacFrame->header.sample_rate;
    sFileInfo.channels = flacFrame->header.channels;

    /*printf("FLAC: process %i samples for %i channels\n",
        iSamples, iChannels);*/

    dScale = 1.0 / pow(2.0, iBits - 1);
    dpConvBuf = (double *) ConvBuf.Size(iSamples * iChannels * sizeof(double));
    for (iChannelCntr = 0; iChannelCntr < iChannels; iChannelCntr++)
    {
        for (iSampleCntr = 0; iSampleCntr < iSamples; iSampleCntr++)
        {
            dpConvBuf[iSampleCntr * iChannels + iChannelCntr] = (double)
                flacBuffer[iChannelCntr][iSampleCntr] * dScale;
        }
    }
    StreamBuf.Put(dpConvBuf, iSamples * iChannels);

    // FLAC__STREAM_DECODER_WRITE_STATUS_ABORT
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


void clFileSrv::FLACMetadataCB (const FLAC__FileDecoder *flacDecoderInst,
    const FLAC__StreamMetadata *flacMetadata)
{
    g_print("FLAC: metadata\n");
}


void clFileSrv::FLACErrorCB (const FLAC__FileDecoder *flacDecoderInst,
    FLAC__StreamDecoderErrorStatus flacError)
{
    switch (flacError)
    {
        case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
            g_print("FLAC: synchronization lost\n");
            break;
        case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
            g_print("FLAC: bad header\n");
            break;
        case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
            g_print("FLAC: frame CRC error\n");
            break;
        default:
            g_print("FLAC: unknown error\n");
    }
}

#endif
