/**********************************************************************************************************/
/* Watchdogd/Include/Sous_serveur.h      Définition des prototypes du serveur watchdog                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 03 jun 2003 10:39:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Sousèserveyr.h
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
 #include <glib.h>

 struct SOUS_SERVEUR
  { pthread_t pid;
    gint nb_client;
    gboolean Thread_run;                /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean Thread_sigusr1;                                      /* TRUE si le thread doit gerer le USR1 */
    pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *Clients;                                         /* La liste des clients qui se sont connectés */
    guint inactivite;                                            /* Dernier top d'activité avec un client */
    GList *new_histo;                                                     /* Envoi d'un histo aux clients */
    GList *del_histo;                                                    /* Destruction d'un histo client */
    GList *new_motif;                                                        /* Changement lié à un motif */
  };

 struct CAPTEUR
  { gint    type;                                                              /* type du bit de controle */
    guint bit_controle;
    gdouble val_ech;
  };

 #define COURBE_NBR_HEURE_ARCHIVE          24       /* 24 heures d'archive pour les courbes en temps réel */
/*---------------------------- Déclarations des prototypes de fonctions ----------------------------------*/

                                                                                        /* Dans serveur.c */
 extern void Unref_client ( struct CLIENT *client );
 extern void Ref_client ( struct CLIENT *client );
 extern void Run_serveur ( gint id );                                                   /* Dans serveur.c */

 extern void Ecouter_client ( gint Id_serveur, struct CLIENT *client );               /* Dans protocole.c */

                                                                                  /* Dans protocole_***.c */
 extern void Gerer_protocole_atelier( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_scenario( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_icone( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_dls( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_utilisateur( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_message( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_mnemonique( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_supervision( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_histo( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_courbe( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_histo_courbe( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_synoptique( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_camera( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_onduleur( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_rs485( gint Id_serveur, struct CLIENT *client );
 extern void Gerer_protocole_modbus( gint Id_serveur, struct CLIENT *client );

                                                                                          /* Dans envoi.c */
 extern void Envoi_clients( gint ss_id, gint tag, gint sstag, gchar *buffer, gint taille );
 extern gint Envoi_client( struct CLIENT *client, gint tag, gint sstag, gchar *buffer, gint taille );
 gboolean Envoyer_gif( struct CLIENT *client );

 extern gboolean Envoyer_palette( struct CLIENT *client );                            /* Dans envoi_syn.c */

 extern gint Ajouter_repertoire_liste( struct CLIENT *client, gchar *Repertoire,          /* Dans liste.c */
                                       time_t version_d_client );
 extern void Liberer_liste( struct CLIENT *client );

 extern gboolean Tester_update_capteur( struct CAPTEUR *capteur );                      /* Dans capteur.c */
 extern struct CMD_ETAT_BIT_CAPTEUR *Formater_capteur( struct CAPTEUR *capteur );

 extern gint Tester_autorisation ( gint Id_serveur, struct CLIENT *client );              /* Dans ident.c */
 extern void Proto_set_password ( gint Id_serveur, struct CLIENT *client, struct CMD_UTIL_SETPASSWORD *util );

 extern void Client_mode ( struct CLIENT *client, gint mode );                          /* Dans Serveur.c */

 extern void *Envoyer_groupes_thread ( struct CLIENT *client );                    /* Dans envoi_groupe.c */
 extern void *Envoyer_groupes_pour_util_thread ( struct CLIENT *client );
 extern void *Envoyer_groupes_pour_synoptique_thread ( struct CLIENT *client );
 extern void *Envoyer_groupes_pour_propriete_synoptique_thread ( struct CLIENT *client );
 extern void Proto_ajouter_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe );
 extern void Proto_effacer_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe );
 extern void Proto_editer_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe );
 extern void Proto_valider_editer_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe );

 extern void *Envoyer_utilisateurs_thread ( struct CLIENT *client );                 /* Dans envoi_util.c */
 extern void Proto_ajouter_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util );
 extern void Proto_effacer_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util );
 extern void Proto_editer_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util );
 extern void Proto_valider_editer_utilisateur ( struct CLIENT *client,
                                                struct CMD_TYPE_UTILISATEUR *rezo_util );

 extern void *Envoyer_plugins_dls_thread ( struct CLIENT *client );                   /* Dans envoi_dls.c */
 extern void Proto_effacer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls );
 extern void Proto_ajouter_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls );
 extern void Proto_editer_source_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls );
 extern gboolean Envoyer_source_dls ( struct CLIENT *client );
 extern void Proto_valider_source_dls( struct CLIENT *client, struct CMD_TYPE_SOURCE_DLS *edit_dls,
                                       gchar *buffer );
 extern void Compiler_source_dls( struct CLIENT *client, gint id );
 extern void *Proto_compiler_source_dls( struct CLIENT *client );
 extern void Proto_effacer_fichier_plugin_dls ( struct CLIENT *client, gint id );
 extern void Proto_editer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls );
 extern void Proto_valider_editer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls );

 extern void *Envoyer_messages_thread ( struct CLIENT *client );                  /* Dans envoi_message.c */
 extern void Proto_editer_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg );
 extern void Proto_valider_editer_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg );
 extern void Proto_effacer_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg );
 extern void Proto_ajouter_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg );

 extern void *Envoyer_entreeANA_for_courbe_thread ( struct CLIENT *client );     /* Dans envoi_entreana.c */
 extern void *Envoyer_entreeANA_for_histo_courbe_thread ( struct CLIENT *client );

                                                                               /* Dans envoi_synoptique.c */
 extern void *Envoyer_synoptiques_thread ( struct CLIENT *client );
 extern void *Envoyer_synoptiques_pour_atelier_thread ( struct CLIENT *client );
 extern void *Envoyer_synoptiques_pour_atelier_palette_thread ( struct CLIENT *client );
 extern void *Envoyer_synoptiques_pour_message_thread ( struct CLIENT *client );
 extern struct CMD_TYPE_SYNOPTIQUE *Preparer_envoi_synoptique ( struct CMD_TYPE_SYNOPTIQUE *syn );
 extern void Proto_editer_synoptique_thread ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn );
 extern void Proto_valider_editer_synoptique ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn );
 extern void Proto_effacer_synoptique ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn );
 extern void Proto_ajouter_synoptique ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn );

                                                                                 /* Dans envoi_bit_init.c */
 extern void Envoyer_bit_init_supervision_thread ( struct CLIENT *client );
 extern gint Chercher_bit_capteurs ( struct CAPTEUR *element, struct CAPTEUR *cherche );

                                                                   /* Dans envoi_synoptique_passerelles.c */
 extern void *Envoyer_passerelle_atelier_thread ( struct CLIENT *client );
 extern void *Envoyer_passerelle_supervision_thread ( struct CLIENT *client );
 extern void Proto_ajouter_passerelle_atelier ( struct CLIENT *client,
                                                struct CMD_TYPE_PASSERELLE *rezo_pass );
 extern void Proto_valider_editer_passerelle_atelier ( struct CLIENT *client,
                                                       struct CMD_TYPE_PASSERELLE *rezo_pass );

                                                                      /* Dans envoi_synoptique_comments.c */
 extern void *Envoyer_comment_atelier_thread ( struct CLIENT *client );
 extern void *Envoyer_comment_supervision_thread ( struct CLIENT *client );
 extern void Proto_effacer_comment_atelier ( struct CLIENT *client, struct CMD_TYPE_COMMENT *rezo_comment );
 extern void Proto_ajouter_comment_atelier ( struct CLIENT *client, struct CMD_TYPE_COMMENT *rezo_motif );
 extern void Proto_valider_editer_comment_atelier ( struct CLIENT *client,
                                                    struct CMD_TYPE_COMMENT *rezo_comment );

                                                                        /* Dans envoi_synoptique_motifs.c */
 extern void *Envoyer_motif_atelier_thread ( struct CLIENT *client );
 extern void *Envoyer_motif_supervision_thread ( struct CLIENT *client );
 extern void Proto_effacer_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif );
 extern void Proto_ajouter_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif );
 extern void Proto_valider_editer_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif );

                                                                        /* Dans envoi_synoptique_motifs.c */
 extern void *Envoyer_camera_sup_atelier_thread ( struct CLIENT *client );
 extern void *Envoyer_camera_sup_supervision_thread ( struct CLIENT *client );
 extern void Proto_effacer_camera_sup_atelier ( struct CLIENT *client, struct CMD_TYPE_CAMERA_SUP *camera_sup );
 extern void Proto_ajouter_camera_sup_atelier ( struct CLIENT *client, struct CMD_TYPE_CAMERA_SUP *camera_sup );
 extern void Proto_valider_editer_camera_sup_atelier ( struct CLIENT *client,
                                                       struct CMD_TYPE_CAMERA_SUP *camera_sup );

                                                                      /* Dans envoi_synoptique_palettes.c */
 extern void *Envoyer_palette_atelier_thread ( struct CLIENT *client );
 extern void *Envoyer_palette_supervision_thread ( struct CLIENT *client );
 extern void Proto_effacer_palette_atelier ( struct CLIENT *client, struct CMD_TYPE_PALETTE *rezo_palette );
 extern void Proto_ajouter_palette_atelier ( struct CLIENT *client, struct CMD_TYPE_PALETTE *rezo_palette );
 extern void Proto_valider_editer_palette_atelier ( struct CLIENT *client, struct CMD_TYPE_PALETTE *rezo_palette );

                                                                         /* Dans envoi_synoptique_capteur.c */
 extern void *Envoyer_capteur_atelier_thread ( struct CLIENT *client );
 extern void *Envoyer_capteur_supervision_thread ( struct CLIENT *client );
 extern void Proto_effacer_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur );
 extern void Proto_ajouter_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur );
 extern void Proto_valider_editer_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur );

 extern void *Envoyer_mnemoniques_thread ( struct CLIENT *client );            /* Dans envoi_mnemonique.c */
 extern void *Envoyer_mnemoniques_for_courbe_thread ( struct CLIENT *client );
 extern void *Envoyer_mnemoniques_for_histo_courbe_thread ( struct CLIENT *client );
 extern void Proto_envoyer_type_num_mnemo_tag( int tag, int ss_tag, struct CLIENT *client,
                                               struct CMD_TYPE_NUM_MNEMONIQUE *critere );
 extern void Proto_editer_option_entreeANA ( struct CLIENT *client,
                                             struct CMD_TYPE_MNEMONIQUE *rezo_mnemo );
 extern void Proto_valider_editer_option_entreeANA ( struct CLIENT *client,
                                                     struct CMD_TYPE_OPTION_ENTREEANA *rezo_entree );
 extern void Proto_editer_option_compteur_imp ( struct CLIENT *client,
                                                struct CMD_TYPE_MNEMONIQUE *rezo_mnemo );
 extern void Proto_valider_editer_option_compteur_imp ( struct CLIENT *client,
                                                        struct CMD_TYPE_OPTION_COMPTEUR_IMP *rezo_cpt );
