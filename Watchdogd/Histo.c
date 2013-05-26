/**********************************************************************************************************/
/* Watchdogd/Histo/Histo.c        Déclaration des fonctions pour la gestion de l'historique               */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 18 avr 2009 16:19:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Histo.c
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
 #include "Erreur.h"

/**********************************************************************************************************/
/* Clear_histoDB: Elimination des messages histo au boot systeme                                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Clear_histoDB ( void )
  { struct DB *db;
    gchar requete[1024];
    
    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Charger_histoDB: Connexion DB failed" );
       return;
     }                                                                           /* Si pas de histos (??) */

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s", NOM_TABLE_HISTO );

    Lancer_requete_SQL ( db, requete );                               /* Execution de la requete SQL */
    Libere_DB_SQL( &db );
  }
/**********************************************************************************************************/
/* Retirer_msgDB: Elimination d'un message                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_histoDB ( struct LOG *log, struct DB *db, guint id )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_HISTO, id );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Ajouter_histoDB ( struct LOG *log, struct DB *db, struct HISTODB *histo )
  { gchar requete[1024];
    gchar *libelle, *nom_ack;

    libelle = Normaliser_chaine ( histo->msg.libelle );             /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_histoDB: Normalisation impossible" );
       return(FALSE);
     }

    nom_ack = Normaliser_chaine ( histo->nom_ack );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_histoDB: Normalisation impossible" );
       g_free(libelle);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,libelle,type,num_syn,nom_ack,"
                "date_create_sec,date_create_usec,"
                "date_fixe) VALUES "
                "(%d,'%s',%d,%d,'%s',%d,%d,0)", NOM_TABLE_HISTO, histo->msg.num, libelle, 
                histo->msg.type,
                histo->msg.num_syn, nom_ack,
                histo->date_create_sec, histo->date_create_usec );
    g_free(libelle);
    g_free(nom_ack);

    return ( Lancer_requete_SQL ( db, requete ) );
  }
/**********************************************************************************************************/
/* Modifier_histoDB: Modification des champs editables d'un histo                                         */
/* Entrée: un log et une database, une structure de controle de la modification                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_histoDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_HISTO *histo )
  { gchar requete[1024];
    gchar *nom_ack;

    nom_ack = Normaliser_chaine ( histo->nom_ack );                 /* Formatage correct des chaines */
    if (!nom_ack)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_histoDB: Normalisation impossible" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET nom_ack='%s',date_fixe=%d WHERE id=%d",
                NOM_TABLE_HISTO, nom_ack, (gint)histo->date_fixe, histo->id );
    g_free(nom_ack);

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_histoDB ( struct LOG *log, struct DB *db )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.libelle,%s.groupe,%s.page,type,num_syn,nom_ack,date_create_sec,date_create_usec,"
                "date_fixe"
                " FROM %s,%s"
                " WHERE %s.num_syn = %s.id"
                " ORDER by date_create_sec,date_create_usec",
                NOM_TABLE_HISTO, NOM_TABLE_HISTO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_HISTO, NOM_TABLE_SYNOPTIQUE, /* From */
                NOM_TABLE_HISTO, NOM_TABLE_SYNOPTIQUE /* Where */
              );
    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct HISTODB *Recuperer_histoDB_suite( struct LOG *log, struct DB *db )
  { struct HISTODB *histo;

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    histo = (struct HISTODB *)g_try_malloc0( sizeof(struct HISTODB) );
    if (!histo) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                         "Recuperer_histoDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &histo->msg.libelle, db->row[1], sizeof(histo->msg.libelle) );/* Recopie dans la structure */
       memcpy( &histo->msg.groupe,  db->row[2], sizeof(histo->msg.groupe ) );/* Recopie dans la structure */
       memcpy( &histo->msg.page,    db->row[3], sizeof(histo->msg.page   ) );/* Recopie dans la structure */
       memcpy( &histo->nom_ack,     db->row[6], sizeof(histo->nom_ack    ) );/* Recopie dans la structure */
       histo->msg.id           = 0;                                /* l'id n'est pas dans la base histo ! */
       histo->msg.num          = atoi(db->row[0]);
       histo->msg.type         = atoi(db->row[4]);
       histo->msg.num_syn      = atoi(db->row[5]);
       histo->date_create_sec  = atoi(db->row[7]);
       histo->date_create_usec = atoi(db->row[8]);
       histo->date_fixe        = atoi(db->row[9]);
     }
    return(histo);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct HISTODB *Rechercher_histoDB( struct LOG *log, struct DB *db, gint id )
  { struct HISTODB *histo;
    gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.libelle,%s.groupe,%s.page,type,num_syn,nom_ack,date_create_sec,date_create_usec,"
                "date_fixe"
                " FROM %s,%s"
                " WHERE %s.num_syn = %s.id AND %s.id=%d LIMIT 1",
                NOM_TABLE_HISTO, NOM_TABLE_HISTO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_HISTO, NOM_TABLE_SYNOPTIQUE, /* From */
                NOM_TABLE_HISTO, NOM_TABLE_SYNOPTIQUE, /* Where */
                NOM_TABLE_HISTO, id
              );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "Rechercher_histoDB: histo %d not found in DB", id );
       return(NULL);
     }

    histo = (struct HISTODB *)g_try_malloc0( sizeof(struct HISTODB) );
    if (!histo) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_histoDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &histo->msg.libelle, db->row[1], sizeof(histo->msg.libelle) );/* Recopie dans la structure */
       memcpy( &histo->msg.groupe,  db->row[2], sizeof(histo->msg.groupe ) );/* Recopie dans la structure */
       memcpy( &histo->msg.page,    db->row[3], sizeof(histo->msg.page   ) );/* Recopie dans la structure */
       memcpy( &histo->nom_ack,     db->row[6], sizeof(histo->nom_ack    ) );/* Recopie dans la structure */
       histo->msg.id           = 0;                                /* l'id n'est pas dans la base histo ! */
       histo->msg.num          = atoi(db->row[0]);
       histo->msg.type         = atoi(db->row[4]);
       histo->msg.num_syn      = atoi(db->row[5]);
       histo->date_create_sec  = atoi(db->row[7]);
       histo->date_create_usec = atoi(db->row[8]);
       histo->date_fixe        = atoi(db->row[9]);
     }
    Liberer_resultat_SQL ( log, db );                                         /* Libération des résultats */
    return(histo);
  }
/*--------------------------------------------------------------------------------------------------------*/
