AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I ../m4
AM_CXXFLAGS = ${CXXFLAGS} -I. -I..

EXTRA_DIST = base64.h font.h images.h keybind.h launch.h orp-conf.h orp.h yuv.h

bin_PROGRAMS = keys

keys_SOURCES = keys.cpp ../config.cpp
keys_CXXFLAGS = ${AM_CXXFLAGS}

