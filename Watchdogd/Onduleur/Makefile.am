#**********************************************************************************************************/
#* Watchdogd/Onduleur/Makefile.am          Profil de compilation de l'interface Onduleur Watchdog         */
#* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 10 nov. 2009 15:55:22 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

noinst_LTLIBRARIES     = libonduleur.la
libonduleur_la_SOURCES = Onduleur.c

INCLUDES= $(GNOME2_CFLAGS) \
	  $(UPS_CFLAGS) \
          -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  $(MYSQL_CFLAGS) \
	  -I../Include
