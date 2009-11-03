CXX=g++
OS_CFLAGS=
OS_LDFLAGS=
OS_LIBS=$(BUILD_ROOT)/lib/libwx_gtk2u-2.8.a $(BUILD_ROOT)/lib/libwxregexu-2.8.a $(shell $(BUILD_ROOT)/bin/wx-config --libs)
DEFS=
TARGET=orpui
