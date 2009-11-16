#!/bin/sh

#LIBS="libSDL_image libSDL_net libSDL_ttf libcurl libfaad libpng"

if [ -z "$LIB" ]; then
	echo "Required variable LIB not set."
	exit 1
fi

if [ -z "$PREFIX" ]; then
	echo "Required variable PREFIX not set."
	exit 1
fi

if [ -d $LIB ]; then
	rm -rf $LIB
fi

mkdir $LIB || exit 1
cd $LIB || exit 1
ar -x $PREFIX/lib/$LIB.a || exit 1
rm -f $PREFIX/lib/$LIB.a
libtool -static *.o -o $PREFIX/lib/$LIB.a || exit 1
cd ..

rm -rf $LIB
exit 0
