-include Config.mak
BUILD_ROOT=./packages.d/work/root
SOURCE=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCE))
DEPS=$(patsubst %.o,%.d,$(OBJECTS))
VERSION="$(VER_MAJOR).$(VER_MINOR) $(VER_RELEASE)"
CFLAGS=-pipe -I. -I$(BUILD_ROOT)/include $(shell $(BUILD_ROOT)/bin/sdl-config --cflags) $(shell $(BUILD_ROOT)/bin/curl-config --cflags) $(shell $(BUILD_ROOT)/bin/libpng-config --cflags) $(OS_CFLAGS)
CXXFLAGS=$(CFLAGS)
LIBS=$(BUILD_ROOT)/lib/libSDL.a $(BUILD_ROOT)/lib/libSDL_image.a $(BUILD_ROOT)/lib/libpng.a $(BUILD_ROOT)/lib/libSDL_net.a $(BUILD_ROOT)/lib/libSDL_ttf.a $(BUILD_ROOT)/lib/libfreetype.a $(BUILD_ROOT)/lib/libcrypto.a $(BUILD_ROOT)/lib/libavformat.a $(BUILD_ROOT)/lib/libavcodec.a $(BUILD_ROOT)/lib/libswscale.a $(BUILD_ROOT)/lib/libavutil.a $(BUILD_ROOT)/lib/libfaad.a $(BUILD_ROOT)/lib/libcurl.a $(OS_LIBS)
LDFLAGS=-L$(BUILD_ROOT)/lib $(OS_LDFLAGS)

all:
	@echo "Host OS: $(OS)"
	@echo "Compiler: $(CXX) $(CXXFLAGS)"
	@echo "Linker: $(LD) $(LDFLAGS)"
	@echo "Defines: $(DEFS)"
	@echo "Libraries: $(LIBS)"
	$(MAKE) -C packages.d
	$(MAKE) MAKEFLAGS= deps
	$(MAKE) -C gui
	$(MAKE) $(TARGET)

gui::
	$(MAKE) -C gui

keys::
	$(MAKE) -C keys

psp::
	$(MAKE) -C psp

deps: $(wildcard *.cpp)
	@echo "[D] $^"
	@$(CXX) -MD -E $(CXXFLAGS) $(DEFS) -D'ORP_VERSION=$(VERSION)' $^ > /dev/null

%.o : %.cpp
	@echo "[C] $@"
	@$(CXX) -c $(CXXFLAGS) $(DEFS) -D'ORP_VERSION=$(VERSION)' -o $@ $<

-include $(DEPS)

$(TARGET): $(OBJECTS)
	@echo "[L] $@"
	@$(CXX) $(LDFLAGS) $(CXXFLAGS) $(OBJECTS) $(LIBS) -o $@

images::
	make -C images

clean:
	rm -f $(OBJECTS) $(TARGET) $(DEPS)

dist-clean:
	$(MAKE) clean
	$(MAKE) -C gui clean
	$(MAKE) -C keys clean
	$(MAKE) -C psp clean
	$(MAKE) -C packages.d clean

# vi: ts=4
