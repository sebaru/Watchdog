#**********************************************************************************************************/
#* Watchdogd/Makefile.am          Profil de compilation du serveur Watchdog                               */
#* Projet WatchDog version 3.0       Gestion d'habitat                      lun 02 jun 2003 14:24:24 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS = 

bin_PROGRAMS = WatchdogdAdmin
WatchdogdAdmin_SOURCES = WatchdogdAdmin.c
WatchdogdAdmin_LDADD   = $(top_builddir)/Commun/libwatchdog.la

WatchdogdAdmin_CFLAGS = `pkg-config --cflags glib-2.0 libczmq` 

AM_CPPFLAGS=-I$(top_srcdir)/Include \
	    -I$(top_srcdir)/Watchdogd/Include

WatchdogdAdmin_LDFLAGS = -lreadline `pkg-config --libs glib-2.0 libczmq popt`

MAINTAINERCLEANFILES = Makefile.in
