/**********************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.c  Gestion des modules ONDULEUR Watchdgo 2.0                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 10 nov. 2009 15:56:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Onduleur.c
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
 #include <stdio.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <upsclient.h>

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */

/**********************************************************************************************************/
/* Retirer_onduleurDB: Elimination d'un onduleur                                                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_onduleurDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_ONDULEUR *onduleur )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_ONDULEUR, onduleur->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_onduleurDB: Ajout ou edition d'un onduleur                                                     */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure onduleur                      */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_onduleurDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_ONDULEUR *onduleur )
  { gchar *host, *ups, *libelle;
    gchar requete[2048];
    
    host = Normaliser_chaine ( log, onduleur->host );                    /* Formatage correct des chaines */
    if (!host)
     { Info( log, DEBUG_ONDULEUR, "Ajouter_onduleurDB: Normalisation host impossible" );
       return(-1);
     }
    ups = Normaliser_chaine ( log, onduleur->ups );                      /* Formatage correct des chaines */
    if (!ups)
     { g_free(host);
       Info( log, DEBUG_ONDULEUR, "Ajouter_onduleurDB: Normalisation ups impossible" );
       return(-1);
     }
    libelle = Normaliser_chaine ( log, onduleur->libelle );              /* Formatage correct des chaines */
    if (!libelle)
     { g_free(host);
       g_free(ups);
       Info( log, DEBUG_ONDULEUR, "Ajouter_onduleurDB: Normalisation libelle impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s"
                "(host,ups,libelle,bit_comm,actif,ea_min,e_min,a_min) "
                "VALUES ('%s','%s','%s',%d,%d,%d,%d,%d)",
                NOM_TABLE_ONDULEUR, host, ups, libelle, onduleur->bit_comm, onduleur->actif,
                onduleur->ea_min, onduleur->e_min, onduleur->a_min
              );
    g_free(host);
    g_free(ups);
    g_free(libelle);

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_onduleurDB: Recupération de la liste des ids des onduleurs                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_onduleurDB ( struct LOG *log, struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,host,ups,bit_comm,actif,ea_min,libelle,e_min_a_min "
                " FROM %s ORDER BY host,ups", NOM_TABLE_ONDULEUR );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_onduleurDB: Recupération de la liste des ids des onduleurs                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_ONDULEUR *Recuperer_onduleurDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_ONDULEUR *onduleur;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    onduleur = (struct CMD_TYPE_ONDULEUR *)g_malloc0( sizeof(struct CMD_TYPE_ONDULEUR) );
    if (!onduleur) Info( log, DEBUG_ONDULEUR, "Recuperer_onduleurDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &onduleur->host,    db->row[1], sizeof(onduleur->host   ) );
       memcpy( &onduleur->ups,     db->row[2], sizeof(onduleur->ups    ) );
       memcpy( &onduleur->libelle, db->row[6], sizeof(onduleur->libelle) );
       onduleur->id                = atoi(db->row[0]);
       onduleur->bit_comm          = atoi(db->row[3]);
       onduleur->actif             = atoi(db->row[4]);
       onduleur->ea_min            = atoi(db->row[5]);
       onduleur->e_min             = atoi(db->row[7]);
       onduleur->a_min             = atoi(db->row[8]);
     }
    return(onduleur);
  }
/**********************************************************************************************************/
/* Rechercher_onduleurDB: Recupération du onduleur dont le id est en parametre                            */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_ONDULEUR *Rechercher_onduleurDB ( struct LOG *log, struct DB *db, guint id )
  { gchar requete[256];
    struct CMD_TYPE_ONDULEUR *onduleur;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT host,ups,bit_comm,actif,ea_min,libelle,e_min,a_min "
                " FROM %s WHERE id=%d",
                NOM_TABLE_ONDULEUR, id );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_ONDULEUR, "Rechercher_onduleurDB: ONDULEUR non trouvé dans la BDD", id );
       return(NULL);
     }

    onduleur = g_malloc0( sizeof(struct CMD_TYPE_ONDULEUR) );
    if (!onduleur)
     { Info( log, DEBUG_ONDULEUR, "Rechercher_onduleurDB: Mem error" ); }
    else
     { memcpy( &onduleur->host,    db->row[0], sizeof(onduleur->host   ) );
       memcpy( &onduleur->ups,     db->row[1], sizeof(onduleur->ups    ) );
       memcpy( &onduleur->libelle, db->row[5], sizeof(onduleur->libelle) );
       onduleur->bit_comm          = atoi(db->row[2]);
       onduleur->actif             = atoi(db->row[3]);
       onduleur->ea_min            = atoi(db->row[4]);
       onduleur->e_min             = atoi(db->row[6]);
       onduleur->a_min             = atoi(db->row[7]);
       onduleur->id                = id;
     }
    Liberer_resultat_SQL ( log, db );
    return(onduleur);
  }
