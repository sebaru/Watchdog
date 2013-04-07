/**********************************************************************************************************/
/* Watchdogd/Synoptiques/Synoptiques.c       Déclaration des fonctions pour la gestion des synoptiques    */
/* Projet WatchDog version 2.0       Gestion d'habitat                     jeu. 29 déc. 2011 14:00:49 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Synoptiques.c
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
/* Tester_groupe_synoptique: renvoie true si l'utilisateur fait partie du groupe en parametre             */
/* Entrées: un id utilisateur, une liste de groupe, un id de groupe                                       */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Tester_groupe_synoptique( struct LOG *log, struct DB *db, struct UTILISATEURDB *util, guint syn_id )
  { struct CMD_TYPE_SYNOPTIQUE *syn;
    gint cpt;

    if (util->id==UID_ROOT) return(TRUE);                            /* Le tech est dans tous les groupes */

    syn = Rechercher_synoptiqueDB ( log, db, syn_id );
    if (!syn) return(FALSE);
    if (syn->groupe == GID_TOUTLEMONDE) return(TRUE);

    cpt=0;
    while( util->gids[cpt] )
     { if( util->gids[cpt] == syn->access_groupe )
        { g_free(syn);
          return(TRUE);
        }
       cpt++;
     }
    g_free(syn);
    return(FALSE);
  }
/**********************************************************************************************************/
/* Retirer_msgDB: Elimination d'un synoptique                                                             */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_synoptiqueDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_SYNOPTIQUE *syn )
  { gchar requete[200];
    
    if (syn->id == 1) return(TRUE);                                 /* Le synoptique 1 est indestructible */
/****************************************** Retrait de la base SYN ****************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_SYNOPTIQUE, syn->id );

    if ( ! Lancer_requete_SQL ( log, db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination ok" ); }

/****************************************** Retrait des capteurs ******************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_id=%d", NOM_TABLE_CAPTEUR, syn->id );

    if ( ! Lancer_requete_SQL ( log, db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination capteur failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination capteur ok" ); }

/****************************************** Retrait des comment *******************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_id=%d", NOM_TABLE_COMMENT, syn->id );

    if ( ! Lancer_requete_SQL ( log, db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination comment failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination comment ok" ); }

/****************************************** Retrait des motif *********************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn=%d", NOM_TABLE_MOTIF, syn->id );

    if ( ! Lancer_requete_SQL ( log, db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination syn failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination syn ok" ); }

/****************************************** Retrait des palette *******************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_cible_id=%d", NOM_TABLE_PALETTE, syn->id );

    if ( ! Lancer_requete_SQL ( log, db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination palette failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination palette ok" ); }

/****************************************** Retrait des passerelle ****************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_cible_id=%d", NOM_TABLE_PASSERELLE, syn->id );

    if ( ! Lancer_requete_SQL ( log, db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination passerelle failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination passerelle ok" ); }

/******************************** Re-affectation des modules D.L.S ****************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET num_syn=1 WHERE num_syn=%d", NOM_TABLE_DLS, syn->id );

    if ( ! Lancer_requete_SQL ( log, db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: re-affectation plugin D.L.S failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: re-affectation plugin D.L.S passerelle ok" ); }

    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_synoptiqueDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_SYNOPTIQUE *syn )
  { gchar requete[512];
    gchar *libelle, *page, *groupe;

    libelle = Normaliser_chaine ( log, syn->libelle );                   /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_synoptiqueDB: Normalisation impossible libelle" );
       return(-1);
     }

    page = Normaliser_chaine ( log, syn->page );                       /* Formatage correct des chaines */
    if (!page)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_synoptiqueDB: Normalisation impossible page" );
       g_free(libelle);
       return(-1);
     }

    groupe = Normaliser_chaine ( log, syn->groupe );                     /* Formatage correct des chaines */
    if (!groupe)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_synoptiqueDB: Normalisation impossible groupe" );
       g_free(libelle);
       g_free(page);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                               /* Requete SQL */
                "INSERT INTO %s(libelle,page,access_groupe,groupe) VALUES "
                "('%s','%s','%d','%s')", NOM_TABLE_SYNOPTIQUE, libelle, page,
                syn->access_groupe, groupe );
    g_free(libelle);
    g_free(page);
    g_free(groupe);

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_synoptiqueDB ( struct LOG *log, struct DB *db )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,page,access_groupe,groupe"
                " FROM %s ORDER BY groupe,page,libelle", NOM_TABLE_SYNOPTIQUE );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_SYNOPTIQUE *Recuperer_synoptiqueDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_SYNOPTIQUE *syn;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    syn = (struct CMD_TYPE_SYNOPTIQUE *)g_try_malloc0( sizeof(struct CMD_TYPE_SYNOPTIQUE) );
    if (!syn) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_synoptiqueDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &syn->libelle, db->row[1], sizeof(syn->libelle) );            /* Recopie dans la structure */
       memcpy( &syn->page,    db->row[2], sizeof(syn->page   ) );            /* Recopie dans la structure */
       memcpy( &syn->groupe,  db->row[4], sizeof(syn->groupe ) );            /* Recopie dans la structure */
       syn->id            = atoi(db->row[0]);
       syn->access_groupe = atoi(db->row[3]);
     }
    return(syn);
  }
/**********************************************************************************************************/
/* Rechercher_synoptiqueDB: Recupération du synoptique dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_SYNOPTIQUE *Rechercher_synoptiqueDB ( struct LOG *log, struct DB *db, guint id )
  { struct CMD_TYPE_SYNOPTIQUE *syn;
    gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,page,access_groupe,groupe"
                " FROM %s WHERE id=%d", NOM_TABLE_SYNOPTIQUE, id );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "Rechercher_synoptiqueDB: Synoptique %d not found in DB", id );
       return(NULL);
     }

    syn = (struct CMD_TYPE_SYNOPTIQUE *)g_try_malloc0( sizeof(struct CMD_TYPE_SYNOPTIQUE) );
    if (!syn)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_synoptiqueDB: Mem error" ); }
    else
     { memcpy( &syn->libelle, db->row[1], sizeof(syn->libelle) );            /* Recopie dans la structure */
       memcpy( &syn->page,    db->row[2], sizeof(syn->page   ) );            /* Recopie dans la structure */
       memcpy( &syn->groupe,  db->row[4], sizeof(syn->groupe ) );            /* Recopie dans la structure */
       syn->id            = atoi(db->row[0]);
       syn->access_groupe = atoi(db->row[3]);
     }
    return(syn);
  }
/**********************************************************************************************************/
/* Modifier_synoptiqueDB: Modification d'un synoptique Watchdog                                           */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_synoptiqueDB( struct LOG *log, struct DB *db, struct CMD_TYPE_SYNOPTIQUE *syn )
  { gchar requete[1024];
    gchar *libelle, *page, *groupe;

    libelle = Normaliser_chaine ( log, syn->libelle );
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_synoptiqueDB: Normalisation impossible libelle" );
       return(FALSE);
     }

    page = Normaliser_chaine ( log, syn->page );
    if (!page)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_synoptiqueDB: Normalisation impossible page" );
       g_free(libelle);
       return(FALSE);
     }

    groupe = Normaliser_chaine ( log, syn->groupe );
    if (!groupe)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_synoptiqueDB: Normalisation impossible groupe" );
       g_free(libelle);
       g_free(page);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                              /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s',page='%s',access_groupe='%d',groupe='%s' "
                "WHERE id='%d'",
                NOM_TABLE_SYNOPTIQUE, libelle, page, syn->access_groupe, groupe, syn->id );
    g_free(libelle);
    g_free(page);
    g_free(groupe);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
