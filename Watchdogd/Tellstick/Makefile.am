#**********************************************************************************************************/
#* Watchdogd/Tellstick/Makefile.am          Profil de compilation de l'interface Tellstick pour Watchdog  */
#* Projet WatchDog version 2.0       Gestion d'habitat                           jeu 17 fév 2011 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-tellstick.la
libwatchdog_tellstick_la_SOURCES    = Tellstick.c

INCLUDES= $(GNOME2_CFLAGS) \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
          $(MYSQL_CFLAGS) \
          -I../Include

libwatchdog_tellstick_la_LDFLAGS = -rdynamic -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION)
