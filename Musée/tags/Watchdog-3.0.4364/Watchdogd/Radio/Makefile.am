#**********************************************************************************************************/
#* Watchdogd/Audio/Makefile.am          Profil de compilation de l'interface Audio Watchdog               */
#* Projet WatchDog version 3.0       Gestion d'habitat                      mar 11 avr 2009 15:48:08 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-radio.la
libwatchdog_server_radio_la_SOURCES    = Radio.c Radio.h admin_radio.c


libwatchdog_server_radio_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0` \
                                       `mysql_config --libs` \
                                       -rdynamic \
                                       -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_radio_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0` \
                                      `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
