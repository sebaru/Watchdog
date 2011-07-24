#**********************************************************************************************************/
#* Watchdogd/Makefile.am          Profil de compilation du serveur Watchdog                               */
#* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:24:24 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS = 

bin_PROGRAMS = WatchdogdAdmin
WatchdogdAdmin_SOURCES = WatchdogdAdmin.c

INCLUDES=-I$(top_srcdir)/Include \
	 -IInclude/ \
	 $(GLIB_CFLAGS)

WatchdogdAdmin_LDFLAGS = -lpopt -lreadline $(GLIB_LIBS)

MAINTAINERCLEANFILES = Makefile.in
