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
PATH=$WORKDIR/root/bin:$PATH

mkdir -p "$WORKDIR" || exit 1
mkdir -p "$SRCDIR" || exit 1
mkdir -p "$PREFIX" || exit 1

PACKAGES=$(ls ??-*)
for PKG in $PACKAGES; do
	CFLAGS=-I$PREFIX/include
	LDFLAGS=-L$PREFIX/lib
	LIBS=$PREFIX/lib/libz.a
	LIBFIX=
	PATCHES=
	TARGET=
	. $PKG || exit 1

	if [ ! -d "$WORKDIR/$SOURCE" ]; then
		if [ ! -f "$SRCDIR/$SOURCE.tar.gz" ]; then
			if [ -x "$(which wget)" ]; then
				wget "$PKGREPO/packages/$SOURCE.tar.gz" \
					-O "$SRCDIR/$SOURCE.tar.gz" || exit 1
			elif [ -x "$(which curl)" ]; then
				curl -o "$SRCDIR/$SOURCE.tar.gz" \
					"$PKGREPO/packages/$SOURCE.tar.gz" || exit 1
			else
				echo "Please install wget or curl."
				exit 1
			fi
		fi
		echo "Extracting: $SOURCE"
		tar -xzf "$SRCDIR/$SOURCE.tar.gz" -C "$WORKDIR" || exit 1
		cd ..
		if [ ! -z "$PATCHES" ]; then
			for PATCH in $PATCHES; do
				cd "$WORKDIR/$SOURCE" && \
				patch -p1 -i "../../patches/$PATCH" || exit 1
				cd ../..
			done
		fi
	elif [ -f "$WORKDIR/$SOURCE/.orp-stamp" ]; then
		continue
	fi

	cd "$WORKDIR/$SOURCE" && \
		PREFIX=$PREFIX CFLAGS=$CFLAGS \
		CXXFLAGS=$CFLAGS LDFLAGS=$LDFLAGS \
		LIBS=$LIBS $CONFIGURE || exit 1
	cd ../..

	make -C "$WORKDIR/$SOURCE/$TARGET" || exit 1
	make -C "$WORKDIR/$SOURCE/$TARGET" install || exit 1
	if [ ! -z "$LIBFIX" ]; then
		PREFIX=$PREFIX LIB=$LIBFIX ./libfix.sh || exit 1
	fi
	touch "$WORKDIR/$SOURCE/.orp-stamp"
done

exit 0

