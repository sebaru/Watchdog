#**********************************************************************************************************/
#* Watchdogd/Onduleur/Makefile.am          Profil de compilation de l'interface Onduleur Watchdog         */
#* Projet WatchDog version 3.0       Gestion d'habitat                     mar. 10 nov. 2009 15:55:22 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-ups.la
libwatchdog_server_ups_la_SOURCES = Onduleur.c admin_onduleur.c Onduleur.h

libwatchdog_server_ups_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 libupsclient json-glib-1.0 libsoup-2.4` \
                                    `mysql_config --libs` \
                                    -rdynamic \
                                    -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_ups_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 libupsclient json-glib-1.0 libsoup-2.4` \
                                   `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
