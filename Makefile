-include Config.mak
BUILD_ROOT=./packages.d/work/root
SOURCE=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCE))
DEPS=$(patsubst %.o,%.d,$(OBJECTS))
VERSION="$(VER_MAJOR).$(VER_MINOR) $(VER_RELEASE)"
CFLAGS=-I. -I$(BUILD_ROOT)/include $(OS_CFLAGS)
CXXFLAGS=$(CFLAGS)
LIBS=$(OS_LIBS)
LDFLAGS=-L$(BUILD_ROOT)/lib $(OS_LDFLAGS)

all:
	@echo "Host OS: $(OS)"
	@echo "Compiler: $(CXX) $(CXXFLAGS)"
	@echo "Linker: $(LD) $(LDFLAGS)"
	@echo "Defines: $(DEFS)"
	@echo "Libraries: $(LIBS)"
	$(MAKE) -C packages.d
	$(MAKE) MAKEFLAGS= deps
	$(MAKE) $(TARGET)
	$(MAKE) -C gui all

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
	$(MAKE) -C util clean
	$(MAKE) -C packages.d clean
	$(MAKE) -C psp clean

# vi: ts=4
