# MinG (Windows) build configuration
CXX=g++
LD=
INCLUDE=-I. -I/mingw/include $(sdl-config --cflags)
CXXFLAGS=$(CFLAGS) -g -mthreads -I/mingw/include/SDL -D_GNU_SOURCE=1 -Dmain=SDL_main
LIBDIR=/mingw/lib
#LDFLAGS=-L$(LIBDIR) -mthreads
LDFLAGS=-L$(LIBDIR) -mthreads  -Wl,--subsystem,windows -mwindows
LIBS=-mwindows -Wl,-Bsymbolic $(LIBDIR)/libSDLmain.a $(LIBDIR)/libSDL.a $(LIBDIR)/libSDL_image.a $(LIBDIR)/libpng.a $(LIBDIR)/libz.a $(LIBDIR)/libSDL_net.a $(LIBDIR)/libcrypto.a $(LIBDIR)/libavformat.a $(LIBDIR)/libavcodec.a $(LIBDIR)/libswscale.a $(LIBDIR)/libavutil.a $(LIBDIR)/libfaad.a $(LIBDIR)/libcurl.a $(LIBDIR)/libdxguid.a $(LIBDIR)/libz.a -lwsock32 -lgdi32 -lwinmm -lmingw32
DEFS=-DCURL_STATICLIB -D_WIN32
TARGET=orp.exe
PKG_VERSION="ORP-$(VER_MAJOR).$(VER_MINOR)-$(VER_RELEASE)-W32"

release: $(TARGET)
	@rm -rf $(PKG_VERSION) $(PKG_VERSION).zip
	@mkdir $(PKG_VERSION)
	@cp -v README $(PKG_VERSION)/README.txt
	@cp -rv psp/ORP_Export $(PKG_VERSION) | grep -v '.svn'
	@cp -v orp $(PKG_VERSION)
	@cp -v gui/orpui $(PKG_VERSION)
	@cp -v gui/icon.ico $(PKG_VERSION)
	@cp -v keys/keys.orp $(PKG_VERSION)
	@cp -v /mingw/bin/mingwm10.dll $(PKG_VERSION)
	@find $(PKG_VERSION) -type d -name '.svn' -print0 | xargs -0 rm -rf
