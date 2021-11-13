#**********************************************************************************************************/
#* Watchdogd/Lirc/Makefile.am          Profil de compilation de l'interface Lirc pour Watchdog            */
#* Projet WatchDog version 2.0       Gestion d'habitat                           mar 01 mar 2011 20:00:00 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-lirc.la
libwatchdog_server_lirc_la_SOURCES    = Lirc.c Lirc.h

AM_CPPFLAGS= -I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include

libwatchdog_server_lirc_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0` \
                                    `mysql_config --cflags`

libwatchdog_server_lirc_la_LDFLAGS = -llirc_client -rdynamic \
                                     `pkg-config --libs glib-2.0` \
                                     `mysql_config --libs`
