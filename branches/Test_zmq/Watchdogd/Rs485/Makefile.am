#**********************************************************************************************************/
#* Watchdogd/Rs485/Makefile.am          Profil de compilation de l'interface RS485 pour Watchdog        */
#* Projet WatchDog version 2.0       Gestion d'habitat                           dim 25 mai 2012 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-rs485.la
libwatchdog_server_rs485_la_SOURCES    = Rs485.c Rs485.h admin_rs485.c

libwatchdog_server_rs485_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0` \
                                      `mysql_config --libs` \
                                      -rdynamic \
                                      -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_rs485_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0` \
                                     `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
