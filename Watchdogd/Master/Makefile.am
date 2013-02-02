#**********************************************************************************************************/
#* Watchdogd/Master/Makefile.am          Profil de compilation de l'interface Master de Watchdog          */
#* Projet WatchDog version 2.0       Gestion d'habitat                           sam 02 fév 2013 13:46:15 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                      = libwatchdog-server-master.la
libwatchdog_server_master_la_SOURCES    = Master.c Master.h admin_master.c 

INCLUDES= `pkg-config --cflags glib-2.0` \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          $(MYSQL_CFLAGS) \
          -I../Include

libwatchdog_server_master_la_LDFLAGS = -rdynamic -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION) \
	                             `pkg-config --libs glib-2.0`
