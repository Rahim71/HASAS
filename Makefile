#
# Makefile for HASAS
# Copyright (C) 1999-2003 Jussi Laako
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

CXX = g++
MPICXX = mpiCC
LIBTOOL = libtool

# gcc 3.0.x
#CXXFLAGS = -march=athlon -mcpu=athlon -O6 -ffast-math -funroll-loops -mfancy-math-387 -malign-jumps=5 -malign-loops=5 -malign-functions=5 -mpreferred-stack-boundary=5 -Wall -Werror #-ggdb
# gcc 3.2.x
#  -tbird, -4, -xp, -mp
CXXFLAGS = -march=pentium2 -mcpu=athlon-xp -mmmx
#CXXFLAGS = -march=athlon-tbird -mcpu=athlon-xp -m3dnow #-msse -mfpmath=sse
#CXXFLAGS = -march=pentium3 -mcpu=pentium3 -msse -mfpmath=sse
#CXXFLAGS = -march=pentium4 -mcpu=pentium4 -msse2 -mfpmath=sse -m128bit-long-double
CXXFLAGS += -O3 -ffast-math -funroll-loops -fprefetch-loop-arrays -Wall #-Werror #-ggdb

# thread safety
DEFS = -D_REENTRANT -D_THREAD_SAFE
# Linux system
DEFS += -DLINUXSYS
# define if you have <endian.h>, otherwise -DLITTLE_ENDIAN or -DBIG_ENDIAN
DEFS += -DHAVE_ENDIAN_H
# define if your compiler/c-library conforms with ISO C 9x standards
DEFS += -D_ISOC9X_SOURCE
# define if you are using GNU C++-compiler and GNU C-library
DEFS += -D_GNU_SOURCE
# define if you have GNU c-library
DEFS += -DHAVE_GLIBC
# define if you like to use g_snprintf() from glib
DEFS += -DUSE_G_SNPRINTF
# define if you don't have the commercial OSS driver
#DEFS += -DUSE_OSSLITE
# define if you have the Mersenne twister PRNG
DEFS += -DUSE_MERSENNE_TWISTER
# define if you have GNU Scientific Library (GSL)
DEFS += -DUSE_GSL
# define to use POSIX read/write-locks instead of mutexes
#DEFS += -D_XOPEN_SOURCE=600 -D_SVID_SOURCE -D_BSD_SOURCE -DUSE_RWLOCK
# define USE_ALSA05 for ALSA 0.5.x and USE_ALSA09 for ALSA 0.9.x
DEFS += -DUSE_ALSA09
# define for FLAC compression support
DEFS += -DUSE_FLAC
# define for forced backing store in spectrogram
#DEFS += -DUSE_BACKING_STORE
# define for trillium based debug output on MPI
#DEFS += -DUSE_TRILLIUM

INCS = `gtk-config --cflags` `freetype-config --cflags` `pkg-config --cflags sndfile` `Magick++-config --cppflags`
MPIINCS = -I/usr/local/LAM/include/mpi2c++ -I/usr/local/LAM/include

LDFLAGS = #-ggdb

LIBS = -ldsp -lm -lrt -lpthread `glib-config --libs` #-lefence
#LIBS = /usr/local/lib/libdsp.a -lm -lrt -lpthread `glib-config --libs`
LIBST = -ldsp -lm -lrt -lpthread `glib-config --libs gthread` -lDynThreads #-lefence
#LIBST = /usr/local/lib/libdsp.a -lm -lrt -lpthread `glib-config --libs gthread` -lDynThreads
FFTWLIBS = #-lsrfftw -lsfftw
PTHLIBS = `pth-config --libdir`/libpth.a
SNDFLIBS = `pkg-config --libs sndfile`
#TIFFLIBS = /usr/local/lib/libtiff.a -lz -ljpeg
TIFFLIBS = -ltiff -lz -ljpeg
#GSLLIBS = `gsl-config --libs`
GSLLIBS = `gsl-config --prefix`/lib/libgsl.a
FTLIBS = `freetype-config --libs`
FLACLIBS = -lFLAC
#FLACLIBS = /usr/local/lib/libFLAC.a
MAGICKLIBS = -L/usr/X11R6/lib `Magick++-config --libs`

CCSRC = cc
OBJ = o

