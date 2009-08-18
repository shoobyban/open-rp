CXX=g++
INCLUDE=-I. -I..
CFLAGS=-g -pipe $(INCLUDE) $(shell sdl-config --cflags) $(shell curl-config --cflags)
CXXFLAGS=$(CFLAGS)
LIBS_SHARED=-lcrypto -lpthread -lz $(shell sdl-config --libs) $(shell curl-config --libs) -lSDL_net -lSDL_image -lavcodec -lavutil -lavformat -lswscale
LIBS_SHARED_EXTRA=
LIBS_STATIC=
LIBS=$(LIBS_STATIC) $(LIBS_SHARED) $(LIBS_SHARED_EXTRA)

all:
	$(MAKE) config-debug stream-debug

config-debug:
	$(CXX) -g -pipe -I. -I.. -DORP_CONFIG_DEBUG config-debug.cpp ../config.cpp -o config-debug $(LDFLAGS)

stream-debug:
	$(CXX) $(INCLUDE) stream-debug.cpp ../orp.o ../config.o ../base64.o -o stream-debug $(LDFLAGS) $(LIBS)

clean:
	@rm -f config-debug stream-debug
