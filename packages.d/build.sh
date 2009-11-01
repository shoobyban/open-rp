# Download, unpack, (possibly) patch, configure, and install required
# packages for Open Remote Play

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
	. $PKG || exit 1
	if [ ! -d "$WORKDIR/$SOURCE" ]; then
		if [ ! -f "$WORKDIR/$SOURCE.tar.gz" ]; then
			wget -c "$PKGREPO/packages/$SOURCE.tar.gz" -O "$SRCDIR/$SOURCE.tar.gz" || exit 1
		fi
		tar -xzf "$SRCDIR/$SOURCE.tar.gz" -C "$WORKDIR" || exit 1
		cd ..
		if [ ! -z "$PATCHES" ]; then
			for PATCH in "$PATCHES"; do
				cd "$WORKDIR/$SOURCE" && patch -p1 -i "../../../patches/$PATCH" || exit 1
				cd ../..
			done
		fi
	fi

	cd "$WORKDIR/$SOURCE" && PREFIX=../root $CONFIGURE || exit 1
	cd ../..

	make -C "$WORKDIR/$SOURCE" || exit 1

	make -C "$WORKDIR/$SOURCE" install || exit 1

	unset LIBS
done

exit 0