SRCS = ArrayBase.$(CCSRC) \
	ArrayDipole.$(CCSRC) \
	ArraySensor.$(CCSRC) \
	Audio.$(CCSRC) \
	AudioA.$(CCSRC) \
	AudioA2.$(CCSRC) \
	Audio3D.$(CCSRC) \
	BeamAudio.$(CCSRC) \
	BeamAudioUI.$(CCSRC) \
	BeamDipole.$(CCSRC) \
	BeamSrv.$(CCSRC) \
	BeamSrv2.$(CCSRC) \
	CfgFile.$(CCSRC) \
	Cluster.$(CCSRC) \
	ComediIO.$(CCSRC) \
	ComediSrv.$(CCSRC) \
	ConvPic.$(CCSRC) \
	CorrDipole.$(CCSRC) \
	Direction.$(CCSRC) \
	Direction2.$(CCSRC) \
	Direction3.$(CCSRC) \
	FileSrv.$(CCSRC) \
	FrameBuf.$(CCSRC) \
	FreqBeamDipole.$(CCSRC) \
	FreqBeamLine.$(CCSRC) \
	GtkUtils.$(CCSRC) \
	GUIDir.$(CCSRC) \
	GUILevel.$(CCSRC) \
	GUILocate.$(CCSRC) \
	GUILofar.$(CCSRC) \
	GUISpect.$(CCSRC) \
	GUITrans.$(CCSRC) \
	Level.$(CCSRC) \
	Locate.$(CCSRC) \
	LocateSensor.$(CCSRC) \
	LocateSystem.$(CCSRC) \
	LofarDemon.$(CCSRC) \
	LogFile.$(CCSRC) \
	Messages.$(CCSRC) \
	Palette.$(CCSRC) \
	RemoveNoise.$(CCSRC) \
	SaveSrv.$(CCSRC) \
	SockClie.$(CCSRC) \
	SockOp.$(CCSRC) \
	SockServ.$(CCSRC) \
	SoundProxy.$(CCSRC) \
	SoundSrv.$(CCSRC) \
	SoundSrv2.$(CCSRC) \
	SoundSrvA.$(CCSRC) \
	SoundUI.$(CCSRC) \
	SpectDir.$(CCSRC) \
	SpectDir2.$(CCSRC) \
	SpectDirDipole.$(CCSRC) \
	SpectDirDipole2.$(CCSRC) \
	SpectDirLine.$(CCSRC) \
	SpectDirLine2.$(CCSRC) \
	Spectrum.$(CCSRC) \
	StreamDist.$(CCSRC) \
	TestSnd.$(CCSRC) \
	UIServ.$(CCSRC)

BINS = soundsrv \
	soundsrv2 \
	soundsrva \
	comedisrv \
	streamdist \
	savesrv \
	filesrv \
	beamsrv \
	beamsrv2 \
	uiserv \
	spectrum \
	direction \
	direction2 \
	direction3 \
	locate \
	lofardemon \
	beamaudio \
	level \
	guispect \
	guidir \
	guilevel \
	guilocate \
	guilofar \
	guitrans \
	soundui \
	beamaudioui \
	soundproxy \
	convpic \
	testsnd

SNDSERVOBJ = Audio.$(OBJ) \
	CfgFile.$(OBJ) \
	LogFile.$(OBJ) \
	Messages.$(OBJ) \
	SoundSrv.$(OBJ)

SNDSERV2OBJ = Audio.$(OBJ) \
	CfgFile.$(OBJ) \
	LogFile.$(OBJ) \
	Messages.$(OBJ) \
	SockOp.$(OBJ) \
	SockServ.$(OBJ) \
	SoundSrv2.$(OBJ)

SNDSERVAOBJ = AudioA.$(OBJ) \
	AudioA2.$(OBJ) \
	CfgFile.$(OBJ) \
	LogFile.$(OBJ) \
	Messages.$(OBJ) \
	SockOp.$(OBJ) \
	SockServ.$(OBJ) \
	SoundSrvA.$(OBJ)

COMSERVOBJ = CfgFile.$(OBJ) \
	ComediIO.$(OBJ) \
	ComediSrv.$(OBJ) \
	LogFile.$(OBJ) \
	Messages.$(OBJ) \
	SockOp.$(OBJ) \
	SockServ.$(OBJ)

SDOBJ = CfgFile.$(OBJ) \
	LogFile.$(OBJ) \
	Messages.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ) \
	SockServ.$(OBJ) \
	StreamDist.$(OBJ)

SAVSOBJ = CfgFile.$(OBJ) \
	LogFile.$(OBJ) \
	SaveSrv.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ)

