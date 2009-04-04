/**********************************************************************************************************/
/* Watchdogd/admin.c        Gestion des connexions Admin au serveur watchdog                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
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
 #include <stdlib.h>
 #include <string.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <errno.h>
 #include <sys/types.h>
 #include <sys/stat.h>

 #include "sysconfig.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "proto_dls.h"
 #include "proto_srv.h"

 extern struct CONFIG Config;
 extern int errno;

 static gint Socket_read;                                                      /* Socket d'administration */
 static gint Socket_write;                                                     /* Socket d'administration */

 #include "watchdogd.h"
 #include "prototype.h"                                      /* Mise en place des prototypes de fonctions */
 #include "Module_DLS.h"

 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
 extern GList *Plugins;
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 gboolean Activer_ecoute_admin ( void )
  { mkfifo( FICHIER_FIFO_ADMIN_READ,  S_IRUSR | S_IWUSR );
    mkfifo( FICHIER_FIFO_ADMIN_WRITE, S_IRUSR | S_IWUSR );
    Socket_read  = open( FICHIER_FIFO_ADMIN_WRITE, O_RDWR );
    if ( Socket_read < 0 ) return(FALSE);

    fcntl( Socket_read, F_SETFL, O_NONBLOCK );                                 /* Mode non bloquant */
    Info( Config.log, DEBUG_INFO, "Ecoute Fifo Admin ON" );
    return( TRUE );                                                              /* Tout s'est bien passé */
  }
/**********************************************************************************************************/
/* Desactiver_ecoute_admin: Ferme la socker fifo d'administration                                         */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Desactiver_ecoute_admin ( void )
  { close (Socket_read);
    Socket_read  = 0;
    Info( Config.log, DEBUG_INFO, "Desactivation Fifo Admin" );
  }
/**********************************************************************************************************/
/* Gerer_fifo_admin: Ecoute les commandes d'admin locale et les traite                                    */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Write_admin ( gint fd, gchar *chaine )
  { write ( fd, chaine, strlen(chaine) ); }
