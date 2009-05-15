CXX=g++
INCLUDE=-I.
CXXFLAGS=$(CFLAGS) $(shell wx-config --cflags)
LIBS=$(shell wx-config --libs)
DEFS=
TARGET=orpui.exe