/**********************************************************************************************************/
/* Modifier_onduleurDB: Modification d'un onduleur Watchdog                                               */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_onduleurDB_set_start( struct LOG *log, struct DB *db, gint id, gint start )

  { gchar requete[128];

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=%d WHERE id=%d",
                NOM_TABLE_ONDULEUR, start, id
              );

    return ( Lancer_requete_SQL ( Config.log, db, requete ) );
  }
/**********************************************************************************************************/
/* Modifier_onduleurDB: Modification d'un onduleur Watchdog                                               */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_onduleurDB( struct LOG *log, struct DB *db, struct CMD_TYPE_ONDULEUR *onduleur )
  { gchar *host, *ups, *libelle;
    gchar requete[2048];

    host = Normaliser_chaine ( log, onduleur->host );                    /* Formatage correct des chaines */
    if (!host)
     { Info( log, DEBUG_ONDULEUR, "Modifier_onduleurDB: Normalisation host impossible" );
       return(-1);
     }
    ups = Normaliser_chaine ( log, onduleur->ups );                      /* Formatage correct des chaines */
    if (!ups)
     { g_free(host);
       Info( log, DEBUG_ONDULEUR, "Modifier_onduleurDB: Normalisation ups impossible" );
       return(-1);
     }
    libelle = Normaliser_chaine ( log, onduleur->libelle );              /* Formatage correct des chaines */
    if (!libelle)
     { g_free(host);
       g_free(ups);
       Info( log, DEBUG_ONDULEUR, "Modifier_onduleurDB: Normalisation libelle impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "host='%s',ups='%s',bit_comm=%d,actif=%d,"
                "ea_min=%d,e_min=%d,a_min=%d,"
                "libelle='%s' "
                "WHERE id=%d",
                NOM_TABLE_ONDULEUR, host, ups, onduleur->bit_comm, onduleur->actif,
                                    onduleur->ea_min, onduleur->e_min, onduleur->a_min,
                                    libelle,
                onduleur->id );
    g_free(host);
    g_free(ups);
    g_free(libelle);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Charger_tous_ONDULEUR: Requete la DB pour charger les modules onduleur                                 */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static struct MODULE_ONDULEUR *Chercher_module_by_id ( gint id )
  { GList *liste;
    liste = Partage->com_onduleur.Modules_ONDULEUR;
    while ( liste )
     { struct MODULE_ONDULEUR *module;
       module = ((struct MODULE_ONDULEUR *)liste->data);
       if (module->onduleur.id == id) return(module);
       liste = liste->next;
     }
    return(NULL);
  }
/**********************************************************************************************************/
/* Charger_tous_ONDULEUR: Requete la DB pour charger les modules onduleur                                 */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_tous_ONDULEUR ( void  )
  { struct DB *db;
    gint cpt;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

/********************************************** Chargement des modules ************************************/
    if ( ! Recuperer_onduleurDB( Config.log, db ) )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    Partage->com_onduleur.Modules_ONDULEUR = NULL;
    cpt = 0;
    for ( ; ; )
     { struct MODULE_ONDULEUR *module;
       struct CMD_TYPE_ONDULEUR *onduleur;

       onduleur = Recuperer_onduleurDB_suite( Config.log, db );
       if (!onduleur) break;

       module = (struct MODULE_ONDULEUR *)g_malloc0( sizeof(struct MODULE_ONDULEUR) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info( Config.log, DEBUG_ONDULEUR,
                "Charger_tous_ONDULEUR: Erreur allocation mémoire struct MODULE_ONDULEUR" );
          g_free(onduleur);
          Libere_DB_SQL( Config.log, &db );
          return(FALSE);
        }
       memcpy( &module->onduleur, onduleur, sizeof(struct CMD_TYPE_ONDULEUR) );
       g_free(onduleur);
       cpt++;                                              /* Nous avons ajouté un module dans la liste ! */
                                                                        /* Ajout dans la liste de travail */
       pthread_mutex_lock( &Partage->com_onduleur.synchro );
       Partage->com_onduleur.Modules_ONDULEUR = g_list_append ( Partage->com_onduleur.Modules_ONDULEUR, module );
       pthread_mutex_unlock( &Partage->com_onduleur.synchro );
       Info_n( Config.log, DEBUG_ONDULEUR, "Charger_modules_ONDULEUR:  id    = ", module->onduleur.id   );
       Info_c( Config.log, DEBUG_ONDULEUR, "                        -  host  = ", module->onduleur.host );
     }
    Info_n( Config.log, DEBUG_ONDULEUR, "Charger_tous_ONDULEUR: module ONDULEUR found  !", cpt );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Rechercher_onduleurDB: Recupération du onduleur dont le num est en parametre                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Charger_un_ONDULEUR ( gint id )
  { struct MODULE_ONDULEUR *module;
    struct CMD_TYPE_ONDULEUR *onduleur;
    struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db) return;

    module = (struct MODULE_ONDULEUR *)g_malloc0( sizeof(struct MODULE_ONDULEUR) );
    if (!module)                                                      /* Si probleme d'allocation mémoire */
     { Info( Config.log, DEBUG_ONDULEUR,
             "Charger_un_ONDULEUR: Erreur allocation mémoire struct MODULE_ONDULEUR" );
       Libere_DB_SQL( Config.log, &db );
       return;
     }

    onduleur = Rechercher_onduleurDB( Config.log, db, id );
    Libere_DB_SQL( Config.log, &db );
    if (!onduleur)                                                 /* Si probleme d'allocation mémoire */
     { Info( Config.log, DEBUG_ONDULEUR,
             "Charger_un_ONDULEUR: Erreur allocation mémoire struct CMD_TYPE_ONDULEUR" );
       g_free(module);
       return;
     }
    memcpy( &module->onduleur, onduleur, sizeof(struct CMD_TYPE_ONDULEUR) );
    g_free(onduleur);

    pthread_mutex_lock( &Partage->com_onduleur.synchro );
    Partage->com_onduleur.Modules_ONDULEUR = g_list_append ( Partage->com_onduleur.Modules_ONDULEUR, module );
    pthread_mutex_unlock( &Partage->com_onduleur.synchro );
  }
/**********************************************************************************************************/
/* Rechercher_onduleurDB: Recupération du onduleur dont le num est en parametre                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_un_ONDULEUR ( struct MODULE_ONDULEUR *module )
  { if (!module) return;
    pthread_mutex_lock( &Partage->com_onduleur.synchro );
    Partage->com_onduleur.Modules_ONDULEUR = g_list_remove ( Partage->com_onduleur.Modules_ONDULEUR, module );
    g_free(module);
    pthread_mutex_unlock( &Partage->com_onduleur.synchro );
  }
/**********************************************************************************************************/
/* Decharger_tous_ONDULEUR: Decharge l'ensemble des modules ONDULEUR                                          */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Decharger_tous_ONDULEUR ( void  )
  { struct MODULE_ONDULEUR *module;
    while ( Partage->com_onduleur.Modules_ONDULEUR )
     { module = (struct MODULE_ONDULEUR *)Partage->com_onduleur.Modules_ONDULEUR->data;
       Decharger_un_ONDULEUR ( module );
     }
  }
/**********************************************************************************************************/
/* Deconnecter: Deconnexion du module                                                                     */
/* Entrée: un id                                                                                          */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Deconnecter_module ( struct MODULE_ONDULEUR *module )
  { if (!module) return;
    if (module->started == FALSE) return;

    upscli_disconnect( &module->upsconn );
    module->started = FALSE;
    module->nbr_deconnect++;
    module->date_retente = 0;
    Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Deconnecter_module", module->onduleur.id );
    SB( module->onduleur.bit_comm, 0 );                       /* Mise a zero du bit interne lié au module */
  }
/**********************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                           */
/* Entrée: une nom et un password                                                                         */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                          */
/**********************************************************************************************************/
 static gboolean Connecter_module ( struct MODULE_ONDULEUR *module )
  { int connexion;

    if ( (connexion = upscli_connect( &module->upsconn, module->onduleur.host,
                                      ONDULEUR_PORT_TCP, UPSCLI_CONN_TRYSSL)) == -1 )
     { Info_c( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Connecter_module: connexion refused by module",
               (char *)upscli_strerror(&module->upsconn) );
       return(FALSE);
     }

    Info_c( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Connecter_module", module->onduleur.host );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Modbus_is_actif: Renvoi TRUE si au moins un des modules modbus est actif                               */
/* Entrée: rien                                                                                           */
/* Sortie: TRUE/FALSE                                                                                     */
/**********************************************************************************************************/
 static gboolean Onduleur_is_actif ( void )
  { GList *liste;
    liste = Partage->com_onduleur.Modules_ONDULEUR;
    while ( liste )
     { struct MODULE_ONDULEUR *module;
       module = ((struct MODULE_ONDULEUR *)liste->data);

       if (module->onduleur.actif) return(TRUE);
       liste = liste->next;
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Interroger_onduleur: Interrogation d'un onduleur                                                       */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static gboolean Interroger_onduleur( struct MODULE_ONDULEUR *module )
  { const char *query[3];
    guint numa, valeur;
    char **answer;
    int retour, num_ea;;


    query[0] = "VAR";                      /* Initialisation des variables communes à toutes les demandes */
    query[1] = module->onduleur.ups;
    num_ea = module->onduleur.ea_min;

    query[2] = "ups.load";
    retour = upscli_get( &module->upsconn, 3, query, &numa, &answer);
    if (retour == -1)
     { if (upscli_upserror(&module->upsconn) != UPSCLI_ERR_VARNOTSUPP)        /* Variable non supportée ? */
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Interroger_onduleur: Wrong ANSWER ups.load",
                  upscli_upserror(&module->upsconn) );
          return(FALSE);
        }
     }
    else { valeur = atoi (answer[3]);
           SEA( num_ea++, valeur );                                      /* Numéro de l'EA pour la valeur */
         }

    query[2] = "ups.realpower";
    retour = upscli_get( &module->upsconn, 3, query, &numa, &answer);
    if (retour == -1)
     { if (upscli_upserror(&module->upsconn) != UPSCLI_ERR_VARNOTSUPP)        /* Variable non supportée ? */
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Interroger_onduleur: Wrong ANSWER ups.realpower",
                  upscli_upserror(&module->upsconn) );
          return(FALSE);
        }
     }
    else { valeur = atoi (answer[3]);
           SEA( num_ea++, valeur );                                      /* Numéro de l'EA pour la valeur */
         }

    query[2] = "battery.charge";
    retour = upscli_get( &module->upsconn, 3, query, &numa, &answer);
    if (retour == -1)
     { if (upscli_upserror(&module->upsconn) != UPSCLI_ERR_VARNOTSUPP)        /* Variable non supportée ? */
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Interroger_onduleur: Wrong ANSWER battery.charge",
                  upscli_upserror(&module->upsconn) );
          return(FALSE);
        }
     }
    else { valeur = atoi (answer[3]);
           SEA( num_ea++, valeur );                                      /* Numéro de l'EA pour la valeur */
         }

    query[2] = "input.voltage";
    retour = upscli_get( &module->upsconn, 3, query, &numa, &answer);
    if (retour == -1)
     { if (upscli_upserror(&module->upsconn) != UPSCLI_ERR_VARNOTSUPP)        /* Variable non supportée ? */
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Interroger_onduleur: Wrong ANSWER input_voltage",
                  upscli_upserror(&module->upsconn) );
          return(FALSE);
        }
     }
    else { valeur = atoi (answer[3]);
           SEA( num_ea++, valeur );                                      /* Numéro de l'EA pour la valeur */
         }

    query[2] = "battery.runtime";
    retour = upscli_get( &module->upsconn, 3, query, &numa, &answer);
    if (retour == -1)
     { if (upscli_upserror(&module->upsconn) != UPSCLI_ERR_VARNOTSUPP)        /* Variable non supportée ? */
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Interroger_onduleur: Wrong ANSWER battery.runtime",
                  upscli_upserror(&module->upsconn) );
          return(FALSE);
        }
     }
    else { valeur = atoi (answer[3]);
           SEA( num_ea++, valeur );                                      /* Numéro de l'EA pour la valeur */
         }

    query[2] = "battery.voltage";
    retour = upscli_get( &module->upsconn, 3, query, &numa, &answer);
    if (retour == -1)
     { if (upscli_upserror(&module->upsconn) != UPSCLI_ERR_VARNOTSUPP)        /* Variable non supportée ? */
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Interroger_onduleur: Wrong ANSWER battery.voltage",
                  upscli_upserror(&module->upsconn) );
          return(FALSE);
        }
     }
    else { valeur = atoi (answer[3]);
           SEA( num_ea++, valeur );                                      /* Numéro de l'EA pour la valeur */
         }

    query[2] = "input.frequency";
    retour = upscli_get( &module->upsconn, 3, query, &numa, &answer);
    if (retour == -1)
     { if (upscli_upserror(&module->upsconn) != UPSCLI_ERR_VARNOTSUPP)        /* Variable non supportée ? */
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Interroger_onduleur: Wrong ANSWER input.frequency",
                  upscli_upserror(&module->upsconn) );
          return(FALSE);
        }
     }
    else { valeur = atoi (answer[3]);
           SEA( num_ea++, valeur );                                      /* Numéro de l'EA pour la valeur */
         }

    query[2] = "output.current";
    retour = upscli_get( &module->upsconn, 3, query, &numa, &answer);
    if (retour == -1)
     { if (upscli_upserror(&module->upsconn) != UPSCLI_ERR_VARNOTSUPP)        /* Variable non supportée ? */
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Interroger_onduleur: Wrong ANSWER output.current",
                  upscli_upserror(&module->upsconn) );
          return(FALSE);
        }
     }
    else { valeur = atoi (answer[3]);
           SEA( num_ea++, valeur );                                      /* Numéro de l'EA pour la valeur */
         }

    query[2] = "output.frequency";
    retour = upscli_get( &module->upsconn, 3, query, &numa, &answer);
    if (retour == -1)
     { if (upscli_upserror(&module->upsconn) != UPSCLI_ERR_VARNOTSUPP)        /* Variable non supportée ? */
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Interroger_onduleur: Wrong ANSWER output.frequency",
                  upscli_upserror(&module->upsconn) );
          return(FALSE);
        }
     }
    else { valeur = atoi (answer[3]);
           SEA( num_ea++, valeur );                                      /* Numéro de l'EA pour la valeur */
         }

    query[2] = "output.voltage";
    retour = upscli_get( &module->upsconn, 3, query, &numa, &answer);
    if (retour == -1)
     { if (upscli_upserror(&module->upsconn) != UPSCLI_ERR_VARNOTSUPP)        /* Variable non supportée ? */
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Interroger_onduleur: Wrong ANSWER output.voltage",
                  upscli_upserror(&module->upsconn) );
          return(FALSE);
        }
     }
    else { valeur = atoi (answer[3]);
           SEA( num_ea++, valeur );                                      /* Numéro de l'EA pour la valeur */
         }

    return(TRUE);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du ONDULEUR                                                                  */
