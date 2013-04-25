#**********************************************************************************************************/
#* Watchdogd/HttpMobile/Makefile.am          Profil de compilation de l'interface Mobile HTTP Watchdog    */
#* Projet WatchDog version 2.0       Gestion d'habitat                           mer 24 avr 2013 13:46:15 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                      = libwatchdog-server-httpmobile.la
libwatchdog_server_httpmobile_la_SOURCES    = HttpMobile.c HttpMobile.h admin_httpmobile.c 

libwatchdog_server_httpmobile_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 libmicrohttpd` \
                                       `mysql_config --libs` \
                                       -rdynamic \
                                       -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_httpmobile_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 libmicrohttpd` \
                                          `mysql_config --cflags`

INCLUDES=-I$(top_srcdir)/Include \
         -I../Include
