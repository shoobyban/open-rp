# Generic Linux build configuration
CXX=g++
OS_CFLAGS=-g -pipe
OS_LDFLAGS=
OS_LIBS=-ldl
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
