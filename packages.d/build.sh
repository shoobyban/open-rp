#!/bin/sh

if [ -z "$OS" ]; then
	echo "Unknown Host OS."
	exit 1
fi

if [ -z "$PKGREPO" ]; then
	echo "Unknown package repository (PKGREPO)."
	exit 1
fi

WORKDIR="$PWD/$(dirname $0)/work"
SRCDIR="$PWD/$(dirname $0)/source"
PREFIX="$WORKDIR/root"
export CFLAGS="-I\"$PREFIX/include\""
export LDFLAGS="-L\"$PREFIX/lib\""

mkdir -p "$WORKDIR" || exit 1
mkdir -p "$SRCDIR" || exit 1
mkdir -p "$PREFIX" || exit 1

PACKAGES=$(ls ??-*)
for PKG in $PACKAGES; do
	source $PKG || exit 1
	if [ ! -d "$WORKDIR/$SOURCE" ]; then
		if [ ! -f "$WORKDIR/$SOURCE/tar.gz" ]; then
			echo -en "\e]2;Downloading: $SOURCE\a"
			wget -c "$PKGREPO/packages/$SOURCE.tar.gz" -O "$SRCDIR/$SOURCE.tar.gz" || exit 1
		fi
		echo -en "\e]2;Unpacking: $SOURCE\a"
		tar -xzf "$SRCDIR/$SOURCE.tar.gz" -C "$WORKDIR" || exit 1
		cd ..
		if [ ! -z "$PATCHES" ]; then
			echo -en "\e]2;Patching: $SOURCE\a"
			for PATCH in "$PATCHES"; do
				cd "$WORKDIR/$SOURCE" && patch -p1 -i "../../../patches/$PATCH" || exit 1
				cd ../..
			done
		fi
		echo -en "\e]2;Configuring: $SOURCE\a"
		cd "$WORKDIR/$SOURCE" && PREFIX=../root $CONFIGURE || exit 1
		cd ../..
	fi

	echo -en "\e]2;Compiling: $SOURCE\a"
	make -C "$WORKDIR/$SOURCE" || exit 1

	echo -en "\e]2;Installing: $SOURCE\a"
	make -C "$WORKDIR/$SOURCE" install || exit 1

	unset LIBS
done

exit 0

