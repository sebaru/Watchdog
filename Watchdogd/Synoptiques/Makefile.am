#**********************************************************************************************************/
#* Commun/Message/Makefile.am          Profil de compilation de la librairies de gestion message          */
#* Projet WatchDog version 1.7       Gestion d'habitat                       lun 28 fév 2005 09:22:39 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = libwatchdog-syn.la
libwatchdog_syn_la_SOURCES = Synoptiques.c motifs.c comment.c passerelle.c palette.c capteur.c camera_sup.c

libwatchdog_syn_la_LDFLAGS =  -rdynamic -export-dynamic

INCLUDES= `pkg-config --cflags glib-2.0` \
	  `mysql_config --cflags` \
          -I$(top_srcdir)/Include \
 	  -I../Include
