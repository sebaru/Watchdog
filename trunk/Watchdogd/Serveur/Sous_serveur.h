/******************************************************************************************************************************/
/* Watchdogd/Include/Sous_serveur.h      Définition des prototypes du serveur watchdog                                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          mar 03 jun 2003 10:39:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Sous_serveur.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 #ifndef _PROTO_SRV_H_
 #define _PROTO_SRV_H_
 #include <openssl/rsa.h>
 #include <openssl/ssl.h>
 #include <openssl/err.h>

 #include "Client.h"

 #define NOM_THREAD                    "ssrv"
 #define SSRV_DEFAUT_PORT              5558
 #define SSRV_DEFAUT_SSL_NEEDED        FALSE
 #define SSRV_DEFAUT_SSL_PEER_CERT     FALSE
 #define SSRV_DEFAUT_NETWORK_BUFFER    16384
 #define SSRV_DEFAUT_FILE_CA           "cacert.pem"
 #define SSRV_DEFAUT_FILE_CERT         "server.crt"
 #define SSRV_DEFAUT_FILE_KEY          "serverkey.pem"

 struct SSRV_CONFIG
  { struct LIBRAIRIE *lib;
    gint Socket_ecoute;                                                          /* Socket de connexion (d'écoute) du serveur */
    SSL_CTX *Ssl_ctx;                                                                  /* Contexte de cryptage des connexions */
    X509 *ssrv_certif; 
    gint  port;                                                                        /* Port d'ecoute des requetes clientes */
    gint  taille_bloc_reseau;
    gboolean ssl_needed;                                                                     /* Cryptage des transmissions ?? */
    gboolean ssl_peer_cert;                                                     /* Devons-nous valider le certificat client ? */
    gchar ssl_file_cert[80];
    gchar ssl_file_key[80];
    gchar ssl_file_ca[80];
    gint  timeout_connexion;                                           /* Temps max d'attente de reponse de la part du client */
    GSList *Clients;                                                                 /* Liste des clients en cours de gestion */
    GSList *Liste_histo;                                                                      /* Envoi d'un histo aux clients */
    GSList *Liste_motif;                                                                     /* Destruction d'un histo client */
    GSList *Liste_events;                                                                       /* Liste des evenements recus */
  } Cfg_ssrv;

 struct CAPTEUR
  { gint   type;                                                                                   /* type du bit de controle */
    guint  bit_controle;
    gfloat val_ech;
  };

