# Package version and release
VER_MAJOR=1
VER_MINOR=3
VER_RELEASE=SVN

# Detect OS type
OS:=$(shell uname -s)

# Is OS 64-bit?
OS_64BIT=0

# Linux
ifeq ($(OS),Linux)
-include Linux.mak
endif

# Mac OSX 
ifeq ($(OS),Darwin)
-include MacOSX.mak
OS_64BIT=$(shell sysctl hw.optional.x86_64 | cut -d' ' -f 2)
endif

# Windows (MINGW)
ifeq ($(OS:MINGW32%=MINGW32),MINGW32)
-include MinGW32.mak
OS=MINGW32
endif

