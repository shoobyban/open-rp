-include ../Config.mak
SHELL=/bin/bash
PKGREPO=http://orp.ps3-hacks.com

all:
	@mkdir -vp work
	@OS=$(OS) PKGREPO=$(PKGREPO) $(SHELL) ./build.sh

build:
	@make all

clean:
	@rm -rf work

dist-clean:
	@make clean
	@rm -rf source

# vi: ts=4
