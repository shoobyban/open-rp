VER_MAJOR=1
VER_MINOR=3
VER_RELEASE=SVN

# Detect OS
OS:=$(shell uname -s)
ifeq ($(OS),Linux)
-include Linux.mak
endif
ifeq ($(OS),Darwin)
-include MacOSX.mak
endif
ifeq ($(OS),MINGW32_NT-5.1)
-include MinGW32.mak
endif
ifeq ($(OS),MINGW32_NT-6.1)
-include MinGW32.mak
endif

