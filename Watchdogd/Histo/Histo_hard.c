/**********************************************************************************************************/
/* Watchdogd/Histo/Histo_hard.c        Déclaration des fonctions pour la gestion des message              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 15 aoû 2003 13:02:48 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Histo_hard.c
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
 #include "Histo_DB.h"

/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Ajouter_histo_hardDB ( struct LOG *log, struct DB *db, struct HISTO_HARDDB *histo )
  { gchar requete[1024];
    gchar *libelle, *nom_ack;

    libelle = Normaliser_chaine ( log, histo->histo.msg.libelle );       /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_SERVEUR, "Ajouter_histo_hardDB: Normalisation impossible" );
       return(FALSE);
     }

    nom_ack = Normaliser_chaine ( log, histo->histo.nom_ack );           /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_SERVEUR, "Ajouter_histo_hardDB: Normalisation impossible" );
       g_free(libelle);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,libelle,type,num_syn,nom_ack,date_create_sec,date_create_usec,"
                "date_fixe,date_fin) VALUES "
                "(%d,'%s',%d,%d,'%s',%d,%d,%d,%d)", NOM_TABLE_HISTO_HARD, histo->histo.msg.num, libelle,
                histo->histo.msg.type, histo->histo.msg.num_syn, 
                nom_ack, histo->histo.date_create_sec, histo->histo.date_create_usec,
                (gint)histo->histo.date_fixe, (gint)histo->date_fin );
    g_free(libelle);
    g_free(nom_ack);

    return ( Lancer_requete_SQL ( log, db, requete ) );
  }
/**********************************************************************************************************/
/* Recuperer_histo_hardDB: Recupération de l'historique HARD du système, via requete                      */
/* Entrée: un log et une database, et des champs de requete                                               */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Rechercher_histo_hardDB ( struct LOG *log, struct DB *db, struct CMD_REQUETE_HISTO_HARD *critere )
  { gchar requete[1024];
    gchar critereSQL[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.libelle,type,num_syn,%s.groupe,%s.page,"
                "nom_ack,date_create_sec,date_create_usec,"
                "date_fixe,date_fin"
                " FROM %s,%s"
                " WHERE %s.num_syn = %s.id ",
                NOM_TABLE_HISTO_HARD, NOM_TABLE_HISTO_HARD, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_HISTO_HARD, NOM_TABLE_SYNOPTIQUE, /* From */
                NOM_TABLE_HISTO_HARD, NOM_TABLE_SYNOPTIQUE /* Where */
              );

    memset( critereSQL, 0, sizeof(critereSQL) );
    if (critere->id != -1)
     { g_snprintf( critereSQL, sizeof(critereSQL), " AND num=%d", critere->id );
       g_strlcat( requete, critereSQL, sizeof(requete) );
     }
    if (critere->type != -1)
     { g_snprintf( critereSQL, sizeof(critereSQL), " AND type=%d", critere->type );
       g_strlcat( requete, critereSQL, sizeof(requete) );
     }
    if (critere->date_create_min!=-1)
     { g_snprintf( critereSQL, sizeof(critereSQL), " AND date_create_sec>%d", (int)critere->date_create_min );
       g_strlcat( requete, critereSQL, sizeof(requete) );
     }
    if ( *(critere->nom_ack) )
     { gchar *norm;
       critere->nom_ack[sizeof(critere->nom_ack)-1] = 0;                          /* Anti buffer overflow */
       norm = Normaliser_chaine(log, critere->nom_ack);
       if (norm)
        { g_snprintf( critereSQL, sizeof(critereSQL), " AND nom_ack LIKE '%%%s%%'", norm );
          g_strlcat( requete, critereSQL, sizeof(requete) );
          g_free(norm);
        }
     }
    if ( *(critere->libelle) )
     { gchar *norm;
       critere->libelle[sizeof(critere->libelle)-1] = 0;                          /* Anti buffer overflow */
       norm = Normaliser_chaine(log, critere->libelle);
       if (norm)
        { g_snprintf( critereSQL, sizeof(critereSQL), " AND libelle LIKE '%%%s%%'", norm );
          g_strlcat( requete, critereSQL, sizeof(requete) );
          g_free(norm);
        }
     }
#ifdef bouh
    if ( *(critere->objet) )
     { gchar *norm;
       critere->objet[sizeof(critere->objet)-1] = 0;                              /* Anti buffer overflow */
       norm = Normaliser_chaine(log, critere->objet);
       if (norm)
        { g_snprintf( critereSQL, sizeof(critereSQL), " AND objet LIKE '%%%s%%'", norm );
          g_strlcat( requete, critereSQL, sizeof(requete) );
          g_free(norm);
        }
     }
#endif
    g_strlcat( requete, " ORDER BY date_create_sec,date_create_usec LIMIT 500;", sizeof(requete) );
 
    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct HISTO_HARDDB *Rechercher_histo_hardDB_suite( struct LOG *log, struct DB *db )
  { struct HISTO_HARDDB *histo_hard;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    histo_hard = (struct HISTO_HARDDB *)g_malloc0( sizeof(struct HISTO_HARDDB) );
    if (!histo_hard) Info( log, DEBUG_SERVEUR, "Recuperer_histo_hardDB_suite: Erreur allocation mémoire" );
    else                                                                     /* Recopie dans la structure */
     { memcpy( &histo_hard->histo.msg.libelle, db->row[1], sizeof(histo_hard->histo.msg.libelle) );
       memcpy( &histo_hard->histo.msg.groupe,  db->row[4], sizeof(histo_hard->histo.msg.groupe ) );
       memcpy( &histo_hard->histo.msg.page,    db->row[5], sizeof(histo_hard->histo.msg.page   ) );
       memcpy( &histo_hard->histo.nom_ack,     db->row[6], sizeof(histo_hard->histo.nom_ack    ) );
       histo_hard->histo.msg.num          = atoi(db->row[0]);
       histo_hard->histo.msg.type         = atoi(db->row[2]);
       histo_hard->histo.msg.num_syn      = atoi(db->row[3]);
       histo_hard->histo.date_create_sec  = atoi(db->row[7]);
       histo_hard->histo.date_create_usec = atoi(db->row[8]);
       histo_hard->histo.date_fixe        = atoi(db->row[9]);
       histo_hard->date_fin               = atoi(db->row[10]);
/*printf("Recup histo: %d %s %s %s\n", histo_hard->histo.msg.num, objet, libelle, histo_hard->histo.nom_ack );*/
     }
    return(histo_hard);
  }
/*--------------------------------------------------------------------------------------------------------*/
