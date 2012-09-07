#**********************************************************************************************************/
#* Watchdogd/Lirc/Makefile.am          Profil de compilation de l'interface Lirc pour Watchdog            */
#* Projet WatchDog version 2.0       Gestion d'habitat                           mar 01 mar 2011 20:00:00 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-lirc.la
libwatchdog_server_lirc_la_SOURCES    = Lirc.c Lirc.h

INCLUDES= -I$(top_srcdir)/Include -I../Include \
          `mysql_config --libs` \
	  `pkg-config --cflags liblircclient0 glib-2.0` \
	  `mysql_config --cflags`

libwatchdog_server_lirc_la_LDFLAGS = -rdynamic `pkg-config --libs liblircclient0 glib-2.0` \
				     -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION)
