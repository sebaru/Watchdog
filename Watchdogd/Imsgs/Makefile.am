#**********************************************************************************************************/
#* Watchdogd/Imsg/Makefile.am          Profil de compilation de l'interface Instant-Messagingpour Watchdog*/
#* Projet Abls-Habitat version 4.0       Gestion d'habitat                      sam 28 juil 2012 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS=

lib_LTLIBRARIES                     = libwatchdog-server-imsgs.la
libwatchdog_server_imsgs_la_SOURCES = Imsg.c Imsg.h

libwatchdog_server_imsgs_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0` -lstrophe \
                                      -rdynamic \
                                      -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_imsgs_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
