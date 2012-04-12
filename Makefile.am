AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = ${CXXFLAGS} -I.

EXTRA_DIST = base64.h font.h images.h keybind.h launch.h launch.wav orp-conf.h orp.h psp/orp.c psp/ICON0.PNG psp/Makefile psp/ORP_Exporter/EBOOT.PBP yuv.h README.zh

SUBDIRS = . ${ORP_SUBDIRS}

bin_PROGRAMS = orp

orp_SOURCES = base64.cpp config.cpp keybind.cpp main.cpp orp.cpp yuv.cpp
orp_CXXFLAGS = ${AM_CXXFLAGS} -D__STDC_CONSTANT_MACROS -DORP_CLOCK_DEBUG -DORP_SYNC_TO_MASTER

docdir = $(datadir)/doc/@PACKAGE@
doc_DATA = AUTHORS README README.zh

pspextradir = $(datadir)/@PACKAGE@/ORP_Exporter
pspextra_DATA = psp/ORP_Exporter/EBOOT.PBP

launch.h: launch.wav
	@xxd -i launch.wav > launch.h

