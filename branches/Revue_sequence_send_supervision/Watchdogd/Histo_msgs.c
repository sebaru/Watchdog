/**********************************************************************************************************/
/* Watchdogd/Histo/Histo_msgs.c        Déclaration des fonctions pour la gestion des message              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 15 aoû 2003 13:02:48 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Histo_msgs.c
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
                "UPDATE %s SET alive=0 WHERE alive=1", NOM_TABLE_HISTO_MSGS );

    Lancer_requete_SQL ( db, requete );                               /* Execution de la requete SQL */
    Libere_DB_SQL( &db );
  }
/**********************************************************************************************************/
/* Modifier_Ajouter_histo_msgsDB: Ajout ou modifier un enregistrement MSGS de la base de données          */
/* Entrée: un flag d'ajout et un enregistrement à modifier                                                */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 static gboolean Modifier_Ajouter_histo_msgsDB ( gboolean ajout, struct CMD_TYPE_HISTO *histo )
  { gchar *libelle, *nom_ack;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;

    if (ajout == TRUE)
     { libelle = Normaliser_chaine ( histo->msg.libelle );               /* Formatage correct des chaines */
       if (!libelle)
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_histo_msgsDB: Normalisation impossible" );
          return(FALSE);
        }

       nom_ack = Normaliser_chaine ( histo->nom_ack );                   /* Formatage correct des chaines */
       if (!nom_ack)
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_histo_msgsDB: Normalisation impossible" );
          g_free(libelle);
          return(FALSE);
        }
          
       g_snprintf( requete, sizeof(requete),                                               /* Requete SQL */
                   "INSERT INTO %s(alive,id_msg,nom_ack,date_create_sec,date_create_usec)"
                   " VALUES "
                   "('%d','%d','%s','%d','%d')", NOM_TABLE_HISTO_MSGS, TRUE,
                   histo->msg.id, 
                   nom_ack, (int)histo->date_create_sec, (int)histo->date_create_usec );
       g_free(libelle);
     }
    else
     { 
       if (histo->alive == TRUE)
        { nom_ack = Normaliser_chaine ( histo->nom_ack );                   /* Formatage correct des chaines */
          if (!nom_ack)
           { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_histo_msgsDB: Normalisation impossible" );
             return(FALSE);
           }
          g_snprintf( requete, sizeof(requete),                                            /* Requete SQL */
                      "UPDATE %s SET nom_ack='%s',date_fixe='%d'"
                      " WHERE id='%d'", NOM_TABLE_HISTO_MSGS, nom_ack, (int)histo->date_fixe, histo->id );
          g_free(nom_ack);
        }
       else
        { g_snprintf( requete, sizeof(requete),                                            /* Requete SQL */
                      "UPDATE %s as histo, %s as msg SET histo.alive=0,histo.date_fin='%d'"
                      " WHERE histo.alive=1 AND histo.id_msg = msg.id AND msg.num='%d'",
                      NOM_TABLE_HISTO_MSGS, NOM_TABLE_MSG,
                      (int)histo->date_fin, histo->msg.num );
        }
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_histoDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (ajout == TRUE)
     { histo->id = Recuperer_last_ID_SQL ( db ); }
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_histo_msgsDB: Ajoute un enregistrement dans la base de données                                 */
/* Entrée: L'enregistrement à modifier, en tant que structure complete                                    */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Ajouter_histo_msgsDB ( struct CMD_TYPE_HISTO *histo )
  { return ( Modifier_Ajouter_histo_msgsDB ( TRUE, histo ) ); }
/**********************************************************************************************************/
/* Modifier_histo_msgsDB: Modifie l'enregistrement MSGS en parametre                                      */
/* Entrée: L'enregistrement à modifier, en tant que structure complete                                    */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_histo_msgsDB ( struct CMD_TYPE_HISTO *histo )
  { return ( Modifier_Ajouter_histo_msgsDB ( FALSE, histo ) ); }
/**********************************************************************************************************/
/* Recuperer_histo_msgsDB: Recupération de l'historique MSGS du système, via requete                      */
/* Entrée: un log et une database, et des champs de requete                                               */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_histo_msgsDB ( struct DB **db_retour, struct CMD_CRITERE_HISTO_MSGS *critere )
  { gchar requete[1024];
    gchar critereSQL[1024];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT histo.id, histo.alive, msg.num, msg.libelle, msg.type, msg.id_syn,"
                "syn.groupe, syn.page, histo.nom_ack, histo.date_create_sec, histo.date_create_usec,"
                "histo.date_fixe,histo.date_fin"
                " FROM %s as histo,%s as syn, %s as msg"
                " WHERE msg.id_syn = syn.id AND histo.id_msg = msg.id",
                NOM_TABLE_HISTO_MSGS, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_MSG /* From */
              );

    memset( critereSQL, 0, sizeof(critereSQL) );
    if (critere->num != -1)
     { g_snprintf( critereSQL, sizeof(critereSQL), " AND msg.num=%d", critere->num );
       g_strlcat( requete, critereSQL, sizeof(requete) );
     }
    if (critere->type != -1)
     { g_snprintf( critereSQL, sizeof(critereSQL), " AND msg.type=%d", critere->type );
       g_strlcat( requete, critereSQL, sizeof(requete) );
     }
    if (critere->date_create_min!=-1)
     { g_snprintf( critereSQL, sizeof(critereSQL), " AND histo.date_create_sec>=%d", (int)critere->date_create_min );
       g_strlcat( requete, critereSQL, sizeof(requete) );
     }
    if (critere->date_create_max!=-1)
     { g_snprintf( critereSQL, sizeof(critereSQL), " AND histo.date_create_sec<=%d", (int)critere->date_create_max );
       g_strlcat( requete, critereSQL, sizeof(requete) );
     }
    if ( *(critere->nom_ack) )
     { gchar *norm;
       critere->nom_ack[sizeof(critere->nom_ack)-1] = 0;                          /* Anti buffer overflow */
       norm = Normaliser_chaine( critere->nom_ack);
       if (norm)
        { g_snprintf( critereSQL, sizeof(critereSQL), " AND histo.nom_ack LIKE '%%%s%%'", norm );
          g_strlcat( requete, critereSQL, sizeof(requete) );
          g_free(norm);
        }
     }
    if ( *(critere->libelle) )
     { gchar *norm;
       critere->libelle[sizeof(critere->libelle)-1] = 0;                          /* Anti buffer overflow */
       norm = Normaliser_chaine( critere->libelle );
       if (norm)
        { g_snprintf( critereSQL, sizeof(critereSQL), " AND msg.libelle LIKE '%%%s%%'", norm );
          g_strlcat( requete, critereSQL, sizeof(requete) );
          g_free(norm);
        }
     }
    if ( *(critere->groupage) )
     { gchar *norm;
       critere->groupage[sizeof(critere->groupage)-1] = 0;                        /* Anti buffer overflow */
       norm = Normaliser_chaine( critere->groupage );
       if (norm)
        { g_snprintf( critereSQL, sizeof(critereSQL),
                      " AND (syn.groupe LIKE '%%%s%%' OR syn.page LIKE '%%%s%%')", norm, norm );
          g_strlcat( requete, critereSQL, sizeof(requete) );
          g_free(norm);
        }
     }
    g_strlcat( requete, " ORDER BY histo.date_create_sec, histo.date_create_usec LIMIT 500;", sizeof(requete) );
 
    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_histoDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_histo_msgsDB_alive: Recupération de l'ensemble des messages encore Alive dans le BDD         */
/* Entrée: La base de données de travail                                                                  */
/* Sortie: False si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Recuperer_histo_msgsDB_alive ( struct DB **db_retour )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT histo.id, histo.alive, msg.num, msg.libelle, msg.type, msg.id_syn,"
                "syn.groupe, syn.page, histo.nom_ack, histo.date_create_sec, histo.date_create_usec,"
                "histo.date_fixe,histo.date_fin"
                " FROM %s as histo,%s as syn, %s as msg"
                " WHERE msg.id_syn = syn.id AND histo.id_msg = msg.id"
                " AND alive = 1 ORDER BY histo.date_create_sec, histo.date_create_usec",
                NOM_TABLE_HISTO_MSGS, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_MSG /* From */
              );
 
    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_histoDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Rechercher_histo_msgsDB_by_id: Recupération du l'histo by id dans la BDD                               */
