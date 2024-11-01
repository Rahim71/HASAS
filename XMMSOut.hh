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


#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <xmms/configfile.h>
#include <xmms/plugin.h>
#include <xmms/util.h>

#include <Alloc.hh>
#include <Condition.hh>
#include <Mutex.hh>
#include <dsp/DSPOp.hh>

#include "Config.h"
#include "LocalMsg.h"
#include "SockOp.hh"
#include "SockServ.hh"
#include "DynThreads.hh"


#ifndef XMMSOUT_HH
#define XMMSOUT_HH

#   define XMMSOUT_DESCRIPTION      "HASAS streamdist"


    class clXMMSOut
    {
            volatile bool bRun;
            volatile bool bPause;
            int iMainThreadH;
            int iWriteTime;
            int iPlayTime;
            volatile int iAudioBufSize;
            volatile int iFragmentSize;
            unsigned long long uiTotalTickCount;
            double dStartTime;
            char *cpLocalSocket;
            AFormat eAudioFormat;
            stRawDataFirst sHdr;
            clAlloc AudioBuf;
            clMutex MtxAudio;
            clCondition CndAudio;
            clDSPOp DSP;
            clSockServ SServ;
            /* UI components */
            gchar *cpMessageTxt;
            GtkWidget *gwMessageBox;
            GtkWidget *gwWinConfig;
            GtkWidget *gwVBox;
            GtkWidget *gwHBox;
            GtkWidget *gwLBufSize;
            GtkWidget *gwEBufSize;
            GtkWidget *gwLLocalSocket;
            GtkWidget *gwELocalSocket;
            GtkWidget *gwBOk;
            GtkWidget *gwBCancel;
            /* - */
            double GetTime ();
            void Convert8s8u (void *, int);
            void Convert16u16s (void *, int);
            void EndianConvert (unsigned short *, int);
            void CopyChannel (GDT *, const GDT *, int);
        public:
            clXMMSOut ();
            ~clXMMSOut ();
            void Init ();
            void About ();
            void Configure ();
            int OpenAudio (AFormat, int, int);
            void WriteAudio (void *, int);
            void CloseAudio ();
            void Flush (int);
            void Pause (short);
            int BufferFree ();
            int BufferPlaying ();
            int OutputTime ();
            int WrittenTime ();
            void *MainThread (void *);
            void *ServeClientThread (void *);
            void Stop () { bRun = false; }
            void OnAboutButton (GtkButton *, gpointer);
            void OnButtonClick (GtkButton *, gpointer);
    };

#endif
