#**********************************************************************************************************/
#* Watchdogd/Sms/Makefile.am          Profil de compilation de l'interface SMS Watchdog                   */
#* Projet WatchDog version 2.0       Gestion d'habitat                          mer 05 sept 2012 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                      = libwatchdog-server-sms.la
libwatchdog_server_sms_la_SOURCES    = Sms.c Sms.h admin_sms.c 

INCLUDES= `pkg-config --cflags glib-2.0 libcurl` \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          $(MYSQL_CFLAGS) \
          -I../Include

libwatchdog_server_sms_la_LDFLAGS = -rdynamic -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION) `pkg-config --libs glib-2.0 libcurl`
