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


#include <ctime>
#include <Magick++.h>


#ifndef CONVPIC_HH
#define CONVPIC_HH

#   define CP_FONT_NAME     "helvetica"
#   define CP_FONT_SMALL    10 /* fbfont/8 */
#   define CP_FONT_NORMAL   12 /* fbfont/10 */
#   define CP_FONT_LARGE    16 /* fbfont/12 */
#   define CP_PAGE_SPEC     "+100+100"
#   define CP_MARGIN_L      100
#   define CP_MARGIN_T      100
#   define CP_MARGIN_R      50
#   define CP_MARGIN_B      50
#   define CP_TICKL_LEN     10
#   define CP_TICKS_LEN     5


    using namespace std;
    using namespace Magick;


    /**
        Types for different kinds of saved images.
    */
    enum
    {
        CP_TYPE_LOFAR,
        CP_TYPE_DEMON,
        CP_TYPE_SGRAM,
        CP_TYPE_TBEAR
    };


    /**
        Information stored in saved LOFAR .tif.inf.
    */
    typedef struct _stLOFARInfo
    {
        time_t ttTime;      ///< Oldest dataline time
        double dLowFreq;    ///< Lower frequency limit
        double dHighFreq;   ///< Upper frequency limit
        double dLineTime;   ///< Length of one scanline in time (s)
    } stLOFARInfo, *stpLOFARInfo;

    /**
        Information stored in saved DEMON .tif.inf.
    */
    typedef struct _stDEMONInfo
    {
        time_t ttTime;      ///< Oldest dataline time
        double dLowFreq;    ///< Lower frequency limit
        double dHighFreq;   ///< Upper frequency limit
        double dDEMONBand;  ///< DEMON bandwidth
        double dLineTime;   ///< Length of one scanline in time (s)
    } stDEMONInfo, *stpDEMONInfo;

    /**
        Information stored in saved spectrogram .tif.inf.
    */
    typedef struct _stSGramInfo
    {
        time_t ttTime;      ///< Newest dataline time
        double dLowFreq;    ///< Lower frequency limit
        double dHighFreq;   ///< Upper frequency limit
        double dLineTime;   ///< Length of one column in time (s)
    } stSGramInfo, *stpSGramInfo;

    /**
        Information stored in saved bearing-time .tif.inf.
    */
    typedef struct _stTBearInfo
    {
        time_t ttTime;      ///< Oldest dataline time
        double dLeftDir;    ///< Left angle (rad)
        double dRightDir;   ///< Right angle (rad)
        double dIntTime;    ///< Integration time (s)
        double dHighFreq;   ///< Upper frequency limit
        int iSectors;       ///< Number of sectors
    } stTBearInfo, *stpTBearInfo;

    /**
        Combination of information in all .inf files.
    */
    typedef union _uPicInfo
    {
        stLOFARInfo sLOFAR;
        stDEMONInfo sDEMON;
        stSGramInfo sSGram;
        stTBearInfo sTBear;
    } utPicInfo, *utpPicInfo;


    /**
        Conversion of .tif + .tif.inf to image containing information in
        the .inf file.
    */
    class clConvPic
    {
            bool bTwoColor;
            int iType;
            int iSrcWidth;
            int iSrcHeight;
            int iDstWidth;
            int iDstHeight;
            char *cpPageSize;
            utPicInfo uPicInfo;
            Image *ImgSrc;
            Image *ImgDst;
            bool Initialize ();
            bool LoadLOFARInfo (const char *);
            bool LoadDEMONInfo (const char *);
            bool LoadSGramInfo (const char *);
            bool LoadTBearInfo (const char *);
            bool LoadAndCreate (const char *);
            bool CopySource ();
            bool DrawLOFARInfo ();
            bool DrawDEMONInfo ();
            bool DrawSGramInfo ();
            bool DrawTBearInfo ();
            bool ConvertColors ();
            bool SaveResult (const char *);
            void Clean ();
            void SplitLine (const char *, string &, string &);
            time_t ParseTime (const char *);
            double RadToDeg (double);
            void GetTextSize (const char *, int *, int *, int * = NULL);
        public:
            clConvPic ();
            ~clConvPic ();
            int Main (int *, char ***);
    };

#endif
