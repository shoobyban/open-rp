CXX=g++
INCLUDE=-I.
CFLAGS=-O2 -pipe $(INCLUDE) $(shell sdl-config --cflags) $(shell curl-config --cflags)
CXXFLAGS=$(CFLAGS)
LDFLAGS=-Wl,-framework,OpenGL -Wl,-framework,Cocoa -Wl,-framework,QuickTime -Wl,-framework,ApplicationServices -Wl,-framework,Carbon -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,IOKit
LIBDIR=/usr/local/lib
LIBS_SHARED=-lcrypto -lpthread -lz
LIBS_STATIC=$(LIBDIR)/libSDL.a $(LIBDIR)/libSDLmain.a $(LIBDIR)/libSDL_image.a $(LIBDIR)/libSDL_net.a $(LIBDIR)/libcurl.a $(LIBDIR)/libfaad.a $(LIBDIR)/libavcodec.a $(LIBDIR)/libavformat.a $(LIBDIR)/libavutil.a $(LIBDIR)/libpng.a $(LIBDIR)/libswscale.a
LIBS=$(LIBS_STATIC) $(LIBS_SHARED) $(LIBS_SHARED_EXTRA)
#LIBS=$(shell sdl-config --libs) $(shell curl-config --libs) -lSDL_image -lSDL_net -lcrypto -lavcodec -lavutil -lavformat -lswscale -lfaad
DEFS=-D_MACOSX_
TARGET=orp

release: $(TARGET)
	@rm -rf ORP-1.1-BETA-OSX*
	@mkdir ORP-1.1-BETA-OSX
	@cp -v README ORP-1.1-BETA-OSX/README.txt
	@cp -rv psp/ORP_Export ORP-1.1-BETA-OSX
	@cp -rv "gui/Open Remote Play.app" ORP-1.1-BETA-OSX
	@cp keys/keys.orp ORP-1.1-BETA-OSX
	@find ORP-1.1-BETA-OSX -type d -name '.svn' -print0 | xargs -0 rm -rf
	@zip -r ORP-1.1-BETA-OSX.zip ORP-1.1-BETA-OSX
