#**********************************************************************************************************/
#* Watchdogd/Asterisk/Makefile.am          Profil de compilation de la librairies de gestion Asterisk     */
#* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 28 mai. 2011 18:53:00 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = libasterisk.la
libasterisk_la_SOURCES = Asterisk.c

INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
	  $(MYSQL_CFLAGS) \
          $(GLIB_CFLAGS)

