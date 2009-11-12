-include ../Config.mak
PKGREPO=http://orp.ps3-hacks.com

all: .orp-stamp
	@mkdir -vp work

.orp-stamp:
	@OS=$(OS) PKGREPO=$(PKGREPO) $(SHELL) ./build.sh
	@touch .orp-stamp

build:
	@rm .orp-stamp
	@make all

clean:
	@rm -f .orp-stamp
	@rm -rf work

dist-clean:
	@make clean
	@rm -rf source

# vi: ts=4
