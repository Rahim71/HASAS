/*

    Picture converter, ads data in .inf to the picture
    Copyright (C) 2002-2003 Jussi Laako

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


#include <cstdio>
#include <cstring>
#include <climits>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <string>

#include "ConvPic.hh"


clConvPic ConvPic;


int main (int argc, char *argv[])
{
    return ConvPic.Main(&argc, &argv);
}


bool clConvPic::Initialize ()
{
    return true;
}


bool clConvPic::LoadLOFARInfo (const char *cpImageFileName)
{
    char cpInfoFileName[_POSIX_PATH_MAX + 1];
    char cpReadBuf[4096];
    FILE *fileInf;
    string strParam;
    string strValue;
    
    sprintf(cpInfoFileName, "%s.inf", cpImageFileName);
    fileInf = fopen(cpInfoFileName, "rt");
    if (fileInf == NULL)
    {
        fprintf(stderr, "Failed to open .inf file for reading!\n");
        return false;
    }

    while (fgets(cpReadBuf, 4096, fileInf))
    {
        SplitLine(cpReadBuf, strParam, strValue);

        if (strParam == "Time:")
            uPicInfo.sLOFAR.ttTime = ParseTime(strValue.c_str());
        if (strParam == "LowFreq:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sLOFAR.dLowFreq);
        if (strParam == "HighFreq:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sLOFAR.dHighFreq);
        if (strParam == "LineTime:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sLOFAR.dLineTime);
    }

    fclose(fileInf);

    return true;
}


bool clConvPic::LoadDEMONInfo (const char *cpImageFileName)
{
    char cpInfoFileName[_POSIX_PATH_MAX + 1];
    char cpReadBuf[4096];
    FILE *fileInf;
    string strParam;
    string strValue;
    
    sprintf(cpInfoFileName, "%s.inf", cpImageFileName);
    fileInf = fopen(cpInfoFileName, "rt");
    if (fileInf == NULL)
    {
        fprintf(stderr, "Failed to open .inf file for reading!\n");
        return false;
    }

    while (fgets(cpReadBuf, 4096, fileInf))
    {
        SplitLine(cpReadBuf, strParam, strValue);

        if (strParam == "Time:")
            uPicInfo.sDEMON.ttTime = ParseTime(strValue.c_str());
        if (strParam == "LowFreq:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sDEMON.dLowFreq);
        if (strParam == "HighFreq:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sDEMON.dHighFreq);
        if (strParam == "DemonBand:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sDEMON.dDEMONBand);
        if (strParam == "LineTime:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sDEMON.dLineTime);
    }

    fclose(fileInf);

    return true;
}


bool clConvPic::LoadSGramInfo (const char *cpImageFileName)
{
    char cpInfoFileName[_POSIX_PATH_MAX + 1];
    char cpReadBuf[4096];
    FILE *fileInf;
    string strParam;
    string strValue;
    
    sprintf(cpInfoFileName, "%s.inf", cpImageFileName);
    fileInf = fopen(cpInfoFileName, "rt");
    if (fileInf == NULL)
    {
        fprintf(stderr, "Failed to open .inf file for reading!\n");
        return false;
    }

    while (fgets(cpReadBuf, 4096, fileInf))
    {
        SplitLine(cpReadBuf, strParam, strValue);

        if (strParam == "Time:")
            uPicInfo.sSGram.ttTime = ParseTime(strValue.c_str());
        if (strParam == "LowFreq:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sSGram.dLowFreq);
        if (strParam == "HighFreq:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sSGram.dHighFreq);
        if (strParam == "LineTime:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sSGram.dLineTime);
    }

    fclose(fileInf);

    return true;
}


bool clConvPic::LoadTBearInfo (const char *cpImageFileName)
{
    char cpInfoFileName[_POSIX_PATH_MAX + 1];
    char cpReadBuf[4096];
    FILE *fileInf;
    string strParam;
    string strValue;
    
    sprintf(cpInfoFileName, "%s.inf", cpImageFileName);
    fileInf = fopen(cpInfoFileName, "rt");
    if (fileInf == NULL)
    {
        fprintf(stderr, "Failed to open .inf file for reading!\n");
        return false;
    }

    while (fgets(cpReadBuf, 4096, fileInf))
    {
        SplitLine(cpReadBuf, strParam, strValue);

        if (strParam == "Time:")
            uPicInfo.sTBear.ttTime = ParseTime(strValue.c_str());
        if (strParam == "LeftDir:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sTBear.dLeftDir);
        if (strParam == "RightDir:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sTBear.dRightDir);
        if (strParam == "IntTime:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sTBear.dIntTime);
        if (strParam == "HighFreq:")
            sscanf(strValue.c_str(), "%lg", &uPicInfo.sTBear.dHighFreq);
        if (strParam == "Sectors:")
            sscanf(strValue.c_str(), "%d", &uPicInfo.sTBear.iSectors);
    }

    fclose(fileInf);

    return true;
}


bool clConvPic::LoadAndCreate (const char *cpSrcFileName)
{
    try
    {
        ImgSrc = new Image(cpSrcFileName);

        iSrcWidth = ImgSrc->columns();
        iSrcHeight = ImgSrc->rows();

        ImgDst = new Image(Geometry(
            iSrcWidth + CP_MARGIN_L + CP_MARGIN_R, 
            iSrcHeight + CP_MARGIN_T + CP_MARGIN_B), 
            ColorRGB(1, 1, 1));
        ImgDst->resolutionUnits(PixelsPerInchResolution);
        if (cpPageSize)
        {
            string PageSpec;

            PageSpec = string(cpPageSize) + string(CP_PAGE_SPEC);
            fprintf(stdout, "page: %s\n", PageSpec.c_str());
            ImgDst->magick("PS2");
            ImgDst->page(Geometry(PageSpec));
            //ImgDst->density(Geometry(300, 300));
        }
        else
        {
            ImgDst->density(Geometry(100, 100));
        }

        if (!bTwoColor)
        {
            ImgDst->compressType(NoCompression);
            ImgDst->quality(100);
            ImgDst->quantizeDither(false);
        }
        else
        {
            ImgDst->compressType(RunlengthEncodedCompression);
            ImgDst->quality(100);
            ImgDst->quantizeDither(true);
        }

        ImgDst->depth(8);
        ImgDst->matte(false);
        if (!bTwoColor)
        {
            ImgDst->antiAlias(true);
            ImgDst->strokeAntiAlias(true);
        }
        else
        {
            ImgDst->antiAlias(false);
            ImgDst->strokeAntiAlias(false);
            new DrawableTextAntialias(false);
        }

        ImgDst->font(CP_FONT_NAME);

        iDstWidth = ImgDst->columns();
        iDstHeight = ImgDst->rows();
    }
    catch (Exception &X)
    {
        fprintf(stderr, "exception: %s\n", X.what());
        return false;
    }

    return true;
}


bool clConvPic::CopySource ()
{
    try
    {
        ImgDst->strokeColor(ColorRGB(0, 0, 0));
        ImgDst->strokeWidth(1);
        ImgDst->draw(DrawableRectangle(CP_MARGIN_L - 1, CP_MARGIN_T - 1,
            CP_MARGIN_L + iSrcWidth, CP_MARGIN_T + iSrcHeight));

        ImgDst->draw(DrawableCompositeImage(CP_MARGIN_L, CP_MARGIN_T, *ImgSrc));
    }
    catch (Exception &X)
    {
        fprintf(stderr, "exception: %s\n", X.what());
        return false;
    }

    return true;
}


bool clConvPic::DrawLOFARInfo ()
{
    time_t ttCreateTime;
    time_t ttTickTime;
    int iMarkCntr;
    int iMarkCount;
    int iMarkPos;
    int iTextWidth;
    int iTextHeight;
    int iAdvanceY;
    double dStartTime;
    double dEndTime;
    double dDeltaTime;
    double dDeltaFreq;
    double dFreqScale;
    double dRoundedRange;
    double dRoundStep;
    double dCountScale;
    double dTickStep;
    double dTickFreq;
    char cpTimeBuf[24];
    char cpTextBuf[4096];
    TypeMetric TextMetrics;

    try
    {
        ImgDst->fontPointsize(CP_FONT_NORMAL);
        ImgDst->strokeColor(ColorRGB(0, 0, 0));

        dStartTime = (double) uPicInfo.sLOFAR.ttTime;
        dEndTime = dStartTime + uPicInfo.sLOFAR.dLineTime * (double) iSrcHeight;
        dDeltaTime = dEndTime - dStartTime;

        ImgDst->fontPointsize(CP_FONT_LARGE);
        ttCreateTime = (time_t) (dEndTime + 0.5);
        strftime(cpTimeBuf, 23, "%Y/%m/%d %H:%M:%S", localtime(&ttCreateTime));
        sprintf(cpTextBuf, "LOFAR %s", cpTimeBuf);
        GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight, &iAdvanceY);
        ImgDst->draw(DrawableText(1, iAdvanceY + 1, cpTextBuf));
        sprintf(cpTextBuf, "Band: %g - %g Hz", 
            uPicInfo.sLOFAR.dLowFreq, uPicInfo.sLOFAR.dHighFreq);
        GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight);
        ImgDst->draw(DrawableText(1, iAdvanceY * 2 + 1, cpTextBuf));

        ImgDst->fontPointsize(CP_FONT_SMALL);
        iMarkCount = (int) (dDeltaTime / 60.0);
        for (iMarkCntr = 0; iMarkCntr <= iMarkCount; iMarkCntr++)
        {
            ttTickTime = (time_t) 
                (dStartTime + (double) iMarkCntr * 60.0 + 0.5);
            strftime(cpTimeBuf, 9, "%H:%M:%S", localtime(&ttTickTime));
            GetTextSize(cpTimeBuf, &iTextWidth, &iTextHeight);
            ImgDst->fontTypeMetrics(cpTimeBuf, &TextMetrics);
            iMarkPos = CP_MARGIN_T + iSrcHeight - 
                (int) ((double) iMarkCntr * 60.0 / 
                uPicInfo.sLOFAR.dLineTime + 0.5);
            if (iMarkPos < (CP_MARGIN_T + iTextHeight))
                continue;
            ImgDst->draw(DrawableText(CP_MARGIN_L - iTextWidth - 1, 
                iMarkPos + TextMetrics.descent() - 1, cpTimeBuf));
            ImgDst->draw(DrawableLine(CP_MARGIN_L - CP_TICKL_LEN, iMarkPos,
                CP_MARGIN_L, iMarkPos));
        }
        ttTickTime = (time_t) (dEndTime + 0.5);
        strftime(cpTimeBuf, 9, "%H:%M:%S", localtime(&ttTickTime));
        GetTextSize(cpTimeBuf, &iTextWidth, &iTextHeight);
        ImgDst->fontTypeMetrics(cpTimeBuf, &TextMetrics);
        iMarkPos = CP_MARGIN_T;
        ImgDst->draw(DrawableText(CP_MARGIN_L - iTextWidth - 1, 
            iMarkPos + TextMetrics.descent() - 1, cpTimeBuf));
        ImgDst->draw(DrawableLine(CP_MARGIN_L - CP_TICKL_LEN, iMarkPos,
            CP_MARGIN_L, iMarkPos));

        dDeltaFreq = uPicInfo.sLOFAR.dHighFreq - uPicInfo.sLOFAR.dLowFreq;
        dFreqScale = (double) iSrcWidth / dDeltaFreq;
        dRoundStep = pow(10.0, floor(log10(dDeltaFreq)));
        dRoundedRange = dDeltaFreq - fmod(dDeltaFreq, dRoundStep);
        if ((dRoundedRange / dRoundStep) <= 5.0)
            dCountScale = 0.01;
        else
            dCountScale = 0.1;
        iMarkCount = (int) (dDeltaFreq / (dRoundStep * dCountScale) + 0.5);
        dTickStep = dRoundStep * dCountScale;
        for (iMarkCntr = 0; iMarkCntr < iMarkCount; iMarkCntr++)
        {
            dTickFreq = uPicInfo.sLOFAR.dLowFreq + 
                (double) iMarkCntr * dTickStep;
            iMarkPos = CP_MARGIN_L + 
                (int) ((double) iMarkCntr * dTickStep * dFreqScale + 0.5);
            if (iMarkCntr % 10)
            {
                ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
                    iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKS_LEN));
            }
            else
            {
                ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
                    iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN));
                sprintf(cpTextBuf, "%g", dTickFreq);
                GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight);
                ImgDst->fontTypeMetrics(cpTextBuf, &TextMetrics);
                ImgDst->draw(DrawableText(iMarkPos - iTextWidth / 2,
                    CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN + TextMetrics.ascent() + 1,
                    cpTextBuf));
            }
        }
        iMarkPos = CP_MARGIN_L + (int) (dDeltaFreq * dFreqScale + 0.5);
        ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
            iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN));
        sprintf(cpTextBuf, "%g", uPicInfo.sLOFAR.dHighFreq);
        GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight);
        ImgDst->fontTypeMetrics(cpTextBuf, &TextMetrics);
        ImgDst->draw(DrawableText(iMarkPos - iTextWidth / 2,
            CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN + TextMetrics.ascent() + 1,
            cpTextBuf));
    }
    catch (Exception &X)
    {
        fprintf(stderr, "exception: %s\n", X.what());
        return false;
    }

    return true;
}


bool clConvPic::DrawDEMONInfo ()
{
    time_t ttCreateTime;
    time_t ttTickTime;
    int iMarkCntr;
    int iMarkCount;
    int iMarkPos;
    int iTextWidth;
    int iTextHeight;
    int iAdvanceY;
    double dStartTime;
    double dEndTime;
    double dDeltaTime;
    double dFreqScale;
    double dRoundedRange;
    double dRoundStep;
    double dCountScale;
    double dTickStep;
    double dTickFreq;
    char cpTimeBuf[24];
    char cpTextBuf[4096];
    TypeMetric TextMetrics;

    try
    {
        ImgDst->fontPointsize(CP_FONT_NORMAL);
        ImgDst->strokeColor(ColorRGB(0, 0, 0));

        dStartTime = (double) uPicInfo.sDEMON.ttTime;
        dEndTime = dStartTime + uPicInfo.sDEMON.dLineTime * (double) iSrcHeight;
        dDeltaTime = dEndTime - dStartTime;

        ImgDst->fontPointsize(CP_FONT_LARGE);
        ttCreateTime = (time_t) (dEndTime + 0.5);
        strftime(cpTimeBuf, 23, "%Y/%m/%d %H:%M:%S", localtime(&ttCreateTime));
        sprintf(cpTextBuf, "DEMON %s", cpTimeBuf);
        GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight, &iAdvanceY);
        ImgDst->draw(DrawableText(1, iAdvanceY + 1, cpTextBuf));
        sprintf(cpTextBuf, "Band: %g - %g Hz, DEMON bandwidth %g Hz", 
            uPicInfo.sDEMON.dLowFreq, uPicInfo.sDEMON.dHighFreq,
            uPicInfo.sDEMON.dDEMONBand);
        ImgDst->draw(DrawableText(1, iAdvanceY * 2 + 1, cpTextBuf));

        ImgDst->fontPointsize(CP_FONT_SMALL);
        iMarkCount = (int) (dDeltaTime / 60.0);
        for (iMarkCntr = 0; iMarkCntr <= iMarkCount; iMarkCntr++)
        {
            ttTickTime = (time_t) 
                (dStartTime + (double) iMarkCntr * 60.0 + 0.5);
            strftime(cpTimeBuf, 9, "%H:%M:%S", localtime(&ttTickTime));
            GetTextSize(cpTimeBuf, &iTextWidth, &iTextHeight);
            ImgDst->fontTypeMetrics(cpTimeBuf, &TextMetrics);
            iMarkPos = CP_MARGIN_T + iSrcHeight - 
                (int) ((double) iMarkCntr * 60.0 / 
                uPicInfo.sDEMON.dLineTime + 0.5);
            if (iMarkPos < (CP_MARGIN_T + iTextHeight))
                continue;
            ImgDst->draw(DrawableText(CP_MARGIN_L - iTextWidth - 1, 
                iMarkPos + TextMetrics.descent() - 1, cpTimeBuf));
            ImgDst->draw(DrawableLine(CP_MARGIN_L - CP_TICKL_LEN, iMarkPos,
                CP_MARGIN_L, iMarkPos));
        }
        ttTickTime = (time_t) (dEndTime + 0.5);
        strftime(cpTimeBuf, 9, "%H:%M:%S", localtime(&ttTickTime));
        GetTextSize(cpTimeBuf, &iTextWidth, &iTextHeight);
        ImgDst->fontTypeMetrics(cpTimeBuf, &TextMetrics);
        iMarkPos = CP_MARGIN_T;
        ImgDst->draw(DrawableText(CP_MARGIN_L - iTextWidth - 1, 
            iMarkPos + TextMetrics.descent() - 1, cpTimeBuf));
        ImgDst->draw(DrawableLine(CP_MARGIN_L - CP_TICKL_LEN, iMarkPos,
            CP_MARGIN_L, iMarkPos));

        dFreqScale = (double) iSrcWidth / uPicInfo.sDEMON.dDEMONBand;
        dRoundStep = pow(10.0, floor(log10(uPicInfo.sDEMON.dDEMONBand)));
        dRoundedRange = uPicInfo.sDEMON.dDEMONBand - 
            fmod(uPicInfo.sDEMON.dDEMONBand, dRoundStep);
        if ((dRoundedRange / dRoundStep) <= 5.0)
            dCountScale = 0.01;
        else
            dCountScale = 0.1;
        iMarkCount = (int) (uPicInfo.sDEMON.dDEMONBand / 
            (dRoundStep * dCountScale) + 0.5);
        dTickStep = dRoundStep * dCountScale;
        for (iMarkCntr = 0; iMarkCntr < iMarkCount; iMarkCntr++)
        {
            dTickFreq = (double) iMarkCntr * dTickStep;
            iMarkPos = CP_MARGIN_L + 
                (int) ((double) iMarkCntr * dTickStep * dFreqScale + 0.5);
            if (iMarkCntr % 10)
            {
                ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
                    iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKS_LEN));
            }
            else
            {
                ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
                    iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN));
                sprintf(cpTextBuf, "%g", dTickFreq);
                GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight);
                ImgDst->fontTypeMetrics(cpTextBuf, &TextMetrics);
                ImgDst->draw(DrawableText(iMarkPos - iTextWidth / 2,
                    CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN + TextMetrics.ascent() + 1,
                    cpTextBuf));
            }
        }
        iMarkPos = CP_MARGIN_L + (int) 
            (uPicInfo.sDEMON.dDEMONBand * dFreqScale + 0.5);
        ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
            iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN));
        sprintf(cpTextBuf, "%g", uPicInfo.sDEMON.dDEMONBand);
        GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight);
        ImgDst->fontTypeMetrics(cpTextBuf, &TextMetrics);
        ImgDst->draw(DrawableText(iMarkPos - iTextWidth / 2,
            CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN + TextMetrics.ascent() + 1,
            cpTextBuf));
    }
    catch (Exception &X)
    {
        fprintf(stderr, "exception: %s\n", X.what());
        return false;
    }

    return true;
}


bool clConvPic::DrawSGramInfo ()
{
    time_t ttCreateTime;
    time_t ttTickTime;
    int iMarkCntr;
    int iMarkCount;
    int iMarkPos;
    int iTextWidth;
    int iTextHeight;
    int iAdvanceY;
    double dStartTime;
    double dEndTime;
    double dDeltaTime;
    double dDeltaFreq;
    double dFreqScale;
    double dRoundedRange;
    double dRoundStep;
    double dCountScale;
    double dTickStep;
    double dTickFreq;
    char cpTimeBuf[24];
    char cpTextBuf[4096];
    TypeMetric TextMetrics;

    try
    {
        ImgDst->fontPointsize(CP_FONT_NORMAL);
        ImgDst->strokeColor(ColorRGB(0, 0, 0));

        dEndTime = (double) uPicInfo.sSGram.ttTime;
        dStartTime = dEndTime - uPicInfo.sSGram.dLineTime * (double) iSrcWidth;
        dDeltaTime = dEndTime - dStartTime;

        ImgDst->fontPointsize(CP_FONT_LARGE);
        ttCreateTime = (time_t) (dEndTime + 0.5);
        strftime(cpTimeBuf, 23, "%Y/%m/%d %H:%M:%S", localtime(&ttCreateTime));
        sprintf(cpTextBuf, "Spectrogram %s", cpTimeBuf);
        GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight, &iAdvanceY);
        ImgDst->draw(DrawableText(1, iAdvanceY + 1, cpTextBuf));
        sprintf(cpTextBuf, "Band: %g - %g Hz", 
            uPicInfo.sSGram.dLowFreq, uPicInfo.sSGram.dHighFreq);
        ImgDst->draw(DrawableText(1, iAdvanceY * 2 + 1, cpTextBuf));

        ImgDst->fontPointsize(CP_FONT_SMALL);
        iMarkCount = 10;
        dTickStep = dDeltaTime / 10.0;
        for (iMarkCntr = 0; iMarkCntr < iMarkCount; iMarkCntr++)
        {
            if (iMarkCntr % 1)
            {
                iMarkPos = CP_MARGIN_L + 
                    (int) ((double) iMarkCntr * dTickStep / 
                    uPicInfo.sSGram.dLineTime + 0.5);
                ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
                    iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKS_LEN));
            }
            else
            {
                ttTickTime = (time_t) 
                    (dStartTime + (double) iMarkCntr * dTickStep);
                strftime(cpTimeBuf, 9, "%H:%M:%S", localtime(&ttTickTime));
                sprintf(cpTextBuf, "%s.%03i", cpTimeBuf, (int) 
                    ((dStartTime + (double) iMarkCntr * (double) dTickStep - 
                    (double) ttTickTime) * 1000.0 + 0.5));
                GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight);
                ImgDst->fontTypeMetrics(cpTextBuf, &TextMetrics);
                iMarkPos = CP_MARGIN_L + 
                    (int) ((double) iMarkCntr * dTickStep / 
                    uPicInfo.sSGram.dLineTime + 0.5);
                ImgDst->draw(DrawableText(iMarkPos - iTextWidth / 2, 
                    CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN + TextMetrics.ascent() + 1,
                    cpTextBuf));
                ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
                    iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN));
            }
        }
        ttTickTime = (time_t) dEndTime;
        strftime(cpTimeBuf, 9, "%H:%M:%S", localtime(&ttTickTime));
        sprintf(cpTextBuf, "%s.%03i", cpTimeBuf, (int) 
            ((dEndTime - (double) ttTickTime) * 1000.0 + 0.5));
        GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight);
        ImgDst->fontTypeMetrics(cpTextBuf, &TextMetrics);
        iMarkPos = CP_MARGIN_L + iSrcWidth;
        ImgDst->draw(DrawableText(iMarkPos - iTextWidth / 2, 
            CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN + TextMetrics.ascent() + 1, 
            cpTextBuf));
        ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
            iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN));

        dDeltaFreq = uPicInfo.sSGram.dHighFreq - uPicInfo.sSGram.dLowFreq;
        dFreqScale = (double) iSrcHeight / dDeltaFreq;
        dRoundStep = pow(10.0, floor(log10(dDeltaFreq)));
        dRoundedRange = dDeltaFreq - fmod(dDeltaFreq, dRoundStep);
        dCountScale = 0.1;
        iMarkCount = (int) (dDeltaFreq / (dRoundStep * dCountScale) + 0.5);
        dTickStep = dRoundStep * dCountScale;
        for (iMarkCntr = 0; iMarkCntr < iMarkCount; iMarkCntr++)
        {
            dTickFreq = uPicInfo.sSGram.dLowFreq + 
                (double) iMarkCntr * dTickStep;
            iMarkPos = CP_MARGIN_T + iSrcHeight - 
                (int) ((double) iMarkCntr * dTickStep * dFreqScale + 0.5);
            if (iMarkCntr % 5)
            {
                ImgDst->draw(DrawableLine(CP_MARGIN_L - CP_TICKS_LEN, iMarkPos, 
                    CP_MARGIN_L, iMarkPos));
            }
            else
            {
                ImgDst->draw(DrawableLine(CP_MARGIN_L - CP_TICKL_LEN, iMarkPos, 
                    CP_MARGIN_L, iMarkPos));
                sprintf(cpTextBuf, "%g", dTickFreq);
                GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight);
                ImgDst->fontTypeMetrics(cpTextBuf, &TextMetrics);
                ImgDst->draw(DrawableText(
                    CP_MARGIN_L - CP_TICKL_LEN - iTextWidth - 1,
                    iMarkPos + TextMetrics.descent() - 1, 
                    cpTextBuf));
            }
        }
        iMarkPos = CP_MARGIN_T;
        ImgDst->draw(DrawableLine(CP_MARGIN_L - CP_TICKL_LEN, iMarkPos, 
            CP_MARGIN_L, iMarkPos));
        sprintf(cpTextBuf, "%g", uPicInfo.sSGram.dHighFreq);
        GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight);
        ImgDst->fontTypeMetrics(cpTextBuf, &TextMetrics);
        ImgDst->draw(DrawableText(CP_MARGIN_L - CP_TICKL_LEN - iTextWidth - 1,
            iMarkPos + TextMetrics.descent() - 1, 
            cpTextBuf));
    }
    catch (Exception &X)
    {
        fprintf(stderr, "exception: %s\n", X.what());
        return false;
    }

    return true;
}


bool clConvPic::DrawTBearInfo ()
{
    time_t ttCreateTime;
    time_t ttTickTime;
    int iMarkCntr;
    int iMarkCount;
    int iMarkPos;
    int iTextWidth;
    int iTextHeight;
    int iAdvanceY;
    double dStartTime;
    double dEndTime;
    double dDeltaTime;
    double dDeltaDir;
    double dDirScale;
    double dTickStep;
    double dTickDir;
    char cpTimeBuf[24];
    char cpTextBuf[4096];
    TypeMetric TextMetrics;

    try
    {
        ImgDst->fontPointsize(CP_FONT_NORMAL);
        ImgDst->strokeColor(ColorRGB(0, 0, 0));

        dStartTime = (double) uPicInfo.sTBear.ttTime;
        dEndTime = dStartTime + uPicInfo.sTBear.dIntTime * (double) iSrcHeight;
        dDeltaTime = dEndTime - dStartTime;

        ImgDst->fontPointsize(CP_FONT_LARGE);
        ttCreateTime = (time_t) (dEndTime + 0.5);
        strftime(cpTimeBuf, 23, "%Y/%m/%d %H:%M:%S", localtime(&ttCreateTime));
        sprintf(cpTextBuf, "Bearing-time %s", cpTimeBuf);
        GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight, &iAdvanceY);
        ImgDst->draw(DrawableText(1, iAdvanceY + 1, cpTextBuf));
        sprintf(cpTextBuf, 
            "Sector: %g - %g deg, %i beams, integration time %g s", 
            RadToDeg(uPicInfo.sTBear.dLeftDir),
            RadToDeg(uPicInfo.sTBear.dRightDir),
            uPicInfo.sTBear.iSectors,
            uPicInfo.sTBear.dIntTime);
        ImgDst->draw(DrawableText(1, iAdvanceY * 2 + 1, cpTextBuf));

        ImgDst->fontPointsize(CP_FONT_SMALL);
        iMarkCount = (int) (dDeltaTime / 60.0);
        for (iMarkCntr = 0; iMarkCntr <= iMarkCount; iMarkCntr++)
        {
            ttTickTime = (time_t) 
                (dStartTime + (double) iMarkCntr * 60.0 + 0.5);
            strftime(cpTimeBuf, 9, "%H:%M:%S", localtime(&ttTickTime));
            GetTextSize(cpTimeBuf, &iTextWidth, &iTextHeight);
            ImgDst->fontTypeMetrics(cpTimeBuf, &TextMetrics);
            iMarkPos = CP_MARGIN_T + iSrcHeight - 
                (int) ((double) iMarkCntr * 60.0 / 
                uPicInfo.sTBear.dIntTime + 0.5);
            if (iMarkPos < (CP_MARGIN_T + iTextHeight))
                continue;
            ImgDst->draw(DrawableText(CP_MARGIN_L - iTextWidth - 1, 
                iMarkPos + TextMetrics.descent() - 1, cpTimeBuf));
            ImgDst->draw(DrawableLine(CP_MARGIN_L - CP_TICKL_LEN, iMarkPos,
                CP_MARGIN_L, iMarkPos));
        }
        ttTickTime = (time_t) (dEndTime + 0.5);
        strftime(cpTimeBuf, 9, "%H:%M:%S", localtime(&ttTickTime));
        GetTextSize(cpTimeBuf, &iTextWidth, &iTextHeight);
        ImgDst->fontTypeMetrics(cpTimeBuf, &TextMetrics);
        iMarkPos = CP_MARGIN_T;
        ImgDst->draw(DrawableText(CP_MARGIN_L - iTextWidth - 1, 
            iMarkPos + TextMetrics.descent() - 1, cpTimeBuf));
        ImgDst->draw(DrawableLine(CP_MARGIN_L - CP_TICKL_LEN, iMarkPos,
            CP_MARGIN_L, iMarkPos));

        dDeltaDir = RadToDeg(uPicInfo.sTBear.dRightDir) - 
            RadToDeg(uPicInfo.sTBear.dLeftDir);
        dDirScale = (double) iSrcWidth / dDeltaDir;
        iMarkCount = 90;
        dTickStep = dDeltaDir / (double) iMarkCount;
        for (iMarkCntr = 0; iMarkCntr < iMarkCount; iMarkCntr++)
        {
            dTickDir = RadToDeg(uPicInfo.sTBear.dLeftDir) + 
                (double) iMarkCntr * dTickStep;
            iMarkPos = CP_MARGIN_L + 
                (int) ((double) iMarkCntr * dTickStep * dDirScale + 0.5);
            if (iMarkCntr % 5)
            {
                ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
                    iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKS_LEN));
            }
            else
            {
                ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
                    iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN));
                sprintf(cpTextBuf, "%0.0f", dTickDir);
                GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight);
                ImgDst->fontTypeMetrics(cpTextBuf, &TextMetrics);
                ImgDst->draw(DrawableText(iMarkPos - iTextWidth / 2,
                    CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN + TextMetrics.ascent() + 1,
                    cpTextBuf));
            }
        }
        iMarkPos = CP_MARGIN_L + (int) (dDeltaDir * dDirScale + 0.5);
        ImgDst->draw(DrawableLine(iMarkPos, CP_MARGIN_T + iSrcHeight,
            iMarkPos, CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN));
        sprintf(cpTextBuf, "%g", RadToDeg(uPicInfo.sTBear.dRightDir));
        GetTextSize(cpTextBuf, &iTextWidth, &iTextHeight);
        ImgDst->fontTypeMetrics(cpTextBuf, &TextMetrics);
        ImgDst->draw(DrawableText(iMarkPos - iTextWidth / 2,
            CP_MARGIN_T + iSrcHeight + CP_TICKL_LEN + TextMetrics.ascent() + 1,
            cpTextBuf));
    }
    catch (Exception &X)
    {
        fprintf(stderr, "exception: %s\n", X.what());
        return false;
    }

    return true;
}


bool clConvPic::ConvertColors ()
{
    /*int iPixCntr;
    int iPixCount;
    float fScale;
    float fPixel;
    DATA32 *uipData;
    
    uipData = imlib_image_get_data();
    if (uipData == NULL)
        return false;
    iPixCount = imlib_image_get_width() * imlib_image_get_height();
    fScale = 1.0f / 255.0f / 3.0f;
    for (iPixCntr = 0; iPixCntr < iPixCount; iPixCntr++)
    {
        fPixel = ((float) (uipData[iPixCntr] & 0xff) * fScale) +
            ((float) ((uipData[iPixCntr] >> 8) & 0xff) * fScale) +
            ((float) ((uipData[iPixCntr] >> 16) & 0xff) * fScale);
        uipData[iPixCntr] = (fPixel >= 0.75f) ? 0x00ffffff : 0x00000000;
    }
    imlib_image_put_back_data(uipData);*/

    try
    {
        ImgDst->quantizeColors(2);
        ImgDst->type(BilevelType);
    }
    catch (Exception &X)
    {
        fprintf(stderr, "exception: %s\n", X.what());
        return false;
    }

    return true;
}


