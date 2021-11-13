/**********************************************************************************************************/
/* Watchdogd/Mnemonique/Mnemonique.c        Déclaration des fonctions pour la gestion des mnemoniques     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 19 avr 2009 15:15:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Mnemonique.c
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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "watchdogd.h"

/**********************************************************************************************************/
/* Retirer_mnemo_baseDB: Elimination d'un mnemo                                                           */
/* Entrée: une structure representant le mnemo à supprimer                                                */
/* Sortie: FALSE si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_mnemo_baseDB ( struct CMD_TYPE_MNEMO_BASE *mnemo )
  { struct CMD_TYPE_MNEMO_BASE *mnemo_a_virer;
    gchar requete[200];
    gboolean retour;
    struct DB *db;

    if (mnemo->id < 10000) return(FALSE);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_mnemoDB: DB connexion failed" );
       return(FALSE);
     }

    mnemo_a_virer = Rechercher_mnemo_baseDB ( mnemo->id );
    if (mnemo_a_virer)
     { switch (mnemo_a_virer->type)
        { case MNEMO_ENTREE:
               g_snprintf( requete, sizeof(requete),                                       /* Requete SQL */
               "DELETE FROM %s WHERE id_mnemo=%d", NOM_TABLE_MNEMO_DI, mnemo_a_virer->id );
               Lancer_requete_SQL ( db, requete );
               break;
          case MNEMO_ENTREE_ANA:
               g_snprintf( requete, sizeof(requete),                                       /* Requete SQL */
               "DELETE FROM %s WHERE id_mnemo=%d", NOM_TABLE_MNEMO_AI, mnemo_a_virer->id );
               Lancer_requete_SQL ( db, requete );
               break;
          case MNEMO_CPT_IMP:
               g_snprintf( requete, sizeof(requete),                                       /* Requete SQL */
               "DELETE FROM %s WHERE id_mnemo=%d", NOM_TABLE_MNEMO_CPTIMP, mnemo_a_virer->id );
               Lancer_requete_SQL ( db, requete );
               break;
          case MNEMO_CPTH:
               g_snprintf( requete, sizeof(requete),                                       /* Requete SQL */
               "DELETE FROM %s WHERE id_mnemo=%d", NOM_TABLE_CPTH, mnemo_a_virer->id );
               Lancer_requete_SQL ( db, requete );
               break;
          case MNEMO_TEMPO:
               g_snprintf( requete, sizeof(requete),                                       /* Requete SQL */
               "DELETE FROM %s WHERE id_mnemo=%d", NOM_TABLE_MNEMO_TEMPO, mnemo_a_virer->id );
               Lancer_requete_SQL ( db, requete );
               break;
          default:
               break;
        }
       g_free(mnemo_a_virer);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MNEMO, mnemo->id );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_Modifier_mnemo_baseDB: Ajout ou modifie le mnemo en parametre                                  */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                      */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                */
/**********************************************************************************************************/
 static gint Ajouter_Modifier_mnemo_baseDB ( struct CMD_TYPE_MNEMO_BASE *mnemo, gboolean ajout )
  { gchar *libelle, *acro, *command_text, *tableau;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;
    gint last_id;

    libelle      = Normaliser_chaine ( mnemo->libelle );                 /* Formatage correct des chaines */
    acro         = Normaliser_chaine ( mnemo->acronyme );                /* Formatage correct des chaines */
    command_text = Normaliser_chaine ( mnemo->command_text );            /* Formatage correct des chaines */
    tableau      = Normaliser_chaine ( mnemo->tableau );                 /* Formatage correct des chaines */
    if ( !(libelle && acro && command_text && tableau) )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Ajouter_Modifier_mnemo_baseDB: Normalisation impossible. Mnemo NOT added nor modified." );
       if (libelle)      g_free(libelle);
       if (acro)         g_free(acro);
       if (command_text) g_free(command_text);
       if (tableau)      g_free(tableau);
       return(-1);
     }

    if (ajout == TRUE)
     { g_snprintf( requete, sizeof(requete),                                               /* Requete SQL */
                   "INSERT INTO %s(type,num,num_plugin,acronyme,libelle,command_text,tableau) VALUES "
                   "(%d,%d,%d,'%s','%s','%s','%s')", NOM_TABLE_MNEMO, mnemo->type,
                   mnemo->num, mnemo->num_plugin, acro, libelle, command_text, tableau );
     } else
     { g_snprintf( requete, sizeof(requete),                                               /* Requete SQL */
                   "UPDATE %s SET "             
                   "type=%d,libelle='%s',acronyme='%s',command_text='%s',num_plugin=%d,num=%d,tableau='%s' "
                   "WHERE id=%d",
                   NOM_TABLE_MNEMO, mnemo->type, libelle, acro, command_text, 
                   mnemo->num_plugin, mnemo->num, tableau, mnemo->id );
     }
    g_free(libelle);
    g_free(acro);
    g_free(command_text);
    g_free(tableau);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_Modifier_mnemo_baseDB: DB connexion failed" );
       return(-1);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if ( retour == FALSE )
     { Libere_DB_SQL(&db); 
       return(-1);
     }

    if (ajout==TRUE) last_id = Recuperer_last_ID_SQL ( db );
    Libere_DB_SQL(&db);

    if (ajout==TRUE) return(last_id);
    else return(0);
  }
