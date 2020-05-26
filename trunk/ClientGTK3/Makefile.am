#**********************************************************************************************************/
#* Client/Makefile.am                                     Profil de compilation du client Watchdog        */
#* Projet WatchDog version 3.0       Gestion d'habitat                       sam 13 mar 2004 17:53:37 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS = Include

gsettings_SCHEMAS = fr.abls-habitat.watchdog.gschema.xml
EXTRA_DIST = $(gsettings_SCHEMAS)

@GSETTINGS_RULES@

desktopdir = $(datadir)/applications
desktop_DATA = fr.abls-habitat.watchdog.desktop

appicondir=$(datadir)/icons/hicolor/scalable/apps
appicon_DATA=fr.abls-habitat.watchdog.svg

bin_PROGRAMS = Watchdog-client-gtk3

ClientResources.c : ClientResources.xml $(shell glib-compile-resources --generate-dependencies ClientResources.xml)
	glib-compile-resources --target=ClientResources.c --generate-source $<
	glib-compile-resources --target=ClientResources.h --generate-header $<

Watchdog_client_gtk3_SOURCES = Watchdog-client.c ClientResources.c ClientResources.h \
                               ihm.c connect.c liste_histo.c json.c \
                               trame.c supervision.c supervision_passerelle.c supervision_comment.c supervision_camera.c \
                               supervision_cadran.c timer.c \
                               liste_plugin_dls.c \
                               liste_synoptique.c \
                               atelier.c atelier_motif.c atelier_passerelle.c atelier_cadran.c atelier_camera.c atelier_comment.c \
                               atelier_clic_trame.c atelier_selection.c atelier_agrandir.c atelier_propriete_motif.c
#                          protocole.c protocole_gtk_message.c \
#                          protocole_dls.c protocole_mnemonique.c \
#                          protocole_connexion.c \
#                          protocole_supervision.c protocole_histo.c \
#                          protocole_atelier.c \
#			  protocole_synoptique.c protocole_admin.c \
#                          menu.c timer.c admin.c \
#			  liste_plugin_dls.c ajout_plugin_dls.c edit_source_dls.c \
#			  liste_synoptique.c ajout_synoptique.c \
#			  liste_mnemonique.c ajout_mnemonique.c option_entreeana.c option_cpt_imp.c \
#			  option_registre.c \
#                          liste_camera.c ajout_camera.c protocole_lowlevel.c \
#			  atelier.c atelier_clic_trame.c atelier_selection.c \
#			  atelier_agrandir.c atelier_propriete_motif.c atelier_ajout_motif.c \
#			  atelier_ajout_comment.c \
#                          atelier_ajout_passerelle.c \
#                          atelier_ajout_palette.c \
#			  atelier_ajout_cadran.c atelier_ajout_camera_sup.c \
#			  supervision.c supervision_clic.c supervision_comment.c \
#                          supervision_passerelle.c supervision_palette.c \
#                          supervision_cadran.c \
#			  supervision_camera.c \
#			  trame.c \
#			  print.c

Watchdog_client_gtk3_LDADD  = $(top_builddir)/Commun/libwatchdog.la

Watchdog_client_gtk3_LDFLAGS = `pkg-config --libs gtk+-3.0 goocanvas-2.0 libsoup-2.4 glib-2.0 json-glib-1.0` -lm

Watchdog_client_gtk3_CFLAGS = `pkg-config --cflags gtk+-3.0 goocanvas-2.0 libsoup-2.4 glib-2.0 json-glib-1.0`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/ClientGTK3/Include

MAINTAINERCLEANFILES = Makefile.in
