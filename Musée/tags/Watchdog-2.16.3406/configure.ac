dnl Process this file with autoconf to produce a configure script.

dnl define([gittag], esyscmd([sh -c "git describe --always  | tr -d '\n'"]))
define([SVNVERSION], esyscmd([sh -c "svnversion -n | sed s/M//g"]))
AC_INIT([Watchdog], [2.16.SVNVERSION], [sebastien.lefevre@abls-habitat.fr], [Watchdog], [https://wiki.abls-habitat.fr] )
AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR([Watchdogd/Watchdogd.c])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_YACC
AC_PROG_CC
AC_PROG_CPP
AM_PROG_LEX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_CONFIG_MACRO_DIR([m4])
AC_ISC_POSIX
AM_PROG_CC_C_O

AM_DISABLE_STATIC
AC_PROG_LIBTOOL

AM_PATH_GLIB_2_0( 2.2.2 )

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(glib.h fcntl.h sys/file.h sys/time.h syslog.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_CHECK_FUNCS(gettimeofday select socket strdup strerror)

AC_CHECK_PROGS(BISON,bison,none)
if test "$BISON" = "none"
then
  AC_MSG_ERROR([no suitable bison found. Please install the 'bison' package.])
fi

AC_CHECK_PROGS(FLEX,flex,none)
if test "$FLEX" = "none"
then
  AC_MSG_ERROR([no suitable flex found. Please install the 'flex' package.])
fi

dnl parametres par defaut
CFLAGS="-O -pipe -g -ggdb"

LIBW_MAJOR_VERSION=4
LIBW_MINOR_VERSION=0
LIBW_MICRO_VERSION=0

AC_SUBST(LIBW_MAJOR_VERSION)
AC_SUBST(LIBW_MINOR_VERSION)
AC_SUBST(LIBW_MICRO_VERSION)

LIBDLS_MAJOR_VERSION=4
LIBDLS_MINOR_VERSION=3
LIBDLS_MICRO_VERSION=0

AC_SUBST(LIBDLS_MAJOR_VERSION)
AC_SUBST(LIBDLS_MINOR_VERSION)
AC_SUBST(LIBDLS_MICRO_VERSION)

ALL_LINGUAS=""

AC_OUTPUT( \
Makefile \
Include/Makefile \
Commun/Makefile \
Client/Include/Makefile \
Commun/Reseaux/Makefile \
Client/Config_cli/Makefile \
Client/Makefile \
Watchdogd/Makefile \
Watchdogd/Include/Makefile \
Watchdogd/Onduleur/Makefile \
Watchdogd/Sms/Makefile \
Watchdogd/Audio/Makefile \
Watchdogd/Rs485/Makefile \
Watchdogd/Modbus/Makefile \
Watchdogd/Archive/Makefile \
Watchdogd/Rfxcom/Makefile \
Watchdogd/Enocean/Makefile \
Watchdogd/Imsg/Makefile \
Watchdogd/Satellite/Makefile \
Watchdogd/TeleinfoEDF/Makefile \
Watchdogd/Http/Makefile \
Watchdogd/Lirc/Makefile \
Watchdogd/Serveur/Makefile \
Watchdogd/TraductionDLS/Makefile \
Watchdogd/Dls/Makefile \
WatchdogdAdmin/Makefile \
)
