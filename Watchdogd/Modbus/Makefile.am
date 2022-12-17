#**********************************************************************************************************/
#* Watchdogd/Modbus/Makefile.am          Profil de compilation de l'interface Modbus Watchdog             */
#* Projet WatchDog version 3.0       Gestion d'habitat                      mar 13 jui 2007 15:48:08 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS=

lib_LTLIBRARIES     = libwatchdog-server-modbus.la
libwatchdog_server_modbus_la_SOURCES = Modbus.c Modbus.h

libwatchdog_server_modbus_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0 libsoup-2.4` \
                                       `mariadb_config --libs` \
                                       -rdynamic \
                                       -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_modbus_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0 libsoup-2.4` \
                                      `mariadb_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
