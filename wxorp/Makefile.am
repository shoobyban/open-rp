AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS = ${CXXFLAGS} -I. -I..

#EXTRA_DIST = csmain.h doc/clearsync.conf doc/clearsyncd.init

bin_PROGRAMS = wxorp

wxorp_SOURCES = wxorp.cpp ../config.cpp ../keybind.cpp
wxorp_CXXFLAGS = ${AM_CXXFLAGS} -D__WXWIDGETS__

