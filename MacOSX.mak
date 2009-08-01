CXX=g++
OS_CFLAGS=-g -pipe $(shell sdl-config --cflags) $(shell curl-config --cflags)
OS_LDFLAGS=-Wl,-framework,OpenGL -Wl,-framework,Cocoa -Wl,-framework,QuickTime -Wl,-framework,ApplicationServices -Wl,-framework,Carbon -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,IOKit
LIBDIR=/usr/local/lib
LIBS_SHARED=-lcrypto -lpthread -lz
LIBS_STATIC=$(LIBDIR)/libSDL.a $(LIBDIR)/libSDLmain.a $(LIBDIR)/libSDL_image.a $(LIBDIR)/libSDL_net.a $(LIBDIR)/libSDL_ttf.a $(LIBDIR)/libfreetype.a $(LIBDIR)/libcurl.a $(LIBDIR)/libfaad.a $(LIBDIR)/libavcodec.a $(LIBDIR)/libavformat.a $(LIBDIR)/libavutil.a $(LIBDIR)/libpng.a $(LIBDIR)/libswscale.a
OS_LIBS=$(LIBS_STATIC) $(LIBS_SHARED) $(LIBS_SHARED_EXTRA)
#LIBS=$(shell sdl-config --libs) $(shell curl-config --libs) -lSDL_image -lSDL_net -lcrypto -lavcodec -lavutil -lavformat -lswscale -lfaad
DEFS=-D_MACOSX_ -DORP_CLOCK_DEBUG
TARGET=orp
PKG_VERSION="ORP-$(VER_MAJOR).$(VER_MINOR)-$(VER_RELEASE)-OSX"

release: all
	@rm -rf $(PKG_VERSION) $(PKG_VERSION).zip
	@mkdir $(PKG_VERSION)
	@cp README $(PKG_VERSION)/README.txt
	@cp README.zh $(PKG_VERSION)/README-zh.txt
	@cp -r psp/ORP_Export $(PKG_VERSION)
	@cp keys/keys.orp $(PKG_VERSION)
	@cp -r "gui/Open Remote Play.app" $(PKG_VERSION)
	@find $(PKG_VERSION) -type d -name '.svn' -print0 | xargs -0 rm -rf
	@[ -x $(which zip) ] || exit 0
	@zip -r $(PKG_VERSION).zip $(PKG_VERSION)
