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
/* Retirer_mnemoDB: Elimination d'un mnemo                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_mnemoDB ( struct CMD_TYPE_MNEMONIQUE *mnemo )
  { struct CMD_TYPE_MNEMONIQUE *mnemo_a_virer;
    gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_mnemoDB: DB connexion failed" );
       return(FALSE);
     }

    mnemo_a_virer = Rechercher_mnemoDB ( mnemo->id );
    if (mnemo_a_virer)
     { switch (mnemo_a_virer->type)
        { case MNEMO_ENTREE_ANA:
               g_snprintf( requete, sizeof(requete),                                       /* Requete SQL */
               "DELETE FROM %s WHERE id_mnemo=%d", NOM_TABLE_ENTREEANA, mnemo_a_virer->id );
               Lancer_requete_SQL ( db, requete );
               break;
          case MNEMO_CPT_IMP:
               g_snprintf( requete, sizeof(requete),                                       /* Requete SQL */
               "DELETE FROM %s WHERE id_mnemo=%d", NOM_TABLE_CPT_IMP, mnemo_a_virer->id );
               Lancer_requete_SQL ( db, requete );
               break;
          case MNEMO_CPTH:
               g_snprintf( requete, sizeof(requete),                                       /* Requete SQL */
               "DELETE FROM %s WHERE id_mnemo=%d", NOM_TABLE_CPTH, mnemo_a_virer->id );
               Lancer_requete_SQL ( db, requete );
               break;
          case MNEMO_TEMPO:
               g_snprintf( requete, sizeof(requete),                                       /* Requete SQL */
               "DELETE FROM %s WHERE id_mnemo=%d", NOM_TABLE_TEMPO, mnemo_a_virer->id );
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
/* Ajouter_mnemoDB: Ajout ou edition d'un mnemo                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure mnemo                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 static gint Ajouter_Modifier_mnemoDB ( struct CMD_TYPE_MNEMONIQUE *mnemo, gboolean ajout )
  { gchar *libelle, *acro, *command_text, *tableau;
    gchar requete[1024];
    gint last_id;
    gboolean retour;
    struct DB *db;

    libelle = Normaliser_chaine ( mnemo->libelle );                      /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Ajouter_Modifier_mnemoDB: Normalisation impossible libelle" );
       return(-1);
     }
    acro = Normaliser_chaine ( mnemo->acronyme );                        /* Formatage correct des chaines */
    if (!acro)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Ajouter_Modifier_mnemoDB: Normalisation impossible acronyme" );
       g_free(libelle);
       return(-1);
     }
    command_text = Normaliser_chaine ( mnemo->command_text );            /* Formatage correct des chaines */
    if (!command_text)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Ajouter_Modifier_mnemoDB: Normalisation impossible command_text" );
       g_free(acro);
       g_free(libelle);
       return(-1);
     }
    tableau = Normaliser_chaine ( mnemo->tableau );                      /* Formatage correct des chaines */
    if (!tableau)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Ajouter_Modifier_mnemoDB: Normalisation impossible tableau" );
       g_free(acro);
       g_free(libelle);
       g_free(command_text);
       return(-1);
     }

    if (ajout == TRUE)
     { g_snprintf( requete, sizeof(requete),                                               /* Requete SQL */
                   "INSERT INTO %s(type,num,num_plugin,acronyme,libelle,command_text,tableau) VALUES "
                   "(%d,%d,%d,'%s','%s','%s','%s')", NOM_TABLE_MNEMO, mnemo->type,
                   mnemo->num, mnemo->num_plugin, acro, libelle, command_text, tableau );
     } else
     { g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                   "UPDATE %s SET "             
                   "libelle='%s',acronyme='%s',command_text='%s',num_plugin=%d,num=%d,tableau='%s' WHERE id=%d",
                   NOM_TABLE_MNEMO, libelle, acro, command_text, 
                   mnemo->num_plugin, mnemo->num, tableau, mnemo->id );
     }
    g_free(libelle);
    g_free(acro);
    g_free(command_text);
    g_free(tableau);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_Modifier_mnemoDB: DB connexion failed" );
       return(-1);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if ( retour == FALSE )
     { Libere_DB_SQL(&db); 
       return(-1);
     }
    if (ajout == TRUE)
     { last_id = Recuperer_last_ID_SQL ( db );
       switch (mnemo->type)
        { case MNEMO_ENTREE_ANA:
               g_snprintf( requete, sizeof(requete),                                          /* Requete SQL */
                           "INSERT INTO %s(min,max,unite,id_mnemo) VALUES "
                           "('%f','%f','%d','%d')", NOM_TABLE_ENTREEANA, 0.0, 100.0, 0, last_id );
               Lancer_requete_SQL ( db, requete );
               break;
          case MNEMO_CPT_IMP:
               g_snprintf( requete, sizeof(requete),                                          /* Requete SQL */
                           "INSERT INTO %s(val,id_mnemo,type_ci,multi,unite) VALUES "
                           "('%d','%d','%d','%f','%s')", NOM_TABLE_CPT_IMP, 0, last_id, 0, 1.0, "n/a" );
               Lancer_requete_SQL ( db, requete );
               break;
          case MNEMO_CPTH:
               g_snprintf( requete, sizeof(requete),                                          /* Requete SQL */
                           "INSERT INTO %s(val,id_mnemo) VALUES "
                           "('%d','%d')", NOM_TABLE_CPTH, 0, last_id );
               Lancer_requete_SQL ( db, requete );
               break;
          case MNEMO_TEMPO:
               g_snprintf( requete, sizeof(requete),                                          /* Requete SQL */
                           "INSERT INTO %s(id_mnemo,delai_on,min_on,max_on,delai_off) VALUES "
                           "('%d','%d','%d','%d','%d')", NOM_TABLE_TEMPO, last_id, 10, 0, 0, 0 );
               Lancer_requete_SQL ( db, requete );
               break;
          default:
               break;
        }
     }

    Libere_DB_SQL(&db);
    if (ajout==TRUE) return( last_id );
    else return(0);
  }
