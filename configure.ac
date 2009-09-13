dnl Process this file with autoconf to produce a configure script.
AC_INIT(Watchdogd/Watchdogd.c)
AC_CONFIG_HEADER(sysconfig.h)
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE( Watchdog, 2.5.`svnversion -n` )

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
AC_PROG_RANLIB
AC_ISC_POSIX

AM_DISABLE_STATIC
AC_PROG_LIBTOOL

GNOME2_CFLAGS="`pkg-config --cflags libgnomeui-2.0 goocanvas gtksourceview-2.0 gstreamer-0.10 gstreamer-interfaces-0.10`"
AC_SUBST(GNOME2_CFLAGS)

BONOBO_CFLAGS=`pkg-config --cflags libbonobo-2.0`
AC_SUBST(BONOBO_CFLAGS)

GNOME2_LIBS="`pkg-config --libs libgnomeui-2.0 goocanvas gtksourceview-2.0 gstreamer-0.10 gstreamer-interfaces-0.10`"
AC_SUBST(GNOME2_LIBS)

MYSQL_CFLAGS=`mysql_config --cflags`
AC_SUBST(MYSQL_CFLAGS)

MYSQL_LIBS=`mysql_config --libs`
AC_SUBST(MYSQL_LIBS)

BONOBO_LIBS=`pkg-config --libs libbonobo-2.0`
AC_SUBST(BONOBO_LIBS)

GNOKII_LIBS="-lgnokii" 
dnl `pkg-config --libs gnokii`
AC_SUBST(GNOKII_LIBS)

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

dnl parametres par defaut
CFLAGS="-Wall -pipe"
YACC="$YACC -d"

LIBW_MAJOR_VERSION=2
LIBW_MINOR_VERSION=5
LIBW_MICRO_VERSION=2

LIBDLS_MAJOR_VERSION=2
LIBDLS_MINOR_VERSION=5
LIBDLS_MICRO_VERSION=2

AC_SUBST(LIBW_MAJOR_VERSION)
AC_SUBST(LIBW_MINOR_VERSION)
AC_SUBST(LIBW_MICRO_VERSION)

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
Watchdogd/Sms/Makefile \
Watchdogd/Audio/Makefile \
Watchdogd/Admin/Makefile \
Watchdogd/Rs485/Makefile \
Watchdogd/Modbus/Makefile \
Watchdogd/Archive/Makefile \
Watchdogd/EntreeANA/Makefile \
Watchdogd/Message/Makefile \
Watchdogd/Mnemonique/Makefile \
Watchdogd/Scenario/Makefile \
Watchdogd/Icones/Makefile \
Watchdogd/Histo/Makefile \
Watchdogd/Utilisateur/Makefile \
Watchdogd/Synoptiques/Makefile \
Watchdogd/CptHoraire/Makefile \
Watchdogd/Config/Makefile \
Watchdogd/Serveur/Makefile \
Watchdogd/TraductionDLS/Makefile \
Watchdogd/Dls/Makefile \
WatchdogdAdmin/Makefile \
)
