#**********************************************************************************************************/
#* Watchdogd/Dls/Makefile.am          Profil de compilation de l'interface DLS Watchdog                   */
#* Projet WatchDog version 2.0       Gestion d'habitat                      ven 01 aoû 2003 11:37:48 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

noinst_LTLIBRARIES   = libdls.la
libdls_la_SOURCES    = plugins.c \
                       heure.c \
		       Dls_db.c \
                       The_dls.c

INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
	  $(MYSQL_CFLAGS) \
          $(GLIB_CFLAGS)
