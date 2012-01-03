AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I ../m4

AM_CXXFLAGS = ${CXXFLAGS} -I. -I..

#EXTRA_DIST =

bin_PROGRAMS = config-debug

config_debug_SOURCES = config-debug.cpp ../config.cpp
config_debug_CXXFLAGS = ${AM_CXXFLAGS} -DORP_CONFIG_DEBUG=1

