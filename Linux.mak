# Generic Linux build configuration
CXX=g++
INCLUDE=-I.
CFLAGS=-O2 -pipe $(INCLUDE) $(shell sdl-config --cflags) $(shell curl-config --cflags)
CXXFLAGS=$(CFLAGS)
LDFLAGS=
LIBDIR=/usr/lib
LIB_FFMPEG=/usr/lib
LIBS_SHARED=-lcrypto -lpthread -lz $(shell sdl-config --libs) $(shell curl-config --libs) -lSDL_net -lSDL_image -lavcodec -lavutil -lavformat -lswscale
LIBS_SHARED_EXTRA=
LIBS_STATIC=
LIBS=$(LIBS_STATIC) $(LIBS_SHARED) $(LIBS_SHARED_EXTRA)
DEFS=
TARGET=orp

release: all
	@rm -rf ORP-1.1-BETA-Linux*
	@mkdir ORP-1.1-BETA-Linux
	@cp -v README ORP-1.1-BETA-Linux
	@cp -rv psp/ORP_Export ORP-1.1-BETA-Linux
	@cp -v orp ORP-1.1-BETA-Linux
	@cp -v gui/orpui ORP-1.1-BETA-Linux
	@cp keys/keys.orp ORP-1.1-BETA-Linux
	@find ORP-1.1-BETA-Linux -type d -name '.svn' -print0 | xargs -0 rm -rf
	@zip -r ORP-1.1-BETA-Linux.zip ORP-1.1-BETA-Linux