/**********************************************************************************************************/
/* Ajouter_mnemoDB: Ajout ou edition d'un mnemo                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure mnemo                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_mnemoDB ( struct CMD_TYPE_MNEMONIQUE *mnemo )
  { return(Ajouter_Modifier_mnemoDB( mnemo, TRUE )); }
/**********************************************************************************************************/
/* Recuperer_liste_id_mnemoDB: Recupération de la liste des ids des mnemos                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_mnemoDB_by_command_text ( struct DB **db_retour, gchar *commande_pure )
  { gchar requete[1024], commande_finale[1024];
    gchar *commande;
    gboolean space;
    gint i, j;
    gboolean retour;
    struct DB *db;

    commande = Normaliser_chaine ( commande_pure );
    if (!commande)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                 "Recuperer_mnemo_by_command_text: Normalisation impossible commande" );
       return(FALSE);
     }

    i = 0; j = 0; space = TRUE;
    while ( commande[i] )
     { if ( commande[i] == ' ') { commande_finale[j++] = ' '; space = TRUE; }
       else if (space == TRUE) { commande_finale[j++] = '+'; commande_finale[j++] = commande[i]; space = FALSE; }
                          else { commande_finale[j++] = commande[i]; space = FALSE; }
       if(i<strlen(commande_pure) && j<sizeof(commande_finale)) 
        { i++; } else break;
     }
    commande_finale[j] = '\0';
    g_free(commande);

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.type,num,num_plugin,acronyme,%s.libelle,%s.command_text,%s.groupe,%s.page,"
                "%s.name, %s.tableau"
                " FROM %s,%s,%s"
                " WHERE %s.num_syn = %s.id AND %s.num_plugin = %s.id AND"
                " MATCH (%s.command_text) AGAINST ('%s' IN BOOLEAN MODE) LIMIT 10",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, 
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_DLS, NOM_TABLE_MNEMO,
                NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,/* FROM */
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE,  /* WHERE */
                NOM_TABLE_MNEMO, NOM_TABLE_DLS, NOM_TABLE_MNEMO, commande_finale
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
/* Recuperer_liste_id_mnemo    if (ajout == TRUE)DB: Recupération de la liste des ids des mnemos          */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_mnemoDB ( struct DB **db_retour )
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
/* Recuperer_liste_id_mnemoDB: Recupération de la liste des ids des mnemos                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MNEMONIQUE *Recuperer_mnemoDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_MNEMONIQUE *mnemo;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    mnemo = (struct CMD_TYPE_MNEMONIQUE *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMONIQUE) );
    if (!mnemo) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_mnemoDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &mnemo->acronyme,     db->row[4], sizeof(mnemo->acronyme  ) );/* Recopie dans la structure */
       memcpy( &mnemo->libelle,      db->row[5], sizeof(mnemo->libelle   ) );/* Recopie dans la structure */
       memcpy( &mnemo->command_text, db->row[6], sizeof(mnemo->command_text ) );
       memcpy( &mnemo->groupe,       db->row[7], sizeof(mnemo->groupe    ) );/* Recopie dans la structure */
       memcpy( &mnemo->page,         db->row[8], sizeof(mnemo->page      ) );/* Recopie dans la structure */
       memcpy( &mnemo->plugin_dls,   db->row[9], sizeof(mnemo->plugin_dls) );/* Recopie dans la structure */
       memcpy( &mnemo->tableau,      db->row[10], sizeof(mnemo->tableau  ) );/* Recopie dans la structure */
       mnemo->id          = atoi(db->row[0]);
       mnemo->type        = atoi(db->row[1]);
       mnemo->num         = atoi(db->row[2]);
       mnemo->num_plugin  = atoi(db->row[3]);
     }
    return(mnemo);
  }
