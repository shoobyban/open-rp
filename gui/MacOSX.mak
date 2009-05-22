CXX=g++
INCLUDE=-I.
CFLAGS=-O2 -pipe $(INCLUDE) $(shell wx-config --cflags)
CXXFLAGS=$(CFLAGS)
LDFLAGS=
LIBS=$(shell wx-config --libs)
DEFS=
TARGET=orpui
VER_PLIST=$(VER_MAJOR).$(VER_MINOR)

install: $(TARGET)
	sed -e "s/%VERSION%/$(VER_PLIST)/" Info.plist.in > "Open Remote Play.app/Contents/Info.plist"
	@cp -v ../orp "Open Remote Play.app/Contents/MacOS/orp"
	@cp -v orpui "Open Remote Play.app/Contents/MacOS/orpui"

# vi: ts=4
