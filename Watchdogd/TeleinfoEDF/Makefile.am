#**********************************************************************************************************/
#* Watchdogd/TeleinfoEDF/Makefile.am     Profil de compilation de l'interface RFXCOM pour Watchdog     */
#* Projet Abls-Habitat version 4.4       Gestion d'habitat                       dim 25 mai 2012 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS=

lib_LTLIBRARIES                         = libwatchdog-server-teleinfoedf.la
libwatchdog_server_teleinfoedf_la_SOURCES    = Teleinfo.c Teleinfo.h

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include

libwatchdog_server_teleinfoedf_la_LDFLAGS = -rdynamic \
                                            `pkg-config --libs glib-2.0 json-glib-1.0`


libwatchdog_server_teleinfoedf_la_CFLAGS = `pkg-config --cflags glib-2.0 json-glib-1.0`

