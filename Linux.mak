# Generic Linux build configuration
CXX=g++
OS_CFLAGS=$(shell $(BUILD_ROOT)/bin/sdl-config --cflags) $(shell $(BUILD_ROOT)/bin/curl-config --cflags) $(shell $(BUILD_ROOT)/bin/libpng-config --cflags)
OS_LDFLAGS=
OS_LIBS=$(shell $(BUILD_ROOT)/bin/sdl-config --static-libs) $(BUILD_ROOT)/lib/libSDL_image.a $(BUILD_ROOT)/lib/libpng.a $(BUILD_ROOT)/lib/libSDL_net.a $(BUILD_ROOT)/lib/libSDL_ttf.a $(BUILD_ROOT)/lib/libfreetype.a $(BUILD_ROOT)/lib/libcrypto.a $(BUILD_ROOT)/lib/libavformat.a $(BUILD_ROOT)/lib/libavcodec.a $(BUILD_ROOT)/lib/libswscale.a $(BUILD_ROOT)/lib/libavutil.a $(BUILD_ROOT)/lib/libfaad.a $(BUILD_ROOT)/lib/libcurl.a -ldl -lz -lm -lpthread -lrt
DEFS=-DORP_CLOCK_DEBUG -DORP_SYNC_TO_MASTER
TARGET=orp
PKG_VERSION="ORP-$(VER_MAJOR).$(VER_MINOR)-$(VER_RELEASE)-Linux"

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
	@find $(PKG_VERSION) -type d -name '.svn' -print0 | xargs -0 rm -rf
	@[ -x $(which zip) ] || exit 0
	@[ -x $(which strip) ] && strip $(PKG_VERSION)/orp $(PKG_VERSION)/orpui
	@zip -r $(PKG_VERSION).zip $(PKG_VERSION)
