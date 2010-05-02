#**********************************************************************************************************/
#* CommunClient/Makefile.am          Profil de compilation des librairies communes aux clients Watchdog2.0*/
#* Projet WatchDog version 2.0       Gestion d'habitat                      mer 04 jun 2010 13:54:13 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

SUBDIRS = 

lib_LTLIBRARIES = libwatchdogclient.la
libwatchdogclient_la_SOURCES = trame.c

#libwatchdog_la_LIBADD  = Reseaux/librezo.la

libwatchdogclient_la_LDFLAGS = -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION)

INCLUDES=$(GNOME_INCLUDEDIR) $(GNOME2_CFLAGS) -I$(top_srcdir)/IncludeClient -I$(top_srcdir)/Include

MAINTAINERCLEANFILES = Makefile.in
