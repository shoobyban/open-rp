# Include version
-include ../Version.mak

# Uncomment one of these
-include Linux.mak
#-include MacOSX.mak
#-include MinGW32.mak
#-include Custom.mak

SOURCE=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCE))
DEPS=$(patsubst %.o,%.d,$(OBJECTS))
VERSION=$(VER_MAJOR).$(VER_MINOR) $(VER_RELEASE)

all:
	@echo "Compiler: $(CXX) $(CXXFLAGS)"
	@echo "Linker: $(LD) $(LDFLAGS)"
	@echo "Defines: $(DEFS)"
	@echo "Libraries: $(LIBS)"
	@ln -sf ../config.h .
	@ln -sf ../config.cpp .
	$(MAKE) MAKEFLAGS= deps
	$(MAKE) $(TARGET)

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

images:
	@echo "#ifndef _IMAGES_H" > images.h
	@echo "#define _IMAGES_H" >> images.h
	@xxd -i ./images/edit.png >> images.h
	@xxd -i ./images/logo.png >> images.h
	@echo "#endif // _IMAGES_H" >> images.h

clean:
	rm -f $(OBJECTS) $(TARGET) $(DEPS)

# vi: ts=4
