#**********************************************************************************************************/
#* Client/Makefile.am                                     Profil de compilation du client Watchdog        */
#* Projet WatchDog version 2.0       Gestion d'habitat                       sam 13 mar 2004 17:53:37 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS = Include Config_cli

bin_PROGRAMS = Watchdog-client
Watchdog_client_SOURCES = Watchdog-client.c connect.c ihm.c \
                          protocole.c protocole_gtk_message.c protocole_icone.c \
                          protocole_dls.c protocole_utilisateur.c protocole_mnemonique.c \
                          protocole_message.c protocole_connexion.c protocole_histo_courbe.c \
                          protocole_supervision.c protocole_histo.c \
                          protocole_atelier.c protocole_courbe.c \
			  protocole_synoptique.c protocole_admin.c \
                          menu.c timer.c admin.c \
			  liste_groupe.c ajout_groupe.c \
			  liste_util.c ajout_util.c \
			  liste_plugin_dls.c ajout_plugin_dls.c edit_source_dls.c \
			  liste_histo.c liste_histo_msgs.c \
			  liste_message.c ajout_message.c \
			  liste_synoptique.c ajout_synoptique.c \
			  liste_mnemonique.c ajout_mnemonique.c option_entreeana.c option_cpt_imp.c \
			  liste_icone.c ajout_icone.c liste_icone.h option_entreetor.c option_tempo.c \
                          liste_camera.c ajout_camera.c protocole_camera.c \
			  atelier.c atelier_clic_trame.c atelier_selection.c \
			  atelier_agrandir.c atelier_propriete_motif.c atelier_ajout_motif.c \
			  atelier_ajout_comment.c \
                          atelier_ajout_passerelle.c atelier_propriete_passerelle.c \
                          atelier_ajout_palette.c \
			  atelier_ajout_capteur.c atelier_ajout_camera_sup.c \
			  supervision.c supervision_clic.c supervision_comment.c \
                          supervision_passerelle.c supervision_palette.c \
                          supervision_capteur.c \
			  supervision_camera.c \
			  trame.c \
			  print.c \
                          courbe.c histo_courbe.c

Watchdog_client_LDADD  = $(top_builddir)/Commun/libwatchdog.la \
                         Config_cli/libconfig_cli.la

Watchdog_client_LDFLAGS = `pkg-config --libs libgnomeui-2.0 libgnome-2.0 openssl glib-2.0 goocanvas gtksourceview-2.0 gtkdatabox libcurl` -lm

Watchdog_client_CFLAGS = `pkg-config --cflags libgnomeui-2.0 libgnome-2.0 openssl glib-2.0 goocanvas gtksourceview-2.0 gtkdatabox libcurl`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Client/Include

MAINTAINERCLEANFILES = Makefile.in
