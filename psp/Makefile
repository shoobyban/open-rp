TARGET = orp
OBJS = orp.o

INCDIR =
CFLAGS = -G0 -Wall -DORP_PSP -DPSPFW3X
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
	@mkdir -p ORP_Exporter
	@cp -v EBOOT.PBP ORP_Exporter
	@zip -r ORP_Exporter.zip ORP_Exporter
