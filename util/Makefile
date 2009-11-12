-include ../Config.mak
BUILD_ROOT=../packages.d/work/root
INCLUDE=-I. -I..
CFLAGS=-pipe -I. -I.. -I$(BUILD_ROOT)/include $(shell $(BUILD_ROOT)/bin/sdl-config --cflags) $(shell $(BUILD_ROOT)/bin/curl-config --cflags) $(shell $(BUILD_ROOT)/bin/libpng-config --cflags) $(OS_CFLAGS)
CXXFLAGS=$(CFLAGS)
LIBS=$(BUILD_ROOT)/lib/libSDL.a $(BUILD_ROOT)/lib/libSDL_image.a $(BUILD_ROOT)/lib/libpng.a $(BUILD_ROOT)/lib/libSDL_net.a $(BUILD_ROOT)/lib/libSDL_ttf.a $(BUILD_ROOT)/lib/libfreetype.a $(BUILD_ROOT)/lib/libcrypto.a $(BUILD_ROOT)/lib/libavformat.a $(BUILD_ROOT)/lib/libavcodec.a $(BUILD_ROOT)/lib/libswscale.a $(BUILD_ROOT)/lib/libavutil.a $(BUILD_ROOT)/lib/libfaad.a $(BUILD_ROOT)/lib/libcurl.a $(OS_LIBS)
LDFLAGS=-L$(BUILD_ROOT)/lib $(OS_LDFLAGS)

all:
	$(MAKE) config-debug export-debug stream-debug

config-debug: config-debug.cpp
	$(CXX) $(CFLAGS) -DORP_CONFIG_DEBUG config-debug.cpp ../config.cpp -o config-debug $(LDFLAGS)

export-debug: export-debug.cpp
	$(CXX) $(CFLAGS) export-debug.cpp ../config.cpp -o export-debug $(LDFLAGS)

stream-debug: stream-debug.cpp
	$(CXX) $(CFLAGS) stream-debug.cpp ../orp.o ../yuv.o ../config.o ../base64.o -o stream-debug $(LDFLAGS) $(LIBS)

clean:
	@rm -f config-debug export-debug stream-debug *.orp
