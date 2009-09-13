#**********************************************************************************************************/
#* Watchdogd/Serveur/Makefile.am          Profil de compilation d'un sous serveur                         */
#* Projet WatchDog version 2.1       Gestion d'habitat                       dim 14 mar 2004 18:54:28 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

noinst_LTLIBRARIES   = libserveur.la
libserveur_la_SOURCES = Serveur.c protocole.c ident.c envoi.c \
                        protocole_utilisateur.c protocole_dls.c protocole_message.c \
                        protocole_entreeana.c protocole_entreeana.c protocole_histo_courbe.c \
                        protocole_mnemonique.c protocole_supervision.c protocole_atelier.c \
                        protocole_icone.c protocole_histo.c protocole_courbe.c  \
			protocole_scenario.c protocole_synoptique.c protocole_camera.c \
                        envoi_groupe.c envoi_util.c envoi_dls.c envoi_histo.c envoi_message.c \
                        envoi_synoptique.c envoi_synoptique_motifs.c envoi_synoptique_palettes.c \
                        envoi_synoptique_comments.c envoi_synoptique_passerelles.c \
 		        envoi_synoptique_capteur.c envoi_synoptique_camera_sup.c envoi_camera.c \
                        envoi_bit_init.c envoi_scenario.c envoi_synoptique_scenario.c \
                        envoi_mnemonique.c envoi_classe.c envoi_icone.c \
			envoi_entreeana.c envoi_courbe.c envoi_histo_courbe.c \
                        accept.c liste.c envoi_histo_hard.c \
                        distrib_cde_dls.c capteur.c


INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
          $(MYSQL_CFLAGS) \
	  $(GNOME2_CFLAGS)
