/**********************************************************************************************************/
/* Watchdogd/Db/Histo/Histo.c        Déclaration des fonctions pour la gestion de l'historique            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 18 avr 2009 16:19:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Histo.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - 
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
 #include "Histo_DB.h"

/**********************************************************************************************************/
/* Clear_histoDB: Elimination des messages histo au boot systeme                                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Clear_histoDB ( void )
  { struct DB *db;
    gchar requete[1024];
    
    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database,        /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info( Config.log, DEBUG_INFO, "Charger_histoDB: Connexion DB failed" );
       return;
     }                                                                           /* Si pas de histos (??) */

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s", NOM_TABLE_HISTO );

    Lancer_requete_SQL ( Config.log, db, requete );                               /* Execution de la requete SQL */
    Libere_DB_SQL( Config.log, &db );
  }
/**********************************************************************************************************/
/* Retirer_msgDB: Elimination d'un message                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_histoDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_HISTO *histo )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_HISTO, histo->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Ajouter_histoDB ( struct LOG *log, struct DB *db, struct HISTODB *histo )
  { gchar requete[1024];
    gchar *libelle, *nom_ack, *objet;

    libelle = Normaliser_chaine ( log, histo->msg.libelle );             /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_histoDB: Normalisation impossible" );
       return(FALSE);
     }

    objet = Normaliser_chaine ( log, histo->msg.objet );                 /* Formatage correct des chaines */
    if (!objet)
     { Info( log, DEBUG_DB, "Ajouter_histoDB: Normalisation impossible" );
       g_free(libelle);
       return(FALSE);
     }

    nom_ack = Normaliser_chaine ( log, histo->nom_ack );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_histoDB: Normalisation impossible" );
       g_free(libelle);
       g_free(objet);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,libelle,objet,type,num_syn,nom_ack,"
                "date_create_sec,date_create_usec,"
                "date_fixe) VALUES "
                "(%d,'%s','%s',%d,%d,'%s',%d,%d,0)", NOM_TABLE_HISTO, histo->msg.num, libelle, 
                objet, histo->msg.type,
                histo->msg.num_syn, nom_ack,
                histo->date_create_sec, histo->date_create_usec );
    g_free(libelle);
    g_free(nom_ack);
    g_free(objet);

    return ( Lancer_requete_SQL ( log, db, requete ) );
  }
/**********************************************************************************************************/
/* Modifier_histoDB: Modification des champs editables d'un histo                                         */
/* Entrée: un log et une database, une structure de controle de la modification                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_histoDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_HISTO *histo )
  { gchar requete[1024];
    gchar *nom_ack;

    nom_ack = Normaliser_chaine ( log, histo->nom_ack );                 /* Formatage correct des chaines */
    if (!nom_ack)
     { Info( log, DEBUG_DB, "Modifier_histoDB: Normalisation impossible" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET nom_ack='%s',date_fixe=%d WHERE id=%d",
                NOM_TABLE_HISTO, nom_ack, (gint)histo->date_fixe, histo->id );
    g_free(nom_ack);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_histoDB ( struct LOG *log, struct DB *db )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,objet,type,num_syn,nom_ack,date_create_sec,date_create_usec,"
                "date_fixe FROM %s ORDER by date_create_sec,date_create_usec", NOM_TABLE_HISTO );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct HISTODB *Recuperer_histoDB_suite( struct LOG *log, struct DB *db )
  { struct HISTODB *histo;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    histo = (struct HISTODB *)g_malloc0( sizeof(struct HISTODB) );
    if (!histo) Info( log, DEBUG_MEM, "Recuperer_histoDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( histo->msg.libelle, db->row[1], sizeof(histo->msg.libelle) );    /* Recopie dans la structure */
       memcpy( histo->msg.objet,   db->row[2], sizeof(histo->msg.objet  ) ); /* Recopie dans la structure */
       memcpy( histo->nom_ack,     db->row[5], sizeof(histo->nom_ack    ) ); /* Recopie dans la structure */
       histo->msg.id           = 0;                                /* l'id n'est pas dans la base histo ! */
       histo->msg.num          = atoi(db->row[0]);
       histo->msg.type         = atoi(db->row[3]);
       histo->msg.num_syn      = atoi(db->row[4]);
       histo->date_create_sec  = atoi(db->row[6]);
       histo->date_create_usec = atoi(db->row[7]);
       histo->date_fixe        = atoi(db->row[8]);
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
                "SELECT libelle,objet,type,num_syn,nom_ack,date_create_sec,date_create_usec,"
                "date_fixe FROM %s WHERE id=%d", NOM_TABLE_HISTO, id );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_histoDB: histo non trouvé dans la BDD", id );
       return(NULL);
     }

    histo = (struct HISTODB *)g_malloc0( sizeof(struct HISTODB) );
    if (!histo) Info( log, DEBUG_MEM, "Recuperer_histoDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( histo->msg.libelle, db->row[0], sizeof(histo->msg.libelle) ); /* Recopie dans la structure */
       memcpy( histo->msg.objet,   db->row[1], sizeof(histo->msg.objet  ) ); /* Recopie dans la structure */
       memcpy( histo->nom_ack,     db->row[4], sizeof(histo->nom_ack    ) ); /* Recopie dans la structure */
       histo->msg.num          = id;
       histo->msg.id           = 0;                  /* L'ID msg en histo n'est pas porteur d'information */
       histo->msg.type         = atoi(db->row[2]);
       histo->msg.num_syn      = atoi(db->row[3]);
       histo->date_create_sec  = atoi(db->row[5]);
       histo->date_create_usec = atoi(db->row[6]);
       histo->date_fixe        = atoi(db->row[7]);
     }
    return(histo);
  }
/*--------------------------------------------------------------------------------------------------------*/
