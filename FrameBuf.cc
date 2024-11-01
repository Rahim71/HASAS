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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <unistd.h>
#include <tiffio.h>
#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>
#include <freetype2/freetype/freetype.h>

#include "Config.h"
#include "FrameBuf.hh"


void clFrameBuf::RGBA2RGB(unsigned char *ucpDest, const unsigned int *upSrc,
    int iCXPos, int iCYPos, int iDestWidth, int iDestHeight,
    int iSrcWidth, int iSrcHeight)
{
    int iYCntr;
    int iXCntr;
    int iSrcIdx = 0;
    int iDestIdx = 0;
    
    for (iYCntr = 0; iYCntr < iSrcHeight; iYCntr++)
    {
        iDestIdx = ((iYCntr + iCYPos) * iDestWidth + iCXPos) * 3;
        for (iXCntr = 0; iXCntr < iSrcWidth; iXCntr++)
        {
            ucpDest[iDestIdx++] = (unsigned char) 
                (upSrc[iSrcIdx] & 0xff);
            ucpDest[iDestIdx++] = (unsigned char) 
                ((upSrc[iSrcIdx] >> 8) & 0xff);
            ucpDest[iDestIdx++] = (unsigned char)
                ((upSrc[iSrcIdx] >> 16) & 0xff);
            iSrcIdx++;
        }
    }
}


bool clFrameBuf::SetTiffTags(const char *cpFileName, int iCompression, 
    int iJPEGQuality, const char *cpDescription, int iContinuous,
    int iTWidth, int iTHeight)
{
    time_t ttTime;
    gchar *cpConv;
    char cpHostName[256];

    if (!TIFFSetField(tiffImg, TIFFTAG_IMAGEWIDTH, iTWidth))
        return false;
    if (iContinuous == FB_TIFF_CONT_NO)
    {
        if (!TIFFSetField(tiffImg, TIFFTAG_IMAGELENGTH, iTHeight))
            return false;
    }
    else
    {
        if (!TIFFSetField(tiffImg, TIFFTAG_IMAGELENGTH, 0))
            return false;
    }
    if (!TIFFSetField(tiffImg, TIFFTAG_BITSPERSAMPLE, 8))
        return false;
    if (!TIFFSetField(tiffImg, TIFFTAG_COMPRESSION, iCompression))
        return false;
    if (!TIFFSetField(tiffImg, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB))
        return false;
    if (!TIFFSetField(tiffImg, TIFFTAG_DOCUMENTNAME, cpFileName))
        return false;
    if (!TIFFSetField(tiffImg, TIFFTAG_IMAGEDESCRIPTION, cpDescription))
        return false;
    if (iContinuous == FB_TIFF_CONT_VERTICAL)
    {
        if (!TIFFSetField(tiffImg, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT))
            return false;
    }
    else if (iContinuous == FB_TIFF_CONT_HORIZONTAL)
    {
        if (!TIFFSetField(tiffImg, TIFFTAG_ORIENTATION, ORIENTATION_LEFTBOT))
            return false;
    }
    else
    {
        if (!TIFFSetField(tiffImg, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT))
            return false;
    }
    if (!TIFFSetField(tiffImg, TIFFTAG_SAMPLESPERPIXEL, 3))
        return false;
    if (iContinuous == FB_TIFF_CONT_NO)
    {
        uiStripSize = TIFFDefaultStripSize(tiffImg, 0);
        if (!TIFFSetField(tiffImg, TIFFTAG_ROWSPERSTRIP, uiStripSize))
            return false;
    }
    else
    {
        uiStripSize = 1;
        if (!TIFFSetField(tiffImg, TIFFTAG_ROWSPERSTRIP, uiStripSize))
            return false;
    }
    if (!TIFFSetField(tiffImg, TIFFTAG_MINSAMPLEVALUE, 0))
        return false;
    if (!TIFFSetField(tiffImg, TIFFTAG_MAXSAMPLEVALUE, 0xff))
        return false;
    if (!TIFFSetField(tiffImg, TIFFTAG_XRESOLUTION, FB_TIFF_RESOLUTION))
        return false;
    if (!TIFFSetField(tiffImg, TIFFTAG_YRESOLUTION, FB_TIFF_RESOLUTION))
        return false;
    if (!TIFFSetField(tiffImg, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG))
        return false;
    if (!TIFFSetField(tiffImg, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH))
        return false;
    cpConv = g_strdup_printf("HASAS %i.%i.%i", 
        GLOBAL_VERSMAJ, GLOBAL_VERSMIN, GLOBAL_VERSPL);
    if (!TIFFSetField(tiffImg, TIFFTAG_SOFTWARE, cpConv))
        return false;
    if (!TIFFSetField(tiffImg, TIFFTAG_ARTIST, cpConv))
        return false;
    g_free(cpConv);
    ttTime = time(NULL);
    strftime(cpDateTime, FB_TIFF_DATELEN + 1, 
        "%Y/%m/%d %H:%M:%S", localtime(&ttTime));
    if (!TIFFSetField(tiffImg, TIFFTAG_DATETIME, cpDateTime))
        return false;
    gethostname(cpHostName, 0xff);
    if (!TIFFSetField(tiffImg, TIFFTAG_HOSTCOMPUTER, cpHostName))
        return false;
    //TIFFSetField(tiffImg, TIFFTAG_EXTRASAMPLES, EXTRASAMPLE_ASSOCALPHA);
    if (!TIFFSetField(tiffImg, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT))
        return false;
    if (iCompression == FB_TIFF_COMPRESS_JPEG)
    {
        if (!TIFFSetField(tiffImg, TIFFTAG_JPEGQUALITY, iJPEGQuality))
            return false;
        if (!TIFFSetField(tiffImg, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB))
            return false;
    }
    return true;
}


