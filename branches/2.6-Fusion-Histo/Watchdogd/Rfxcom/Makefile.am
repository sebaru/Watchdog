#**********************************************************************************************************/
#* Watchdogd/Rfxcom/Makefile.am          Profil de compilation de l'interface RFXCOM pour Watchdog        */
#* Projet WatchDog version 2.0       Gestion d'habitat                           dim 25 mai 2012 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                         = libwatchdog-server-rfxcom.la
libwatchdog_server_rfxcom_la_SOURCES    = Rfxcom.c Rfxcom.h admin_rfxcom.c

AM_CPPFLAGS= `pkg-config --cflags glib-2.0` \
          `mysql_config --cflags` \
          -I$(top_srcdir)/Include \
          -I../Include

libwatchdog_server_rfxcom_la_LDFLAGS = -rdynamic -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION) \
                                      `pkg-config --libs glib-2.0`
