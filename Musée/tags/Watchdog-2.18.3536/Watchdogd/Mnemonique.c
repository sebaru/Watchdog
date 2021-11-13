/******************************************************************************************************************************/
/* Watchdogd/Mnemonique/Mnemonique.c        Déclaration des fonctions pour la gestion des mnemoniques                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          dim 19 avr 2009 15:15:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

 #define MNEMO_SQL_SELECT "SELECT mnemo.id,mnemo.type,num,dls_id,acronyme,mnemo.libelle,mnemo.ev_text,parent_syn.page,syn.page," \
                          "dls.name, mnemo.tableau, mnemo.acro_syn, mnemo.ev_host, mnemo.ev_thread" \
                          " FROM mnemos as mnemo" \
                          " INNER JOIN dls as dls ON mnemo.dls_id=dls.id" \
                          " INNER JOIN syns as syn ON dls.syn_id = syn.id" \
                          " INNER JOIN syns as parent_syn ON parent_syn.id = syn.parent_id"

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Retirer_mnemo_baseDB: Elimination d'un mnemo                                                                               */
/* Entrée: une structure representant le mnemo à supprimer                                                                    */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Retirer_mnemo_baseDB ( struct CMD_TYPE_MNEMO_BASE *mnemo )
  { struct CMD_TYPE_MNEMO_BASE *mnemo_a_virer;
    gchar requete[200];
    gboolean retour;
    struct DB *db;

    if (mnemo->id < 10000) return(FALSE);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MNEMO, mnemo->id );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_Modifier_mnemo_baseDB: Ajout ou modifie le mnemo en parametre                                                      */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 static gint Ajouter_Modifier_mnemo_baseDB ( struct CMD_TYPE_MNEMO_BASE *mnemo, gboolean ajout )
  { gchar *libelle, *acro, *ev_text, *tableau, *acro_syn, *ev_thread, *ev_host;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;
    gint last_id;

    libelle    = Normaliser_chaine ( mnemo->libelle );                                       /* Formatage correct des chaines */
    acro       = Normaliser_chaine ( mnemo->acronyme );                                      /* Formatage correct des chaines */
    ev_text    = Normaliser_chaine ( mnemo->ev_text );                                       /* Formatage correct des chaines */
    tableau    = Normaliser_chaine ( mnemo->tableau );                                       /* Formatage correct des chaines */
    acro_syn   = Normaliser_chaine ( mnemo->acro_syn );                                      /* Formatage correct des chaines */
    ev_thread  = Normaliser_chaine ( mnemo->ev_thread );                                     /* Formatage correct des chaines */
    ev_host    = Normaliser_chaine ( mnemo->ev_host );                                       /* Formatage correct des chaines */
    if ( !(libelle && acro && ev_text && ev_thread && ev_host && tableau && acro_syn) )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation impossible. Mnemo NOT added nor modified.", __func__ );
       if (libelle)   g_free(libelle);
       if (acro)      g_free(acro);
       if (tableau)   g_free(tableau);
       if (acro_syn)  g_free(acro_syn);
       if (ev_thread) g_free(ev_thread);
       if (ev_host)   g_free(ev_host);
       if (ev_text)   g_free(ev_text);
       return(-1);
     }

    if (ajout == TRUE)
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "INSERT INTO %s(type,num,dls_id,acronyme,libelle,ev_host,ev_thread,ev_text,tableau,acro_syn) VALUES "
                   "(%d,%d,%d,'%s','%s','%s','%s','%s','%s','%s')", NOM_TABLE_MNEMO, mnemo->type,
                   mnemo->num, mnemo->dls_id, acro, libelle, ev_host, ev_thread, ev_text, tableau, acro_syn );
     } else
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE %s SET "             
                   "type=%d,libelle='%s',acronyme='%s',ev_host='%s',ev_thread='%s',ev_text='%s',dls_id=%d,num=%d,tableau='%s',"
                   "acro_syn='%s' "
                   "WHERE id=%d",
                   NOM_TABLE_MNEMO, mnemo->type, libelle, acro, ev_host, ev_thread, ev_text, 
                   mnemo->dls_id, mnemo->num, tableau, acro_syn, mnemo->id );
     }
    g_free(libelle);
    g_free(acro);
    g_free(tableau);
    g_free(acro_syn);
    g_free(ev_host);
    g_free(ev_thread);
    g_free(ev_text);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(-1);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if ( retour == FALSE )
     { Libere_DB_SQL(&db); 
       return(-1);
     }

    if (ajout==TRUE) last_id = Recuperer_last_ID_SQL ( db );
    Libere_DB_SQL(&db);

    if (ajout==TRUE) return(last_id);
    else return(0);
  }
