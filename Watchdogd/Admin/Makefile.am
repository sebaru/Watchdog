#**********************************************************************************************************/
#* Watchdogd/Admin/Makefile.am          Profil de compilation de l'interface d'admin Watchdog             */
#* Projet WatchDog version 2.0       Gestion d'habitat                      mar 11 avr 2009 15:48:08 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

noinst_LTLIBRARIES     = libadmin.la
libadmin_la_SOURCES    = Admin.c admin_running.c admin_modbus.c admin_process.c admin_dls.c \
			 admin_tellstick.c admin_set.c admin_get.c

INCLUDES= $(GNOME2_CFLAGS) \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  $(MYSQL_CFLAGS) \
	  -I../Include
