AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I ../m4

AM_CXXFLAGS = ${CXXFLAGS} -I. -I..

#EXTRA_DIST =

noinst_PROGRAMS = keys

keys_SOURCES = keys.cpp ../config.cpp
keys_CXXFLAGS = ${AM_CXXFLAGS}

keysextradir = $(datadir)/@PACKAGE@
keysextra_DATA = keys.orp

all-local:
	@./keys$(EXEEXT)