bool clFrameBuf::WriteTiffData()
{
    uint32 uiStripMod;
    uint32 *upFrame;
    uint8 *ucpFrame24;
    tstrip_t uiStrip;
    tstrip_t uiStripCount;
    clAlloc Frame24;
    
    ucpFrame24 = (uint8 *) 
        Frame24.Size(iWidth * iHeight * 3 * sizeof(unsigned char));
    memset(ucpFrame24, 0xff, iWidth * iHeight * 3 * sizeof(unsigned char));

    upFrame = (uint32 *) GetCurPtr(0, 0);
    RGBA2RGB((unsigned char *) ucpFrame24, 
        (const unsigned int *) upFrame, 
        0, 0, iWidth, iHeight, iWidth, iHeight);
    uiStripCount = iHeight / uiStripSize;
    for (uiStrip = 0; uiStrip < uiStripCount; uiStrip++)
    {
        if (TIFFWriteEncodedStrip(tiffImg, uiStrip,
            (tdata_t) &ucpFrame24[uiStrip * uiStripSize * iWidth * 3],
            uiStripSize * iWidth * 3 * sizeof(uint8)) < 0)
        {
            return false;
        }
    }
    uiStripMod = iHeight % uiStripSize;
    if (uiStripMod != 0)
    {
        if (TIFFWriteEncodedStrip(tiffImg, uiStripCount,
            (tdata_t) &ucpFrame24[uiStripCount * uiStripSize * iWidth * 3],
            uiStripMod * iWidth * 3 * sizeof(uint8)) < 0)
        {
            return false;
        }
    }
    return true;
}