/* Entrée: La base de données de travail                                                                  */
/* Sortie: False si probleme                                                                              */
/**********************************************************************************************************/
 struct CMD_TYPE_HISTO *Rechercher_histo_msgsDB_by_id ( guint id )
  { struct CMD_TYPE_HISTO *histo;
    gchar requete[1024];
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT histo.id, histo.alive, msg.num, msg.libelle, msg.type, msg.id_syn,"
                "syn.groupe, syn.page, histo.nom_ack, histo.date_create_sec, histo.date_create_usec,"
                "histo.date_fixe,histo.date_fin"
                " FROM %s as histo,%s as syn, %s as msg"
                " WHERE msg.id_syn = syn.id AND histo.id_msg = msg.id"
                " AND histo.id = %d",
                NOM_TABLE_HISTO_MSGS, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_MSG, /* From */
                id
              );
 
    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_histo_msgsDB_by_id: DB connexion failed" );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    histo = Recuperer_histo_msgsDB_suite( &db );
    if (histo) Libere_DB_SQL ( &db );
    return(histo);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_HISTO *Recuperer_histo_msgsDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_HISTO *histo_msgs;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    histo_msgs = (struct CMD_TYPE_HISTO *)g_try_malloc0( sizeof(struct CMD_TYPE_HISTO) );
    if (!histo_msgs) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                              "Recuperer_histo_msgsDB_suite: Erreur allocation mémoire" );
    else                                                                     /* Recopie dans la structure */
     { g_snprintf( histo_msgs->msg.libelle, sizeof(histo_msgs->msg.libelle), "%s", db->row[3] );
       g_snprintf( histo_msgs->msg.groupe,  sizeof(histo_msgs->msg.groupe),  "%s", db->row[6] );
       g_snprintf( histo_msgs->msg.page,    sizeof(histo_msgs->msg.page),    "%s", db->row[7] );
       g_snprintf( histo_msgs->nom_ack,     sizeof(histo_msgs->nom_ack),     "%s", db->row[8] );
       histo_msgs->id               = atoi(db->row[0]);
       histo_msgs->alive            = atoi(db->row[1]);
       histo_msgs->msg.num          = atoi(db->row[2]);
       histo_msgs->msg.type         = atoi(db->row[4]);
       histo_msgs->msg.id_syn      = atoi(db->row[5]);
       histo_msgs->date_create_sec  = atoi(db->row[9]);
       histo_msgs->date_create_usec = atoi(db->row[10]);
       histo_msgs->date_fixe        = atoi(db->row[11]);
       histo_msgs->date_fin         = atoi(db->row[12]);
     }
    return(histo_msgs);
  }
/*--------------------------------------------------------------------------------------------------------*/