/**********************************************************************************************************/
/* Rechercher_mnemoDB: Recupération du mnemo dont l'id est en parametre                                   */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MNEMONIQUE *Rechercher_mnemoDB ( guint id )
  { struct CMD_TYPE_MNEMONIQUE *mnemo;
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
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_mnemoDB: DB connexion failed" );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    mnemo = Recuperer_mnemoDB_suite( &db );
    if (mnemo) Libere_DB_SQL ( &db );
    return(mnemo);
  }
/**********************************************************************************************************/
/* Rechercher_mnemoDB: Recupération du mnemo dont l'id est en parametre                                   */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MNEMONIQUE *Rechercher_mnemoDB_type_num ( struct CMD_TYPE_NUM_MNEMONIQUE *critere )
  { struct CMD_TYPE_MNEMONIQUE *mnemo;
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
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_mnemoDB: DB connexion failed" );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    mnemo = Recuperer_mnemoDB_suite ( &db );
    if (mnemo) Libere_DB_SQL( &db );
    return( mnemo );
  }
/**********************************************************************************************************/
/* Modifier_mnemoDB: Modification d'un mnemo Watchdog                                                     */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_mnemoDB( struct CMD_TYPE_MNEMONIQUE *mnemo )
  { if (Ajouter_Modifier_mnemoDB( mnemo, FALSE ) == -1) return(FALSE);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_mnemoDB_for_courbe: Recupération de la liste des ids des mnemos pour les courbes             */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_mnemoDB_for_courbe ( struct DB **db_retour )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.type,num,num_plugin,acronyme,%s.libelle,%s.command_text,%s.groupe,%s.page,%s.name"
                " FROM %s,%s,%s"
                " WHERE %s.num_syn = %s.id AND %s.num_plugin = %s.id"
                " AND (%s.type=%d OR %s.type=%d)"
                " ORDER BY groupe,page,name,type,num",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, 
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_DLS,
                NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,/* FROM */
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE,  /* WHERE */
                NOM_TABLE_MNEMO, NOM_TABLE_DLS,
                NOM_TABLE_MNEMO, MNEMO_ENTREE, NOM_TABLE_MNEMO, MNEMO_SORTIE
              );                                                                /* order by test 25/01/06 */

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_mnemoDB_for_courbe: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/*--------------------------------------------------------------------------------------------------------*/
