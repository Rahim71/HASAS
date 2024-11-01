/*

    File replay server, header
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


#include <time.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <sndfile.h>
#ifdef USE_FLAC
    #include <FLAC/all.h>
#endif

#include <Alloc.hh>
#include <Mutex.hh>
#include <Condition.hh>
#include <dsp/ReBuffer3.hh>

#include "Config.h"
#include "CfgFile.hh"
#include "GtkUtils.hh"
#include "Messages.hh"
#include "SockServ.hh"
#include "SockOp.hh"


#ifndef FILESRV_HH
    #define FILESRV_HH

    #define FS_VER_MAJ              GLOBAL_VERSMAJ
    #define FS_VER_MIN              GLOBAL_VERSMIN
    #define FS_VER_PL               GLOBAL_VERSPL
    #define FS_WSPACING             8
    #define FS_ACCEPT_TIMEOUT       250


    /**
        Input server for playback of previously recorded data.
    */
    class clFileSrv
    {
            /* Thread-shared */
            volatile bool bRun;
            struct timeval sTimeStamp;
            SNDFILE *sndfileFile;
            SF_INFO sFileInfo;
#           ifdef USE_FLAC
            FLAC__FileDecoder *flacDecoder;
#           endif
            clAlloc AudioBlock;
            clCondition CndReady;
            clMutex MtxAudio;
            clReBuffer3<double> StreamBuf;
            /* - */
            long lEpoch;
            clAlloc ConvBuf;
            clCfgFile Cfg;
            /* Widgets */
            GtkWidget *gwWindow;
            GtkWidget *gwVBox;
            GtkWidget *gwFileSelect;
            // Table1
            GtkWidget *gwTable1;
            GtkWidget *gwLFile;
            GtkWidget *gwEFile;
            GtkWidget *gwBBrowse;
            // Table2
            GtkObject *gaPosition;
            GtkWidget *gwTable2;
            GtkWidget *gwTBPlayStop;
            GtkWidget *gwLPosition;
            GtkWidget *gwHSPosition;
            /* - */
            bool Build ();
            bool BuildTable1 ();
            bool BuildTable2 ();
            bool ConnectSignals ();
            double GetTime ();
            void ShortSleep (long);
#           ifdef USE_FLAC
            bool OpenFLAC (const char *);
            void CloseFLAC ();
#           endif
        public:
            clFileSrv ();
            ~clFileSrv ();
            int Main (int *, char ***);
            gboolean OnDelete (GtkWidget *, GdkEvent *, gpointer);
            void OnFileSelectOkClick (GtkButton *, gpointer);
            void OnFileSelectCancelClick (GtkButton *, gpointer);
            void OnBrowseClick (GtkButton *, gpointer);
            void OnPlayStopToggle (GtkToggleButton *, gpointer);
            void OnPositionChange (GtkAdjustment *, gpointer);
            void *ReaderThread (void *);
            void *ServerThread (void *);
            void *ServeClientThread (void *);
#           ifdef USE_FLAC
            FLAC__StreamDecoderWriteStatus FLACWriteCB (
                const FLAC__FileDecoder *, const FLAC__Frame *, 
                const FLAC__int32 * const *);
            void FLACMetadataCB (const FLAC__FileDecoder *, 
                const FLAC__StreamMetadata *);
            void FLACErrorCB (const FLAC__FileDecoder *, 
                FLAC__StreamDecoderErrorStatus);
#           endif
    };

#endif
