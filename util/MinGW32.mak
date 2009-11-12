# MinG (Windows) build configuration
CXX=g++
OS_CFLAGS=-I/mingw/include
OS_LDFLAGS=-mthreads  -Wl,--subsystem,windows -mwindows -Wl,-Bsymbolic -static
OS_LIBS=$(BUILD_ROOT)/lib/libz.a /mingw/lib/libdxguid.a -lwsock32 -lgdi32 -lwinmm -lmingw32
