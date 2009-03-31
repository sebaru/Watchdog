#**********************************************************************************************************/
#* Watchdogd/Modbus/Makefile.am          Profil de compilation de l'interface Modbus Watchdog             */
#* Projet WatchDog version 2.0       Gestion d'habitat                      mar 13 jui 2007 15:48:08 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

noinst_LTLIBRARIES     = libmodbus.la
libmodbus_la_SOURCES    = Modbus.c

INCLUDES= $(GNOME2_CFLAGS) \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include
