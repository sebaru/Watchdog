/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_courbe.c        Configuration des courbes de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 12 jun 2005 18:01:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_courbe.c
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
 #include <sys/prctl.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Proto_effacer_entree: Retrait du entree en parametre                                                   */
/* Entrée: le client demandeur et le entree en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_courbe ( struct CLIENT *client, struct CMD_TYPE_COURBE *rezo_courbe )
  { struct CMD_TYPE_COURBE *courbe;
    GList *liste_courbe;
    courbe = NULL;
    liste_courbe = client->courbes;                                          /* Recherche de la structure */
    while (liste_courbe)
     { courbe = (struct CMD_TYPE_COURBE *)liste_courbe->data;
       if (courbe->slot_id == rezo_courbe->slot_id)
        { client->courbes = g_list_remove( client->courbes, courbe );
          return;
        }
       liste_courbe = liste_courbe -> next;
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_entree: Un client nous demande d'ajouter un entree Watchdog                              */
/* Entrée: le entree à créer                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_courbe_thread ( struct CLIENT *client )
  { struct CMD_START_COURBE *envoi_courbe;
    struct ARCHDB *arch;
    struct CMD_TYPE_COURBE rezo_courbe;
    struct CMD_TYPE_COURBE *courbe;
    struct DB *db;
    GList *liste_courbe;
    guint max_enreg;
    time_t date;

    prctl(PR_SET_NAME, "W-EnvoiCOURBE", 0, 0, 0 );

    memcpy ( &rezo_courbe, &client->courbe, sizeof( struct CMD_TYPE_COURBE ) );
                 /* La sauvegarde en local a été effectuée, nous indiquons au master qu'il peut continuer */
    client->courbe.num=-1;

/******************************************** Recherche si courbe deja existante **************************/
    courbe = NULL;
    liste_courbe = client->courbes;                         /* Recherche d'une structure deja initialisée */
    while (liste_courbe)
     { courbe = (struct CMD_TYPE_COURBE *)liste_courbe->data;
       if (courbe->num == rezo_courbe.num && courbe->type == rezo_courbe.type) break;
       liste_courbe = liste_courbe -> next;
     }

    if ( liste_courbe )           /* Si on en a trouvé une, c'est qu'il ne faut pas l'ajouter a nouveau ! */
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message), "Courbe deja affichee" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit(NULL);
     }
                                                                    /* Sinon, on l'ajoute dans la liste ! */
    courbe = (struct CMD_TYPE_COURBE *)g_try_malloc0( sizeof( struct CMD_TYPE_COURBE ) );
    if (!courbe)
     { struct CMD_GTK_MESSAGE erreur;
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "Proto_ajouter_courbe_thread: Pb d'allocation memoire" );
       g_snprintf( erreur.message, sizeof(erreur.message), "Pb d'allocation memoire" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit(NULL);
     }
    memcpy ( courbe, &rezo_courbe, sizeof( struct CMD_TYPE_COURBE ) );
        
/******************************************** Préparation structure d'envoi *******************************/
    envoi_courbe = (struct CMD_START_COURBE *)g_try_malloc0( Cfg_ssrv.taille_bloc_reseau );
    if (!envoi_courbe)
     { struct CMD_GTK_MESSAGE erreur;
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "Proto_ajouter_courbe_thread: Pb d'allocation memoire envoi_courbe" );
       g_snprintf( erreur.message, sizeof(erreur.message), "Pb d'allocation memoire" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       g_free(courbe);
       pthread_exit(NULL);
     }
    envoi_courbe->slot_id        = courbe->slot_id;         /* Valeurs par defaut si pas d'enregistrement */
    envoi_courbe->type           = courbe->type;
    envoi_courbe->taille_donnees = 0;

    db = Init_DB_SQL();       
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "Proto_ajouter_courbe_thread: Unable to open database (dsn)" );
       g_free(courbe);
       g_free(envoi_courbe);
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */
       
    Envoi_client ( client, TAG_COURBE, SSTAG_SERVEUR_ADD_COURBE_OK,        /* Envoi préparation au client */
                   (gchar *)&rezo_courbe, sizeof(struct CMD_TYPE_COURBE) );

/******************************************** Préparation des buffers d'envoi *****************************/
    date = time(NULL);                                                    /* On recupere la date actuelle */

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG, "Proto_ajouter_courbe_thread: début d'envoi" );
    max_enreg = (Cfg_ssrv.taille_bloc_reseau - sizeof(struct CMD_START_COURBE)) / sizeof(struct CMD_START_COURBE_VALEUR);
    if (Recuperer_archDB ( Config.log, db, courbe->type, courbe->num,
                          (date - COURBE_NBR_HEURE_ARCHIVE*3600), date ))
     { do
        { arch = Recuperer_archDB_suite( Config.log, db );                  /* On prend le premier enreg. */
          if (arch)                                            /* Si enregegistrement, alors on le pousse */
           { envoi_courbe->valeurs[envoi_courbe->taille_donnees].date    = arch->date_sec;
             envoi_courbe->valeurs[envoi_courbe->taille_donnees].val_ech = arch->valeur;
             envoi_courbe->taille_donnees++;/* Nous avons 1 enregistrement de plus dans la structure d'envoi */
             g_free(arch);
           }

          if ( (arch == NULL) || envoi_courbe->taille_donnees == max_enreg )
           { Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_START_COURBE, (gchar *)envoi_courbe,
                           sizeof(struct CMD_START_COURBE) + envoi_courbe->taille_donnees * sizeof(struct CMD_START_COURBE_VALEUR) );
             Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                      "Proto_ajouter_courbe_thread: taille donnees=%d", envoi_courbe->taille_donnees );
             envoi_courbe->taille_donnees = 0;
           }
        }
       while (arch);                                       /* On tourne tant qu'il y a des enregistrement */
     }

    g_free(envoi_courbe);                             /* Nous n'avons plus besoin de la structure d'envoi */
    client->courbes = g_list_append ( client->courbes, courbe ); /* Ajout dans la liste des courbes a updater */
    Libere_DB_SQL( &db );
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