extern void Proto_editer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMONIQUE *rezo_mnemo );
 extern void Proto_valider_editer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMONIQUE *rezo_mnemo );
 extern void Proto_effacer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMONIQUE *rezo_mnemo );
 extern void Proto_ajouter_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMONIQUE *rezo_mnemo );
 
 extern void *Envoyer_histo_thread ( struct CLIENT *client );                       /* Dans envoi_histo.c */
 extern void Proto_acquitter_histo ( struct CLIENT *client, struct CMD_TYPE_HISTO *rezo_histo );

 extern void *Envoyer_classes_thread ( struct CLIENT *client );                    /* Dans envoi_classe.c */
 extern void *Envoyer_classes_pour_atelier_thread ( struct CLIENT *client );
 extern void Proto_editer_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe );
 extern void Proto_valider_editer_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe );
 extern void Proto_effacer_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe );
 extern void Proto_ajouter_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe );

 extern void *Envoyer_icones_thread ( struct CLIENT *client );                      /* Dans envoi_icone.c */
 extern void *Envoyer_icones_pour_atelier_thread ( struct CLIENT *client );
 extern void Proto_editer_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone );
 extern void Proto_valider_editer_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone );
 extern void Proto_effacer_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone );
 extern void Proto_ajouter_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone );
 extern void Proto_ajouter_icone_deb_file( struct CLIENT *client, struct CMD_TYPE_ICONE *icone );
 extern void Proto_ajouter_icone_file( struct CLIENT *client, struct CMD_TYPE_ICONE *icone,
                                       gint taille, gchar *buffer );
 extern void Proto_ajouter_icone_fin_file( struct CLIENT *client, struct CMD_TYPE_ICONE *icone );
 
 extern void *Proto_envoyer_histo_hard_thread ( struct CLIENT *client );       /* Dans envoi_histo_hard.c */

                                                                                   /* Dans envoi_courbe.c */
 extern void Proto_ajouter_courbe_thread ( struct CLIENT *client );
 extern void Proto_effacer_courbe ( struct CLIENT *client, struct CMD_TYPE_COURBE *rezo_courbe );

                                                                               /* Dans envoi_histo_courbe */
 extern void Proto_ajouter_histo_courbe_thread ( struct CLIENT *client );

                                                                                 /* Dans envoi_scenario.h */
 extern void Proto_editer_scenario ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc );
 extern void Proto_valider_editer_scenario ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc );
 extern void Proto_effacer_scenario ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc );
 extern void Proto_effacer_scenario_tag ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc,
                                          gint tag, gint sstag );
 extern void Proto_ajouter_scenario ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc );
 extern void *Envoyer_scenario_thread ( struct CLIENT *client );

                                                                      /* Dans envoi_synoptique_scenario.h */
 extern void Proto_editer_scenario_sup ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc );
 extern void Proto_valider_editer_scenario_sup ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc );
 extern void Proto_effacer_scenario_sup ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc );
 extern void Proto_ajouter_scenario_sup ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc );
 extern void *Envoyer_scenario_sup_thread ( struct CLIENT *client );

                                                                                   /* Dans envoi_camera.c */
 extern void Proto_ajouter_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera );
 extern void Proto_effacer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera );
 extern void Proto_editer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera );
 extern void Proto_valider_editer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera );
 extern void *Envoyer_cameras_thread ( struct CLIENT *client );
 extern void *Envoyer_cameras_for_atelier_thread ( struct CLIENT *client );

                                                                                 /* Dans envoi_onduleur.c */
 extern void Proto_ajouter_onduleur ( struct CLIENT *client, struct CMD_TYPE_ONDULEUR *rezo_onduleur );
 extern void Proto_effacer_onduleur ( struct CLIENT *client, struct CMD_TYPE_ONDULEUR *rezo_onduleur );
 extern void Proto_editer_onduleur ( struct CLIENT *client, struct CMD_TYPE_ONDULEUR *rezo_onduleur );
 extern void Proto_valider_editer_onduleur ( struct CLIENT *client, struct CMD_TYPE_ONDULEUR *rezo_onduleur );
 extern void *Envoyer_onduleurs_thread ( struct CLIENT *client );

                                                                                    /* Dans envoi_rs485.c */
 extern void Proto_ajouter_rs485 ( struct CLIENT *client, struct CMD_TYPE_RS485 *rezo_rs485 );
 extern void Proto_effacer_rs485 ( struct CLIENT *client, struct CMD_TYPE_RS485 *rezo_rs485 );
 extern void Proto_editer_rs485 ( struct CLIENT *client, struct CMD_TYPE_RS485 *rezo_rs485 );
 extern void Proto_valider_editer_rs485 ( struct CLIENT *client, struct CMD_TYPE_RS485 *rezo_rs485 );
 extern void *Envoyer_rs485_thread ( struct CLIENT *client );

                                                                                   /* Dans envoi_modbus.c */
 extern void Proto_ajouter_modbus ( struct CLIENT *client, struct CMD_TYPE_MODBUS *rezo_modbus );
 extern void Proto_effacer_modbus ( struct CLIENT *client, struct CMD_TYPE_MODBUS *rezo_modbus );
 extern void Proto_editer_modbus ( struct CLIENT *client, struct CMD_TYPE_MODBUS *rezo_modbus );
 extern void Proto_valider_editer_modbus ( struct CLIENT *client, struct CMD_TYPE_MODBUS *rezo_modbus );
 extern void *Envoyer_modbus_thread ( struct CLIENT *client );
#endif
/*--------------------------------------------------------------------------------------------------------*/
