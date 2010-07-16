#**********************************************************************************************************/
#* Watchdogd/Rs485/Makefile.am          Profil de compilation de l'interface Rs485 Watchdog               */
#* Projet WatchDog version 2.0       Gestion d'habitat                      mar 22 jun 2004 15:48:08 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

noinst_LTLIBRARIES   = libsms.la
libsms_la_SOURCES    = Sms.c

INCLUDES= $(GNOME2_CFLAGS) \
	  $(MYSQL_CFLAGS) \
	  $(CURL_CFLAGS) \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include
