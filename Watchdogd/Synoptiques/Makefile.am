#**********************************************************************************************************/
#* Commun/Message/Makefile.am          Profil de compilation de la librairies de gestion message          */
#* Projet WatchDog version 1.7       Gestion d'habitat                       lun 28 fév 2005 09:22:39 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = libsyn.la
libsyn_la_SOURCES = Synoptiques.c motifs.c comment.c passerelle.c palette.c capteur.c camera_sup.c

libsyn_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0` \
                    `mysql_config --libs`

libsyn_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0` \
                   `mysql_config --cflags`

INCLUDES=-I$(top_srcdir)/Include \
         -I../Include

