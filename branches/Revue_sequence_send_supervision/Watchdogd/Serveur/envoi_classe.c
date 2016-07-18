/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_classe.c        Configuration des classes de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 30 sep 2003 12:20:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_classe.c
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
 #include <pthread.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Preparer_envoi_classe: convertit une structure MSG en structure CMD_TYPE_CLASSE                        */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_TYPE_CLASSE *Preparer_envoi_classe ( struct CLASSEDB *classe )
  { struct CMD_TYPE_CLASSE *rezo_classe;

    rezo_classe = (struct CMD_TYPE_CLASSE *)g_try_malloc0( sizeof(struct CMD_TYPE_CLASSE) );
    if (!rezo_classe) { return(NULL); }

    rezo_classe->id         = classe->id;
    memcpy( &rezo_classe->libelle, classe->libelle, sizeof(rezo_classe->libelle) );
    return( rezo_classe );
  }
/**********************************************************************************************************/
/* Proto_editer_classe: Le client desire editer un classe                                                 */
/* Entrée: le client demandeur et le classe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe )
  { struct CMD_TYPE_CLASSE edit_classe;
    struct CLASSEDB *classe;

    classe = Rechercher_classeDB( rezo_classe->id );

    if (classe)
     { edit_classe.id         = classe->id;                                 /* Recopie des info editables */
       memcpy( &edit_classe.libelle, classe->libelle, sizeof(edit_classe.libelle) );

       Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_EDIT_CLASSE_OK,
                  (gchar *)&edit_classe, sizeof(struct CMD_TYPE_CLASSE) );
       g_free(classe);                                                              /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate classe %s", rezo_classe->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_classe: Le client valide l'edition d'un classe                                    */
/* Entrée: le client demandeur et le classe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe )
  { struct CLASSEDB *result;
    gboolean retour;

    retour = Modifier_classeDB ( rezo_classe );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit classe %s", rezo_classe->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_classeDB( rezo_classe->id );
           if (result) 
            { struct CMD_TYPE_CLASSE *classe;
              classe = Preparer_envoi_classe ( result );
              g_free(result);
              if (!classe)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_VALIDE_EDIT_CLASSE_OK,
                                   (gchar *)classe, sizeof(struct CMD_TYPE_CLASSE) );
                     g_free(classe);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate classe %s", rezo_classe->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_classe: Retrait du classe en parametre                                                   */
/* Entrée: le client demandeur et le classe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe )
  { gboolean retour;

    retour = Retirer_classeDB( rezo_classe );

    if (retour)
     { Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_DEL_CLASSE_OK,
                     (gchar *)rezo_classe, sizeof(struct CMD_TYPE_CLASSE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete classe %s", rezo_classe->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_classe: Un client nous demande d'ajouter un classe Watchdog                              */
/* Entrée: le classe à créer                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_classe ( struct CLIENT *client, struct CMD_TYPE_CLASSE *rezo_classe )
  { struct CLASSEDB *result;
    gint id;

    id = Ajouter_classeDB ( rezo_classe );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add classe %s", rezo_classe->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_classeDB( id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate classe %s", rezo_classe->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_TYPE_CLASSE *classe;
              classe = Preparer_envoi_classe ( result );
              g_free(result);
              if (!classe)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_ADD_CLASSE_OK,
                                   (gchar *)classe, sizeof(struct CMD_TYPE_CLASSE) );
                     g_free(classe);
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_classes: Envoi des classes au client GID_CLASSE                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_classes_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_TYPE_CLASSE *rezo_classe;
    struct CMD_ENREG nbr;
    struct CLASSEDB *classe;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiCLASSE", 0, 0, 0 );

    if ( ! Recuperer_classeDB( &db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d classes", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { classe = Recuperer_classeDB_suite( &db );
       if (!classe)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          return;
        }

       rezo_classe = Preparer_envoi_classe( classe );
       g_free(classe);
       if (rezo_classe)
        { Envoi_client ( client, tag, sstag,
                         (gchar *)rezo_classe, sizeof(struct CMD_TYPE_CLASSE) );
          g_free(rezo_classe);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_classes: Envoi des classes au client GID_CLASSE                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_classes_thread ( struct CLIENT *client )
  { Envoyer_classes_tag( client, TAG_ICONE, SSTAG_SERVEUR_ADDPROGRESS_CLASSE,
                                            SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FIN );
    pthread_exit ( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_classes: Envoi des classes au client GID_CLASSE                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_classes_pour_atelier_thread ( struct CLIENT *client )
  { Envoyer_classes_tag( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FOR_ATELIER,
                                              SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FOR_ATELIER_FIN );
    pthread_exit ( NULL );
  }
/*--------------------------------------------------------------------------------------------------------*/
