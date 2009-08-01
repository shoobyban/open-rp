# Include version
-include Version.mak

SOURCE=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCE))
DEPS=$(patsubst %.o,%.d,$(OBJECTS))
VERSION="$(VER_MAJOR).$(VER_MINOR) $(VER_RELEASE)"
BUILD_ROOT=./packages.d/work/root
CFLAGS=-I. -I$(BUILD_ROOT)/include $(shell $(BUILD_ROOT)/bin/sdl-config --cflags) $(shell $(BUILD_ROOT)/bin/curl-config --cflags) $(shell $(BUILD_ROOT)/bin/libpng-config --cflags) $(OS_CFLAGS)
CXXFLAGS=$(CFLAGS)
LIBS=-lSDL_net -lSDL_image -lSDL_ttf -lavcodec -lavutil -lavformat -lswscale -lfreetype -lz -lfaad -lcrypto $(shell $(BUILD_ROOT)/bin/sdl-config --libs) $(shell $(BUILD_ROOT)/bin/curl-config --libs) $(shell $(BUILD_ROOT)/bin/libpng-config --libs) $(OS_LIBS)
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
	@echo -en "\e]2;Depend: $^\a"
	@echo "[D] $^"
	@$(CXX) -MD -E $(CXXFLAGS) $(DEFS) -D'ORP_VERSION=$(VERSION)' $^ > /dev/null

%.o : %.cpp
	@echo -en "\e]2;Compile: $@\a"
	@echo "[C] $@"
	@$(CXX) -c $(CXXFLAGS) $(DEFS) -D'ORP_VERSION=$(VERSION)' -o $@ $<

-include $(DEPS)

$(TARGET): $(OBJECTS)
	@echo -en "\e]2;Link: $@\a"
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
