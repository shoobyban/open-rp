CXX=g++
OS_CFLAGS=
OS_LDFLAGS=-mthreads -Wl,--subsystem,windows -mwindows -static
OS_LIBS=$(BUILD_ROOT)/lib/libwx_msw-2.8.a $(BUILD_ROOT)/lib/libwxregex-2.8.a $(BUILD_ROOT)/lib/libwxtiff-2.8.a $(BUILD_ROOT)/lib/libwxjpeg-2.8.a -lrpcrt4 -loleaut32 -lole32 -lwinspool -lwinmm -lshell32 -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lgdi32 -luuid
DEFS=
TARGET=orpui.exe

