AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS = ${CXXFLAGS} -I.

#EXTRA_DIST = csmain.h doc/clearsync.conf doc/clearsyncd.init

bin_PROGRAMS = orp

orp_SOURCES = base64.cpp config.cpp keybind.cpp main.cpp orp.cpp yuv.cpp
orp_CXXFLAGS = ${AM_CXXFLAGS} -DORP_CLOCK_DEBUG -DORP_SYNC_TO_MASTER