bool clFrameBuf::WriteTiffData(int iLMarg, int iBMarg, 
    double dXMin, double dXMax, double dYMin, double dYMax)
{
    int iTWidth;
    int iTHeight;
    char *cpText;
    uint32 uiStripMod;
    uint32 *upFrame;
    uint8 *ucpFrame24;
    tstrip_t uiStrip;
    tstrip_t uiStripCount;
    clAlloc Frame24;
    
    iTWidth = iWidth + iLMarg;
    iTHeight = iHeight + iBMarg;
    ucpFrame24 = (uint8 *) 
        Frame24.Size(iTWidth * iTHeight * 3 * sizeof(unsigned char));
    memset(ucpFrame24, 0xff, iTWidth * iTHeight * 3 * sizeof(unsigned char));

    DrawHLine((unsigned char *) ucpFrame24,
        iLMarg - iPixFontSize, iTWidth, 0, iTWidth, iTHeight);
    DrawHLine((unsigned char *) ucpFrame24,
        iLMarg - iPixFontSize, iTWidth, iHeight, iTWidth, iTHeight);
    DrawVLine((unsigned char *) ucpFrame24,
        iLMarg - 1, 0, iHeight + iPixFontSize, iTWidth, iTHeight);
    DrawVLine((unsigned char *) ucpFrame24,
        iTWidth - 1, 0, iHeight + iPixFontSize, iTWidth, iTHeight);

    if (!DrawText((unsigned char *) ucpFrame24, 
        1, iTHeight - iPixFontSize, iTWidth, iTHeight, cpDateTime))
        return false;
    cpText = g_strdup_printf("%g", dXMin);
    if (!DrawText((unsigned char *) ucpFrame24,
        iLMarg, iHeight + iPixFontSize, iTWidth, iTHeight, cpText))
        return false;
    g_free(cpText);
    cpText = g_strdup_printf("%g", dXMax);
    if (!DrawText((unsigned char *) ucpFrame24,
        iTWidth - strlen(cpText) * iMaxFontWidth, iHeight + iPixFontSize, 
        iTWidth, iTHeight, cpText))
        return false;
    g_free(cpText);
    cpText = g_strdup_printf("%g", dYMin);
    if (!DrawText((unsigned char *) ucpFrame24,
        1, iHeight - iPixFontSize, iTWidth, iTHeight, cpText))
        return false;
    g_free(cpText);
    cpText = g_strdup_printf("%g", dYMax);
    if (!DrawText((unsigned char *) ucpFrame24,
        1, 1, iTWidth, iTHeight, cpText))
        return false;
    g_free(cpText);

    upFrame = (uint32 *) GetCurPtr(0, 0);
    RGBA2RGB((unsigned char *) ucpFrame24, 
        (const unsigned int *) upFrame, 
        iLMarg, 0, iTWidth, iTHeight, iWidth, iHeight);
    uiStripCount = iTHeight / uiStripSize;
    for (uiStrip = 0; uiStrip < uiStripCount; uiStrip++)
    {
        if (TIFFWriteEncodedStrip(tiffImg, uiStrip,
            (tdata_t) &ucpFrame24[uiStrip * uiStripSize * iTWidth * 3],
            uiStripSize * iTWidth * 3 * sizeof(uint8)) < 0)
        {
            return false;
        }
    }
    uiStripMod = iTHeight % uiStripSize;
    if (uiStripMod != 0)
    {
        if (TIFFWriteEncodedStrip(tiffImg, uiStripCount,
            (tdata_t) &ucpFrame24[uiStripCount * uiStripSize * iTWidth * 3],
            uiStripMod * iTWidth * 3 * sizeof(uint8)) < 0)
        {
            return false;
        }
    }
    return true;
}


bool clFrameBuf::WriteTiffScanData()
{
    uint8 *ucpScan24;
    clAlloc Scan24;

    ucpScan24 = (uint8 *) Scan24.Size(iWidth * 3 * sizeof(unsigned char));
    if (!TIFFSetField(tiffImg, TIFFTAG_IMAGELENGTH, uiContStrip + 1))
        return false;
    RGBA2RGB((unsigned char *) ucpScan24, 
        (const unsigned int *) upScanBuf, 
        0, 0, iWidth, 1, iWidth, 1);
    if (TIFFWriteEncodedStrip(tiffImg, uiContStrip,
        (tdata_t) ucpScan24, iWidth * 3 * sizeof(uint8)) < 0)
    {
        return false;
    }
    uiContStrip++;
    return true;
}


void clFrameBuf::DrawHLine (unsigned char *ucpFrame24, int iXPos1, int iXPos2,
    int iYPos, int iTWidth, int iTHeight)
{
    int iXCntr;
    int iPixIdx;

    iPixIdx = (iYPos * iTWidth + iXPos1) * 3;
    for (iXCntr = iXPos1; iXCntr < iXPos2; iXCntr++)
    {
        ucpFrame24[iPixIdx++] = 0x00;
        ucpFrame24[iPixIdx++] = 0x00;
        ucpFrame24[iPixIdx++] = 0x00;
    }
}


void clFrameBuf::DrawVLine (unsigned char *ucpFrame24, int iXPos, 
    int iYPos1, int iYPos2, int iTWidth, int iTHeight)
{
    int iYCntr;
    int iPixIdx;
    
    iPixIdx = (iYPos1 * iTWidth + iXPos) * 3;
    for (iYCntr = iYPos1; iYCntr < iYPos2; iYCntr++)
    {
        ucpFrame24[iPixIdx] = 0x00;
        ucpFrame24[iPixIdx + 1] = 0x00;
        ucpFrame24[iPixIdx + 2] = 0x00;
        iPixIdx += iTWidth * 3;
    }
}