/******************************************************************************************************************************/
/* Modifier_mnemo_fullDB: Modifie la configuration du mnemo paramètre                                                         */
/* Entrée: une structure de mnemo FULL                                                                                        */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Modifier_mnemo_optionsDB ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { switch (mnemo_full->mnemo_base.type)
     { case MNEMO_ENTREE_ANA: return( Modifier_mnemo_aiDB      ( mnemo_full ) );
       case MNEMO_CPT_IMP   : return( Modifier_mnemo_cptimpDB  ( mnemo_full ) );
       case MNEMO_CPTH      : return( Modifier_mnemo_cpthDB    ( mnemo_full ) );
       case MNEMO_TEMPO     : return( Modifier_mnemo_tempoDB   ( mnemo_full ) );
       case MNEMO_REGISTRE  : return( Modifier_mnemo_registreDB( mnemo_full ) );
       default : return(TRUE);
     }
  }
/******************************************************************************************************************************/
/* Ajouter_mnemo_baseDB: Ajout d'un nouvel mnemonique de base                                                                 */
/* Entrée: une structure representant le mnemonique                                                                           */
/* Sortie: l'id du nouveau mnemonique, ou -1 si erreur                                                                        */
/******************************************************************************************************************************/
 gint Ajouter_mnemo_fullDB ( struct CMD_TYPE_MNEMO_FULL *mnemo )
  { mnemo->mnemo_base.id = Ajouter_Modifier_mnemo_baseDB( &mnemo->mnemo_base, TRUE );
    Modifier_mnemo_optionsDB ( mnemo );
    return(mnemo->mnemo_base.id);
  }
/******************************************************************************************************************************/
/* Modifier_mnemo_baseDB: Modification d'un mnemo Watchdog                                                                    */
/* Entrée: un mnemonique de base                                                                                              */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Modifier_mnemo_baseDB( struct CMD_TYPE_MNEMO_BASE *mnemo )
  { if (Ajouter_Modifier_mnemo_baseDB( mnemo, FALSE ) == -1) return(FALSE);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Recuperer_mnemo_baseDB_by_command_text: Recupération de la liste des mnemo par command_text                                */
/* Entrée: un pointeur vers une nouvelle connexion de base de données, le critere de recherche                                */
/* Sortie: FALSE si erreur                                                      ********************                          */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemo_baseDB_by_event_text ( struct DB **db_retour, gchar *thread, gchar *commande_pure )
  { gchar requete[1024];
    gchar *commande;
    gboolean retour;
    struct DB *db;

    commande = Normaliser_chaine ( commande_pure );
    if (!commande)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                 "%s: Normalisation impossible commande", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete), MNEMO_SQL_SELECT                                                     /* Requete SQL */
               " WHERE (mnemo.ev_host='*' OR mnemo.ev_host='%s') AND (mnemo.ev_thread='*' OR mnemo.ev_thread='%s') AND mnemo.ev_text = '%s'",
               g_get_host_name(), thread, commande_pure );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_mnemo_base_db: Récupération de la liste des mnemos de base                                                       */
/* Entrée: un pointeur vers la nouvelle connexion base de données                                                             */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemo_baseDB_with_conditions ( struct DB **db_retour, gchar *conditions, gint start, gint length )
  { gchar requete[1024], critere[256];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete), MNEMO_SQL_SELECT                                                     /* Requete SQL */
                " WHERE %s ORDER BY parent_syn.page,syn.page,name,type,num", (conditions ? conditions : "1=1")
              );                                                                                    /* order by test 25/01/06 */

    if (start != -1 && length != -1)                                                 /* Critere d'affichage (offset et count) */
     { g_snprintf( critere, sizeof(critere), " LIMIT %d,%d", start, length );
       g_strlcat( requete, critere, sizeof(requete) );
     }
    else if (length!=-1)
     { g_snprintf( critere, sizeof(critere), " LIMIT %d", length );
       g_strlcat( requete, critere, sizeof(requete) );
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_mnemo_base_DB_suite: Fonction itérative de récupération des mnémoniques de base                                  */
/* Entrée: un pointeur sur la connexion de baase de données                                                                   */
/* Sortie: une structure nouvellement allouée                                                                                 */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_BASE *Recuperer_mnemo_baseDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_MNEMO_BASE *mnemo;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    mnemo = (struct CMD_TYPE_MNEMO_BASE *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_BASE) );
    if (!mnemo) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                         "%s: Erreur allocation mémoire", __func__ );
    else                                                                                /* Recopie dans la nouvelle structure */
     { g_snprintf( mnemo->acronyme,        sizeof(mnemo->acronyme),        "%s", db->row[4] );
       g_snprintf( mnemo->libelle,         sizeof(mnemo->libelle),         "%s", db->row[5] );
       g_snprintf( mnemo->ev_text,         sizeof(mnemo->ev_text),         "%s", db->row[6] );
       g_snprintf( mnemo->syn_parent_page, sizeof(mnemo->syn_parent_page), "%s", db->row[7] );
       g_snprintf( mnemo->syn_page,        sizeof(mnemo->syn_page),        "%s", db->row[8] );
       g_snprintf( mnemo->dls_shortname,   sizeof(mnemo->dls_shortname),   "%s", db->row[9] );
       g_snprintf( mnemo->tableau,         sizeof(mnemo->tableau),         "%s", db->row[10] );
       g_snprintf( mnemo->acro_syn,        sizeof(mnemo->acro_syn),        "%s", db->row[11] );
       g_snprintf( mnemo->ev_host,         sizeof(mnemo->ev_host),         "%s", db->row[12] );
       g_snprintf( mnemo->ev_thread,       sizeof(mnemo->ev_thread),       "%s", db->row[13] );
       mnemo->id     = atoi(db->row[0]);
       mnemo->type   = atoi(db->row[1]);
       mnemo->num    = atoi(db->row[2]);
       mnemo->dls_id = atoi(db->row[3]);
     }
    return(mnemo);
  }
