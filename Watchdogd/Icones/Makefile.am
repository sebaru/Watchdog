#**********************************************************************************************************/
#* Watchdogd/Db/Makefile.am          Profil de compilation de la librairies de gestion icone              */
#* Projet WatchDog version 2.0       Gestion d'habitat                      ven 30 mai 2003 10:51:35 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = libico.la
libico_la_SOURCES = Icones.c classe.c

INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
          $(MYSQL_CFLAGS) \
	  $(GNOME2_CFLAGS)
