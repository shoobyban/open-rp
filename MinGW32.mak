# MinG (Windows) build configuration
CXX=g++
OS_CFLAGS=-I/mingw/include
OS_LDFLAGS=-mthreads  -Wl,--subsystem,windows -mwindows -Wl,-Bsymbolic -static
OS_LIBS=$(BUILD_ROOT)/lib/libz.a /mingw/lib/libdxguid.a -lwsock32 -lgdi32 -lwinmm -lmingw32
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
