#**********************************************************************************************************/
#* Watchdogd/Rs485/Makefile.am          Profil de compilation de l'interface Rs485 Watchdog               */
#* Projet WatchDog version 3.0       Gestion d'habitat                      mar 22 jun 2004 15:48:08 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

noinst_LTLIBRARIES     = libarchive.la
libarchive_la_SOURCES    = Archive.c archiveDB.c admin_arch.c

AM_CPPFLAGS= `pkg-config --cflags glib-2.0 json-glib-1.0` \
             `mysql_config --cflags` \
            -I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
