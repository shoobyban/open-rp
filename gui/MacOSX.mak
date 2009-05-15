CXX=g++
INCLUDE=-I.
CFLAGS=-O2 -pipe $(INCLUDE) $(shell wx-config --cflags)
CXXFLAGS=$(CFLAGS)
LDFLAGS=
LIBS=$(shell wx-config --libs)
DEFS=
TARGET=orpui

install: $(TARGET)
	@cp -v ../orp "Open Remote Play.app/Contents/MacOS/orp"
	@cp -v orpui "Open Remote Play.app/Contents/MacOS/orpui"

# vi: ts=4