FILSOBJ = CfgFile.$(OBJ) \
	FileSrv.$(OBJ) \
	GtkUtils.$(OBJ) \
	Messages.$(OBJ) \
	SockOp.$(OBJ) \
	SockServ.$(OBJ)

BEAMSOBJ = ArraySensor.$(OBJ) \
	BeamSrv.$(OBJ) \
	CfgFile.$(OBJ) \
	FreqBeamDipole.$(OBJ) \
	FreqBeamLine.$(OBJ) \
	LogFile.$(OBJ) \
	Messages.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ) \
	SockServ.$(OBJ)

BEAMS2OBJ = ArraySensor.$(OBJ) \
	BeamSrv2.$(OBJ) \
	CfgFile.$(OBJ) \
	FreqBeamDipole.$(OBJ) \
	FreqBeamLine.$(OBJ) \
	LogFile.$(OBJ) \
	Messages.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ) \
	SockServ.$(OBJ)

UISOBJ = CfgFile.$(OBJ) \
	LogFile.$(OBJ) \
	SockOp.$(OBJ) \
	SockServ.$(OBJ) \
	UIServ.$(OBJ)

SPECTOBJ = CfgFile.$(OBJ) \
	Messages.$(OBJ) \
	RemoveNoise.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ) \
	Spectrum.$(OBJ)

DIROBJ = ArrayBase.$(OBJ) \
	ArrayDipole.$(OBJ) \
	BeamDipole.$(OBJ) \
	CfgFile.$(OBJ) \
	CorrDipole.$(OBJ) \
	Direction.$(OBJ) \
	Messages.$(OBJ) \
	RemoveNoise.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ)

DIR2OBJ = CfgFile.$(OBJ) \
	Direction2.$(OBJ) \
	Messages.$(OBJ) \
	RemoveNoise.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ) \
	SpectDir.$(OBJ) \
	SpectDirDipole.$(OBJ) \
	SpectDirLine.$(OBJ)

DIR3OBJ = CfgFile.$(OBJ) \
	Direction3.$(OBJ) \
	Messages.$(OBJ) \
	RemoveNoise.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ) \
	SpectDir2.$(OBJ) \
	SpectDirDipole2.$(OBJ) \
	SpectDirLine2.$(OBJ)

LDSERVOBJ = CfgFile.$(OBJ) \
	LofarDemon.$(OBJ) \
	Messages.$(OBJ) \
	RemoveNoise.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ)

BASERVOBJ = ArraySensor.$(OBJ) \
	CfgFile.$(OBJ) \
	BeamAudio.$(OBJ) \
	FreqBeamDipole.$(OBJ) \
	FreqBeamLine.$(OBJ) \
	Messages.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ)

LEVSERVOBJ = CfgFile.$(OBJ) \
	Level.$(OBJ) \
	Messages.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ)

LOCATEOBJ = CfgFile.$(OBJ) \
	Cluster.$(OBJ) \
	Locate.$(OBJ) \
	LocateSensor.$(OBJ) \
	LocateSystem.$(OBJ) \
	LogFile.$(OBJ) \
	Messages.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ) \
	SockServ.$(OBJ)

GUISPECTOBJ = CfgFile.$(OBJ) \
	FrameBuf.$(OBJ) \
	GtkUtils.$(OBJ) \
	GUISpect.$(OBJ) \
	Messages.$(OBJ) \
	Palette.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ)

GUIDIROBJ = CfgFile.$(OBJ) \
	FrameBuf.$(OBJ) \
	GtkUtils.$(OBJ) \
	GUIDir.$(OBJ) \
	Messages.$(OBJ) \
	Palette.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ)

GUILOFAROBJ = CfgFile.$(OBJ) \
	FrameBuf.$(OBJ) \
	GtkUtils.$(OBJ) \
	GUILofar.$(OBJ) \
	Messages.$(OBJ) \
	Palette.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ)

GUILEVELOBJ = CfgFile.$(OBJ) \
	GtkUtils.$(OBJ) \
	GUILevel.$(OBJ) \
	Messages.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ)

GUILOCATEOBJ = CfgFile.$(OBJ) \
	GtkUtils.$(OBJ) \
	GUILocate.$(OBJ) \
	Messages.$(OBJ) \
	Palette.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ)

GUITRANSOBJ = CfgFile.$(OBJ) \
	GtkUtils.$(OBJ) \
	GUITrans.$(OBJ)

SUIOBJ = Audio.$(OBJ) \
	AudioA.$(OBJ) \
	AudioA2.$(OBJ) \
	CfgFile.$(OBJ) \
	GtkUtils.$(OBJ) \
	Messages.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ) \
	SoundUI.$(OBJ)

