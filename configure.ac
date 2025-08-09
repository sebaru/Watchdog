dnl Process this file with autoconf to produce a configure script.

AC_INIT([Watchdog],[4.x],[sebastien.lefevre@abls-habitat.fr],[Watchdog],[https://github.com/sebaru])

AC_DEFINE_UNQUOTED([WTD_VERSION], "[m4_esyscmd_s([sh -c "git describe --tags | tr -d '\n'"])]", [Numero de version])
AC_DEFINE_UNQUOTED([WTD_BRANCHE], "[m4_esyscmd_s([sh -c "git branch --show-current"])]", [Branche GIT])

AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR([Watchdogd/Watchdogd.c])
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE([foreign])

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_CONFIG_MACRO_DIR([m4])
AC_SEARCH_LIBS([strerror],[cposix])
AM_PROG_CC_C_O
PKG_PROG_PKG_CONFIG

AC_SUBST([systemdsystemunitdir], $($PKG_CONFIG --variable=systemdsystemunitdir systemd) )
AC_SUBST([systemduserunitdir], $($PKG_CONFIG --variable=systemduserunitdir systemd) )
AC_SUBST([sudoersdir], "/etc/sudoers.d" )
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$with_systemdsystemunitdir" != "xno"])

AC_DISABLE_STATIC([])
LT_INIT

AM_PATH_GLIB_2_0( 2.2.2 )
GLIB_GSETTINGS
dnl PKG_CHECK_MODULES( LIBSOUP, >= 3.3 )

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(glib.h fcntl.h sys/file.h sys/time.h syslog.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_MEMCMP

AC_FUNC_STRFTIME
AC_CHECK_FUNCS(gettimeofday select socket strdup strerror)

dnl parametres par defaut
CFLAGS="-Wall -Wno-unused-result -Wno-comment -Wno-pointer-sign -Wno-discarded-qualifiers -pipe -g -ggdb -fdiagnostics-color"

LIBW_MAJOR_VERSION=6
LIBW_MINOR_VERSION=0
LIBW_MICRO_VERSION=0

AC_SUBST(LIBW_MAJOR_VERSION)
AC_SUBST(LIBW_MINOR_VERSION)
AC_SUBST(LIBW_MICRO_VERSION)

LIBDLS_MAJOR_VERSION=6
LIBDLS_MINOR_VERSION=0
LIBDLS_MICRO_VERSION=0

AC_SUBST(LIBDLS_MAJOR_VERSION)
AC_SUBST(LIBDLS_MINOR_VERSION)
AC_SUBST(LIBDLS_MICRO_VERSION)

ALL_LINGUAS=""

AC_CONFIG_FILES([\
Makefile \
Watchdogd/Makefile \
Watchdogd/Include/Makefile \
Watchdogd/Onduleur/Makefile \
Watchdogd/Smsg/Makefile \
Watchdogd/Audio/Makefile \
Watchdogd/Modbus/Makefile \
Watchdogd/Imsgs/Makefile \
Watchdogd/TeleinfoEDF/Makefile \
Watchdogd/Dls/Makefile \
Watchdogd/Radio/Makefile \
Watchdogd/Dmx/Makefile \
Watchdogd/Meteo/Makefile \
Watchdogd/Phidget/Makefile \
Watchdogd/Gpiod/Makefile \
Watchdogd/Shelly/Makefile \
])
AC_OUTPUT
