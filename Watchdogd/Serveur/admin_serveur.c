/**********************************************************************************************************/
/* Watchdogd/Serveur/admin_serveur.c        Gestion des connexions Admin IMSG au serveur watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                  sam. 28 juil. 2012 16:35:09 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_serveur.c
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
 
 #include "watchdogd.h"
 #include "Sous_serveur.h"

/**********************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                 */
/* Entrée: Le connexion d'admin, la ligne a traiter                                                       */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[80];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

#ifdef bouh
    if ( ! strcmp ( commande, "ssrv" ) )
     { int i;

       g_snprintf( chaine, sizeof(chaine), " Jeton au SSRV %02d\n", Partage->jeton );
       Admin_write ( connexion, chaine );

       for (i=0; i<Config.max_serveur; i++)
        { pthread_mutex_lock( &Partage->Sous_serveur[i].synchro );
          g_snprintf( chaine, sizeof(chaine), " SSRV[%02d] -> %02d connexions\n",
                      i, g_list_length(Partage->Sous_serveur[i].Clients) );
          pthread_mutex_unlock( &Partage->Sous_serveur[i].synchro );
          Admin_write ( connexion, chaine );
        }
     } else
    if ( ! strcmp ( commande, "connexion" ) )
     { GList *liste;
       gint i;
        
       g_snprintf( chaine, sizeof(chaine), " -- Liste des connexions connectés au serveur\n" );
       Admin_write ( connexion, chaine );
       for (i=0; i<Config.max_serveur; i++)
         { if (Partage->Sous_serveur[i].Thread_run == FALSE) continue;

           pthread_mutex_lock( &Partage->Sous_serveur[i].synchro );
           liste = Partage->Sous_serveur[i].Clients;
           while(liste)                                               /* Parcours de la liste des connexions */
            { struct CONNEXION *connexion_srv;
              connexion_srv = (struct CONNEXION *)liste->data;

              g_snprintf( chaine, sizeof(chaine), " SSRV%02d - v%s %s@%s - mode %d defaut %d date %s",
                          i, connexion_srv->ident.version, connexion_srv->util->nom, connexion_srv->machine,
                          connexion_srv->mode, connexion_srv->defaut, ctime(&connexion_srv->date_connexion) );
              Admin_write ( connexion, chaine );     /* ctime ajoute un \n à la fin !! */

              liste = liste->next;
            }
           pthread_mutex_unlock( &Partage->Sous_serveur[i].synchro );
         }
     } else
    if ( ! strcmp ( commande, "msgs" ) )
     { GList *liste;
       gint i;

       g_snprintf( chaine, sizeof(chaine), " -- Liste des connexions recevant le message\n" );
       Admin_write ( connexion, chaine );
       for (i=0; i<Config.max_serveur; i++)
         { if (Partage->Sous_serveur[i].Thread_run == FALSE) continue;
           liste = Partage->Sous_serveur[i].Clients;
           while(liste)                                               /* Parcours de la liste des connexions */
            { struct CMD_GTK_MESSAGE erreur;
              struct CONNEXION *connexion_wat;
              connexion_wat = (struct CONNEXION *)liste->data;

              g_snprintf( erreur.message, sizeof(erreur.message), "AdminMSG : %s", ligne + 5 );
              Envoi_connexion( connexion_wat, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );

              g_snprintf( chaine, sizeof(chaine), " - %s@%s\n",
                          connexion_wat->util->nom, connexion_wat->machine );
              Admin_write ( connexion, chaine );
              liste = liste->next;
            }
         }
     } else
#endif
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
