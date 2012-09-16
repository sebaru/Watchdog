#**********************************************************************************************************/
#* Watchdogd/Tellstick/Makefile.am          Profil de compilation de l'interface Tellstick pour Watchdog  */
#* Projet WatchDog version 2.0       Gestion d'habitat                           jeu 17 fév 2011 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server_tellstick.la
libwatchdog_server_tellstick_la_SOURCES    = Tellstick.c Tellstick.h admin_tellstick.c

INCLUDES= `pkg-config --cflags glib-2.0` \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          $(MYSQL_CFLAGS) \
          -I../Include

libwatchdog_tellstick_la_LDFLAGS = -ltelldus-core -rdynamic \
                                   -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION) \
                                   `pkg-config --libs glib-2.0`
