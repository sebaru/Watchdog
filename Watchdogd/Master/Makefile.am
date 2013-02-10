#**********************************************************************************************************/
#* Watchdogd/Master/Makefile.am          Profil de compilation de l'interface Master de Watchdog          */
#* Projet WatchDog version 2.0       Gestion d'habitat                           sam 02 fév 2013 13:46:15 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                      = libwatchdog-server-master.la
libwatchdog_server_master_la_SOURCES    = Master.c Master.h admin_master.c 

libwatchdog_server_master_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0` \
                                       `mysql_config --libs` \
                                       -rdynamic \
                                       -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_master_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0` \
                                      `mysql_config --cflags`

INCLUDES=-I$(top_srcdir)/Include \
         -I../Include
