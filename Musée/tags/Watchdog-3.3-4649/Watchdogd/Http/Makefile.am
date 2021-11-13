#**********************************************************************************************************/
#* Watchdogd/Http/Makefile.am          Profil de compilation de l'interface Mobile HTTP Watchdog          */
#* Projet WatchDog version 3.0       Gestion d'habitat                           mer 24 avr 2013 13:46:15 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS=

lib_LTLIBRARIES                      = libwatchdog-server-http.la
libwatchdog_server_http_la_SOURCES    = Http.c Http.h admin_http.c getstatus.c bus.c \
                                        ws_motifs.c ws_histo_msgs.c memory.c getprocess.c getdls.c getsyn.c \
                                        getarchive.c getusers.c getmnemos.c getinstance.c

libwatchdog_server_http_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0 libsoup-2.4 uuid` \
                                       `mysql_config --libs` -lcrypt \
                                       -rdynamic \
                                       -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_http_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0 libsoup-2.4 uuid` \
                                          `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
