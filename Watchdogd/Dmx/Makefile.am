#**********************************************************************************************************/
#* Watchdogd/Dmx/Makefile.am          Profil de compilation de l'interface Dmx Watchdog                   */
#* Projet WatchDog version 4.0       Gestion d'habitat                      mar 22 oct 2019 23:13:10 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-dmx.la
libwatchdog_server_dmx_la_SOURCES    = Dmx.c Dmx.h admin_dmx.c

libwatchdog_server_dmx_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0` \
                                    `mysql_config --libs` \
                                    -rdynamic \
                                    -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_dmx_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0` \
                                   `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
