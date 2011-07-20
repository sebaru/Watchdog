/**********************************************************************************************************/
/* Watchdogd/Admin/admin_running.c        Gestion des connexions Admin RUNNING au serveur watchdog        */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 17 nov. 2010 20:00:45 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_running.c
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
 
 #include <glib.h>
 #include <unistd.h>                                                                  /* Pour gethostname */
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Ecouter_admin: Ecoute ce que dis le client                                                             */
/* Entrée: le client                                                                                      */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_running ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { gint i;
       Write_admin ( client->connexion, "  -- Watchdog ADMIN -- Help du mode 'running'\n" );
       Write_admin ( client->connexion, "  audit                 - Audit bit/s\n" );
       Write_admin ( client->connexion, "  ident                 - ID du serveur Watchdog\n" );
       Write_admin ( client->connexion, "  dls                   - D.L.S. Status\n" );
       Write_admin ( client->connexion, "  ssrv                  - SousServers Status\n" );
       Write_admin ( client->connexion, "  client                - Client Status\n" );
       Write_admin ( client->connexion, "  kick nom machine      - Kick client nom@machine\n" );
       Write_admin ( client->connexion, "  gete xxx              - Get Exxx\n" );
       Write_admin ( client->connexion, "  sete xxx i            - Set Exxx = i\n" );
       Write_admin ( client->connexion, "  getea xxx             - Get EAxxx\n" );
       Write_admin ( client->connexion, "  getm xxx              - Get Mxxx\n" );
       Write_admin ( client->connexion, "  setm xxx i            - Set Mxxx = i\n" );
       Write_admin ( client->connexion, "  getb xxx              - Get Bxxx\n" );
       Write_admin ( client->connexion, "  setb xxx i            - Set Bxxx = i\n" );
       Write_admin ( client->connexion, "  geta xxx i            - Get Axxx\n" );
       Write_admin ( client->connexion, "  seta xxx i            - Set Axxx = i\n" );
       Write_admin ( client->connexion, "  getg xxx              - Get MSGxxx\n" );
       Write_admin ( client->connexion, "  setg xxx i            - Set MSGxxx = i\n" );
       Write_admin ( client->connexion, "  gettr xxx             - Get TRxxx\n" );
       Write_admin ( client->connexion, "  geti xxx              - Get Ixxx\n" );
       Write_admin ( client->connexion, "  seti xxx E R V B C    - Set Ixxx Etat Rouge Vert Bleu Cligno\n" );
       Write_admin ( client->connexion, "  getci xxx             - Get CIxxx\n" );
       Write_admin ( client->connexion, "  tell message num      - Envoi AUDIO num\n" );
       Write_admin ( client->connexion, "  sms message           - Envoi du message SMS via SMSBOX\n" );
       Write_admin ( client->connexion, "  msgs message          - Envoi d'un message a tous les clients\n" );
       Write_admin ( client->connexion, "  mbus                  - Liste les modules MODBUS+Borne\n" );
       Write_admin ( client->connexion, "  rs                    - Affiche les status des equipements RS485\n" );
       Write_admin ( client->connexion, "  onduleur              - Affiche les status des equipements ONDULEUR\n" );
       Write_admin ( client->connexion, "  ping                  - Ping Watchdog\n" );
       Write_admin ( client->connexion, "  setrootpasswd         - Set the Watchdog root password\n" );
       Write_admin ( client->connexion, "  help                  - This help\n" );
       Write_admin ( client->connexion, "  mode type_mode        - Change de mode (" );
       i = 1;
       while ( i < NBR_MODE_ADMIN )
        { Write_admin ( client->connexion, Mode_admin[i] );
          Write_admin ( client->connexion, ", " );
          i++;
        }
       Write_admin ( client->connexion, "running)\n" );
       Write_admin ( client->connexion, "  debug debug_to_switch - Switch Debug Mode (all,none,signaux,db,config,user,crypto,info,serveur,\n" );
       Write_admin ( client->connexion, "                                             cdg,network,arch,connexion,dls,modbus,admin,rs485,\n" );
       Write_admin ( client->connexion, "                                             onduleur,sms,audio,camera,courbe,tellstick,lirc)\n" );
       Write_admin ( client->connexion, "  exit                  - Revient au mode RUNNING\n" );
     } else
    if ( ! strcmp ( commande, "ident" ) )
     { char nom[128];
       gethostname( nom, sizeof(nom) );
       g_snprintf( chaine, sizeof(chaine), " Watchdogd %s on %s\n", VERSION, nom );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "ssrv" ) )
     { int i;

       g_snprintf( chaine, sizeof(chaine), " Jeton au SSRV %02d\n", Partage->jeton );
       Write_admin ( client->connexion, chaine );

       for (i=0; i<Config.max_serveur; i++)
        { g_snprintf( chaine, sizeof(chaine), " SSRV[%02d] -> %02d clients\n",
                      i, Partage->Sous_serveur[i].nb_client );
          Write_admin ( client->connexion, chaine );
        }
     } else
    if ( ! strcmp ( commande, "client" ) )
     { GList *liste;
       gint i;
        
       for (i=0; i<Config.max_serveur; i++)
         { if (Partage->Sous_serveur[i].Thread_run == FALSE) continue;

           pthread_mutex_lock( &Partage->Sous_serveur[i].synchro );
           liste = Partage->Sous_serveur[i].Clients;
           while(liste)                                               /* Parcours de la liste des clients */
            { struct CLIENT *client_srv;
              client_srv = (struct CLIENT *)liste->data;

              g_snprintf( chaine, sizeof(chaine), " SSRV%02d - v%s %s@%s - mode %d defaut %d date %s\n",
                          i, client_srv->ident.version, client_srv->util->nom, client_srv->machine,
                          client_srv->mode, client_srv->defaut, ctime(&client_srv->date_connexion) );
              Write_admin ( client->connexion, chaine );

              liste = liste->next;
            }
           pthread_mutex_unlock( &Partage->Sous_serveur[i].synchro );
         }
     } else
    if ( ! strcmp ( commande, "mbus" ) )
     { Admin_modbus_list ( client );
     } else
    if ( ! strcmp ( commande, "rs" ) )
     { Admin_rs485_list ( client );
     } else
    if ( ! strcmp ( commande, "onduleur" ) )
     { Admin_onduleur_list ( client );
     } else
    if ( ! strcmp ( commande, "dls" ) )
     { Admin_dls_list ( client );
     } else
    if ( ! strcmp ( commande, "gettr" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " TR%03d = %d consigne %d, top=%d\n",
                   num, TR(num), Partage->Tempo_R[num].consigne, Partage->top );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "geti" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " I%03d = etat=%d, rouge=%d, vert=%d, bleu=%d, cligno=%d, "
                                           "changes=%d, last_change=%d, top=%d\n",
                   num, Partage->i[num].etat,
                   Partage->i[num].rouge, Partage->i[num].vert, Partage->i[num].bleu,
                   Partage->i[num].cligno, Partage->i[num].changes, Partage->i[num].last_change,
                   Partage->top );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "seti" ) )
     { int num, etat, rouge, vert, bleu, cligno;                     /* Découpage de la ligne de commande */
       sscanf ( ligne, "%s %d %d %d %d %d %d", commande, &num, &etat, &rouge, &vert, &bleu, &cligno );
       SI(num, etat, rouge, vert, bleu, cligno, -1);
       sleep(1);
       g_snprintf( chaine, sizeof(chaine), " I%03d = etat=%d, rouge=%d, vert=%d, bleu=%d, cligno=%d, "
                                           "changes=%d, last_change=%d, top=%d\n",
                   num, Partage->i[num].etat,
                   Partage->i[num].rouge, Partage->i[num].vert, Partage->i[num].bleu,
                   Partage->i[num].cligno, Partage->i[num].changes, Partage->i[num].last_change,
                   Partage->top );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "getg" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " MSG%03d = %d, changes = %d, last_change = %d top=%d\n",
                   num, Partage->g[num].etat, Partage->g[num].changes,
                   Partage->g[num].last_change, Partage->top );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "setg" ) )
     { int num, val;
       sscanf ( ligne, "%s %d %d", commande, &num, &val );           /* Découpage de la ligne de commande */
       MSG ( num, val );
       g_snprintf( chaine, sizeof(chaine), " MSG%03d = %d\n", num, val );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "getm" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " M%03d = %d\n", num, M(num) );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "gete" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " E%03d = %d\n", num, E(num) );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "sete" ) )
     { int num, val;
       sscanf ( ligne, "%s %d %d", commande, &num, &val );           /* Découpage de la ligne de commande */
       SE(num, val );
       g_snprintf( chaine, sizeof(chaine), " E%03d = %d\n", num, val );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "getea" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " Partage->Top : %d\n", Partage->top );
       Write_admin ( client->connexion, chaine );
       g_snprintf( chaine, sizeof(chaine),
                   " EA%03d = %8.2f, val_int=%d, inrange=%d, type=%d, last_arch=%d, min=%8.2f, max=%8.2f\n",
                   num, EA_ech(num), Partage->ea[num].val_int, EA_inrange(num),
                   Partage->ea[num].cmd_type_eana.type, Partage->ea[num].last_arch, 
                   Partage->ea[num].cmd_type_eana.min, Partage->ea[num].cmd_type_eana.max 
                 );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "setm" ) )
     { int num, val;
       sscanf ( ligne, "%s %d %d", commande, &num, &val );           /* Découpage de la ligne de commande */
       if (val) Envoyer_commande_dls( num );
           else SM ( num, 0 );
       g_snprintf( chaine, sizeof(chaine), " M%03d = %d\n", num, val );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "getb" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " B%03d = %d\n", num, B(num) );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "geta" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " A%03d = %d\n", num, A(num) );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "getci" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " CI%03d = %8.2f, type=%d, actif=%d, unite=%s, multi=%8.2f, val1=%8.2f, val2=%8.2f\n",
                   num, Partage->ci[num].cpt_impdb.valeur, Partage->ci[num].cpt_impdb.type, Partage->ci[num].actif,
                   Partage->ci[num].cpt_impdb.unite, Partage->ci[num].cpt_impdb.multi,
                   Partage->ci[num].val_en_cours1, Partage->ci[num].val_en_cours2
                 );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "setb" ) )
     { int num, val;
       sscanf ( ligne, "%s %d %d", commande, &num, &val );           /* Découpage de la ligne de commande */
       SB ( num, val );
       g_snprintf( chaine, sizeof(chaine), " B%03d = %d\n", num, val );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "seta" ) )
     { int num, val;
       sscanf ( ligne, "%s %d %d", commande, &num, &val );           /* Découpage de la ligne de commande */
       SA ( num, val );
       g_snprintf( chaine, sizeof(chaine), " A%03d = %d\n", num, val );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "tell" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Ajouter_audio ( num );
       g_snprintf( chaine, sizeof(chaine), " Message id %d sent\n", num );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "sms" ) )
     { gchar message[80];
       sscanf ( ligne, "%s %s", commande, message );                 /* Découpage de la ligne de commande */
       Envoyer_sms_smsbox_text ( message );
       g_snprintf( chaine, sizeof(chaine), " Message sent\n" );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "msgs" ) )
     { GList *liste;
       gint i;

       for (i=0; i<Config.max_serveur; i++)
         { if (Partage->Sous_serveur[i].Thread_run == FALSE) continue;
           liste = Partage->Sous_serveur[i].Clients;
           while(liste)                                               /* Parcours de la liste des clients */
            { struct CMD_GTK_MESSAGE erreur;
              struct CLIENT *client_wat;
              client_wat = (struct CLIENT *)liste->data;

              g_snprintf( erreur.message, sizeof(erreur.message), "AdminMSG : %s", commande + 5 );
              Envoi_client( client_wat, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );

              g_snprintf( chaine, sizeof(chaine), " Envoi du message a %s@%s\n",
                          client_wat->util->nom, client_wat->machine );
              Write_admin ( client->connexion, chaine );
              liste = liste->next;
            }
         }
     } else
    if ( ! strcmp ( commande, "setrootpasswd" ) )
     { struct CMD_TYPE_UTILISATEUR util;
       gchar password[80];
       struct DB *db;

       sscanf ( ligne, "%s %s", commande, password );                /* Découpage de la ligne de commande */

       db = Init_DB_SQL( Config.log );
       if (!db)
        { g_snprintf( chaine, sizeof(chaine), " Unable to connect to Database\n" );
          Write_admin ( client->connexion, chaine );
        }
       else
        { util.id = 0;
          g_snprintf( util.nom, sizeof(util.nom), "root" );
          g_snprintf( util.commentaire, sizeof(util.commentaire), "Watchdog root user" );
          util.cansetpass = TRUE;
          util.setpassnow = TRUE;
          g_snprintf( util.code_en_clair, sizeof(util.code_en_clair), "%s", password );
          util.actif = TRUE;
          util.expire = FALSE;
          util.changepass = FALSE;
          memset ( &util.gids, 0, sizeof(util.gids) );
          if( Modifier_utilisateurDB( Config.log, db, Config.crypto_key, &util ) )
           { g_snprintf( chaine, sizeof(chaine), " Password set\n" ); }
          else
           { g_snprintf( chaine, sizeof(chaine), " Error while setting password\n" ); }
          Write_admin ( client->connexion, chaine );
          Libere_DB_SQL( Config.log, &db );
        }
     } else
    if ( ! strcmp ( commande, "audit" ) )
     { gint num;
       g_snprintf( chaine, sizeof(chaine), " Partage->Top : %d\n", Partage->top );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Bit/s        : %d\n", Partage->audit_bit_interne_per_sec_hold );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Tour/s       : %d\n", Partage->audit_tour_dls_per_sec_hold );
       Write_admin ( client->connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_list_length( Partage->com_msrv.liste_i );                   /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " Distribution des I      : reste %d\n", num );
       Write_admin ( client->connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_list_length( Partage->com_msrv.liste_msg_off );             /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " Distribution des Msg OFF: reste %d\n", num );
       Write_admin ( client->connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_list_length( Partage->com_msrv.liste_msg_on );              /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " Distribution des Msg ON : reste %d\n", num );
       Write_admin ( client->connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_list_length( Partage->com_msrv.liste_msg_repeat );                     /* liste des repeat */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), "          MSgs en REPEAT : reste %d\n", num );
       Write_admin ( client->connexion, chaine );
       
     } else
    if ( ! strcmp ( commande, "debug" ) )
     { gchar debug[128];

       sscanf ( ligne, "%s %s", commande, debug );

       if ( ! strcmp ( debug, "all"       ) )
        { Info_change_debug ( Config.log, ~0 ); } else
       if ( ! strcmp ( debug, "none"      ) )
        { Info_change_debug ( Config.log,  0 ); } else
       if ( ! strcmp ( debug, "signaux"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SIGNAUX   ); } else
       if ( ! strcmp ( debug, "db"        ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_DB        ); } else
       if ( ! strcmp ( debug, "config"    ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_CONFIG    ); } else
       if ( ! strcmp ( debug, "user"      ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_USER      ); } else
       if ( ! strcmp ( debug, "crypto"    ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_CRYPTO    ); } else
       if ( ! strcmp ( debug, "info"      ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_INFO      ); } else
       if ( ! strcmp ( debug, "serveur"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SERVEUR   ); } else
       if ( ! strcmp ( debug, "cdg"       ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_CDG       ); } else
       if ( ! strcmp ( debug, "network"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_NETWORK   ); } else
       if ( ! strcmp ( debug, "arch"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_ARCHIVE   ); } else
       if ( ! strcmp ( debug, "connexion" ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_CONNEXION ); } else
       if ( ! strcmp ( debug, "dls"       ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_DLS       ); } else
       if ( ! strcmp ( debug, "modbus"    ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_MODBUS    ); } else
       if ( ! strcmp ( debug, "admin"     ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_ADMIN     ); } else
       if ( ! strcmp ( debug, "rs485"     ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_RS485     ); } else
       if ( ! strcmp ( debug, "onduleur"  ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_ONDULEUR  ); } else
       if ( ! strcmp ( debug, "sms"       ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SMS       ); } else
       if ( ! strcmp ( debug, "audio"     ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_AUDIO     ); } else
       if ( ! strcmp ( debug, "camera"    ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_CAMERA    ); } else
       if ( ! strcmp ( debug, "courbe"    ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_COURBE    ); } else
       if ( ! strcmp ( debug, "tellstick" ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_TELLSTICK ); } else
       if ( ! strcmp ( debug, "lirc"      ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_LIRC      ); } else
       if ( ! strcmp ( debug, "asterisk"  ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_ASTERISK  ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " -- Unknown debug switch\n" );
          Write_admin ( client->connexion, chaine );
        }
       g_snprintf( chaine, sizeof(chaine), " Debug_level is now %d\n", Config.log->debug_level );
       Write_admin ( client->connexion, chaine );
       Config.debug_level = Config.log->debug_level;  /* Sauvegarde pour persistence (export des données) */
     } else
    if ( ! strcmp ( commande, "ping" ) )
     { Write_admin ( client->connexion, " Pong !\n" );
     } else
    if ( ! strcmp ( commande, "nocde" ) )
     { 
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
