CXX=g++
INCLUDE=-I.
CFLAGS=-O2 -pipe $(INCLUDE) $(shell sdl-config --cflags) $(shell curl-config --cflags)
CXXFLAGS=$(CFLAGS)
LDFLAGS=-Wl,-framework,OpenGL -Wl,-framework,Cocoa -Wl,-framework,QuickTime -Wl,-framework,ApplicationServices -Wl,-framework,Carbon -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,IOKit
LIBDIR=/usr/local/lib
LIBS_SHARED=-lcrypto -lpthread -lz
LIBS_STATIC=$(LIBDIR)/libSDL.a $(LIBDIR)/libSDLmain.a $(LIBDIR)/libSDL_image.a $(LIBDIR)/libSDL_net.a $(LIBDIR)/libcurl.a $(LIBDIR)/libfaad.a $(LIBDIR)/libavcodec.a $(LIBDIR)/libavformat.a $(LIBDIR)/libavutil.a $(LIBDIR)/libpng.a $(LIBDIR)/libswscale.a
LIBS=$(LIBS_STATIC) $(LIBS_SHARED) $(LIBS_SHARED_EXTRA)
#LIBS=$(shell sdl-config --libs) $(shell curl-config --libs) -lSDL_image -lSDL_net -lcrypto -lavcodec -lavutil -lavformat -lswscale -lfaad
DEFS=-D_MACOSX_ -DORP_CLOCK_DEBUG
TARGET=orp
PKG_VERSION="ORP-$(VER_MAJOR).$(VER_MINOR)-$(VER_RELEASE)-OSX"

release: $(TARGET)
	@rm -rf $(PKG_VERSION) $(PKG_VERSION).zip
	@mkdir $(PKG_VERSION)
	@cp -v README $(PKG_VERSION)
	@cp -rv psp/ORP_Export $(PKG_VERSION) | grep -v '.svn'
	@cp keys/keys.orp $(PKG_VERSION)
	@cp -rv "gui/Open Remote Play.app" $(PKG_VERSION)
	@find $(PKG_VERSION) -type d -name '.svn' -print0 | xargs -0 rm -rf
	@zip -r $(PKG_VERSION).zip $(PKG_VERSION)