/**********************************************************************************************************/
 void Run_onduleur ( void )
  { struct MODULE_ONDULEUR *module;
    GList *liste;

    prctl(PR_SET_NAME, "W-ONDULEUR", 0, 0, 0 );
    Info( Config.log, DEBUG_ONDULEUR, "ONDULEUR: demarrage" );

    Partage->com_onduleur.Modules_ONDULEUR = NULL;                        /* Init des variables du thread */

    if ( Charger_tous_ONDULEUR() == FALSE )                            /* Chargement des modules onduleur */
     { Info( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: No module ONDULEUR found -> stop" );
       Partage->com_onduleur.TID = 0;                     /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     }

    Partage->com_onduleur.Thread_run = TRUE;                     /* On dit au maitre que le thread tourne */
    while(Partage->com_onduleur.Thread_run == TRUE)                   /* On tourne tant que l'on a besoin */
     { sleep(1);
       sched_yield();

       if (Partage->com_onduleur.Thread_reload == TRUE)
        { Info( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Reloading conf" );
          Decharger_tous_ONDULEUR();
          Charger_tous_ONDULEUR();
          Partage->com_onduleur.Thread_reload = FALSE;
        }

       if (Partage->com_onduleur.Thread_sigusr1 == TRUE)
        { Info( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: SIGUSR1" );
          Partage->com_onduleur.Thread_sigusr1 = FALSE;
        }

       if (Partage->com_onduleur.admin_del)
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Deleting module",
                  Partage->com_onduleur.admin_del );
          module = Chercher_module_by_id ( Partage->com_onduleur.admin_del );
          Deconnecter_module  ( module );
          Decharger_un_ONDULEUR ( module );
          Partage->com_onduleur.admin_del = 0;
        }

       if (Partage->com_onduleur.admin_add)
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Adding module",
                  Partage->com_onduleur.admin_add );
          Charger_un_ONDULEUR ( Partage->com_onduleur.admin_add );
          Partage->com_onduleur.admin_add = 0;
        }

       if (Partage->com_onduleur.admin_start)
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Starting module",
                  Partage->com_onduleur.admin_start );
          module = Chercher_module_by_id ( Partage->com_onduleur.admin_start );
          if (module) module->onduleur.actif = 1;
          Partage->com_onduleur.admin_start = 0;
        }

       if (Partage->com_onduleur.admin_stop)
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Stoping module",
                  Partage->com_onduleur.admin_stop );
          module = Chercher_module_by_id ( Partage->com_onduleur.admin_stop );
          if (module) module->onduleur.actif = 0;
          Deconnecter_module  ( module );
          Partage->com_onduleur.admin_stop = 0;
        }

       if (Partage->com_onduleur.Modules_ONDULEUR == NULL ||    /* Si pas de module référencés, on attend */
           Onduleur_is_actif() == FALSE)
        { sleep(2); continue; }

       liste = Partage->com_onduleur.Modules_ONDULEUR;
       while (liste)
        { module = (struct MODULE_ONDULEUR *)liste->data;
          if ( module->onduleur.actif != TRUE || 
               Partage->top < module->date_retente )           /* Si attente retente, on change de module */
           { liste = liste->next;                      /* On prépare le prochain accès au prochain module */
             continue;
           }

/*********************************** Début de l'interrogation du module ***********************************/
          if ( ! module->started )                                           /* Communication OK ou non ? */
           { if ( Connecter_module( module ) )
              { module->date_retente = 0;;
                module->started = TRUE;
              }
             else
              { Info_n( Config.log, DEBUG_ONDULEUR,
                        "ONDULEUR: Run_onduleur: Module DOWN", module->onduleur.id );
                module->date_retente = Partage->top + ONDULEUR_RETRY;
              }
           }
          else
           { Info_n( Config.log, DEBUG_ONDULEUR,
                     "ONDULEUR: Run_onduleur: Interrogation onduleur ID", module->onduleur.id );
             if ( Interroger_onduleur ( module ) )
              { module->date_retente = Partage->top + ONDULEUR_POLLING; }/* Update toutes les xx secondes */
             else
              { Deconnecter_module ( module );
                module->date_retente = Partage->top + ONDULEUR_RETRY;        /* On retente dans longtemps */
              }
           }
          liste = liste->next;                         /* On prépare le prochain accès au prochain module */
        }
     }

    Decharger_tous_ONDULEUR();
    Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Down", pthread_self() );
    Partage->com_onduleur.TID = 0;                        /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
