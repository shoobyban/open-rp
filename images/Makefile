all:
	@echo "#ifndef _IMAGES_H" > ../images.h
	@echo "#define _IMAGES_H" >> ../images.h
	xxd -i icon.bmp >> ../images.h
	xxd -i icon-osx.bmp >> ../images.h
	xxd -i splash.png >> ../images.h
	@echo "#endif // _IMAGES_H" >> ../images.h

# vi: ts=4
