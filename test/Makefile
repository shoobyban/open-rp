CXX=g++
INCLUDE=-I. -I..
CFLAGS=-g -pipe
DEFS=-DORP_CONFIG_DEBUG
CXXFLAGS=$(CFLAGS)

all:
	$(CXX) $(INCLUDE) $(DEFS) config.cpp ../config.cpp -o config $(LDFLAGS) $(LIBS)

clean:
	@rm -f config