bool clConvPic::SaveResult (const char *cpDstFileName)
{
    try
    {
        //ImgDst->display();
        if (cpPageSize)
        {
            ImgDst->rotate(270);
            //ImgDst->zoom(Geometry("A4+100+100"));
            //ImgDst->zoom(Geometry("300x300%"));
        }
        ImgDst->write(cpDstFileName);
    }
    catch (Exception &X)
    {
        fprintf(stderr, "exception: %s\n", X.what());
        return false;
    }

    return true;
}


void clConvPic::Clean ()
{
    try
    {
        delete ImgSrc;
        delete ImgDst;
    }
    catch (Exception &X)
    {
        fprintf(stderr, "exception: %s\n", X.what());
    }
}


void clConvPic::SplitLine (const char *cpSource, string &strFirst,
    string &strSecond)
{
    char *cpSrcCopy;
    char *cpParsePtr;

    cpSrcCopy = strdup(cpSource);
    cpParsePtr = strtok(cpSrcCopy, " \t\n");
    if (cpParsePtr != NULL)
        strFirst = cpParsePtr;
    cpParsePtr = strtok(NULL, "\n");
    if (cpParsePtr != NULL)
        strSecond = cpParsePtr;
    free(cpSrcCopy);
}


time_t clConvPic::ParseTime (const char *cpTimeString)
{
    int iYear;
    int iMonth;
    int iDay;
    int iHour;
    int iMin;
    int iSec;
    struct tm sTM;

    sscanf(cpTimeString, "%d/%d/%d %d:%d:%d",
        &iYear, &iMonth, &iDay, &iHour, &iMin, &iSec);

    memset(&sTM, 0x00, sizeof(sTM));
    sTM.tm_year = iYear - 1900;
    sTM.tm_mon = iMonth - 1;
    sTM.tm_mday = iDay;
    sTM.tm_hour = iHour;
    sTM.tm_min = iMin;
    sTM.tm_sec = iSec;
#   ifdef LINUXSYS
    sTM.tm_isdst = daylight;
#   endif

    return mktime(&sTM);
}