BAUIOBJ = Audio.$(OBJ) \
	Audio3D.$(OBJ) \
	AudioA.$(OBJ) \
	AudioA2.$(OBJ) \
	BeamAudioUI.$(OBJ) \
	CfgFile.$(OBJ) \
	GtkUtils.$(OBJ) \
	Messages.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ)

SPOBJ = CfgFile.$(OBJ) \
	LogFile.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ) \
	SockServ.$(OBJ) \
	SoundProxy.$(OBJ)

CONVPOBJ = ConvPic.$(OBJ)

TESTSSOBJ = Audio.$(OBJ) \
	Messages.$(OBJ) \
	SockClie.$(OBJ) \
	SockOp.$(OBJ) \
	TestSnd.$(OBJ)

#
# rules
#

default: all

.cc.o: $(SRCS)
	$(CXX) $(CXXFLAGS) $(DEFS) $(INCS) -c $<

BeamSrv2.$(OBJ): BeamSrv2.$(CCSRC)
	$(MPICXX) $(CXXFLAGS) $(DEFS) $(INCS) -c $<

Cluster.$(OBJ): Cluster.$(CCSRC)
	$(MPICXX) $(CXXFLAGS) $(DEFS) $(INCS) -c $<

Locate.$(OBJ): Locate.$(CCSRC)
	$(MPICXX) $(CXXFLAGS) $(DEFS) $(INCS) -c $<

all: $(BINS)

clean:
	rm -f *.$(OBJ) *.lo *.bak *.~* *~ core*

cleanall:
	rm -f $(BINS)
	rm -f hasas.dep *.$(OBJ) *.lo *.bak *.~* *~ core*
	rm -rf .libs

#

soundsrv: $(SNDSERVOBJ)
	$(CXX) $(LDFLAGS) -o soundsrv $(SNDSERVOBJ) $(LIBS) $(FFTWLIBS) $(PTHLIBS)

soundsrv2: $(SNDSERV2OBJ)
	$(CXX) $(LDFLAGS) -o soundsrv2 $(SNDSERV2OBJ) $(LIBS) $(FFTWLIBS) $(FLACLIBS)

soundsrva: $(SNDSERVAOBJ)
	$(CXX) $(LDFLAGS) -o soundsrva $(SNDSERVAOBJ) $(LIBS) $(FFTWLIBS) $(FLACLIBS) -lasound

comedisrv: $(COMSERVOBJ)
	$(CXX) $(LDFLAGS) -o comedisrv $(COMSERVOBJ) $(LIBS) $(FFTWLIBS) $(FLACLIBS) -lcomedi

streamdist: $(SDOBJ)
	$(CXX) $(LDFLAGS) -o streamdist $(SDOBJ) $(LIBS) $(FLACLIBS)

savesrv: $(SAVSOBJ)
	$(CXX) $(LDFLAGS) -o savesrv $(SAVSOBJ) $(LIBS) $(FFTWLIBS) $(SNDFLIBS) $(FLACLIBS)

filesrv: $(FILSOBJ)
	$(CXX) $(LDFLAGS) -o filesrv $(FILSOBJ) $(LIBST) $(FFTWLIBS) $(SNDFLIBS) $(FLACLIBS) `gtk-config --libs gthread`

beamsrv: $(BEAMSOBJ)
	$(CXX) $(LDFLAGS) -o beamsrv $(BEAMSOBJ) $(LIBST) $(FFTWLIBS)

beamsrv2: $(BEAMS2OBJ)
	$(MPICXX) $(LDFLAGS) -o beamsrv2 $(BEAMS2OBJ) $(LIBST) $(FFTWLIBS)

uiserv: $(UISOBJ)
	$(CXX) $(LDFLAGS) -o uiserv $(UISOBJ) $(LIBS)

spectrum: $(SPECTOBJ)
	$(CXX) $(LDFLAGS) -o spectrum $(SPECTOBJ) $(LIBS) $(FFTWLIBS) $(GSLLIBS)

direction: $(DIROBJ)
	$(CXX) $(LDFLAGS) -o direction $(DIROBJ) $(LIBS) $(FFTWLIBS) $(GSLLIBS)

direction2: $(DIR2OBJ)
	$(CXX) $(LDFLAGS) -o direction2 $(DIR2OBJ) $(LIBS) $(FFTWLIBS) $(GSLLIBS)

