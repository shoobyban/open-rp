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
PKG_VERSION="ORP-$(VER_MAJOR).$(VER_MINOR)-$(VER_RELEASE)-Linux"

release: all
	@rm -rf $(PKG_VERSION) $(PKG_VERSION).zip
	@mkdir $(PKG_VERSION)
	@cp -v README $(PKG_VERSION)
	@cp -rv psp/ORP_Export $(PKG_VERSION)
	@cp -v orp $(PKG_VERSION)
	@cp -v gui/orpui $(PKG_VERSION)
	@cp keys/keys.orp $(PKG_VERSION)
	@find $(PKG_VERSION) -type d -name '.svn' -print0 | xargs -0 rm -rf
	@zip -r $(PKG_VERSION).zip $(PKG_VERSION)
