# MinG (Windows) build configuration
CXX=g++
OS_CFLAGS=-I/mingw/include $(shell $(BUILD_ROOT)/bin/sdl-config --cflags) $(shell $(BUILD_ROOT)/bin/curl-config --cflags) $(shell $(BUILD_ROOT)/bin/libpng-config --cflags)
OS_LDFLAGS=-mthreads  -Wl,--subsystem,windows -mwindows -Wl,-Bsymbolic -static
OS_LIBS=$(BUILD_ROOT)/lib/libSDL.a $(BUILD_ROOT)/lib/libSDL_image.a $(BUILD_ROOT)/lib/libpng.a $(BUILD_ROOT)/lib/libSDL_net.a $(BUILD_ROOT)/lib/libSDL_ttf.a $(BUILD_ROOT)/lib/libfreetype.a $(BUILD_ROOT)/lib/libcrypto.a $(BUILD_ROOT)/lib/libavformat.a $(BUILD_ROOT)/lib/libavcodec.a $(BUILD_ROOT)/lib/libswscale.a $(BUILD_ROOT)/lib/libavutil.a $(BUILD_ROOT)/lib/libfaad.a $(BUILD_ROOT)/lib/libcurl.a $(BUILD_ROOT)/lib/libz.a /mingw/lib/libdxguid.a -lwsock32 -lgdi32 -lwinmm -lmingw32
DEFS=-DCURL_STATICLIB -D_WIN32 -DORP_CLOCK_DEBUG
TARGET=orp.exe
PKG_VERSION="ORP-$(VER_MAJOR).$(VER_MINOR)-$(VER_RELEASE)-W32"

release: all
	@rm -rf $(PKG_VERSION) $(PKG_VERSION).zip
	@mkdir $(PKG_VERSION)
	@cp README $(PKG_VERSION)/README.txt
	@cp README.zh $(PKG_VERSION)/README-zh.txt
	@cp -r psp/ORP_Export $(PKG_VERSION)
	@cp orp $(PKG_VERSION)
	@cp gui/orpui $(PKG_VERSION)
	@cp gui/icon.ico $(PKG_VERSION)
	@cp keys/keys.orp $(PKG_VERSION)
	@cp /mingw/bin/mingwm10.dll $(PKG_VERSION)
	@find $(PKG_VERSION) -type d -name '.svn' -print0 | xargs -0 rm -rf