/**********************************************************************************************************/
/* Gerer_fifo_admin: Ecoute les commandes d'admin locale et les traite                                    */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Gerer_fifo_admin ( void )
  { gchar ligne[128], commande[20];
    gint taille;

    taille = read ( Socket_read, ligne, sizeof(ligne) );

    if ( taille > 0 )
     { 
       ligne[taille] = 0;

       Socket_write = open( FICHIER_FIFO_ADMIN_READ, O_WRONLY );        /* Préparation de la socket write */
       if ( Socket_write < 0 ) return;

       sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */

       Info_c( Config.log, DEBUG_INFO, "Admin : received command", ligne );

       if ( ! strcmp ( commande, "dls" ) )
        { struct PLUGIN_DLS_DL *plugin_actuel;
          char chaine[128];
          GList *plugins;

          plugins = Plugins;
          while(plugins)                                         /* On execute tous les modules un par un */
           { plugin_actuel = (struct PLUGIN_DLS_DL *)plugins->data;

             g_snprintf( chaine, sizeof(chaine), " DLS %03d actif=%d conso=%f\n",
                         plugin_actuel->id, plugin_actuel->actif, plugin_actuel->conso );
             Write_admin ( Socket_write, chaine );
             plugins = plugins->next;
           }
        } else
       if ( ! strcmp ( commande, "ssrv" ) )
        { char chaine[128];
          int i;

          g_snprintf( chaine, sizeof(chaine), " Jeton au SSRV %02d\n", Partage->jeton );
          Write_admin ( Socket_write, chaine );

          for (i=0; i<Config.max_serveur; i++)
           { g_snprintf( chaine, sizeof(chaine), " SSRV[%02d] -> %02d clients\n",
                         i, Partage->Sous_serveur[i].nb_client );
             Write_admin ( Socket_write, chaine );
           }
        } else
       if ( ! strcmp ( commande, "kick" ) )
        { char chaine[128], nom[128], machine[128];
          GList *liste;
          gint i;

          memset( nom, 0, sizeof(nom) );
          memset( machine, 0, sizeof(machine) );
          sscanf ( ligne, "%s %s %s", commande, nom, machine );      /* Découpage de la ligne de commande */

          g_snprintf( chaine, sizeof(chaine), " Searching for client %s@%s ... \n",
                      nom, machine );
          Write_admin ( Socket_write, chaine );

          for (i=0; i<Config.max_serveur; i++)
            { liste = Partage->Sous_serveur[i].Clients;
              while(liste)                                            /* Parcours de la liste des clients */
               { struct CLIENT *client;
                 client = (struct CLIENT *)liste->data;

                 if ( (! strncmp( client->util->nom, nom, sizeof(client->util->nom))) &&
                      (! strncmp( client->machine, machine, sizeof(client->machine)))
                    )
                  { Client_mode ( client, DECONNECTE );
                    g_snprintf( chaine, sizeof(chaine),
                                " Found ... Kicking ... SSRV%02d - v%s %s@%s - mode %d defaut %d\n",
                                i, client->ident.version, client->util->nom, client->machine,
                               client->mode, client->defaut );
                    Write_admin ( Socket_write, chaine );
                  }
                 liste = liste->next;
               }
           }
        } else
       if ( ! strcmp ( commande, "client" ) )
        { char chaine[128];
          GList *liste;
          gint i;

          for (i=0; i<Config.max_serveur; i++)
            { if (Partage->Sous_serveur[i].pid == -1) continue;

              liste = Partage->Sous_serveur[i].Clients;
              while(liste)                                            /* Parcours de la liste des clients */
               { struct CLIENT *client;
                 client = (struct CLIENT *)liste->data;

                 g_snprintf( chaine, sizeof(chaine), " SSRV%02d - v%s %s@%s - mode %d defaut %d date %s\n",
                             i, client->ident.version, client->util->nom, client->machine,
                             client->mode, client->defaut, ctime(&client->seconde) );
                 Write_admin ( Socket_write, chaine );

                 liste = liste->next;
               }
            }
        } else
       if ( ! strcmp ( commande, "dlson" ) )
        { char chaine[20];
          int num;
          sscanf ( ligne, "%s %d", commande, &num );                 /* Découpage de la ligne de commande */
          Activer_plugins ( num, TRUE );
          g_snprintf( chaine, sizeof(chaine), " Plugin %d started\n", num );
          Write_admin ( Socket_write, chaine );
        } else
       if ( ! strcmp ( commande, "dlsoff" ) )
        { char chaine[20];
          int num;
          sscanf ( ligne, "%s %d", commande, &num );                 /* Découpage de la ligne de commande */
          Activer_plugins ( num, FALSE );
          g_snprintf( chaine, sizeof(chaine), " Plugin %d stopped\n", num );
          Write_admin ( Socket_write, chaine );
        } else
       if ( ! strcmp ( commande, "getm" ) )
        { char chaine[20];
          int num;
          sscanf ( ligne, "%s %d", commande, &num );                 /* Découpage de la ligne de commande */
          g_snprintf( chaine, sizeof(chaine), " M%03d = %d\n", num, M(num) );
          Write_admin ( Socket_write, chaine );
        } else
       if ( ! strcmp ( commande, "gete" ) )
        { char chaine[20];
          int num;
          sscanf ( ligne, "%s %d", commande, &num );                 /* Découpage de la ligne de commande */
          g_snprintf( chaine, sizeof(chaine), " E%03d = %d\n", num, E(num) );
          Write_admin ( Socket_write, chaine );
        } else
       if ( ! strcmp ( commande, "getea" ) )
        { char chaine[20];
          int num;
          sscanf ( ligne, "%s %d", commande, &num );                 /* Découpage de la ligne de commande */
          g_snprintf( chaine, sizeof(chaine), " EA%03d = %d, inrange=%d\n", num, EA_int(num), EA_inrange(num) );
          Write_admin ( Socket_write, chaine );
        } else
       if ( ! strcmp ( commande, "setm" ) )
        { char chaine[20];
          int num, val;
          sscanf ( ligne, "%s %d %d", commande, &num, &val );        /* Découpage de la ligne de commande */
          SM ( num, val );
          g_snprintf( chaine, sizeof(chaine), " M%03d = %d\n", num, val );
          Write_admin ( Socket_write, chaine );
        } else
       if ( ! strcmp ( commande, "getb" ) )
        { char chaine[20];
          int num;
          sscanf ( ligne, "%s %d", commande, &num );                 /* Découpage de la ligne de commande */
          g_snprintf( chaine, sizeof(chaine), " B%03d = %d\n", num, B(num) );
          Write_admin ( Socket_write, chaine );
        } else
       if ( ! strcmp ( commande, "setb" ) )
        { char chaine[20];
          int num, val;
          sscanf ( ligne, "%s %d %d", commande, &num, &val );        /* Découpage de la ligne de commande */
          SB ( num, val );
          g_snprintf( chaine, sizeof(chaine), " B%03d = %d\n", num, val );
          Write_admin ( Socket_write, chaine );
        } else
       if ( ! strcmp ( commande, "seta" ) )
        { char chaine[20];
          int num, val;
          sscanf ( ligne, "%s %d %d", commande, &num, &val );        /* Découpage de la ligne de commande */
          SA ( num, val );
          g_snprintf( chaine, sizeof(chaine), " A%03d = %d\n", num, val );
          Write_admin ( Socket_write, chaine );
        } else
       if ( ! strcmp ( commande, "msgs" ) )
        { char chaine[128], msg[128];
          GList *liste;
          gint i;

          memset( msg, 0, sizeof(msg) );
          sscanf ( ligne, "%s %s", commande, msg );                  /* Découpage de la ligne de commande */

          for (i=0; i<Config.max_serveur; i++)
            { if (Partage->Sous_serveur[i].pid == -1) continue;

              liste = Partage->Sous_serveur[i].Clients;
              while(liste)                                            /* Parcours de la liste des clients */
               { struct CMD_GTK_MESSAGE erreur;
                 struct CLIENT *client;
                 client = (struct CLIENT *)liste->data;


                 g_snprintf( erreur.message, sizeof(erreur.message), msg );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );

                 g_snprintf( chaine, sizeof(chaine), " Envoi du message a %s@%s\n",
                             client->util->nom, client->machine );
                 Write_admin ( Socket_write, chaine );

                 liste = liste->next;
               }
            }
        } else
       if ( ! strcmp ( commande, "ident" ) )
        { char chaine[128], nom[128];
          gethostname( nom, sizeof(nom) );
          g_snprintf( chaine, sizeof(chaine), " Watchdogd %s on %s\n", VERSION, nom );
          Write_admin ( Socket_write, chaine );
        } else
       if ( ! strcmp ( commande, "audit" ) )
        { char chaine[128], nom[128];
          gethostname( nom, sizeof(nom) );
          g_snprintf( chaine, sizeof(chaine), " Bit/s : %d\n", Partage->audit_bit_interne_per_sec_hold );
          Write_admin ( Socket_write, chaine );
        } else
       if ( ! strcmp ( commande, "ping" ) )
        { Write_admin ( Socket_write, " Pong !\n" );
        } else
       if ( ! strcmp ( commande, "shutdown" ) )
        { Info( Config.log, DEBUG_INFO, "Gerer_fifo_admin : SHUTDOWN demandé" );
          Write_admin ( Socket_write, "SHUTDOWN in progress\n" );
          Partage->Arret = FIN;
        } else
       if ( ! strcmp ( commande, "restart" ) )
        { Info( Config.log, DEBUG_INFO, "Gerer_fifo_admin : RESTART demandé" );
          Write_admin ( Socket_write, "RESTART in progress\n" );
          Partage->Arret = RESTART;
        } else
       if ( ! strcmp ( commande, "reload" ) )
        { Info( Config.log, DEBUG_INFO, "Gerer_fifo_admin : RELOAD demandé" );
          Write_admin ( Socket_write, "RELOAD in progress\n" );
          Partage->Arret = RELOAD;
        } else
       if ( ! strcmp ( commande, "help" ) )
        { Write_admin ( Socket_write, "  -- Watchdog ADMIN -- \n" );
          Write_admin ( Socket_write, "  audit                - Audit bit/s\n" );
          Write_admin ( Socket_write, "  ident                - ID du serveur Watchdog\n" );
          Write_admin ( Socket_write, "  dls                  - D.L.S. Status\n" );
          Write_admin ( Socket_write, "  dlson xx             - D.L.S. Start plugin xx\n" );
          Write_admin ( Socket_write, "  dlsoff xx            - D.L.S. Stop plugin xx\n" );
          Write_admin ( Socket_write, "  ssrv                 - SousServers Status\n" );
          Write_admin ( Socket_write, "  client               - Client Status\n" );
          Write_admin ( Socket_write, "  kick nom machine     - Kick client nom@machine\n" );
          Write_admin ( Socket_write, "  gete xxx             - Get Exxx\n" );
          Write_admin ( Socket_write, "  getea xxx            - Get EAxxx\n" );
          Write_admin ( Socket_write, "  getm xxx             - Get Mxxx\n" );
          Write_admin ( Socket_write, "  setm xxx i           - Set Mxxx = i\n" );
          Write_admin ( Socket_write, "  getb xxx             - Get Bxxx\n" );
          Write_admin ( Socket_write, "  setb xxx i           - Set Bxxx = i\n" );
          Write_admin ( Socket_write, "  seta xxx i           - Set Axxx = i\n" );
          Write_admin ( Socket_write, "  msgs message         - Envoi d'un message a tous les clients\n" );
          Write_admin ( Socket_write, "  ping                 - Ping Watchdog\n" );
          Write_admin ( Socket_write, "  help                 - This help\n" );
          Write_admin ( Socket_write, "  -- Watchdog ADMIN -- Use with CAUTION\n" );
          Write_admin ( Socket_write, "  restart              - Restart processes\n" );
          Write_admin ( Socket_write, "  reload               - Shutdown and Restart processes\n" );
          Write_admin ( Socket_write, "  shutdown             - Stop processes\n" );
        } else
       if ( ! strcmp ( commande, "nocde" ) )
        { 
        } else
        { char chaine[128];
          g_snprintf( chaine, sizeof(chaine), " - command %s not found -\n", commande );
          Write_admin ( Socket_write, chaine );
        }

       Write_admin( Socket_write, "#> " );
       close (Socket_write);
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
