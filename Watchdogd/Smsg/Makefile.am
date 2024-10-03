#**********************************************************************************************************/
#* Watchdogd/Sms/Makefile.am          Profil de compilation de l'interface SMS Watchdog                   */
#* Projet Abls-Habitat version 4.0       Gestion d'habitat                      mer 05 sept 2012 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS=

lib_LTLIBRARIES                      = libwatchdog-server-smsg.la
libwatchdog_server_smsg_la_SOURCES   = Sms.c Sms.h

libwatchdog_server_smsg_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 gammu json-glib-1.0 libsoup-3.0` \
                                    -rdynamic \
                                    -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_smsg_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 gammu json-glib-1.0 libsoup-3.0`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
