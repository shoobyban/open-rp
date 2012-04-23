#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([open-rp], [1.4], [http://code.google.com/p/open-rp/])
AM_INIT_AUTOMAKE([open-rp], [1.4])
AC_CONFIG_SRCDIR([orp.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL
AC_CHECK_PROG([PROG_ZIP], [zip], [zip], [false])

# Checks for libraries.
AC_CHECK_LIB([SDL], [SDL_Init], [], [
	AC_MSG_ERROR([libSDL not found.])])
AC_CHECK_LIB([SDL_ttf], [TTF_Init], [], [
	AC_MSG_ERROR([libSDL_ttf not found.])])
AC_CHECK_LIB([SDL_image], [IMG_LoadPNG_RW], [], [
	AC_MSG_ERROR([libSDL_image with PNG support not found.])])
AC_CHECK_LIB([SDL_net], [SDLNet_UDP_Open], [], [
	AC_MSG_ERROR([libSDL_net not found.])])
AC_CHECK_LIB([freetype], [FT_Init_FreeType], [], [
	AC_MSG_ERROR([libfreetype not found.])])
AC_CHECK_LIB([crypto], [AES_cbc_encrypt], [], [
	AC_MSG_ERROR([libcrypto not found.])])
#AC_CHECK_LIB([avcodec], [avcodec_init], [], [
#	AC_MSG_ERROR([libavcodec not found.])])
#AC_CHECK_LIB([avutil], [av_log_set_callback], [], [
#	AC_MSG_ERROR([libavutil not found.])])
#AC_CHECK_LIB([swscale], [sws_getContext], [], [
#	AC_MSG_ERROR([libswscale not found.])])
AC_CHECK_LIB([curl], [curl_easy_init], [], [
	AC_MSG_ERROR([libcurl not found.])])
AC_CHECK_LIB([z], [deflate], [], [
	AC_MSG_ERROR([libz not found.])])

PKG_CHECK_MODULES([FFMPEG], [libavcodec libavutil libswscale])

# Checks for header files.
AC_CHECK_HEADERS([malloc.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getpagesize gettimeofday inet_ntoa memset select socket strcasecmp strdup strerror strncasecmp])

# Configure sub-projects
ORP_SUBDIRS="keys util"
AC_ARG_WITH([wxorp],
	[AS_HELP_STRING([--with-wxorp],
		[build wxWidgets GUI @<:@default=yes@:>@])],
	[],
	[with_wxorp=yes])

if test "x$with_wxorp" = xyes; then
	ORP_SUBDIRS="$ORP_SUBDIRS wxorp"
	AC_CONFIG_SUBDIRS([wxorp])
fi
AC_SUBST(ORP_SUBDIRS)

AC_CONFIG_FILES([Makefile images/Makefile keys/Makefile util/Makefile open-rp.spec])
AC_OUTPUT
