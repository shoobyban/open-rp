# Custom build configuration
CXX=g++
INCLUDE=-I. -I./ffmpeg/include
CFLAGS=-g -pipe $(INCLUDE) $(shell sdl-config --cflags) $(shell curl-config --cflags)
CXXFLAGS=$(CFLAGS)
LDFLAGS=
LIBDIR=/usr/lib
LIB_FFMPEG=./ffmpeg/lib
LIBS_SHARED=-lcrypto -lpthread -lz
#LIBS_SHARED_EXTRA=-lgnutls -lrt
LIBS_SHARED_EXTRA=-lxvidcore -lx264 -lvorbis -lvorbisenc -lmp3lame -lfaac -lfaad -lgnutls -lrt
LIBS_STATIC=$(LIBDIR)/libpng.a $(LIBDIR)/libSDL.a $(LIBDIR)/libSDL_image.a $(LIBDIR)/libSDL_net.a $(LIBDIR)/libcurl.a $(LIB_FFMPEG)/libavcodec.a $(LIB_FFMPEG)/libavutil.a $(LIB_FFMPEG)/libavformat.a $(LIB_FFMPEG)/libswscale.a $(LIBDIR)/libfaad.a
LIBS=$(LIBS_STATIC) $(LIBS_SHARED) $(LIBS_SHARED_EXTRA)
DEFS=-DORP_DUMP_VIDEO_HEADER
TARGET=orp
PKG_VERSION="ORP-$(VER_MAJOR).$(VER_MINOR)-$(VER_RELEASE)-Custom"

release: $(TARGET)
	@rm -rf $(PKG_VERSION) $(PKG_VERSION).zip
	@mkdir $(PKG_VERSION)
	@cp -v README $(PKG_VERSION)
	@cp -rv psp/ORP_Export $(PKG_VERSION) | grep -v '.svn'
	@cp -v orp $(PKG_VERSION)
	@cp -v gui/orpui $(PKG_VERSION)
	@cp keys/keys.orp $(PKG_VERSION)
	@find $(PKG_VERSION) -type d -name '.svn' -print0 | xargs -0 rm -rf
	@zip -r $(PKG_VERSION).zip $(PKG_VERSION)
