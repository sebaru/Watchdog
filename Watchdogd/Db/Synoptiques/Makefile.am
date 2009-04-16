#**********************************************************************************************************/
#* Commun/Message/Makefile.am          Profil de compilation de la librairies de gestion message          */
#* Projet WatchDog version 1.7       Gestion d'habitat                       lun 28 fév 2005 09:22:39 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = libsyn.la
libsyn_la_SOURCES = Synoptiques.c motifs.c comment.c passerelle.c palette.c capteur.c

INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../../Include \
          $(MYSQL_CFLAGS) \
	  $(GNOME2_CFLAGS)
