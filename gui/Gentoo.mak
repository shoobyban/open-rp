CXX=g++
INCLUDE=-I.
CFLAGS=-g -pipe $(INCLUDE) $(shell wx-config --cflags)
CXXFLAGS=$(CFLAGS)
LDFLAGS=
LIBS=$(shell wx-config --libs)
DEFS=
TARGET=orpui
