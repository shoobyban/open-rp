CXX=g++
LD=
INCLUDE=-I. -I/mingw/include $(sdl-config --cflags)
CXXFLAGS=$(CFLAGS) -g -mthreads -I/mingw/include/SDL -D_GNU_SOURCE=1 -Dmain=SDL_main
LIBDIR=/mingw/lib
LDFLAGS=-L$(LIBDIR) -mthreads
#LDFLAGS=-L$(LIBDIR) -mthreads  -Wl,--subsystem,windows -mwindows
LIBS=-mwindows -Wl,-Bsymbolic $(LIBDIR)/libSDLmain.a $(LIBDIR)/libSDL.a $(LIBDIR)/libSDL_image.a $(LIBDIR)/libpng.a $(LIBDIR)/libz.a $(LIBDIR)/libSDL_net.a $(LIBDIR)/libcrypto.a $(LIBDIR)/libavformat.a $(LIBDIR)/libavcodec.a $(LIBDIR)/libswscale.a $(LIBDIR)/libavutil.a $(LIBDIR)/libfaad.a $(LIBDIR)/libcurl.a $(LIBDIR)/libdxguid.a $(LIBDIR)/libz.a -lwsock32 -lgdi32 -lwinmm -lmingw32
DEFS=-DCURL_STATICLIB
TARGET=orp.exe
