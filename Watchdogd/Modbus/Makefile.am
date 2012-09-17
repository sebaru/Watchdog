#**********************************************************************************************************/
#* Watchdogd/Modbus/Makefile.am          Profil de compilation de l'interface Modbus Watchdog             */
#* Projet WatchDog version 2.0       Gestion d'habitat                      mar 13 jui 2007 15:48:08 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-modbus.la
libwatchdog_server_modbus_la_SOURCES    = Modbus.c Modbus.h admin_modbus.c

INCLUDES= `pkg-config --cflags glib-2.0` \
          -I$(top_srcdir)/Include \
	  $(MYSQL_CFLAGS) \
	  -I../Include

libwatchdog_server_modbus_la_LDFLAGS = -rdynamic -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION) \
	                               `pkg-config --libs glib-2.0`

