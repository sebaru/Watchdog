#**********************************************************************************************************/
#* Watchdogd/Meteo/Makefile.am          Profil de compilation de l'interface Ephemeide de Watchdog        */
#* Projet WatchDog version 3.0       Gestion d'habitat                                12.03.2021 18:35:36 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS=

lib_LTLIBRARIES                     = libwatchdog-server-meteo.la
libwatchdog_server_meteo_la_SOURCES = Meteo.c Meteo.h admin_meteo.c

libwatchdog_server_meteo_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0 libsoup-2.4` \
                                      `mysql_config --libs` \
                                      -rdynamic

libwatchdog_server_meteo_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0 libsoup-2.4` \
                                     `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
