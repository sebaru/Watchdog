#**********************************************************************************************************/
#* Watchdogd/Rfxcom/Makefile.am          Profil de compilation de l'interface RFXCOM pour Watchdog        */
#* Projet WatchDog version 3.0       Gestion d'habitat                           dim 25 mai 2012 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                         = libwatchdog-server-rfxcom.la
libwatchdog_server_rfxcom_la_SOURCES    = Rfxcom.c Rfxcom.h admin_rfxcom.c

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include

libwatchdog_server_rfxcom_la_CFLAGS = `pkg-config --cflags glib-2.0 json-glib-1.0` \
                                      `mysql_config --cflags`
libwatchdog_server_rfxcom_la_LDFLAGS = -rdynamic \
                                       `pkg-config --libs glib-2.0 json-glib-1.0` \
                                       `mysql_config --libs`