inline double clConvPic::RadToDeg (double dRad)
{
    return (180.0 / acos(-1.0) * dRad);
}


void clConvPic::GetTextSize (const char *cpText, int *ipWidth, int *ipHeight,
    int *ipAdvanceY)
{
    try
    {
        TypeMetric TextMetrics;

        ImgDst->fontTypeMetrics(cpText, &TextMetrics);
        *ipWidth = (int) (TextMetrics.textWidth() + 0.5);
        *ipHeight = (int) (TextMetrics.textHeight() + 0.5);
        if (ipAdvanceY)
            *ipAdvanceY = (int) 
                (TextMetrics.ascent() - TextMetrics.descent() + 0.5);
    }
    catch (Exception &X)
    {
        fprintf(stderr, "exception: %s\n", X.what());
    }
}


clConvPic::clConvPic ()
{
    bTwoColor = false;
    iType = -1;
    cpPageSize = NULL;
}


clConvPic::~clConvPic ()
{
}


int clConvPic::Main (int *ipArgC, char ***cpppArgV)
{
    int iArgCntr = 1;
    int iArgC = *ipArgC;
    char **cppArgV = *cpppArgV;

    if (iArgC < 4)
    {
        printf("%s <-l|-d|-s|-b> [-2] [-pagesize] <input> <output>\n", 
            cppArgV[0]);
        puts("\t-l  LOFAR");
        puts("\t-d  DEMON");
        puts("\t-s  spectrogram");
        puts("\t-b  time-bearing");
        puts("\t-2  convert to 2-colors");
        return 1;
    }

    while (iArgCntr < (iArgC - 2))
    {
        if (strcmp(cppArgV[iArgCntr], "-l") == 0)
            iType = CP_TYPE_LOFAR;
        else if (strcmp(cppArgV[iArgCntr], "-d") == 0)
            iType = CP_TYPE_DEMON;
        else if (strcmp(cppArgV[iArgCntr], "-s") == 0)
            iType = CP_TYPE_SGRAM;
        else if (strcmp(cppArgV[iArgCntr], "-b") == 0)
            iType = CP_TYPE_TBEAR;
        else if (strcmp(cppArgV[iArgCntr], "-2") == 0)
            bTwoColor = true;
        else if (cppArgV[iArgCntr][0] == '-')
            cpPageSize = strdup(&cppArgV[iArgCntr][1]);
        if (cppArgV[iArgCntr][0] != '-')
            break;
        iArgCntr++;
    }
    if (iType < 0)
    {
        puts("Type not specified!");
        return 2;
    }

    if (!Initialize())
        return 3;

    switch (iType)
    {
        case CP_TYPE_LOFAR:
            if (!LoadLOFARInfo(cppArgV[iArgCntr]))
                return 4;
            break;
        case CP_TYPE_DEMON:
            if (!LoadDEMONInfo(cppArgV[iArgCntr]))
                return 4;
            break;
        case CP_TYPE_SGRAM:
            if (!LoadSGramInfo(cppArgV[iArgCntr]))
                return 4;
            break;
        case CP_TYPE_TBEAR:
            if (!LoadTBearInfo(cppArgV[iArgCntr]))
                return 4;
            break;
    }

    if (!LoadAndCreate(cppArgV[iArgCntr]))
        return 5;

    iArgCntr++;

    if (!CopySource())
        return 6;

    switch (iType)
    {
        case CP_TYPE_LOFAR:
            if (!DrawLOFARInfo())
                return 7;
            break;
        case CP_TYPE_DEMON:
            if (!DrawDEMONInfo())
                return 7;
            break;
        case CP_TYPE_SGRAM:
            if (!DrawSGramInfo())
                return 7;
            break;
        case CP_TYPE_TBEAR:
            if (!DrawTBearInfo())
                return 7;
            break;
    }

    if (bTwoColor)
    {
        if (!ConvertColors())
            return 8;
    }

    if (!SaveResult(cppArgV[iArgCntr]))
        return 10;

    Clean();

    return 0;
}
