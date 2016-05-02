#**********************************************************************************************************/
#* Commun/Makefile.am          Profil de compilation des librairies communes Watchdog2.0                  */
#* Projet WatchDog version 2.0       Gestion d'habitat                      mer 04 jun 2003 13:54:13 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

SUBDIRS = Reseaux

lib_LTLIBRARIES = libwatchdog.la
libwatchdog_la_SOURCES = Erreur.c

libwatchdog_la_LIBADD  = Reseaux/librezo.la
# Trame/libtrame.la

libwatchdog_la_LDFLAGS = `pkg-config --libs openssl` -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION)

AM_CPPFLAGS=$(GLIB_CFLAGS) -I$(top_srcdir)/Include
MAINTAINERCLEANFILES = Makefile.in
