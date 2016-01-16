#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([Phidget21 C Library],m4_esyscmd([./version.sh]),[support@phidgets.com],[libphidget],[www.phidgets.com])
AM_INIT_AUTOMAKE([-Wall])
AC_CONFIG_MACRO_DIR([m4])
LT_INIT
AC_CONFIG_SRCDIR([cphidgetir.h])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Install in /usr
AC_PREFIX_DEFAULT([/usr])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

AC_ARG_VAR([LDCONFIG], [ldconfig utility])
AC_PATH_PROG([LDCONFIG], [ldconfig], [true], [/sbin /usr/sbin $PATH])

# Checks for libraries.
AC_SEARCH_LIBS([dlopen], [dl], [], 
	AC_MSG_ERROR([Missing libdl!]))
AC_SEARCH_LIBS([sqrt], [m], [], 
	AC_MSG_ERROR([Missing libm!]))
AC_SEARCH_LIBS([pthread_join], [pthread], [], 
	AC_MSG_ERROR([Missing libpthread!]))

# we need iconv - if it's not available in libc, look for the 'libiconv' function from libiconv.so
AC_SEARCH_LIBS([iconv], [iconv], [], 
	AC_SEARCH_LIBS([libiconv], [iconv], [], 
	[internaluniconv=true]))

AC_ARG_ENABLE([jni],
[  --enable-jni    Compile in Java support],
[case "${enableval}" in
	yes) jni=true ;;
	no)  jni=false ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-jni]) ;;
esac],[jni=true])
AM_CONDITIONAL([JNI], [test x$jni = xtrue])

AC_ARG_ENABLE([debug],
[  --enable-debug    Turn on debugging],
[case "${enableval}" in
	yes) debug=true ;;
	no)  debug=false ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],[debug=false])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

AC_ARG_ENABLE([zeroconf],
[  --enable-zeroconf    Turn on zeroconf, choose avahi or bonjour],
[case "${enableval}" in
	yes) zeroconf=avahi ;;
	avahi) zeroconf=avahi ;;
	bonjour) zeroconf=bonjour ;;
	no)  zeroconf=false ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-zeroconf]) ;;
esac],[zeroconf=avahi])
AM_CONDITIONAL([ZEROCONF_AVAHI], [test x$zeroconf = xavahi])
AM_CONDITIONAL([ZEROCONF_BONJOUR], [test x$zeroconf = xbonjour])
AM_CONDITIONAL([ZEROCONF], [test x$zeroconf != xfalse])

AC_ARG_ENABLE([zeroconf-lookup],
[  --enable-zeroconf-lookup    Turn on zeroconf lookup],
[case "${enableval}" in
	yes) zeroconflookup=true ;;
	no)  zeroconflookup=false ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-zeroconf-lookup]) ;;
esac],[zeroconflookup=false])
AM_CONDITIONAL([ZEROCONF_LOOKUP], [test x$zeroconflookup = xtrue])

AC_ARG_ENABLE([labview],
[  --enable-labview    Turn on Labview support],
[case "${enableval}" in
	yes) labview=true ;;
	no)  labview=false ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-labview]) ;;
esac],[labview=false])
AM_CONDITIONAL([LABVIEW], [test x$labview = xtrue])

AC_ARG_ENABLE([oldlibusb],
[  --enable-oldlibusb    Use libusb-0.1 instead of 1.0],
[case "${enableval}" in
	yes) newlibusb=false ;;
	no)  newlibusb=true ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-oldlibusb]) ;;
esac],[newlibusb=true])

if test "$newlibusb" = "true"; then
AC_SEARCH_LIBS([libusb_init], [usb-1.0], [newlibusb=true], 
	AC_SEARCH_LIBS([usb_find_busses], [usb], [newlibusb=false], 
	AC_MSG_ERROR([Missing libusb!])))
else
AC_SEARCH_LIBS([usb_find_busses], [usb], [newlibusb=false], 
	AC_MSG_ERROR([Missing libusb!]))
fi

AM_CONDITIONAL([NEW_LIBUSB], [test x$newlibusb = xtrue])

AC_ARG_ENABLE([ldconfig],
[AS_HELP_STRING([--disable-ldconfig],[do not update dynamic linker cache using ldconfig])],
,
[enable_ldconfig="yes"])
AM_CONDITIONAL([RUN_LDCONFIG], [test "${enable_ldconfig}" = "yes"])

AM_CONDITIONAL([INTERNAL_UNICONV], [test x$internaluniconv = xtrue])


AC_CONFIG_FILES([Makefile
                 examples/Makefile])
AC_OUTPUT([libphidget21.pc])
