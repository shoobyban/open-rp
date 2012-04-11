AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = ${CXXFLAGS} -I. -I..

EXTRA_DIST = images.h wxorp.h

bin_PROGRAMS = wxorp

wxorp_SOURCES = wxorp.cpp ../config.cpp ../keybind.cpp
wxorp_CXXFLAGS = ${AM_CXXFLAGS} -D__WXWIDGETS__

all-local:
	@ln -vfs ../orp$(EXEEXT) .
	@cp -v Info.plist "./Open Remote Play.app/Contents/"
	@cp -v wxorp$(EXEEXT) "Open Remote Play.app/Contents/MacOS/wxorp$(EXEEXT)"