bool clFrameBuf::DrawText (unsigned char *ucpFrame24, 
    int iTXPos, int iTYPos, int iTWidth, int iTHeight, 
    const char *cpTText)
{
    int iCharCntr;
    int iCurXPos;
    int iGYCntr;
    int iGXCntr;
    int iPixPos;
    unsigned char ucGPix;
    double dPointSize;

    dPointSize = 1.0 / 72.0;
    iCurXPos = iTXPos;
    for (iCharCntr = 0; iCharCntr < (int) strlen(cpTText); iCharCntr++)
    {
        if (FT_Load_Char(ftFace, cpTText[iCharCntr], 
            FT_LOAD_RENDER|FT_LOAD_NO_HINTING) != 0)
            return false;
        if ((iCurXPos + ftFace->glyph->bitmap.width) > iTWidth)
            return true;
        for (iGYCntr = 0; iGYCntr < ftFace->glyph->bitmap.rows; iGYCntr++)
        {
            for (iGXCntr = 0; iGXCntr < ftFace->glyph->bitmap.width; iGXCntr++)
            {
                iPixPos = 
                    (iTYPos + iGYCntr + 
                    (iPixFontSize - ftFace->glyph->bitmap_top)) * 
                    iTWidth + iCurXPos + iGXCntr + ftFace->glyph->bitmap_left;
                iPixPos *= 3;
                ucGPix = *(ftFace->glyph->bitmap.buffer + 
                    iGYCntr * ftFace->glyph->bitmap.pitch + iGXCntr);
                ucpFrame24[iPixPos++] -= ucGPix;
                ucpFrame24[iPixPos++] -= ucGPix;
                ucpFrame24[iPixPos] -= ucGPix;
            }
        }
        if (ftFace->glyph->bitmap.width == 0)
        {
            iCurXPos += (int) ((ftFace->glyph->advance.x >> 6) * dPointSize *
                FB_TIFF_RESOLUTION + 0.5);
        }
        else
        {
            iCurXPos += 
                ftFace->glyph->bitmap.width + ftFace->glyph->bitmap_left + 2;
        }
    }
    return true;
}


clFrameBuf::clFrameBuf()
{
    double dPhysFontSize;

    bSaving = false;
    iType = FB_TYPE_NONE;
    iWidth = 0;
    iHeight = 0;
    iCurPos = 0;
    upFrameBuf = NULL;
    upScanBuf = NULL;
    if (FT_Init_FreeType(&ftLib) != 0)
    {
        fprintf(stderr, "clFrameBuf::clFrameBuf(): FT_Init_FreeType()\n");
        return;
    }
    if (FT_New_Face(ftLib, "fbfont.ttf", 0, &ftFace) != 0)
    {
        fprintf(stderr, "clFrameBuf::clFrameBuf(): FT_New_Face()\n");
        return;
    }
    if (!(ftFace->face_flags & FT_FACE_FLAG_SCALABLE))
    {
        fprintf(stderr, "clFrameBuf::clFrameBuf(): !FT_FACE_FLAG_SCALABLE\n");
    }
    if (FT_Set_Char_Size(ftFace, 0 * 64, FB_TIFF_FONTSIZE * 64, 
        FB_TIFF_RESOLUTION, FB_TIFF_RESOLUTION) != 0)
    {
        fprintf(stderr, "clFrameBuf::clFrameBuf(): FT_Set_Char_Size()\n");
        return;
    }
    dPhysFontSize = 1.0 / 72.0 * FB_TIFF_FONTSIZE;
    iPixFontSize = (int) ceil(dPhysFontSize * FB_TIFF_RESOLUTION);
    iMaxFontWidth = (int) ftFace->size->metrics.x_ppem;
}


clFrameBuf::~clFrameBuf()
{
    if (bSaving) StopSaveToFile();
    FT_Done_Face(ftFace);
    FT_Done_FreeType(ftLib);
}


