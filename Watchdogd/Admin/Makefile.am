#**********************************************************************************************************/
#* Watchdogd/Admin/Makefile.am          Profil de compilation de l'interface d'admin Watchdog             */
#* Projet WatchDog version 2.0       Gestion d'habitat                      mar 11 avr 2009 15:48:08 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

noinst_LTLIBRARIES     = libadmin.la
libadmin_la_SOURCES    = Admin.c admin_running.c admin_process.c admin_dls.c \
			 admin_set.c admin_get.c

INCLUDES= `pkg-config --cflags glib-2.0` \
          -I$(top_srcdir)/Include \
	  $(MYSQL_CFLAGS) \
	  -I../Include
