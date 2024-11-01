/*

    Framebuffer class
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


#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>
#include <tiffio.h>
#include <freetype2/freetype/freetype.h>
#ifndef __FREETYPE_H__
#error no freetype
#endif
#include <Alloc.hh>

#include "Config.h"
#include "Palette.hh"


#ifndef FRAMEBUF_HH
    #define FRAMEBUF_HH

    #define FB_TIFF_MODE        "w"         // w = write, a = append
    #define FB_TIFF_RESOLUTION  100         // dpi
    #define FB_TIFF_FONTSIZE    12          // points
    #define FB_TIFF_DATELEN     19          // characters


    /**
        Frame buffer types
    */
    enum
    {
        FB_TYPE_NONE = 0,
        FB_TYPE_LINE = 1,
        FB_TYPE_COLUMN = 2
    };

    /**
        TIFF CODECs
    */
    enum
    {
        FB_TIFF_COMPRESS_NONE = COMPRESSION_NONE,
        FB_TIFF_COMPRESS_RLE = COMPRESSION_CCITTRLE,
        FB_TIFF_COMPRESS_LZW = COMPRESSION_LZW,
        FB_TIFF_COMPRESS_JPEG = COMPRESSION_JPEG,
        FB_TIFF_COMPRESS_DEFLATE = COMPRESSION_ADOBE_DEFLATE
    };

    /**
        TIFF save mode
    */
    enum
    {
        FB_TIFF_CONT_NO,
        FB_TIFF_CONT_VERTICAL,
        FB_TIFF_CONT_HORIZONTAL
    };


    /**
        Framebuffer class

        Framebuffer class for vertically or horizontally scrolling intensity
        graph. Capable of saving contents to tiff and also continuous saving.
        Uses doublebuffering for speed.

        Dataformat is 32-bit BGRA (0xAABBGGRR).
    */
    class clFrameBuf : public clPalette
    {
            bool bSaving;
            int iType;
            int iWidth;
            int iHeight;
            int iCurPos;
            int iPixFontSize;
            int iMaxFontWidth;
            char cpDateTime[FB_TIFF_DATELEN + 1];
            unsigned int *upFrameBuf;
            uint32 *upScanBuf;
            uint32 uiStripSize;
            tstrip_t uiContStrip;
            TIFF *tiffImg;
            FT_Library ftLib;
            FT_Face ftFace;
            clAlloc FrameBuf;
            clAlloc ScanBuf;
            void RGBA2RGB (unsigned char *, const unsigned int *, int, int, 
                int, int, int, int);
            bool SetTiffTags (const char *, int, int, const char *, int,
                int, int);
            bool WriteTiffData ();
            bool WriteTiffData (int, int, double, double, double, double);
            bool WriteTiffScanData ();
            void DrawHLine (unsigned char *, int, int, int, int, int);
            void DrawVLine (unsigned char *, int, int, int, int, int);
            bool DrawText (unsigned char *, int, int, int, int, const char *);
        public:
            clFrameBuf ();
            ~clFrameBuf ();
            /**
                Set size of framebuffer.

                Resize is non-destructive.

                \param iReqWidth Width
                \param iReqHeight Height
            */
            void SetSize (int, int);
            /**
                Clear framebuffer.
            */
            void Clear ();
            /**
                Get pointer to raw framebuffer.

                \return Pointer to raw framebuffer
            */
            unsigned int *GetFBPtr () { return upFrameBuf; }
            /**
                Get pointer to palette buffer.

                \return Pointer to palette LUT
            */
            unsigned int *GetPalPtr () { return upPalette; }
            /**
                Get pointer to current position in doublebuffered framebuffer.

                \param iXPos X position offset
                \param iYPos Y position offset
                \return Pointer to framebuffer position
            */
            guchar *GetCurPtr (int, int);
            /**
                Get row stride.

                Get size of scanline in bytes. For use with gdk_rgb_*

                \return Size of scanline
            */
            gint GetRowStride () { return (iWidth * sizeof(unsigned int)); }
            /**
                Get number of colors in palette LUT.

                \return Number of colors
            */
            int GetNumColors () { return iPalSize; }
            /**
                Get width of framebuffer.

                \return Width of framebuffer
            */
            int GetWidth () { return iWidth; }
            /**
                Get height of framebuffer.

                \return Height of framebuffer
            */
            int GetHeight () { return iHeight; }
            /**
                Draw line to vertically scrolling graph.

                Scroll graph down and draw new scanline to top.
                Indexes to palette LUT as parameter.

                \param upLineBuf Scanline LUT indexes
            */
            void DrawLine (unsigned int *);
            /**
                \overload
                \param fpDataBuf Normalized data values [0..1]
            */
            void DrawLine (GDT *);
            /**
                Draw column to horizontally scrolling graph.

                Scroll graph left and draw new column to right.
                Indexes to palette LUT as parameter.

                \param upColumnBuf Column LUT indexes
            */
            void DrawColumn (unsigned int *);
            /**
                \overload
                \param fpDataBuf Normalized data values [0..1]
            */
            void DrawColumn (GDT *);
            /**
                Get single pixel from framebuffer.

                \return Pixel value
            */
            unsigned int operator [] (int iPix)
                { return upFrameBuf[iPix]; }
            /**
                Generate white-to-black palette.
            */
            void PalGenBW () { GenBW(); }
            /**
                Generate black-blue-cyan-green-yellow-red palette.
            */
            void PalGenHSV () { GenHSV(); }
            /**
                Generate palette matching to light's spectrum.
            */
            void PalGenLight () { GenLight(); }
            /**
                Generate black-red-yellow-white palette.
            */
            void PalGenTemp () { GenTemp(); }
            /**
                Generate black-white palette with two highest values red.
            */
            void PalGenDir () { GenDir(); }
            /**
                Generate black-green-red palette. "NATO-style"
            */
            void PalGenGreen () { GenGreen(); }
            /**
                Generate black-green-white palette.
            */
            void PalGenGreen2 () { GenGreen2(); }
            /**
                Generate black-green-yellow palette.
            */
            void PalGenGreen3 () { GenGreen3(); }
            /**
                Generate black-green-red-yellow palette.
            */
            void PalGenGreen4 () { GenGreen4(); }
            /**
                Generate black-green palette.
            */
            void PalGenPureGreen () { GenPureGreen(); }
            /**
                Generate black-white palette.
            */
            void PalGenWB () { GenWB(); }
            /**
                Save current framebuffer contents to TIFF file.

                \param cpFileName Name of TIFF file
                \param iCompression TIFF CODEC
                \param iJPEGQuality Quality for JPEG CODEC
                \param cpDescription Description
                \param dXMin X-axis minimum
                \param dXMax X-axis maximum
                \param dYMin Y-axis minimum
                \param dYMax Y-axis maximum
                \return Success
            */
            bool SaveToFile (const char *, int, int, const char *);
            bool SaveToFile (const char *, int, int, const char *,
                double, double, double, double);
            /// \overload
            /**
                Start continuous save to TIFF file.

                All new data is saved to TIFF until saving is stopped.

                \param cpFileName Name of TIFF file
                \param iCompression TIFF CODEC
                \param iJPEGQuality Quality for JPEG CODEC
                \param cpDescription Description
                \param iDirection Drawing direction
                \return Success
            */
            bool StartSaveToFile (const char *, int, int, const char *, int);
            /**
                Stop continuous save to TIFF file.
            */
            void StopSaveToFile ();
    };

#endif

