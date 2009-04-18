#**********************************************************************************************************/
#* Commun/Message/Makefile.am          Profil de compilation de la librairies de gestion message          */
#* Projet WatchDog version 1.7       Gestion d'habitat                      ven 30 mai 2003 10:51:35 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = libeana.la
libeana_la_SOURCES = EntreeANA.c

INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
          $(MYSQL_CFLAGS) \
	  $(GLIB_CFLAGS)