/**********************************************************************************************************/
/* Modifier_mnemo_fullDB: Modifie la configuration du mnemo paramètre                                     */
/* Entrée: une structure de mnemo FULL                                                                    */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gboolean Modifier_mnemo_optionsDB ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { switch (mnemo_full->mnemo_base.type)
     { case MNEMO_ENTREE    : return( Modifier_mnemo_diDB     ( mnemo_full ) );
       case MNEMO_ENTREE_ANA: return( Modifier_mnemo_aiDB     ( mnemo_full ) );
       case MNEMO_CPT_IMP   : return( Modifier_mnemo_cptimpDB ( mnemo_full ) );
       case MNEMO_TEMPO     : return( Modifier_mnemo_tempoDB  ( mnemo_full ) );
       default : return(TRUE);
     }
  }
/**********************************************************************************************************/
/* Ajouter_mnemo_baseDB: Ajout d'un nouvel mnemonique de base                                             */
/* Entrée: une structure representant le mnemonique                                                       */
/* Sortie: l'id du nouveau mnemonique, ou -1 si erreur                                                    */
/**********************************************************************************************************/
 gint Ajouter_mnemo_fullDB ( struct CMD_TYPE_MNEMO_FULL *mnemo )
  { mnemo->mnemo_base.id = Ajouter_Modifier_mnemo_baseDB( &mnemo->mnemo_base, TRUE );
    Modifier_mnemo_optionsDB ( mnemo );
    return(mnemo->mnemo_base.id);
  }
/**********************************************************************************************************/
/* Modifier_mnemo_baseDB: Modification d'un mnemo Watchdog                                                */
/* Entrée: un mnemonique de base                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gboolean Modifier_mnemo_baseDB( struct CMD_TYPE_MNEMO_BASE *mnemo )
  { if (Ajouter_Modifier_mnemo_baseDB( mnemo, FALSE ) == -1) return(FALSE);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Recuperer_mnemo_baseDB_by_command_text: Recupération de la liste des mnemo par command_text                                */
/* Entrée: un pointeur vers une nouvelle connexion de base de données, le critere de recherche                                */
/* Sortie: FALSE si erreur                                                      ********************                          */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemo_baseDB_by_command_text ( struct DB **db_retour, gchar *commande_pure )
  { gchar requete[1024], critere[256];
    gchar *commande;
    gboolean retour;
    struct DB *db;

    commande = Normaliser_chaine ( commande_pure );
    if (!commande)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                 "Recuperer_mnemo_by_command_text: Normalisation impossible commande" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.type,num,num_plugin,acronyme,%s.libelle,%s.command_text,%s.groupe,%s.page,"
                "%s.name, %s.tableau"
                " FROM %s,%s,%s"
                " WHERE %s.num_syn = %s.id AND %s.num_plugin = %s.id",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, 
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_DLS, NOM_TABLE_MNEMO,
                NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,/* FROM */
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE,  /* WHERE */
                NOM_TABLE_MNEMO, NOM_TABLE_DLS
              );

    g_snprintf( critere, sizeof(critere), " AND %s.command_text = '%s'",
                NOM_TABLE_MNEMO, commande_pure );
    g_strlcat( requete, critere, sizeof(requete) );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_mnemo_by_command_text: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_mnemo_base_db: Récupération de la liste des mnemos de base                                   */
