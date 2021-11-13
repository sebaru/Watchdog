#**********************************************************************************************************/
#* Watchdogd/Imsg/Makefile.am          Profil de compilation de l'interface Instant-Messagingpour Watchdog*/
#* Projet WatchDog version 2.0       Gestion d'habitat                          sam 28 juil 2012 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                         = libwatchdog-server-imsg.la
libwatchdog_server_imsg_la_SOURCES    = Imsg.c Imsg.h admin_imsg.c 

libwatchdog_server_imsg_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 loudmouth-1.0` \
                                     `mysql_config --libs` \
                                     -rdynamic \
                                     -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_imsg_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 loudmouth-1.0` \
                                      `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
