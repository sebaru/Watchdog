#**********************************************************************************************************/
#* Watchdogd/Imsg/Makefile.am          Profil de compilation de l'interface Instant-Messagingpour Watchdog*/
#* Projet WatchDog version 2.0       Gestion d'habitat                          sam 28 juil 2012 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                         = libwatchdog-server-imsg.la
libwatchdog_server_imsg_la_SOURCES    = Imsg.c Imsg.h admin_imsg.c 

INCLUDES= `pkg-config --cflags glib-2.0 loudmouth-1.0` \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          $(MYSQL_CFLAGS) \
          -I../Include

libwatchdog_server_imsg_la_LDFLAGS = -rdynamic -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION) `pkg-config --libs glib-2.0 loudmouth-1.0`