/* Entrée: un pointeur vers la nouvelle connexion base de données                                         */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 gboolean Recuperer_mnemo_baseDB ( struct DB **db_retour )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.type,num,num_plugin,acronyme,%s.libelle,%s.command_text,%s.groupe,%s.page,"
                "%s.name, %s.tableau"
                " FROM %s,%s,%s"
                " WHERE %s.num_syn = %s.id AND %s.num_plugin = %s.id"
                " ORDER BY groupe,page,name,type,num",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, 
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_DLS, NOM_TABLE_MNEMO,
                NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,/* FROM */
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE,  /* WHERE */
                NOM_TABLE_MNEMO, NOM_TABLE_DLS
              );                                                                /* order by test 25/01/06 */

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_mnemoDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_mnemo_base_DB_suite: Fonction itérative de récupération des mnémoniques de base              */
/* Entrée: un pointeur sur la connexion de baase de données                                               */
/* Sortie: une structure nouvellement allouée                                                             */
/**********************************************************************************************************/
 struct CMD_TYPE_MNEMO_BASE *Recuperer_mnemo_baseDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_MNEMO_BASE *mnemo;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    mnemo = (struct CMD_TYPE_MNEMO_BASE *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_BASE) );
    if (!mnemo) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                         "Recuperer_mnemoDB_suite: Erreur allocation mémoire" );
    else                                                            /* Recopie dans la nouvelle structure */
     { g_snprintf( mnemo->acronyme,     sizeof(mnemo->acronyme),     "%s", db->row[4] );
       g_snprintf( mnemo->libelle,      sizeof(mnemo->libelle),      "%s", db->row[5] );
       g_snprintf( mnemo->command_text, sizeof(mnemo->command_text), "%s", db->row[6] );
       g_snprintf( mnemo->groupe,       sizeof(mnemo->groupe),       "%s", db->row[7] );
       g_snprintf( mnemo->page,         sizeof(mnemo->page),         "%s", db->row[8] );
       g_snprintf( mnemo->plugin_dls,   sizeof(mnemo->plugin_dls),   "%s", db->row[9] );
       g_snprintf( mnemo->tableau,      sizeof(mnemo->tableau),      "%s", db->row[10] );
       mnemo->id          = atoi(db->row[0]);
       mnemo->type        = atoi(db->row[1]);
       mnemo->num         = atoi(db->row[2]);
       mnemo->num_plugin  = atoi(db->row[3]);
     }
    return(mnemo);
  }
/**********************************************************************************************************/
/* Rechercher_mnemo_baseDB: Recupération du mnemo dont l'id est en parametre                              */
/* Entrée: l'id du mnemonique a récupérer                                                                 */
/* Sortie: la structure representant le mnemonique de base                                                */
/**********************************************************************************************************/
 struct CMD_TYPE_MNEMO_BASE *Rechercher_mnemo_baseDB ( guint id )
  { struct CMD_TYPE_MNEMO_BASE *mnemo;
    gchar requete[512];
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.type,num,num_plugin,acronyme,%s.libelle,%s.command_text,%s.groupe,%s.page,"
                "%s.name, %s.tableau"
                " FROM %s,%s,%s"
                " WHERE %s.num_syn = %s.id AND %s.num_plugin = %s.id"
                " AND %s.id = %d",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, 
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_DLS, NOM_TABLE_MNEMO,
                NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,/* FROM */
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE,  /* WHERE */
                NOM_TABLE_MNEMO, NOM_TABLE_DLS,
                NOM_TABLE_MNEMO, id
              );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_mnemo_baseDB: DB connexion failed" );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    mnemo = Recuperer_mnemo_baseDB_suite( &db );
    if (mnemo) Libere_DB_SQL ( &db );
    return(mnemo);
  }
/**********************************************************************************************************/
/* Rechercher_mnemo_baseDB_type_num: Recupération du mnemo par critere type/numéro                        */
/* Entrée: une structure de critere                                                                       */
/* Sortie: le mnemonique de base                                                                          */
/**********************************************************************************************************/
 struct CMD_TYPE_MNEMO_BASE *Rechercher_mnemo_baseDB_type_num ( struct CMD_TYPE_NUM_MNEMONIQUE *critere )
  { struct CMD_TYPE_MNEMO_BASE *mnemo;
    gchar requete[512];
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.type,num,num_plugin,acronyme,%s.libelle,%s.command_text,%s.groupe,%s.page,"
                "%s.name, %s.tableau"
                " FROM %s,%s,%s"
                " WHERE %s.num_syn = %s.id AND %s.num_plugin = %s.id"
                " AND %s.type = %d AND %s.num = %d",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, 
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_DLS, NOM_TABLE_MNEMO,
                NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,/* FROM */
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE,  /* WHERE */
                NOM_TABLE_MNEMO, NOM_TABLE_DLS,
                NOM_TABLE_MNEMO, critere->type, NOM_TABLE_MNEMO, critere->num
              );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_mnemo_baseDB: DB connexion failed" );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    mnemo = Recuperer_mnemo_baseDB_suite ( &db );
    if (mnemo) Libere_DB_SQL( &db );
    return( mnemo );
  }
