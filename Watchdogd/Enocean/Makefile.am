#**********************************************************************************************************/
#* Watchdogd/Enocean/Makefile.am          Profil de compilation de l'interface ENOCEAN pour Watchdog      */
#* Projet Abls-Habitat version 4.6       Gestion d'habitat                 dim. 28 déc. 2014 15:43:17 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                          = libwatchdog-server-enocean.la
libwatchdog_server_enocean_la_SOURCES    = Enocean.c Enocean.h admin_enocean.c

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include

libwatchdog_server_enocean_la_CFLAGS = `pkg-config --cflags glib-2.0 json-glib-1.0` \
                                       `mysql_config --cflags`
libwatchdog_server_enocean_la_LDFLAGS = -rdynamic \
                                       `pkg-config --libs glib-2.0 json-glib-1.0` \
                                       `mysql_config --libs`
