#**********************************************************************************************************/
#* Watchdogd/Rs485/Makefile.am          Profil de compilation de l'interface Rs485 Watchdog               */
#* Projet WatchDog version 2.0       Gestion d'habitat                      mar 22 jun 2004 15:48:08 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

noinst_LTLIBRARIES     = librs485.la
librs485_la_SOURCES    = Rs485.c

INCLUDES= $(GNOME2_CFLAGS) \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
          $(MYSQL_CFLAGS)
