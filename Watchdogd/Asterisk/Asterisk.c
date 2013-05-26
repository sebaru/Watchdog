/**********************************************************************************************************/
/* Watchdogd/Asterisk/Asterisk.c        Déclaration des fonctions pour la gestion des asterisk            */
/* Projet WatchDog version 2.0       Gestion d'habitat                     sam. 28 mai 2011 18:53:52 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Asterisk.c
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
/* Retirer_asteriskDB: Elimination d'un asterisk                                                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 static gboolean Retirer_asteriskDB ( struct LOG *log, struct DB *db, gint id )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_ASTERISK, id );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_asteriskDB: Recupération de la liste des ids des asterisks                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Recuperer_asteriskDB ( struct LOG *log, struct DB *db )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,calleridnum,calleridname,bit"
                " FROM %s ORDER BY id LIMIT 1",
                NOM_TABLE_ASTERISK                                                                /* FROM */
              );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_asteriskDB: Recupération de la liste des ids des asterisks                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct CMD_TYPE_ASTERISK *Recuperer_asteriskDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_ASTERISK *asterisk;

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       return(NULL);
     }

    asterisk = (struct CMD_TYPE_ASTERISK *)g_try_malloc0( sizeof(struct CMD_TYPE_ASTERISK) );
    if (!asterisk) Info_new( Config.log, FALSE, LOG_WARNING, "Recuperer_asteriskDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &asterisk->calleridnum,  db->row[1], sizeof(asterisk->calleridnum  ) );
       memcpy( &asterisk->calleridname, db->row[2], sizeof(asterisk->calleridname ) );
       asterisk->id         = atoi(db->row[0]);
       asterisk->bit        = atoi(db->row[3]);
     }
    return(asterisk);
  }
/**********************************************************************************************************/
/* Asterisk_check_motion : Vérifie si l'outil motion a donner un bit a activer                            */
/* Entrée: un log et une database                                                                         */
/* Sortie: néant. Les bits DLS sont positionnés                                                           */
/**********************************************************************************************************/
 void Asterisk_check_call ( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_ASTERISK *asterisk;
    gint id;

    if ( ! Recuperer_asteriskDB ( log, db ) ) return;             /* On récupère UN seul enregistrement ! */
    id = 0;
    while ( (asterisk = Recuperer_asteriskDB_suite ( log, db )) != NULL )
     {
       Info_new( Config.log, FALSE, LOG_NOTICE, "Asterisk_check_call: Call from %s (num=%s)",
                 asterisk->calleridname, asterisk->calleridnum );
       if ( Config.asterisk_m_min <= asterisk->bit &&
                                     asterisk->bit <= Config.asterisk_m_max )
        { Info_new( Config.log, FALSE, LOG_INFO, "Asterisk_check_call: Mise a un du bit M%03d", asterisk->bit );
          Envoyer_commande_dls ( asterisk->bit );
        }
       else
        { Info_new( Config.log, FALSE, LOG_WARNING, "Asterisk_check_call: Numero de bit hors range M%03d", asterisk->bit ); }
       id = asterisk->id;
       g_free(asterisk);
     }
    if (id) Retirer_asteriskDB( log, db, id );
  }
/*--------------------------------------------------------------------------------------------------------*/
