-include ../Version.mak
VERSION="v$(VER_MAJOR).$(VER_MINOR) $(VER_RELEASE)"

all:
	@pbmtext -font ter-u16b.bdf -nomargins $(VERSION) > version.pbm
	@pngtopnm splash.png > splash.pnm
	@pnmcomp -align=right -valign=bottom -xoff=-10 -yoff=-10 -opacity=0.3 version.pbm splash.pnm | pnmtopng > splash-version.png
	@echo "#ifndef _IMAGES_H" > ../images.h
	@echo "#define _IMAGES_H" >> ../images.h
	xxd -i icon.bmp >> ../images.h
	xxd -i icon-osx.bmp >> ../images.h
	xxd -i splash-version.png >> ../images.h
	@echo "#endif // _IMAGES_H" >> ../images.h

# vi: ts=4
