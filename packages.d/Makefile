-include ../Config.mak
SHELL=/bin/bash
PKGREPO=http://orp.ps3-hacks.com

all:
	@mkdir -vp work
	@OS=$(OS) OS_64BIT=$(OS_64BIT) PKGREPO=$(PKGREPO) $(SHELL) ./build.sh
	@find ./work/root/lib/ -name '*.dylib' -o -name '*.so' -o -name '*.dll' | xargs rm -rf

build:
	@make all

clean:
	@rm -rf work

dist-clean:
	@make clean
	@rm -rf source

# vi: ts=4
