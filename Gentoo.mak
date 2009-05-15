# Gentoo Linux build configuration
CXX=g++
INCLUDE=-I. -I./ffmpeg/include
CFLAGS=-O2 -s -pipe $(INCLUDE) $(shell sdl-config --cflags) $(shell curl-config --cflags)
CXXFLAGS=$(CFLAGS)
LDFLAGS=
LIBDIR=/usr/lib
LIB_FFMPEG=/usr/lib
LIBS_SHARED=-lcrypto -lpthread -lz
LIBS_SHARED_EXTRA=-lgnutls -lrt
#LIBS_SHARED_EXTRA=-lxvidcore -lx264 -lvorbis -lvorbisenc -lmp3lame -lfaac -lfaad -lgnutls -lrt
LIBS_STATIC=$(LIBDIR)/libpng.a $(LIBDIR)/libSDL.a $(LIBDIR)/libSDL_image.a $(LIBDIR)/libSDL_net.a $(LIBDIR)/libcurl.a $(LIB_FFMPEG)/libavcodec.a $(LIB_FFMPEG)/libavutil.a $(LIB_FFMPEG)/libavformat.a $(LIB_FFMPEG)/libswscale.a $(LIBDIR)/libfaad.a
LIBS=$(LIBS_STATIC) $(LIBS_SHARED) $(LIBS_SHARED_EXTRA)
DEFS=

release: all
	@rm -rf ORP-1.0-BETA-Gentoo*
	@mkdir ORP-1.0-BETA-Gentoo
	@cp -v README ORP-1.0-BETA-Gentoo
	@cp -rv psp/ORP_Export ORP-1.0-BETA-Gentoo
	@cp -v orp ORP-1.0-BETA-Gentoo
	@cp -v gui/orpui ORP-1.0-BETA-Gentoo
	@cp keys/keys.orp ORP-1.0-BETA-Gentoo
	@zip -r ORP-1.0-BETA-Gentoo.zip ORP-1.0-BETA-Gentoo
