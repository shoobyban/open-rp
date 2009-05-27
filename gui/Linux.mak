CXX=g++
INCLUDE=-I.
CFLAGS=-g -pipe $(INCLUDE) $(shell wx-config --cflags) $(shell sdl-config --cflags)
CXXFLAGS=$(CFLAGS)
LDFLAGS=
LIBS=$(shell wx-config --libs)
DEFS=-DORP_UI
TARGET=orpui
