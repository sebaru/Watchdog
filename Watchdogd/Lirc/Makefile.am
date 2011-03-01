#**********************************************************************************************************/
#* Watchdogd/Lirc/Makefile.am          Profil de compilation de l'interface Lirc pour Watchdog            */
#* Projet WatchDog version 2.0       Gestion d'habitat                           mar 01 mar 2011 20:00:00 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-lirc.la
libwatchdog_lirc_la_SOURCES    = Lirc.c

INCLUDES= $(GNOME2_CFLAGS) \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
          $(MYSQL_CFLAGS) \
	  $(LIRC_CFLAGS) \
          -I../Include

libwatchdog_lirc_la_LDFLAGS = -rdynamic -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION) $(LIRC_LIBS)
