#**********************************************************************************************************/
#* Watchdogd/Serveur/Makefile.am          Profil de compilation d'un sous serveur                         */
#* Projet WatchDog version 2.6       Gestion d'habitat                       dim 31 mar 2013 18:54:28 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

lib_LTLIBRARIES     = libwatchdog-server-ssrv.la
libwatchdog_server_ssrv_la_SOURCES    = Serveur.c protocole.c ident.c envoi.c Utilisateur.c \
                        protocole_dls.c \
                        protocole_admin.c \
                        protocole_mnemonique.c protocole_supervision.c protocole_atelier.c \
                        protocole_histo.c \
                        protocole_synoptique.c protocole_lowlevel.c \
                        envoi_dls.c envoi_histo.c \
                        envoi_synoptique.c envoi_synoptique_motifs.c envoi_synoptique_palettes.c \
                        envoi_synoptique_comments.c envoi_synoptique_passerelles.c \
                        envoi_synoptique_cadran.c envoi_synoptique_camera_sup.c envoi_camera.c \
                        envoi_mnemonique.c \
                        accept.c \
                        cadran.c ecoute.c ssl.c handle_client.c \
                        Sous_serveur.h Utilisateur_DB.h Client.h admin_serveur.c

libwatchdog_server_ssrv_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0` \
                                      `mysql_config --libs` \
                                      -rdynamic \
                                      -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_server_ssrv_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0` \
                                     `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
