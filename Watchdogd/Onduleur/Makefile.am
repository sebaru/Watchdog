#**********************************************************************************************************/
#* Watchdogd/Onduleur/Makefile.am          Profil de compilation de l'interface Onduleur Watchdog         */
#* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 10 nov. 2009 15:55:22 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-ups.la
libwatchdog_server_ups_la_SOURCES = Onduleur.c admin_onduleur.c Onduleur.h

INCLUDES= `pkg-config --cflags glib-2.0 libupsclient` \
          -I$(includedir) \
          -I$(top_srcdir)/Include \
	  $(MYSQL_CFLAGS) \
	  -I../Include

libwatchdog_server_ups_la_LDFLAGS = -rdynamic -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION)
