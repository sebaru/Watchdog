#**********************************************************************************************************/
#* Watchdogd/Rs485/Makefile.am          Profil de compilation de l'interface RS485 pour Watchdog        */
#* Projet WatchDog version 2.0       Gestion d'habitat                           dim 25 mai 2012 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-rs485.la
libwatchdog_server_rs485_la_SOURCES    = Rs485.c Rs485.h admin_rs485.c

INCLUDES= $(GNOME2_CFLAGS) \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          $(MYSQL_CFLAGS) \
          -I../Include

libwatchdog_server_rs485_la_LDFLAGS = -rdynamic -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION)
