-include Gentoo.mak
#-include MacOSX.mak
#-include MinGW32.mak

SOURCE=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCE))
DEPS=$(patsubst %.o,%.d,$(OBJECTS))

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
	@$(CXX) -MD -E $(CXXFLAGS) $(DEFS) $^ > /dev/null

%.o : %.cpp
	@echo "[C] $@"
	@$(CXX) -c $(CXXFLAGS) $(DEFS) -o $@ $<

-include $(DEPS)

$(TARGET): $(OBJECTS)
	@echo "[L] $@"
	@$(CXX) $(LDFLAGS) $(CXXFLAGS) $(OBJECTS) $(LIBS) -o $@

images:
	@echo "#ifndef _IMAGES_H" > images.h
	@echo "#define _IMAGES_H" >> images.h
	xxd -i ./images/icon.bmp >> images.h
	xxd -i ./images/splash.png >> images.h
	@echo "#endif // _IMAGES_H" >> images.h

clean:
	rm -f $(OBJECTS) $(TARGET) $(DEPS)

dist:
	$(MAKE) clean
	$(MAKE) -C gui clean
	$(MAKE) -C keys clean
	$(MAKE) -C psp clean

# vi: ts=4
