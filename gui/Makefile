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

images::
	@echo "#ifndef _IMAGES_H" > images.h
	@echo "#define _IMAGES_H" >> images.h
	@xxd -i ./images/logo.png >> images.h
	@xxd -i ./images/dash_hacks.png >> images.h
	@xxd -i ./images/circle0.png >> images.h
	@xxd -i ./images/circle1.png >> images.h
	@xxd -i ./images/dp-down0.png >> images.h
	@xxd -i ./images/dp-down1.png >> images.h
	@xxd -i ./images/dp-left0.png >> images.h
	@xxd -i ./images/dp-left1.png >> images.h
	@xxd -i ./images/dp-right0.png >> images.h
	@xxd -i ./images/dp-right1.png >> images.h
	@xxd -i ./images/dp-up0.png >> images.h
	@xxd -i ./images/dp-up1.png >> images.h
	@xxd -i ./images/l10.png >> images.h
	@xxd -i ./images/l11.png >> images.h
	@xxd -i ./images/l20.png >> images.h
	@xxd -i ./images/l21.png >> images.h
	@xxd -i ./images/l30.png >> images.h
	@xxd -i ./images/l31.png >> images.h
	@xxd -i ./images/ps0.png >> images.h
	@xxd -i ./images/ps1.png >> images.h
	@xxd -i ./images/r10.png >> images.h
	@xxd -i ./images/r11.png >> images.h
	@xxd -i ./images/r20.png >> images.h
	@xxd -i ./images/r21.png >> images.h
	@xxd -i ./images/r30.png >> images.h
	@xxd -i ./images/r31.png >> images.h
	@xxd -i ./images/select0.png >> images.h
	@xxd -i ./images/select1.png >> images.h
	@xxd -i ./images/square0.png >> images.h
	@xxd -i ./images/square1.png >> images.h
	@xxd -i ./images/start0.png >> images.h
	@xxd -i ./images/start1.png >> images.h
	@xxd -i ./images/triangle0.png >> images.h
	@xxd -i ./images/triangle1.png >> images.h
	@xxd -i ./images/x0.png >> images.h
	@xxd -i ./images/x1.png >> images.h
	@echo "#endif // _IMAGES_H" >> images.h

clean:
	rm -f $(OBJECTS) $(TARGET) $(DEPS)

# vi: ts=4