/*---------------------------------------- Déclarations des prototypes de fonctions ------------------------------------------*/
                                                                                                            /* Dans serveur.c */
 extern gboolean Ssrv_Lire_config ( void );
 extern void Unref_client ( struct CLIENT *client );
 extern void Ref_client ( struct CLIENT *client, gchar *reason );
 extern void Deconnecter ( struct CLIENT *client );
 extern void Run_handle_client ( struct CLIENT *client );
 extern gchar *Mode_vers_string ( gint mode );

 extern void Ecouter_client ( struct CLIENT *client );                                                    /* Dans protocole.c */

 extern void Liberer_SSL ( void );                                                                              /* Dans ssl.c */ 

                                                                                                      /* Dans protocole_***.c */
 extern void Gerer_protocole_atelier( struct CLIENT *client );
 extern void Gerer_protocole_icone( struct CLIENT *client );
 extern void Gerer_protocole_dls( struct CLIENT *client );
 extern void Gerer_protocole_utilisateur( struct CLIENT *client );
 extern void Gerer_protocole_message( struct CLIENT *client );
 extern void Gerer_protocole_mnemonique( struct CLIENT *client );
 extern void Gerer_protocole_supervision( struct CLIENT *client );
 extern void Gerer_protocole_histo( struct CLIENT *client );
 extern void Gerer_protocole_courbe( struct CLIENT *client );
 extern void Gerer_protocole_histo_courbe( struct CLIENT *client );
 extern void Gerer_protocole_synoptique( struct CLIENT *client );
 extern void Gerer_protocole_camera( struct CLIENT *client );
 extern void Gerer_protocole_admin( struct CLIENT *client );
 extern void Gerer_protocole_satellite( struct CLIENT *client );

                                                                                                              /* Dans envoi.c */
 extern gint Envoi_client( struct CLIENT *client, gint tag, gint sstag, gchar *buffer, gint taille );

 extern gboolean Envoyer_palette( struct CLIENT *client );                                                /* Dans envoi_syn.c */

 extern SSL_CTX *Init_ssl ( void );                                                                             /* Dans ssl.c */

 extern void Connecter_ssl( struct CLIENT *client );                                                         /* Dans accept.c */


 extern gboolean Tester_update_capteur( struct CAPTEUR *capteur );                                          /* Dans capteur.c */
 extern struct CMD_ETAT_BIT_CAPTEUR *Formater_capteur( struct CAPTEUR *capteur );

                                                                                                              /* Dans ident.c */
 extern gboolean Tester_autorisation ( struct CLIENT *client, struct REZO_CLI_IDENT *ident );
 extern void Proto_set_password ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *util );

 extern void Client_mode ( struct CLIENT *client, gint mode );                                              /* Dans Serveur.c */

 extern void *Envoyer_groupes_thread ( struct CLIENT *client );                                        /* Dans envoi_groupe.c */
 extern void *Envoyer_groupes_pour_util_thread ( struct CLIENT *client );
 extern void *Envoyer_groupes_pour_synoptique_thread ( struct CLIENT *client );
 extern void *Envoyer_groupes_pour_propriete_synoptique_thread ( struct CLIENT *client );
 extern void Proto_ajouter_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe );
 extern void Proto_effacer_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe );
 extern void Proto_editer_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe );
 extern void Proto_valider_editer_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe );

 extern void *Envoyer_utilisateurs_thread ( struct CLIENT *client );                                     /* Dans envoi_util.c */
 extern void Proto_ajouter_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util );
 extern void Proto_effacer_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util );
 extern void Proto_editer_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util );
 extern void Proto_valider_editer_utilisateur ( struct CLIENT *client,
                                                struct CMD_TYPE_UTILISATEUR *rezo_util );

 extern void *Envoyer_plugins_dls_thread ( struct CLIENT *client );                                       /* Dans envoi_dls.c */
 extern void *Envoyer_plugins_dls_pour_mnemo_thread ( struct CLIENT *client );
 extern void Proto_effacer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls );
 extern void Proto_ajouter_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls );
 extern void Proto_editer_source_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls );
 extern gboolean Envoyer_source_dls ( struct CLIENT *client );
 extern void Proto_valider_source_dls( struct CLIENT *client, struct CMD_TYPE_SOURCE_DLS *edit_dls,
                                       gchar *buffer );
 extern void *Proto_compiler_source_dls_thread( struct CLIENT *client );
 extern void Proto_effacer_source_dls ( struct CLIENT *client, struct CMD_TYPE_SOURCE_DLS *edit_dls );
 extern void Save_source_dls_to_disk ( struct CMD_TYPE_SOURCE_DLS *edit_dls, gchar *buffer, guint taille );
 extern void Proto_editer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls );
 extern void Proto_valider_editer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls );

 extern void *Envoyer_messages_thread ( struct CLIENT *client );                                      /* Dans envoi_message.c */
 extern void Proto_editer_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg );
 extern void Proto_valider_editer_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg );
 extern void Proto_effacer_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg );
 extern void Proto_ajouter_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg );
 extern void Proto_effacer_message_mp3 ( struct CLIENT *client, struct CMD_TYPE_MESSAGE_MP3 *msg_mp3 );
 extern void Proto_valider_message_mp3( struct CLIENT *client, struct CMD_TYPE_MESSAGE_MP3 *msg_mp3,
                                        gchar *buffer );

                                                                                                   /* Dans envoi_synoptique.c */
 extern void *Envoyer_synoptiques_thread ( struct CLIENT *client );
 extern void *Envoyer_synoptiques_pour_atelier_thread ( struct CLIENT *client );
 extern void *Envoyer_synoptiques_pour_atelier_palette_thread ( struct CLIENT *client );
 extern void *Envoyer_synoptiques_pour_message_thread ( struct CLIENT *client );
 extern void *Envoyer_synoptiques_pour_plugin_dls_thread ( struct CLIENT *client );
 extern void Proto_editer_synoptique_thread ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn );
 extern void Proto_valider_editer_synoptique ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn );
 extern void Proto_effacer_synoptique ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn );
 extern void Proto_ajouter_synoptique ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn );
 extern void Proto_editer_synoptique ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn );

                                                                                       /* Dans envoi_synoptique_passerelles.c */
 extern void Envoyer_passerelle_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin );
 extern void Proto_ajouter_passerelle_atelier ( struct CLIENT *client,
                                                struct CMD_TYPE_PASSERELLE *rezo_pass );
 extern void Proto_valider_editer_passerelle_atelier ( struct CLIENT *client,
                                                       struct CMD_TYPE_PASSERELLE *rezo_pass );
 extern void Proto_effacer_passerelle_atelier ( struct CLIENT *client,
                                                struct CMD_TYPE_PASSERELLE *rezo_pass );
                                                
                                                                                          /* Dans envoi_synoptique_comments.c */
 extern void Envoyer_comment_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin );
 extern void Proto_effacer_comment_atelier ( struct CLIENT *client, struct CMD_TYPE_COMMENT *rezo_comment );
 extern void Proto_ajouter_comment_atelier ( struct CLIENT *client, struct CMD_TYPE_COMMENT *rezo_motif );
 extern void Proto_valider_editer_comment_atelier ( struct CLIENT *client,
                                                    struct CMD_TYPE_COMMENT *rezo_comment );

                                                                                            /* Dans envoi_synoptique_motifs.c */
 extern void Envoyer_bit_init_motif ( struct CLIENT *client, GSList *list_bit_init );
 extern void Envoyer_motif_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin );
 extern void Proto_effacer_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif );
 extern void Proto_ajouter_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif );
 extern void Proto_valider_editer_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif );

                                                                                            /* Dans envoi_synoptique_motifs.c */
 extern void Proto_effacer_camera_sup_atelier ( struct CLIENT *client, struct CMD_TYPE_CAMERA_SUP *camera_sup );
 extern void Proto_ajouter_camera_sup_atelier ( struct CLIENT *client, struct CMD_TYPE_CAMERA_SUP *camera_sup );
 extern void Proto_valider_editer_camera_sup_atelier ( struct CLIENT *client,
                                                       struct CMD_TYPE_CAMERA_SUP *camera_sup );

                                                                                          /* Dans envoi_synoptique_palettes.c */
 extern void Envoyer_palette_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin );
 extern void Proto_effacer_palette_atelier ( struct CLIENT *client, struct CMD_TYPE_PALETTE *rezo_palette );
 extern void Proto_ajouter_palette_atelier ( struct CLIENT *client, struct CMD_TYPE_PALETTE *rezo_palette );
 extern void Proto_valider_editer_palette_atelier ( struct CLIENT *client, struct CMD_TYPE_PALETTE *rezo_palette );

                                                                                           /* Dans envoi_synoptique_capteur.c */
 extern void Envoyer_capteur_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin );
 extern void Proto_effacer_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur );
 extern void Proto_ajouter_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur );
 extern void Proto_valider_editer_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur );

 extern void *Envoyer_mnemoniques_thread ( struct CLIENT *client );                                /* Dans envoi_mnemonique.c */
 extern void *Envoyer_mnemoniques_for_courbe_thread ( struct CLIENT *client );
 extern void *Envoyer_mnemoniques_for_histo_courbe_thread ( struct CLIENT *client );
 extern void Proto_envoyer_type_num_mnemo_tag( int tag, int ss_tag, struct CLIENT *client,
                                               struct CMD_TYPE_NUM_MNEMONIQUE *critere );
 extern void Proto_editer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMO_BASE *rezo_mnemo );
 extern void Proto_valider_editer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMO_FULL *rezo_mnemo );
 extern void Proto_effacer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMO_BASE *rezo_mnemo );
 extern void Proto_ajouter_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMO_FULL *rezo_mnemo );

 
 extern void *Envoyer_histo_thread ( struct CLIENT *client );                                           /* Dans envoi_histo.c */
 extern void Proto_acquitter_histo ( struct CLIENT *client, struct CMD_TYPE_HISTO *rezo_histo );

 extern void *Envoyer_classes_thread ( struct CLIENT *client );                                        /* Dans envoi_classe.c */
 extern void *Envoyer_classes_pour_atelier_thread ( struct CLIENT *client );
 extern void Proto_editer_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe );
 extern void Proto_valider_editer_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe );
 extern void Proto_effacer_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe );
 extern void Proto_ajouter_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe );

 extern void *Envoyer_icones_thread ( struct CLIENT *client );                                          /* Dans envoi_icone.c */
 extern void *Envoyer_icones_pour_atelier_thread ( struct CLIENT *client );
 extern void Proto_editer_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone );
 extern void Proto_valider_editer_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone );
 extern void Proto_effacer_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone );
 extern void Proto_ajouter_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone );
 extern void Proto_ajouter_icone_deb_file( struct CLIENT *client, struct CMD_TYPE_ICONE *icone );
 extern void Proto_ajouter_icone_file( struct CLIENT *client, struct CMD_TYPE_ICONE *icone,
                                       gint taille, gchar *buffer );
 
 extern void *Proto_envoyer_histo_msgs_thread ( struct CLIENT *client );                           /* Dans envoi_histo_hard.c */

                                                                                                       /* Dans envoi_courbe.c */
 extern void Proto_ajouter_courbe_thread ( struct CLIENT *client );
 extern void Proto_effacer_courbe ( struct CLIENT *client, struct CMD_TYPE_COURBE *rezo_courbe );

                                                                                                   /* Dans envoi_histo_courbe */
 extern void Proto_ajouter_histo_courbe_thread ( struct CLIENT *client );

                                                                                                       /* Dans envoi_camera.c */
 extern void Proto_ajouter_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera );
 extern void Proto_effacer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera );
 extern void Proto_editer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera );
 extern void Proto_valider_editer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera );
 extern void *Envoyer_cameras_thread ( struct CLIENT *client );
 extern void *Envoyer_cameras_for_atelier_thread ( struct CLIENT *client );

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
