#**********************************************************************************************************/
#* Watchdogd/Db/CptHoraire/Makefile.am      Profil de compilation de la librairies de gestion CPT_H       */
#* Projet WatchDog version 1.7       Gestion d'habitat                       mar 14 fév 2006 15:02:34 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = libcompteur.la
libcompteur_la_SOURCES = Compteur_horaire.c Compteur_impulsion.c

INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
          $(MYSQL_CFLAGS) \
	  $(GLIB_CFLAGS)
