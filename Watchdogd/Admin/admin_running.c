/**********************************************************************************************************/
/* Watchdogd/Admin/admin_running.c        Gestion des connexions Admin RUNNING au serveur watchdog        */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 jan 2009 14:43:27 CET */
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
 #include <unistd.h>

 #include "config.h"
 #include "Admin.h"
 #include "Modbus.h"
 #include "watchdogd.h"

 extern struct CONFIG Config;
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
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
       Write_admin ( client->connexion, "  audit                - Audit bit/s\n" );
       Write_admin ( client->connexion, "  ident                - ID du serveur Watchdog\n" );
       Write_admin ( client->connexion, "  dls                  - D.L.S. Status\n" );
       Write_admin ( client->connexion, "  debug debug_level    - Set debug level\n" );
       Write_admin ( client->connexion, "  ssrv                 - SousServers Status\n" );
       Write_admin ( client->connexion, "  client               - Client Status\n" );
       Write_admin ( client->connexion, "  kick nom machine     - Kick client nom@machine\n" );
       Write_admin ( client->connexion, "  gete xxx             - Get Exxx\n" );
       Write_admin ( client->connexion, "  getea xxx            - Get EAxxx\n" );
       Write_admin ( client->connexion, "  getm xxx             - Get Mxxx\n" );
       Write_admin ( client->connexion, "  setm xxx i           - Set Mxxx = i\n" );
       Write_admin ( client->connexion, "  getb xxx             - Get Bxxx\n" );
       Write_admin ( client->connexion, "  setb xxx i           - Set Bxxx = i\n" );
       Write_admin ( client->connexion, "  geta xxx i           - Get Axxx\n" );
       Write_admin ( client->connexion, "  seta xxx i           - Set Axxx = i\n" );
       Write_admin ( client->connexion, "  getg xxx             - Get MSGxxx\n" );
       Write_admin ( client->connexion, "  setg xxx i           - Set MSGxxx = i\n" );
       Write_admin ( client->connexion, "  gettr xxx            - Get TRxxx\n" );
       Write_admin ( client->connexion, "  tell message num     - Envoi AUDIO num\n" );
       Write_admin ( client->connexion, "  msgs message         - Envoi d'un message a tous les clients\n" );
       Write_admin ( client->connexion, "  mbus                 - Liste les modules MODBUS+Borne\n" );
       Write_admin ( client->connexion, "  rs                   - Affiche les status des equipements RS485\n" );
       Write_admin ( client->connexion, "  onduleur             - Affiche les status des equipements ONDULEUR\n" );
       Write_admin ( client->connexion, "  ping                 - Ping Watchdog\n" );
       Write_admin ( client->connexion, "  help                 - This help\n" );
       Write_admin ( client->connexion, "  mode type_mode       - Change de mode (" );
       i = 1;
       while ( i < NBR_MODE_ADMIN )
        { Write_admin ( client->connexion, Mode_admin[i] );
          Write_admin ( client->connexion, ", " );
          i++;
        }
       Write_admin ( client->connexion, "running)\n" );
       Write_admin ( client->connexion, "  exit                 - Revient au mode RUNNING\n" );
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
         { if (Partage->Sous_serveur[i].pid == -1) continue;

           pthread_mutex_lock( &Partage->Sous_serveur[i].synchro );
           liste = Partage->Sous_serveur[i].Clients;
           while(liste)                                               /* Parcours de la liste des clients */
            { struct CLIENT *client_srv;
              client_srv = (struct CLIENT *)liste->data;

              g_snprintf( chaine, sizeof(chaine), " SSRV%02d - v%s %s@%s - mode %d defaut %d date %s\n",
                          i, client_srv->ident.version, client_srv->util->nom, client_srv->machine,
                          client_srv->mode, client_srv->defaut, ctime(&client_srv->seconde) );
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
    if ( ! strcmp ( commande, "getg" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " MSG%03d = %d ... last_send = %d top=%d\n",
                   num, Partage->g[num].etat, Partage->g[num].last_send, Partage->top );
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
    if ( ! strcmp ( commande, "getea" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " EA%03d = %f, inrange=%d\n", num, EA_ech(num), EA_inrange(num) );
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
    if ( ! strcmp ( commande, "msgs" ) )
     { GList *liste;
       gint i;

       for (i=0; i<Config.max_serveur; i++)
         { if (Partage->Sous_serveur[i].pid == -1) continue;
           liste = Partage->Sous_serveur[i].Clients;
           while(liste)                                               /* Parcours de la liste des clients */
            { struct CMD_GTK_MESSAGE erreur;
              struct CLIENT *client_wat;
              client_wat = (struct CLIENT *)liste->data;

              g_snprintf( erreur.message, sizeof(erreur.message), commande + 5 );
              Envoi_client( client_wat, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );

              g_snprintf( chaine, sizeof(chaine), " Envoi du message a %s@%s\n",
                          client_wat->util->nom, client_wat->machine );
              Write_admin ( client->connexion, chaine );
              liste = liste->next;
            }
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
       
     } else
    if ( ! strcmp ( commande, "debug" ) )
     { int debug;
       sscanf ( ligne, "%s %d", commande, &debug );                  /* Découpage de la ligne de commande */
       Config.debug_level = debug;
       g_snprintf( chaine, sizeof(chaine), " Debug_level is now %d\n", debug );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "ping" ) )
     { Write_admin ( client->connexion, " Pong !\n" );
     } else
    if ( ! strcmp ( commande, "nocde" ) )
     { 
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