void clFrameBuf::SetSize(int iReqWidth, int iReqHeight)
{
    int iFBSize;
    int iScanSize;
    
    iFBSize = iReqWidth * iReqHeight * 2 * sizeof(unsigned int);
    switch (iType)
    {
        case FB_TYPE_LINE:
            iScanSize = iReqWidth * sizeof(uint32);
            break;
        case FB_TYPE_COLUMN:
            iScanSize = iReqHeight * sizeof(uint32);
            break;
        case FB_TYPE_NONE:
        default:
            iScanSize = 
                ((iReqWidth >= iReqHeight) ? iReqWidth : iReqHeight) *
                sizeof(uint32);
    }
    upFrameBuf = (unsigned int *) FrameBuf.Resize(iFBSize);
    upScanBuf = (uint32 *) ScanBuf.Size(iScanSize);
    iWidth = iReqWidth;
    iHeight = iReqHeight;
    if (iType == FB_TYPE_LINE && iCurPos >= iHeight) iCurPos = iHeight - 1;
    if (iType == FB_TYPE_COLUMN && iCurPos >= iWidth) iCurPos = 0;
}


void clFrameBuf::Clear()
{
    int iPixCount;
    int iPixCntr;

    iPixCount = iWidth * iHeight * 2;
    for (iPixCntr = 0; iPixCntr < iPixCount; iPixCntr++)
    {
        upFrameBuf[iPixCntr] = 0;
    }
}


guchar *clFrameBuf::GetCurPtr(int iXPos, int iYPos)
{
    int iPixIdx;

    if (iType == FB_TYPE_LINE)
    {
        iPixIdx = (iCurPos + iYPos) * iWidth + iXPos;
        return (guchar *) (&upFrameBuf[iPixIdx]);
    }
    else if (iType == FB_TYPE_COLUMN)
    {
        iPixIdx = iYPos * iWidth + iCurPos + iXPos;
        return (guchar *) (&upFrameBuf[iPixIdx]);
    }
    else return (guchar *) (&upFrameBuf[iYPos * iWidth + iXPos]);
}


void clFrameBuf::DrawLine(unsigned int *upLineBuf)
{
    int iPixCntr;
    int iPixIdx1;
    int iPixIdx2;

    if (iType != FB_TYPE_LINE)
    {
        iCurPos = iHeight;
        iType = FB_TYPE_LINE;
    }
    iCurPos--;
    if (iCurPos < 0) iCurPos = iHeight - 1;
    iPixIdx1 = iCurPos * iWidth;
    iPixIdx2 = iWidth * iHeight + iPixIdx1;
    for (iPixCntr = 0; iPixCntr < iWidth; iPixCntr++)
    {
        upFrameBuf[iPixIdx1] = upFrameBuf[iPixIdx2] = upScanBuf[iPixCntr] =
            upPalette[upLineBuf[iPixCntr]];
        iPixIdx1++;
        iPixIdx2++;
    }
    if (bSaving) WriteTiffScanData();
}


void clFrameBuf::DrawLine(GDT *fpDataBuf)
{
    int iPixCntr;
    int iPixIdx1;
    int iPixIdx2;
    int iPalIdx;

    if (iType != FB_TYPE_LINE)
    {
        iCurPos = iHeight;
        iType = FB_TYPE_LINE;
    }
    iCurPos--;
    if (iCurPos < 0) iCurPos = iHeight - 1;
    iPixIdx1 = iCurPos * iWidth;
    iPixIdx2 = iWidth * iHeight + iPixIdx1;
    for (iPixCntr = 0; iPixCntr < iWidth; iPixCntr++)
    {
        iPalIdx = (int) (fpDataBuf[iPixCntr] * 
            (GDT) (iPalSize - 1) + (GDT) 0.5);
        if (iPalIdx < 0) iPalIdx = 0;
        if (iPalIdx >= iPalSize) iPalIdx = iPalSize - 1;
        upFrameBuf[iPixIdx1] = upFrameBuf[iPixIdx2] = upScanBuf[iPixCntr] = 
            upPalette[iPalIdx];
        iPixIdx1++;
        iPixIdx2++;
    }
    if (bSaving) WriteTiffScanData();
}


