#**********************************************************************************************************/
#* Watchdogd/Voice/Makefile.am          Profil de compilation de l'interface SMS Watchdog                 */
#* Projet WatchDog version 2.0       Gestion d'habitat                                29/12/2018 22:04:04 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                      = libwatchdog-server-voice.la
libwatchdog_server_voice_la_SOURCES    = Voice.c Voice.h admin_voice.c 

libwatchdog_server_voice_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0` \
                                      `mysql_config --libs` \
                                      -rdynamic \
                                      -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_voice_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0` \
                                     `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