/******************************************************************************************************************************/
/* Rechercher_mnemo_baseDB: Recupération du mnemo dont l'id est en parametre                                                  */
/* Entrée: l'id du mnemonique a récupérer                                                                                     */
/* Sortie: la structure representant le mnemonique de base                                                                    */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_BASE *Rechercher_mnemo_baseDB ( guint id )
  { struct CMD_TYPE_MNEMO_BASE *mnemo;
    gchar requete[1024];
    struct DB *db;

    g_snprintf( requete, sizeof(requete), MNEMO_SQL_SELECT                                                     /* Requete SQL */
                " WHERE mnemo.id = %d", id
              );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
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
/******************************************************************************************************************************/
/* Rechercher_mnemo_baseDB_type_num: Recupération du mnemo par critere type/numéro                                            */
/* Entrée: une structure de critere                                                                                           */
/* Sortie: le mnemonique de base                                                                                              */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_BASE *Rechercher_mnemo_baseDB_type_num ( struct CMD_TYPE_NUM_MNEMONIQUE *critere )
  { struct CMD_TYPE_MNEMO_BASE *mnemo;
    gchar requete[1024];
    struct DB *db;

    g_snprintf( requete, sizeof(requete), MNEMO_SQL_SELECT                                                     /* Requete SQL */
                " WHERE mnemo.type = %d AND mnemo.num = %d LIMIT 1", critere->type, critere->num
              );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
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
/******************************************************************************************************************************/
/* Rechercher_mnemo_fullDB: Recupération de l'ensemble du mnemo et de sa conf spécifique                                      */
/* Entrée: l'id du mnemonique a récupérer                                                                                     */
/* Sortie: la structure representant le mnemonique de base                                                                    */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_FULL *Rechercher_mnemo_fullDB ( guint id )
  { struct CMD_TYPE_MNEMO_BASE *mnemo_base;
    struct CMD_TYPE_MNEMO_FULL *mnemo_full;

    mnemo_base = Rechercher_mnemo_baseDB ( id );
    if (!mnemo_base)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "%s: Mnemo %d not found", __func__, id );
       return(NULL);
     }

    mnemo_full = (struct CMD_TYPE_MNEMO_FULL *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_FULL) );
    if (!mnemo_full)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "%s: Erreur allocation mémoire", __func__ );
       g_free(mnemo_base);
       return(NULL);
     }

    memcpy ( &mnemo_full->mnemo_base, mnemo_base, sizeof( struct CMD_TYPE_MNEMO_BASE ) );
    g_free(mnemo_base);

    switch( mnemo_full->mnemo_base.type )
     { case MNEMO_ENTREE_ANA:
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
       case MNEMO_REGISTRE:
        { struct CMD_TYPE_MNEMO_REGISTRE *mnemo_r;
          mnemo_r = Rechercher_mnemo_registreDB ( id );
          if (mnemo_r) 
           { memcpy ( &mnemo_full->mnemo_r, mnemo_r, sizeof(struct CMD_TYPE_MNEMO_REGISTRE) );
             g_free(mnemo_r);
           }
          break;
        }
     }
    return(mnemo_full);
  }
/******************************************************************************************************************************/
/* Modifier_mnemo_fullDB: Modifie la configuration du mnemo paramètre                                                         */
/* Entrée: une structure de mnemo FULL                                                                                        */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Modifier_mnemo_fullDB ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { if (Modifier_mnemo_baseDB ( &mnemo_full->mnemo_base ) == FALSE )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Modifier_mnemo_baseDB failed", __func__ );
       return(FALSE);
     }
    if (Modifier_mnemo_optionsDB ( mnemo_full ) == FALSE)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Modifier_mnemo_optionsDB failed", __func__ );
       return(FALSE);
     }
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
