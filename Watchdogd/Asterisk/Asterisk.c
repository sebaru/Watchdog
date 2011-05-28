/**********************************************************************************************************/
/* Watchdogd/Asterisk/Asterisk.c        D�claration des fonctions pour la gestion des asterisk            */
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
/* Entr�e: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_asteriskDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_ASTERISK *asterisk )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_ASTERISK, asterisk->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_asteriskDB: Recup�ration de la liste des ids des asterisks                          */
/* Entr�e: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_asteriskDB ( struct LOG *log, struct DB *db )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,calleridnum,calleridname,bit"
                " FROM %s ORDER BY id",
                NOM_TABLE_ASTERISK                                                                /* FROM */
              );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_asteriskDB: Recup�ration de la liste des ids des asterisks                          */
/* Entr�e: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_ASTERISK *Recuperer_asteriskDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_ASTERISK *asterisk;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    asterisk = (struct CMD_TYPE_ASTERISK *)g_malloc0( sizeof(struct CMD_TYPE_ASTERISK) );
    if (!asterisk) Info( log, DEBUG_ASTERISK, "Recuperer_asteriskDB_suite: Erreur allocation m�moire" );
    else
     { memcpy( &asterisk->calleridnum,  db->row[1], sizeof(asterisk->calleridnum  ) );
       memcpy( &asterisk->calleridname, db->row[2], sizeof(asterisk->calleridname ) );
       asterisk->id         = atoi(db->row[0]);
       asterisk->bit        = atoi(db->row[3]);
     }
    return(asterisk);
  }
/**********************************************************************************************************/
/* Asterisk_check_motion : V�rifie si l'outil motion a donner un bit a activer                            */
/* Entr�e: un log et une database                                                                         */
/* Sortie: n�ant. Les bits DLS sont positionn�s                                                           */
/**********************************************************************************************************/
 void Asterisk_check_call ( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_ASTERISK *asterisk;

    if ( ! Recuperer_asteriskDB ( log, db ) ) return;

    while ( (asterisk = Recuperer_asteriskDB_suite ( log, db )) != NULL )
     {
       Info_c( log, DEBUG_ASTERISK, "Asterisk_check_call:          Call from", asterisk->calleridname );
       Info_c( log, DEBUG_ASTERISK, "Asterisk_check_call:                Num", asterisk->calleridnum );
       Info_n( log, DEBUG_ASTERISK, "Asterisk_check_call: Mise a un du bit M", asterisk->bit );
       Envoyer_commande_dls ( asterisk->bit ); 
       Retirer_asteriskDB( log, db, asterisk );
       g_free(asterisk);
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
