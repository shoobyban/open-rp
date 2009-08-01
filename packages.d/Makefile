-include ../Version.mak

PKGREPO=http://orp.ps3-hacks.com

all: orp-build
	@mkdir -vp work

orp-build:
	@rm -rf ./work/root
	@OS=$(OS) PKGREPO=$(PKGREPO) ./build.sh
	@touch orp-build

build:
	@rm orp-build
	@make all

clean:
	@echo -en "\e]2;Cleaning: packages.d\a"
	@rm -rf ./work orp-build

dist-clean:
	@make clean
	@rm -rf source

# vi: ts=4
