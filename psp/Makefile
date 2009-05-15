TARGET = orp
OBJS = orp.o

INCDIR =
CFLAGS = -G0 -Wall -DPSPFW3X
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
BUILD_PRX=1
PSP_FW_VERSION=303

LIBDIR =
LDFLAGS =
LIBS= -lpspreg -lpspopenpsid -lpspwlan

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Open Remote Play Exporter
PSP_EBOOT_ICON = ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
include $(PSPSDK)/lib/build.mak

install: $(EXTRA_TARGETS)
	@mkdir -p ORP_Export
	@cp -v EBOOT.PBP ORP_Export
	@zip -r ORP_Export.zip ORP_Export
