#**********************************************************************************************************/
#* Watchdogd/Shelly/Makefile.am          Profil de compilation de l'interface SHELLY Watchdog             */
#* Projet Abls-Habitat version 4.2       Gestion d'habitat                            08.03.2024 23:31:51 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS=

lib_LTLIBRARIES                      = libwatchdog-server-shelly.la
libwatchdog_server_shelly_la_SOURCES = Shelly.c Shelly.h

libwatchdog_server_shelly_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0 libmosquitto` \
                                       -rdynamic

libwatchdog_server_shelly_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0 libmosquitto`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
