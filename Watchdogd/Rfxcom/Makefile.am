#**********************************************************************************************************/
#* Watchdogd/Rfxcom/Makefile.am          Profil de compilation de l'interface RFXCOM pour Watchdog        */
#* Projet WatchDog version 2.0       Gestion d'habitat                           dim 25 mai 2012 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                         = libwatchdog-server-rfxcom.la
libwatchdog_server_rfxcom_la_SOURCES    = Rfxcom.c admin_rfxcom.c

INCLUDES= $(GNOME2_CFLAGS) \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
          $(MYSQL_CFLAGS) \
          -I../Include

libwatchdog_server_rfxcom_la_LDFLAGS = -rdynamic -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION)
