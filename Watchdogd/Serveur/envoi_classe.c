/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_classe.c        Configuration des classes de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 30 sep 2003 12:20:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

 #include "Reseaux.h"
 #include "Icones_DB.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "Client.h"

 #include "watchdogd.h"
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Preparer_envoi_classe: convertit une structure MSG en structure CMD_SHOW_CLASSE                        */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_CLASSE *Preparer_envoi_classe ( struct CLASSEDB *classe )
  { struct CMD_SHOW_CLASSE *rezo_classe;

    rezo_classe = (struct CMD_SHOW_CLASSE *)g_malloc0( sizeof(struct CMD_SHOW_CLASSE) );
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
 void Proto_editer_classe ( struct CLIENT *client, struct CMD_ID_CLASSE *rezo_classe )
  { struct CMD_EDIT_CLASSE edit_classe;
    struct CLASSEDB *classe;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    classe = Rechercher_classeDB( Config.log, Db_watchdog, rezo_classe->id );

    if (classe)
     { edit_classe.id         = classe->id;                                 /* Recopie des info editables */
       memcpy( &edit_classe.libelle, classe->libelle, sizeof(edit_classe.libelle) );

       Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_EDIT_CLASSE_OK,
                  (gchar *)&edit_classe, sizeof(struct CMD_EDIT_CLASSE) );
       g_free(classe);                                                              /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to locate classe %s:\n%s"), rezo_classe->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_classe: Le client valide l'edition d'un classe                                    */
/* Entrée: le client demandeur et le classe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_classe ( struct CLIENT *client, struct CMD_EDIT_CLASSE *rezo_classe )
  { struct CLASSEDB *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_classeDB ( Config.log, Db_watchdog, rezo_classe );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to edit classe %s:\n%s"), rezo_classe->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_classeDB( Config.log, Db_watchdog, rezo_classe->id );
           if (result) 
            { struct CMD_SHOW_CLASSE *classe;
              classe = Preparer_envoi_classe ( result );
              g_free(result);
              if (!classe)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_VALIDE_EDIT_CLASSE_OK,
                                   (gchar *)classe, sizeof(struct CMD_SHOW_CLASSE) );
                     g_free(classe);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate classe %s:\n%s"), rezo_classe->libelle, Db_watchdog->last_err);
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
 void Proto_effacer_classe ( struct CLIENT *client, struct CMD_ID_CLASSE *rezo_classe )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    if (rezo_classe->id == 0) return;                          /* La classe 'Default' n'est pas effacable */
    retour = Retirer_classeDB( Config.log, Db_watchdog, rezo_classe );

    if (retour)
     { Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_DEL_CLASSE_OK,
                     (gchar *)rezo_classe, sizeof(struct CMD_ID_CLASSE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to delete classe %s:\n%s"), rezo_classe->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_classe: Un client nous demande d'ajouter un classe Watchdog                              */
/* Entrée: le classe à créer                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_classe ( struct CLIENT *client, struct CMD_ADD_CLASSE *rezo_classe )
  { struct CLASSEDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_classeDB ( Config.log, Db_watchdog, rezo_classe );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to add classe %s:\n%s"), rezo_classe->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_classeDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate classe %s:\n%s"), rezo_classe->libelle, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_SHOW_CLASSE *classe;
              classe = Preparer_envoi_classe ( result );
              g_free(result);
              if (!classe)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_ADD_CLASSE_OK,
                                   (gchar *)classe, sizeof(struct CMD_SHOW_CLASSE) );
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
  { struct CMD_SHOW_CLASSE *rezo_classe;
    struct CMD_ENREG nbr;
    struct CLASSEDB *classe;
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_classeDB( Config.log, Db_watchdog );
    if (!hquery)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading classes") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { classe = Recuperer_classeDB_suite( Config.log, Db_watchdog, hquery );
       if (!classe)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       rezo_classe = Preparer_envoi_classe( classe );
       g_free(classe);
       if (rezo_classe)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Envoi_client ( client, tag, sstag,
                         (gchar *)rezo_classe, sizeof(struct CMD_SHOW_CLASSE) );
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