/**********************************************************************************************************/
/* Recuperer_mnemoDB_for_courbe: Recupération de la liste des ids des mnemos pour les courbes             */
/* Entrée: un pointeur sur la nouvelle connexion base de données                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 gboolean Recuperer_mnemo_baseDB_for_courbe ( struct DB **db_retour )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.type,num,num_plugin,acronyme,%s.libelle,%s.command_text,%s.groupe,%s.page,"
                "%s.name, %s.tableau"
                " FROM %s,%s,%s"
                " WHERE %s.num_syn = %s.id AND %s.num_plugin = %s.id"
                " AND (%s.type=%d OR %s.type=%d OR %s.type=%d OR %s.type=%d)"
                " ORDER BY groupe,page,name,type,num",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, 
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_DLS, NOM_TABLE_MNEMO,
                NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,/* FROM */
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE,  /* WHERE */
                NOM_TABLE_MNEMO, NOM_TABLE_DLS,
                NOM_TABLE_MNEMO, MNEMO_ENTREE,
                NOM_TABLE_MNEMO, MNEMO_ENTREE_ANA,
                NOM_TABLE_MNEMO, MNEMO_SORTIE,
                NOM_TABLE_MNEMO, MNEMO_SORTIE_ANA
              );                                                                /* order by test 25/01/06 */

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_mnemo_baseDB_for_courbe: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Rechercher_mnemo_fullDB: Recupération de l'ensemble du mnemo et de sa conf spécifique                  */
/* Entrée: l'id du mnemonique a récupérer                                                                 */
/* Sortie: la structure representant le mnemonique de base                                                */
/**********************************************************************************************************/
 struct CMD_TYPE_MNEMO_FULL *Rechercher_mnemo_fullDB ( guint id )
  { struct CMD_TYPE_MNEMO_BASE *mnemo_base;
    struct CMD_TYPE_MNEMO_FULL *mnemo_full;

    mnemo_base = Rechercher_mnemo_baseDB ( id );
    if (!mnemo_base)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "Recuperer_mnemo_fullDB: Mnemo %d not found", id );
       return(NULL);
     }

    mnemo_full = (struct CMD_TYPE_MNEMO_FULL *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_FULL) );
    if (!mnemo_full)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "Recuperer_mnemo_fullDB: Erreur allocation mémoire" );
       g_free(mnemo_base);
       return(NULL);
     }

    memcpy ( &mnemo_full->mnemo_base, mnemo_base, sizeof( struct CMD_TYPE_MNEMO_BASE ) );
    g_free(mnemo_base);

    switch( mnemo_full->mnemo_base.type )
     { case MNEMO_ENTREE:
        { struct CMD_TYPE_MNEMO_DI *mnemo_di;
          mnemo_di = Rechercher_mnemo_diDB ( id );
          if (mnemo_di) 
           { memcpy ( &mnemo_full->mnemo_di, mnemo_di, sizeof(struct CMD_TYPE_MNEMO_DI) );
             g_free(mnemo_di);
           }
          break;
        }
       case MNEMO_ENTREE_ANA:
        { struct CMD_TYPE_MNEMO_AI *mnemo_ai;
          mnemo_ai = Rechercher_mnemo_aiDB ( id );
          if (mnemo_ai) 
           { memcpy ( &mnemo_full->mnemo_ai, mnemo_ai, sizeof(struct CMD_TYPE_MNEMO_AI) );
             g_free(mnemo_ai);
           }
          break;
        }
       case MNEMO_CPT_IMP:
        { struct CMD_TYPE_MNEMO_CPT_IMP *mnemo_cpt;
          mnemo_cpt = Rechercher_mnemo_cptimpDB ( id );
          if (mnemo_cpt) 
           { memcpy ( &mnemo_full->mnemo_cptimp, mnemo_cpt, sizeof(struct CMD_TYPE_MNEMO_CPT_IMP) );
             g_free(mnemo_cpt);
           }
          break;
        }
       case MNEMO_TEMPO:
        { struct CMD_TYPE_MNEMO_TEMPO *mnemo_tempo;
          mnemo_tempo = Rechercher_mnemo_tempoDB ( id );
          if (mnemo_tempo) 
           { memcpy ( &mnemo_full->mnemo_tempo, mnemo_tempo, sizeof(struct CMD_TYPE_MNEMO_TEMPO) );
             g_free(mnemo_tempo);
           }
          break;
        }
     }
    return(mnemo_full);
  }
/**********************************************************************************************************/
/* Modifier_mnemo_fullDB: Modifie la configuration du mnemo paramètre                                     */
/* Entrée: une structure de mnemo FULL                                                                    */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 gboolean Modifier_mnemo_fullDB ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { if (Modifier_mnemo_baseDB ( &mnemo_full->mnemo_base ) == FALSE )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_mnemo_fullDB: Modifier_mnemo_baseDB failed" );
       return(FALSE);
     }
    if (Modifier_mnemo_optionsDB ( mnemo_full ) == FALSE)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_mnemo_fullDB: Modifier_mnemo_optionsDB failed" );
       return(FALSE);
     }
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
