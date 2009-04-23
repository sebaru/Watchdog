/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_icone.c        Configuration des icones de Watchdog v2.0                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 01 nov 2003 13:13:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_icone.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2007 - Sébastien Lefevre
 *
 * <program name> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 #include <glib.h>
 #include <sys/prctl.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

 #include <sys/stat.h>
 #include <sys/types.h>
 #include <fcntl.h>

 #include "Reseaux.h"
 #include "Icones_DB.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "Client.h"

 #include "watchdogd.h"
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Preparer_envoi_icone: convertit une structure MSG en structure CMD_SHOW_ICONE                          */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_ICONE *Preparer_envoi_icone ( struct ICONEDB *icone )
  { struct CMD_SHOW_ICONE *rezo_icone;

    rezo_icone = (struct CMD_SHOW_ICONE *)g_malloc0( sizeof(struct CMD_SHOW_ICONE) );
    if (!rezo_icone) { return(NULL); }

    rezo_icone->id         = icone->id;
    rezo_icone->id_classe  = icone->id_classe;
    memcpy( &rezo_icone->libelle, icone->libelle, sizeof(rezo_icone->libelle) );
    return( rezo_icone );
  }
/**********************************************************************************************************/
/* Proto_editer_icone: Le client desire editer un icone                                                   */
/* Entrée: le client demandeur et le icone en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_icone ( struct CLIENT *client, struct CMD_ID_ICONE *rezo_icone )
  { struct CMD_EDIT_ICONE edit_icone;
    struct ICONEDB *icone;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    icone = Rechercher_iconeDB( Config.log, Db_watchdog, rezo_icone->id );

    if (icone)
     { edit_icone.id         = icone->id;                                   /* Recopie des info editables */
       edit_icone.id_classe  = icone->id_classe;
       memcpy( &edit_icone.libelle, icone->libelle, sizeof(edit_icone.libelle) );

       Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_EDIT_ICONE_OK,
                  (gchar *)&edit_icone, sizeof(struct CMD_EDIT_ICONE) );
       g_free(icone);                                                               /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate icone %s", rezo_icone->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_icone: Le client valide l'edition d'un icone                                      */
/* Entrée: le client demandeur et le icone en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_icone ( struct CLIENT *client, struct CMD_EDIT_ICONE *rezo_icone )
  { struct ICONEDB *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_iconeDB ( Config.log, Db_watchdog, rezo_icone );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit icone %s", rezo_icone->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_iconeDB( Config.log, Db_watchdog, rezo_icone->id );
           if (result) 
            { struct CMD_SHOW_ICONE *icone;
              icone = Preparer_envoi_icone ( result );
              g_free(result);
              if (!icone)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_VALIDE_EDIT_ICONE_OK,
                                   (gchar *)icone, sizeof(struct CMD_SHOW_ICONE) );
                     g_free(icone);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate icone %s", rezo_icone->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_icone: Retrait du icone en parametre                                                     */
/* Entrée: le client demandeur et le icone en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_icone ( struct CLIENT *client, struct CMD_ID_ICONE *rezo_icone )
  { gchar nom_fichier[80];
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_iconeDB( Config.log, Db_watchdog, rezo_icone );
printf("Proto_effacer_icone: id=%d retour = %d\n", rezo_icone->id, retour );

    if (retour)
     { Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_DEL_ICONE_OK,
                     (gchar *)rezo_icone, sizeof(struct CMD_ID_ICONE) );
       g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%d.gif", rezo_icone->id );
       unlink(nom_fichier);
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete icone %s", rezo_icone->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_icone: Un client nous demande d'ajouter un icone Watchdog                                */
/* Entrée: le icone à créer                                                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_icone ( struct CLIENT *client, struct CMD_ADD_ICONE *rezo_icone )
  { struct ICONEDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_iconeDB ( Config.log, Db_watchdog, rezo_icone );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add icone %s", rezo_icone->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_iconeDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate icone %s", rezo_icone->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_SHOW_ICONE *icone;
              icone = Preparer_envoi_icone ( result );
              g_free(result);
              if (!icone)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { rezo_icone->id = id;
                     Info_c( Config.log, DEBUG_INFO, "Envoi demande file icone", rezo_icone->libelle );
                     Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_ADD_ICONE_WANT_FILE,
                                   (gchar *)rezo_icone, sizeof(struct CMD_ADD_ICONE) );
                     Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_ADD_ICONE_OK,
                                   (gchar *)icone, sizeof(struct CMD_SHOW_ICONE) );
                     g_free(icone);
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Proto_ajouter_icone_deb_file: Debut de reception du fichier gif associé à l'icone                      */
/* Entrée: le icone à créer                                                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_icone_deb_file( struct CLIENT *client, struct CMD_ADD_ICONE *icone )
  { gchar nom_fichier[80];
    g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%s", icone->nom_fichier );
    Info_c ( Config.log, DEBUG_INFO, "Rapatriement du fichier", icone->nom_fichier );
    unlink( nom_fichier );
  }
/**********************************************************************************************************/
/* Proto_ajouter_icone_file: Reception du fichier gif associé à l'icone                                   */
/* Entrée: le icone à créer                                                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_icone_file( struct CLIENT *client, struct CMD_ADD_ICONE *icone,
                                gint taille, gchar *buffer )
  { gchar nom_fichier[80];
    gint id, test;
    g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%s", icone->nom_fichier );
    id = open( nom_fichier, O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR );
    if (id<0) return;
    test = write( id, buffer, taille );
    close(id);
  }
/**********************************************************************************************************/
/* Proto_ajouter_icone_fin_file: Fin de reception du fichier gif associé à l'icone                        */
/* Entrée: le icone à créer                                                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_icone_fin_file( struct CLIENT *client, struct CMD_ADD_ICONE *icone )
  { Changer_version_donnees( Config.log, 0 );          /* Mise à jour de la date de dernière modification */
  }
/**********************************************************************************************************/
/* Envoyer_icones: Envoi des icones au client GID_ICONE                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_icones_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_SHOW_ICONE *rezo_icone;
    struct CMD_ENREG nbr;
    struct ICONEDB *icone;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiICO", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_iconeDB( Config.log, db, client->classe_icone ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d  icones", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { icone = Recuperer_iconeDB_suite( Config.log, db );
       if (!icone)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          return;
        }

       rezo_icone = Preparer_envoi_icone( icone );
       g_free(icone);
       if (rezo_icone)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Envoi_client ( client, tag, sstag,
                         (gchar *)rezo_icone, sizeof(struct CMD_SHOW_ICONE) );
          g_free(rezo_icone);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_icones: Envoi des icones au client GID_ICONE                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_icones_thread ( struct CLIENT *client )
  { Envoyer_icones_tag( client, TAG_ICONE, SSTAG_SERVEUR_ADDPROGRESS_ICONE,
                                           SSTAG_SERVEUR_ADDPROGRESS_ICONE_FIN );
    pthread_exit ( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_icones: Envoi des icones au client GID_ICONE                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_icones_pour_atelier_thread ( struct CLIENT *client )
  { Envoyer_icones_tag( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ICONE_FOR_ATELIER,
                                             SSTAG_SERVEUR_ADDPROGRESS_ICONE_FOR_ATELIER_FIN );
    pthread_exit ( NULL );
  }
/*--------------------------------------------------------------------------------------------------------*/
