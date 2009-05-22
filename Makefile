# Include version
-include Version.mak

# Uncomment one of these
-include Gentoo.mak
#-include Linux.mak
#-include MacOSX.mak
#-include MinGW32.mak

SOURCE=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCE))
DEPS=$(patsubst %.o,%.d,$(OBJECTS))
VERSION="$(VER_MAJOR).$(VER_MINOR) $(VER_RELEASE)"

all:
	@echo "Compiler: $(CXX) $(CXXFLAGS)"
	@echo "Linker: $(LD) $(LDFLAGS)"
	@echo "Defines: $(DEFS)"
	@echo "Libraries: $(LIBS)"
	$(MAKE) MAKEFLAGS= deps
	$(MAKE) $(TARGET)

gui::
	$(MAKE) -C gui

keys::
	$(MAKE) -C keys

psp::
	$(MAKE) -C psp

deps: $(wildcard *.cpp)
	@echo "[D] $^"
	@$(CXX) -MD -E $(CXXFLAGS) $(DEFS) -D'ORP_VERSION="$(VERSION)"' $^ > /dev/null

%.o : %.cpp
	@echo "[C] $@"
	@$(CXX) -c $(CXXFLAGS) $(DEFS) -D'ORP_VERSION="$(VERSION)"' -o $@ $<

-include $(DEPS)

$(TARGET): $(OBJECTS)
	@echo "[L] $@"
	@$(CXX) $(LDFLAGS) $(CXXFLAGS) $(OBJECTS) $(LIBS) -o $@

images::
	make -C images

clean:
	rm -f $(OBJECTS) $(TARGET) $(DEPS)

dist:
	$(MAKE) clean
	$(MAKE) -C gui clean
	$(MAKE) -C keys clean
	$(MAKE) -C psp clean

# vi: ts=4
