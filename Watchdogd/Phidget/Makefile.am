#**********************************************************************************************************/
#* Watchdogd/Phidget/Makefile.am          Profil de compilation de l'interface Phidget de Watchdog        */
#* Projet WatchDog version 3.0       Gestion d'habitat                      jeu 18 mar 2021 21:56:00 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-phidget.la
libwatchdog_server_phidget_la_SOURCES = Phidget.c Phidget.h admin_phidget.c

libwatchdog_server_phidget_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0` \
                                        `mysql_config --libs` -lphidget22 \
                                        -rdynamic
                                      
libwatchdog_server_phidget_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0` \
                                       `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