direction3: $(DIR3OBJ)
	$(CXX) $(LDFLAGS) -o direction3 $(DIR3OBJ) $(LIBS) $(FFTWLIBS) $(GSLLIBS)

lofardemon: $(LDSERVOBJ)
	$(CXX) $(LDFLAGS) -o lofardemon $(LDSERVOBJ) $(LIBS) $(FFTWLIBS) $(GSLLIBS)

beamaudio: $(BASERVOBJ)
	$(CXX) $(LDFLAGS) -o beamaudio $(BASERVOBJ) $(LIBS) $(FFTWLIBS)

level: $(LEVSERVOBJ)
	$(CXX) $(LDFLAGS) -o level $(LEVSERVOBJ) $(LIBS) $(FFTWLIBS)

locate: $(LOCATEOBJ)
	$(MPICXX) $(LDFLAGS) -o locate $(LOCATEOBJ) $(LIBS) $(FFTWLIBS)

guispect: $(GUISPECTOBJ)
	$(CXX) $(LDFLAGS) -o guispect $(GUISPECTOBJ) $(LIBS) `gtk-config --libs` $(TIFFLIBS) $(FTLIBS)

guidir: $(GUIDIROBJ)
	$(CXX) $(LDFLAGS) -o guidir $(GUIDIROBJ) $(LIBS) `gtk-config --libs` $(TIFFLIBS) $(FTLIBS)

guilofar: $(GUILOFAROBJ)
	$(CXX) $(LDFLAGS) -o guilofar $(GUILOFAROBJ) $(LIBS) `gtk-config --libs` $(TIFFLIBS) $(FTLIBS)

guilevel: $(GUILEVELOBJ)
	$(CXX) $(LDFLAGS) -o guilevel $(GUILEVELOBJ) $(LIBS) `gtk-config --libs`

guilocate: $(GUILOCATEOBJ)
	$(CXX) $(LDFLAGS) -o guilocate $(GUILOCATEOBJ) $(LIBS) `gtk-config --libs`

guitrans: $(GUITRANSOBJ)
	$(CXX) $(LDFLAGS) -o guitrans $(GUITRANSOBJ) $(LIBS) `gtk-config --libs gthread`

soundui: $(SUIOBJ)
	$(CXX) $(LDFLAGS) -o soundui $(SUIOBJ) $(LIBST) -lasound `gtk-config --libs gthread`

beamaudioui: $(BAUIOBJ)
	$(CXX) $(LDFLAGS) -o beamaudioui $(BAUIOBJ) $(LIBST) -lasound `gtk-config --libs gthread`

soundproxy: $(SPOBJ)
	$(CXX) $(LDFLAGS) -o soundproxy $(SPOBJ) $(LIBS)

convpic: $(CONVPOBJ)
	$(CXX) $(LDFLAGS) -o convpic $(CONVPOBJ) $(LIBS) $(MAGICKLIBS)

testsnd: $(TESTSSOBJ)
	$(CXX) $(LDFLAGS) -o testsnd $(TESTSSOBJ) $(LIBS) $(FFTWLIBS)

libhasas.so: XMMSOut.$(CCSRC) SockOp.$(CCSRC) SockServ.$(CCSRC)
	$(LIBTOOL) --mode=compile $(CXX) $(CXXFLAGS) $(DEFS) -I/usr/local/include `xmms-config --cflags` `gtk-config --cflags gthread` -c XMMSOut.$(CCSRC)
	$(LIBTOOL) --mode=compile $(CXX) $(CXXFLAGS) $(DEFS) -I/usr/local/include -c SockOp.$(CCSRC)
	$(LIBTOOL) --mode=compile $(CXX) $(CXXFLAGS) $(DEFS) -I/usr/local/include -c SockServ.$(CCSRC)
	$(LIBTOOL) --mode=link $(CXX) $(LDFLAGS) -export-dynamic -avoid-version -rpath `xmms-config --output-plugin-dir` -o libhasas.la XMMSOut.lo SockOp.lo SockServ.lo -lDynThreads -ldsp `xmms-config --libs` `gtk-config --libs gthread`

libhasas.so-install:
	$(LIBTOOL) --mode=install install libhasas.la `xmms-config --output-plugin-dir`

hasas.dep: $(SRCS)
	$(CXX) $(DEFS) $(INCS) $(MPIINCS) -MM $(SRCS) >hasas.dep

strip: $(BINS)
	strip $(BINS)

#
# dependencies
#

include hasas.dep

