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
                          protocole_entreeana.c protocole_supervision.c protocole_histo.c \
                          protocole_atelier.c protocole_courbe.c protocole_fichier.c \
			  protocole_synoptique.c \
                          ssl.c menu.c ident.c timer.c \
			  liste_groupe.c ajout_groupe.c \
			  liste_util.c ajout_util.c \
			  liste_plugin_dls.c ajout_plugin_dls.c edit_source_dls.c \
			  liste_histo.c liste_histo_hard.c \
			  liste_message.c ajout_message.c \
			  liste_synoptique.c ajout_synoptique.c \
			  liste_mnemonique.c ajout_mnemonique.c \
			  liste_icone.c ajout_icone.c \
			  liste_entreeana.c ajout_entreeana.c \
                          liste_camera.c ajout_camera.c protocole_camera.c \
			  liste_scenario.c ajout_scenario.c protocole_scenario.c \
			  liste_onduleur.c ajout_onduleur.c protocole_onduleur.c \
			  liste_rs485.c ajout_rs485.c protocole_rs485.c \
			  liste_modbus.c ajout_modbus.c protocole_modbus.c \
			  atelier.c atelier_clic_trame.c atelier_selection.c \
			  atelier_agrandir.c atelier_propriete_motif.c atelier_ajout_motif.c \
			  atelier_ajout_comment.c \
                          atelier_ajout_passerelle.c atelier_propriete_passerelle.c \
                          atelier_ajout_palette.c \
			  atelier_ajout_capteur.c atelier_ajout_camera_sup.c \
			  supervision.c supervision_clic.c supervision_comment.c \
                          supervision_passerelle.c supervision_palette.c \
                          supervision_capteur.c supervision_scenario.c \
			  supervision_camera.c \
			  trame.c \
			  print.c \
                          courbe.c histo_courbe.c

Watchdog_client_LDADD  = $(top_srcdir)/Commun/libwatchdog.la \
                         Config_cli/libconfig_cli.la \
                         $(GNOME2_LIBS) \
			 -lgtkdatabox

INCLUDES=$(GNOME_INCLUDEDIR) -I$(top_srcdir)/Include \
                             -IInclude \
                             $(GNOME2_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
