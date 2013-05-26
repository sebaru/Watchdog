/**********************************************************************************************************/
/* Watchdogd/Synoptiques/capteur.c       Ajout/retrait de module capteur dans les synoptiques             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 29 jan 2006 15:09:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * capteur.c
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
/* Retirer_capteurDB: Elimination d'un capteur                                                            */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_capteurDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAPTEUR *capteur )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_CAPTEUR, capteur->id );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_capteurDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAPTEUR *capteur )
  { gchar requete[512];
    gchar *libelle;

    libelle = Normaliser_chaine ( capteur->libelle );               /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_capteurDB: Normalisation impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(syn_id,type,bitctrl,posx,posy,libelle,angle)"
                " VALUES (%d,%d,%d,%d,%d,'%s','%f')", NOM_TABLE_CAPTEUR,
                capteur->syn_id, capteur->type, capteur->bit_controle,
                capteur->position_x, capteur->position_y, libelle, capteur->angle );
    g_free(libelle);

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_capteurDB ( struct LOG *log, struct DB *db, gint id_syn )
  { gchar requete[2048];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,syn_id,type,bitctrl,libelle,posx,posy,angle"
                " FROM %s WHERE syn_id=%d",
                NOM_TABLE_CAPTEUR, id_syn );
    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_CAPTEUR *Recuperer_capteurDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_CAPTEUR *capteur;

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    capteur = (struct CMD_TYPE_CAPTEUR *)g_try_malloc0( sizeof(struct CMD_TYPE_CAPTEUR) );
    if (!capteur) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                           "Recuperer_capteurDB_suite: memory error" );
    else
     { capteur->id           = atoi(db->row[0]);
       capteur->syn_id       = atoi(db->row[1]);                   /* Synoptique ou est placée le capteur */
       capteur->type         = atoi(db->row[2]);
       capteur->bit_controle = atoi(db->row[3]);                                            /* Ixxx, Cxxx */
       capteur->position_x   = atoi(db->row[5]);                             /* en abscisses et ordonnées */
       capteur->position_y   = atoi(db->row[6]);
       capteur->angle        = atof(db->row[7]);
       memcpy( &capteur->libelle, db->row[4], sizeof(capteur->libelle) );    /* Recopie dans la structure */
     }
    return(capteur);
  }
/**********************************************************************************************************/
/* Rechercher_capteurDB: Recupération du capteur dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_CAPTEUR *Rechercher_capteurDB ( struct LOG *log, struct DB *db, guint id )
  { struct CMD_TYPE_CAPTEUR *capteur;
    gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT syn_id,bitctrl,type,libelle,posx,posy,angle "
                "FROM %s WHERE id=%d", 
                NOM_TABLE_CAPTEUR, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "Rechercher_capteurDB: Capteur %d not found in DB", id );
       return(NULL);
     }

    capteur = (struct CMD_TYPE_CAPTEUR *)g_try_malloc0( sizeof(struct CMD_TYPE_CAPTEUR) );
    if (!capteur) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_capteurDB: Erreur allocation mémoire" );
    else
     { capteur->id           = id;
       capteur->syn_id       = atoi(db->row[0]);                   /* Synoptique ou est placée le capteur */
       capteur->type         = atoi(db->row[2]);
       capteur->bit_controle = atoi(db->row[1]);                                            /* Ixxx, Cxxx */
       capteur->position_x   = atoi(db->row[4]);                             /* en abscisses et ordonnées */
       capteur->position_y   = atoi(db->row[5]);
       capteur->angle        = atof(db->row[6]);
       memcpy( &capteur->libelle, db->row[3], sizeof(capteur->libelle) );    /* Recopie dans la structure */
     }
    return(capteur);
  }
/**********************************************************************************************************/
/* Modifier_capteurDB: Modification d'un capteur Watchdog                                                 */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_capteurDB( struct LOG *log, struct DB *db, struct CMD_TYPE_CAPTEUR *capteur )
  { gchar requete[1024];
    gchar *libelle;

    libelle = Normaliser_chaine ( capteur->libelle );               /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_capteurDB: Normalisation impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "type=%d,bitctrl=%d,libelle='%s',posx=%d,posy=%d,angle='%f'"
                " WHERE id=%d;", NOM_TABLE_CAPTEUR,
                capteur->type, capteur->bit_controle, libelle,
                capteur->position_x, capteur->position_y, capteur->angle,
                capteur->id );
    g_free(libelle);

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