void clFrameBuf::DrawColumn(unsigned int *upColumnBuf)
{
    int iPixCntr;
    int iPixIdx1;
    int iPixIdx2;

    if (iType != FB_TYPE_COLUMN)
    {
        iCurPos = 0;
        iType = FB_TYPE_COLUMN;
    }
    if (iCurPos >= iWidth) iCurPos = 0;
    iPixIdx1 = iCurPos;
    iPixIdx2 = iWidth * iHeight + iPixIdx1;
    for (iPixCntr = 0; iPixCntr < iHeight; iPixCntr++)
    {
        upFrameBuf[iPixIdx1] = upFrameBuf[iPixIdx2] = upScanBuf[iPixCntr] =
            upPalette[upColumnBuf[iPixCntr]];
        iPixIdx1 += iWidth;
        iPixIdx2 += iWidth;
    }
    iCurPos++;
    if (bSaving) WriteTiffScanData();
}


void clFrameBuf::DrawColumn(GDT *fpDataBuf)
{
    int iPixCntr;
    int iPixIdx1;
    int iPixIdx2;
    int iPalIdx;

    if (iType != FB_TYPE_COLUMN)
    {
        iCurPos = 0;
        iType = FB_TYPE_COLUMN;
    }
    if (iCurPos >= iWidth) iCurPos = 0;
    iPixIdx1 = iCurPos;
    iPixIdx2 = iWidth * iHeight + iPixIdx1;
    for (iPixCntr = 0; iPixCntr < iHeight; iPixCntr++)
    {
        iPalIdx = (int) (fpDataBuf[iHeight - 1 - iPixCntr] *
            (GDT) (iPalSize - 1) + (GDT) 0.5);
        if (iPalIdx < 0) iPalIdx = 0;
        if (iPalIdx >= iPalSize) iPalIdx = iPalSize - 1;
        upFrameBuf[iPixIdx1] = upFrameBuf[iPixIdx2] = upScanBuf[iPixCntr] = 
            upPalette[iPalIdx];
        iPixIdx1 += iWidth;
        iPixIdx2 += iWidth;
    }
    iCurPos++;
    if (bSaving) WriteTiffScanData();
}


bool clFrameBuf::SaveToFile(const char *cpFileName, int iCompression,
    int iJPEGQuality, const char *cpDescription)
{
    bool bRetVal = true;
    
    if (bSaving) return false;
    tiffImg = TIFFOpen(cpFileName, FB_TIFF_MODE);
    if (tiffImg == NULL) return false;
    if (SetTiffTags(cpFileName, iCompression, iJPEGQuality, cpDescription,
        FB_TIFF_CONT_NO, iWidth, iHeight))
    {
        if (!WriteTiffData()) 
            bRetVal = false;
    }
    else bRetVal = false;
    TIFFClose(tiffImg);
    return bRetVal;
}


bool clFrameBuf::SaveToFile(const char *cpFileName, int iCompression,
    int iJPEGQuality, const char *cpDescription,
    double dXMin, double dXMax, double dYMin, double dYMax)
{
    bool bRetVal = true;
    int iLeftMargin;
    int iBottomMargin;
    
    //iLeftMargin = iMaxFontWidth * 12 + iPixFontSize;
    iLeftMargin = iMaxFontWidth * 8 + iPixFontSize;
    iBottomMargin = iPixFontSize * 4;

    if (bSaving) return false;
    tiffImg = TIFFOpen(cpFileName, FB_TIFF_MODE);
    if (tiffImg == NULL) return false;
    if (SetTiffTags(cpFileName, iCompression, iJPEGQuality, cpDescription,
        FB_TIFF_CONT_NO, iWidth + iLeftMargin, iHeight + iBottomMargin))
    {
        if (!WriteTiffData(iLeftMargin, iBottomMargin, 
            dXMin, dXMax, dYMin, dYMax)) 
            bRetVal = false;
    }
    else bRetVal = false;
    TIFFClose(tiffImg);
    return bRetVal;
}


bool clFrameBuf::StartSaveToFile(const char *cpFileName, int iCompression,
    int iJPEGQuality, const char *cpDescription, int iDirection)
{
    tiffImg = TIFFOpen(cpFileName, FB_TIFF_MODE);
    if (tiffImg == NULL) return false;
    if (!SetTiffTags(cpFileName, iCompression, iJPEGQuality, cpDescription,
        iDirection, iWidth, iHeight))
    {
        return false;
    }
    uiContStrip = 0;
    bSaving = true;
    return true;
}


void clFrameBuf::StopSaveToFile()
{
    bSaving = false;
    if (tiffImg != NULL) TIFFClose(tiffImg);
}
