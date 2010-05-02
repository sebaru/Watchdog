#**********************************************************************************************************/
#* Client/Makefile.am                                     Profil de compilation du client Watchdog        */
#* Projet WatchDog version 2.0       Gestion d'habitat                       sam 13 mar 2004 17:53:37 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS = Include Config_cli

bin_PROGRAMS = Watchdog-client
Watchdog_client_SOURCES = Watchdog-client.c connect.c ihm.c \
                          protocole.c protocole_gtk_message.c \
                          protocole_utilisateur.c \
                          protocole_connexion.c protocole_histo_courbe.c \
                          protocole_supervision.c protocole_histo.c \
                          protocole_courbe.c protocole_fichier.c \
			  ssl.c menu.c ident.c timer.c \
			  liste_groupe.c ajout_groupe.c \
			  liste_util.c ajout_util.c \
			  liste_histo.c liste_histo_hard.c \
			  supervision.c supervision_clic.c supervision_comment.c \
                          supervision_passerelle.c supervision_palette.c \
                          supervision_capteur.c supervision_scenario.c \
			  supervision_camera_page.c supervision_camera.c \
			  print.c courbe.c histo_courbe.c

Watchdog_client_LDADD  = $(top_srcdir)/Commun/libwatchdog.la \
			 $(top_srcdir)/CommunClient/libwatchdogclient.la \
                         Config_cli/libconfig_cli.la \
                         $(GNOME2_LIBS) \
			 -lgtkdatabox \
			 -lvlc-gtk
#                         -lungif

INCLUDES=$(GNOME_INCLUDEDIR) $(GNOME2_CFLAGS) -IInclude -I$(top_srcdir)/Include -I$(top_srcdir)/IncludeClient

MAINTAINERCLEANFILES = Makefile.in
