/** \mainpage

    HASAS - HydroAcoustic Signal Analysis System

    Copyright (C) 1999-2002 Jussi Laako

    \author Jussi Laako
    $Date: 2003/06/16 17:08:30 $

    \par Overview
    HASAS is signal analysis software for passive sonar systems.

    \par Licensing
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    \par
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    \par
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


#ifndef __QNX__
#include "sys/soundcard.h"
#endif


#ifndef CONFIG_H
    #define CONFIG_H

    #ifdef __GNUC__
        #define CONSTF __const__
    #else
        #define CONSTF
    #endif

    /* Global */
    #define GLOBAL_DATATYPE         float
    #define GDT                     GLOBAL_DATATYPE
    #define GCDT                    stSCplx
    #define GPDT                    stSPolar
    #define GUDT                    utSCoord
    #define GDT_SCAN                "%f"
    #define GLOBAL_HEADER_LEN       256
    #define GLOBAL_SOCKET_PATH      "/tmp"
    #define GLOBAL_VERSMAJ          1
    #define GLOBAL_VERSMIN          6
    #define GLOBAL_VERSPL           0

    /* Sound card input server */
    #define SS_VERSMAJ              GLOBAL_VERSMAJ
    #define SS_VERSMIN              GLOBAL_VERSMIN
    #define SS_VERSPL               GLOBAL_VERSPL
    #define SS_SND_FORMAT           AFMT_S16_LE
    #define SS_SND_FORMAT_SIZE      2
    #define SS_SND_SAMPLERATE       44100
    #define SS_SND_CHANNELS         2
    #define SS_SND_DEVICE           "dsp0"
    #define SS_LOGFILE              "log/soundsrv.log"
    #define SS_SHUTDOWNFILE         "/tmp/soundsrv.shutdown"
    #define SS_MAXCLIENTS           8
    #define SS_TIMEOUT              250
    #define SS_DEFAULT_PORT         30000
    #define SS_MAXERRORS            100
    #define SS_SCHED_PRIORITY       1
    #define SS_SOCKET_BUF_FRAGS     16

    /* Sound card input server 2 */
    #define SS2_CFGFILE             "soundsrv2.cfg"
    #define SS2_LOGFILE             "log/soundsrv2.log"
    #define SS2_SHUTDOWNFILE        "/tmp/soundsrv2.shutdown"
    #define SS2_SOCKET_BUF_FRAGS    16
    #define SS2_FRAG_SIZE_DEFAULT   4096
    #define SS2_CONNECT_TIMEOUT     250
    #define SS2_INTHREAD_PRIORITY   4
    #define SS2_OUTTHREAD_PRIORITY  3

    /* Sound card input server for ALSA */
    #define SSA_CFGFILE             "soundsrva.cfg"
    #define SSA_LOGFILE             "log/soundsrva.log"
    #define SSA_SHUTDOWNFILE        "/tmp/soundsrva.shutdown"
    #define SSA_SOCKET_BUF_FRAGS    16
    #define SSA_FRAG_SIZE_DEFAULT   4096
    #define SSA_CONNECT_TIMEOUT     250
    #define SSA_INTHREAD_PRIORITY   4
    #define SSA_OUTTHREAD_PRIORITY  3

    /* Input server for Comedi */
    #define COM_CFGFILE             "comedisrv.cfg"
    #define COM_LOGFILE             "log/comedisrv.log"
    #define COM_SHUTDOWNFILE        "/tmp/comedisrv.shutdown"
    #define COM_SOCKET_BUF_FRAGS    16
    #define COM_FRAG_SIZE_DEFAULT   4096
    #define COM_CONNECT_TIMEOUT     250
    #define COM_INTHREAD_PRIORITY   4
    #define COM_OUTTHREAD_PRIORITY  3
    #define COM_USE_DITHER          true

    /* File input server */
    #define FS_CFGFILE              "filesrv.cfg"
    #define FS_FRAG_SIZE_DEFAULT    4096
    #define FS_DEFAULT_PORT         30000
    #define FS_INTHREAD_PRIORITY    4
    #define FS_OUTTHREAD_PRIORITY   3

    /* Beamforming input server */
    #define BS_CFGFILE              "beamsrv.cfg"
    #define BS_LOGFILE              "log/beamsrv.log"
    #define BS_INTHREAD_PRIORITY    4
    #define BS_OUTTHREAD_PRIORITY   3

    /* Local input data stream distributor */
    #define SD_VERSMAJ              GLOBAL_VERSMAJ
    #define SD_VERSMIN              GLOBAL_VERSMIN
    #define SD_VERSPL               GLOBAL_VERSPL
    #define SD_MAXCLIENTS           128
    #define SD_CONNECT_TIMEOUT      250
    #define SD_TIMEOUT              1000
    #define SD_CFGFILE              "streamdist.cfg"
    #define SD_LOGFILE              "log/streamdist.log"
    #define SD_SHUTDOWNFILE         "/tmp/streamdist.shutdown"
    #define SD_MAX_ADDR_LEN         128
    #define SD_SCHED_PRIORITY       1
    #define SD_BUFFER_SIZE          4096                /* in bytes */
    #define SD_INTHREAD_PRIORITY    2
    #define SD_OUTTHREAD_PRIORITY   1

    /* Save server */
    #define SAVS_VERSMAJ            GLOBAL_VERSMAJ
    #define SAVS_VERSMIN            GLOBAL_VERSMIN
    #define SAVS_VERSPL             GLOBAL_VERSPL
    #define SAVS_CFGFILE            "savesrv.cfg"
    #define SAVS_LOGFILE            "log/savesrv.log"
    #define SAVS_SHUTDOWNFILE       "/tmp/savesrv.shutdown"
    #define SAVS_TIMEOUT            1000

    /* User interface server */
    #define UIS_VERSMAJ             GLOBAL_VERSMAJ
    #define UIS_VERSMIN             GLOBAL_VERSMIN
    #define UIS_VERSPL              GLOBAL_VERSPL
    #define UIS_CFGFILE             "uiserv.cfg"
    #define UIS_LOGFILE             "log/uiserv.log"
    #define UIS_SHUTDOWNFILE        "/tmp/uiserv.shutdown"
    #define UIS_DEFAULT_PORT        30001
    #define UIS_TIMEOUT             250
    #define UIS_MSG_TIMEOUT         1000

    /* Spectrum server; see Spectrum.hh for complex datatype! */
    #define SPECT_CFGFILE           "spectrum.cfg"
    #define SPECT_TIMEOUT           250
    #define SPECT_RAW1ST_TIMEOUT    1000
    #define SPECT_REQ_TIMEOUT       5000
    #define SPECT_DEF_FILTSIZE      4096
    /*#define SPECT_DB_SCALE          144.4943979*/
    /*#define SPECT_DB_SCALE          96.32959861*/
    #define SPECT_BAND_LIMIT        10.0F       /* lower frequency limit
                                                   for MR-STFT */

    /* Transient spectrum GUI */
    #define SGUI_CFGFILE            "guispect.cfg"
    #define SGUI_HOSTFILE           "guispect.hosts"
    #define SGUI_REQ_PROC           "spectrum"

    /* Array processing */
    #define AB_KBWIN_ALPHA          3.0
    #define BF_MAX_X_SENSORS        64
    #define BF_MAX_Y_SENSORS        16

    /* Direction server */
    #define DIR_CFGFILE             "direction.cfg"
    #define DIR_MAX_CPUS            8
    #define DIR_REQ_TIMEOUT         5000
    #define DIR_RAW1ST_TIMEOUT      1000
    #define DIR_TIMEOUT             250
    #define DIR_DB_SCALE            144.4943979
    #define DIR_DEF_WIN_SIZE        2048

    /* Direction server 2 */
    #define DIR2_CFGFILE            "direction2.cfg"
    #define DIR2_REQ_TIMEOUT        5000
    #define DIR2_RAW1ST_TIMEOUT     1000
    #define DIR2_TIMEOUT            250
    #define DIR2_DEF_FILT_SIZE      2048
    #define DIR2_DEF_FFT_SIZE       4096

    /* Direction server 3 */
    #define DIR3_CFGFILE            "direction3.cfg"
    #define DIR3_REQ_TIMEOUT        5000
    #define DIR3_RAW1ST_TIMEOUT     1000
    #define DIR3_TIMEOUT            250
    #define DIR3_DEF_FILT_SIZE      2048
    /*#define DIR3_DEF_FFT_SIZE       4096*/  /* requested by client */

    /* Direction finding GUI */
    #define DGUI_CFGFILE            "guidir.cfg"
    #define DGUI_HOSTFILE           "guidir.hosts"
    #define DGUI_REQ_PROC           "direction"
    #define DGUI_REQ_PROC2          "direction2"
    #define DGUI_DEF_LINES          300
    #define DGUI_DEF_SOUNDSPEED     1430.0

    /* LOFAR/DEMON server */
    #define LOFAR_CFGFILE           "lofardemon.cfg"
    #define LOFAR_REQ_TIMEOUT       5000
    #define LOFAR_RAW1ST_TIMEOUT    1000
    #define LOFAR_TIMEOUT           250
    #define LOFAR_DEF_FILTER_SIZE   4096
    #define LOFAR_DEF_DC_BLOCK      1

    /* LOFAR/DEMON GUI */
    #define LGUI_CFGFILE            "guilofar.cfg"
    #define LGUI_HOSTFILE           "guilofar.hosts"
    #define LGUI_REQ_PROC           "lofardemon"
    #define LGUI_DEF_WIN_LENGTH     1024
    #define LGUI_DEF_LOW_FREQ       0.0
    #define LGUI_DEF_HIGH_FREQ      1000.0
    #define LGUI_DEF_REMOVE_NOISE   0
    #define LGUI_DEF_ALPHA          1.5
    #define LGUI_DEF_MEAN_LENGTH    100
    #define LGUI_DEF_GAP_LENGTH     3
    #define LGUI_DEF_HEIGHT         100

    /* Sound UI */
    #define SUI_MAX_CHANNELS        8
    #define SUI_SND_FORMAT          AFMT_S16_LE
    #define SUI_SND_BITS            16
    #define SUI_SND_DATATYPE        signed short
    #define SUI_SND_QUEUESIZE       4096
    #define SUI_SAMPLECOUNT         2048
    #define SUI_DEF_DEVICE          "/dev/dsp"
    #define SUI_DEF_CHANNELS        2
    #define SUI_DEF_SAMPLERATE      44100
    #define SUI_CFGFILE             "soundui.cfg"
    #define SUI_HOSTFILE            "soundui.hosts"
    #define SUI_FIRST_TIMEOUT       1000
    #define SUI_IN_TIMEOUT          250
    #define SUI_VU_TIMEOUT          200

    /* Sound proxy */
    #define SP_VERSMAJ              GLOBAL_VERSMAJ
    #define SP_VERSMIN              GLOBAL_VERSMIN
    #define SP_VERSPL               GLOBAL_VERSPL
    #define SP_CFGFILE              "soundproxy.cfg"
    #define SP_DEF_LOGFILE          "log/soundproxy.cfg"
    #define SP_SERV_MAXLEN          255
    #define SP_MAXCLIENTS           255
    #define SP_1ST_MSG_TIMEOUT      1000
    #define SP_MSG_TIMEOUT          250
    #define SP_WAIT_CONN_TIMEOUT    250
    #define SP_SCHED_PRIORITY       1
    #define SP_BUFFER_SIZE          4096                /* in bytes */

    /* Beam audio */
    #define BA_CFGFILE              "beamaudio.cfg"
    #define BA_1STREQ_TIMEOUT       5000
    #define BA_RAW1ST_TIMEOUT       1000
    #define BA_TIMEOUT              250
    #define BA_DEF_FRAGMENT_SIZE    4096
    #define BA_SCHED_PRIORITY       2

    /* Beam audio UI */
    #define BAUI_CFGFILE            "beamaudioui.cfg"
    #define BAUI_HOSTFILE           "beamaudioui.hosts"
    #define BAUI_REQ_PROC           "beamaudio"
    #define BAUI_1ST_TIMEOUT        5000
    #define BAUI_DEF_SOUNDSPEED     "1430.0"
    #define BAUI_DEF_DIR_RANGE      180.0
    #define BAUI_INT_DATATYPE       int
    #define BAUI_SND_FORMAT         AFMT_S16_LE
    #define BAUI_SND_BITS           16
    #define BAUI_SND_DATATYPE       signed short

    /* Level server */
    #define LEVEL_CFGFILE           "level.cfg"
    #define LEVEL_1STREQ_TIMEOUT    5000
    #define LEVEL_RAW1ST_TIMEOUT    1000
    #define LEVEL_TIMEOUT           250
    #define LEVEL_DEF_FILTER_SIZE   4096

    /* Level GUI */
    #define GUILEV_CFGFILE          "guilevel.cfg"
    #define GUILEV_HOSTFILE         "guilevel.hosts"
    #define GUILEV_REQ_PROC         "level"

    /* Locate system */
    #define LOCATE_CFGFILE          "locate.cfg"
    #define LOCATE_LOGFILE          "log/locate.log"
    #define LOCATE_SENSOR_LIST      "locate.sensors"
    #define LOCATE_DIR_PROC         "direction3"
    #define LOCATE_TIMEOUT          250
    #define LOCATE_DEF_WINDOWSIZE   4096
    #define LOCATE_DEF_SOUNDSPEED   1430.0
    #define LOCATE_DEF_LOWFREQ      10.0
    #define LOCATE_DEF_INTTIME      1.0
    #define LOCATE_DEF_SCALING      0
    #define LOCATE_DEF_SCALINGEXP   2.0
    #define LOCATE_DEF_REMOVENOISE  0
    #define LOCATE_DEF_ALPHA        2.0
    #define LOCATE_DEF_MEANLENGTH   10
    #define LOCATE_DEF_GAPLENGTH    3

    /* Locate GUI */
    #define GUILOC_CFGFILE          "guilocate.cfg"
    #define GUILOC_HOSTFILE         "guilocate.hosts"

    /* Transient GUI */
    #define GUITRANS_CFGFILE        "guitrans.cfg"
    #define GUITRANS_HOSTFILE       "guitrans.hosts"
    
#endif

