/**********************************************************************************************************/
/* Watchdogd/Utilisateur/Utilisateur.c    Interface DB mots de passe pour watchdog2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 20:35:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Utilisateur.c
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
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>

 #include "watchdogd.h"
 #include "Erreur.h"
 #include "Utilisateur_DB.h"

 static gchar *UTILISATEURDB_RESERVE[NBR_UTILISATEUR_RESERVE][2]=
  { { "root", "Watchdog administrator" }
  };

/************************************ Prototypes des fonctions ********************************************/
 #include "proto_util.h"

/**********************************************************************************************************/
/* Nom_groupe_reserve: renvoie le nom en clair du groupe reserve d'id id                                  */
/* Entrée: l'id du groupe                                                                                 */
/* Sortie: une chaine de caractere non freable                                                            */
/**********************************************************************************************************/
 gchar *Nom_utilisateur_reserve( gint id )
  { if (id>=NBR_UTILISATEUR_RESERVE) return( "Unknown" );
    else { return( UTILISATEURDB_RESERVE[id][0] ); }
  }
/**********************************************************************************************************/
/* Commentaire_groupe_reserve: renvoie le commentaire en clair du groupe reserve d'id id                  */
/* Entrée: l'id du groupe                                                                                 */
/* Sortie: une chaine de caractere non freable                                                            */
/**********************************************************************************************************/
 gchar *Commentaire_utilisateur_reserve( gint id )
  { if (id>=NBR_UTILISATEUR_RESERVE) return( "Unknown" );
    else return( UTILISATEURDB_RESERVE[id][1] );
  }
/**********************************************************************************************************/
/* Retirer_utilisateur: Elimine un utilisateur dans la base de données                                    */
/* Entrées: un log, une db, un nom                                                                        */
/* Sortie: true si pas de pb, false sinon                                                                 */
/**********************************************************************************************************/
 gboolean Retirer_utilisateurDB( struct LOG *log, struct DB *db, struct CMD_TYPE_UTILISATEUR *util )
  { gchar requete[512];

    if (util->id < NBR_UTILISATEUR_RESERVE) 
     { Info_c( log, DEBUG_DB, "Retirer_utilisateurDB: elimination failed: id reserve", util->nom );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_UTIL, util->id );
    Lancer_requete_SQL ( log, db, requete );

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id_util=%d", NOM_TABLE_GIDS, util->id );
    Lancer_requete_SQL ( log, db, requete );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_utilisateur: Ajout d'un utilisateur dans la base de données                                    */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gint Ajouter_utilisateurDB( struct LOG *log, struct DB *db, gchar *clef,
                             struct CMD_TYPE_UTILISATEUR *util )
  { gchar requete[1024];
    gchar *crypt;
    gint id;

    if(g_utf8_strlen(util->code_en_clair, -1))
         { crypt = Crypter( log, clef, util->code_en_clair ); }
    else { crypt = Crypter( log, clef, "bouh" ); }

    if (crypt)
     { gchar *nom, *comment, *code_crypt;

       nom        = Normaliser_chaine ( log, util->nom );                /* Formatage correct des chaines */
       comment    = Normaliser_chaine ( log, util->commentaire );
       code_crypt = Normaliser_chaine ( log, crypt );
       g_free(crypt);

       if (!(nom && comment && code_crypt))
        { Info( log, DEBUG_DB, "Ajouter_utilisateurDB: Normalisation impossible" );
          return(-1);
        }

       g_snprintf( requete, sizeof(requete),                                              /* Requete SQL */
                   "INSERT INTO %s"             
                   "(name,changepass,cansetpass,crypt,comment,login_failed,enable,"
                   "date_create,enable_expire,date_expire,date_modif)"
                   "VALUES ('%s', %s, %s, '%s', '%s', '0', true, %d, %s, '%d', '%d' );",
                   NOM_TABLE_UTIL, nom,
                   (util->changepass ? "true" : "false"),
                   (util->cansetpass ? "true" : "false"), code_crypt,
                   comment, (gint)time(NULL),
                   (util->expire ? "true" : "false"), (gint)util->date_expire,
                   (gint)time(NULL) );
       g_free(nom);
       g_free(code_crypt);
       g_free(comment);

       Lancer_requete_SQL ( log, db, requete );
       id = Recuperer_last_ID_SQL ( log, db );
       Groupe_set_groupe_utilDB ( log, db, id, (guint *)&util->gids );      /* Positionnement des groupes */ 
       return(id);
     }
    Info_c( log, DEBUG_DB, "Ajouter_utilisateurDB: failed to encrypt password for", util->nom );
    return(-1);
  }
/**********************************************************************************************************/
/* Modifier_utilisateurDB: Modification d'u nutilisateur Watchdog                                         */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_utilisateurDB( struct LOG *log, struct DB *db, gchar *clef,
                                  struct CMD_TYPE_UTILISATEUR *util )
  { gchar requete[1024], chaine[100];
    gchar *comment;
    gchar *crypt;

    if (util->id < NBR_UTILISATEUR_RESERVE) 
     { return(FALSE); }

    comment = Normaliser_chaine ( log, util->commentaire );
    if (!comment)
     { Info( log, DEBUG_DB, "Modifier_utilisateurDB: Normalisation impossible" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                              /* Requete SQL */
                "UPDATE %s SET "             
                "changepass=%s,comment='%s',enable=%s,enable_expire=%s,"
                "cansetpass=%s,date_expire=%d,date_modif='%d'",
                NOM_TABLE_UTIL, (util->changepass ? "true" : "false"), comment,
                                (util->actif ? "true" : "false"),
                                (util->expire ? "true" : "false"),
                                (util->cansetpass ? "true" : "false"),
                                (gint)util->date_expire,
                                (gint)time(NULL) );
    g_free(comment);

    if (util->setpassnow)
     { gchar *code_crypt;
       if(g_utf8_strlen(util->code_en_clair, -1))
            { crypt = Crypter( log, clef, util->code_en_clair ); }
       else { crypt = Crypter( log, clef, "bouh" ); }
       if (!crypt)
        { Info( log, DEBUG_DB, "Modifier_utilisateurDB: cryptage password impossible" );
          return(FALSE);
        }

       code_crypt = Normaliser_chaine ( log, crypt );
       g_free(crypt);
       if (!code_crypt)
        { Info( log, DEBUG_DB, "Modifier_utilisateurDB: Normalisation code crypte impossible" );
          return(FALSE);
        }
       g_snprintf( chaine, sizeof(chaine), ",crypt='%s'", code_crypt );
       g_strlcat ( requete, chaine, sizeof(requete) );
       g_free(code_crypt);
     }

    g_snprintf( chaine, sizeof(chaine), " WHERE id='%d'", util->id ); 
    g_strlcat ( requete, chaine, sizeof(requete) );

    Lancer_requete_SQL ( log, db, requete );                               /* Execution de la requete SQL */
    Groupe_set_groupe_utilDB ( log, db, util->id, (guint *)&util->gids );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Rechercher_utilsDB: Recuperation de tous les champs des utilisateurs                                   */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 gboolean Recuperer_utilsDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
  
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT name,id,changepass,comment,enable,date_create,"
                "enable_expire,date_expire,cansetpass,date_modif "
                "FROM %s", NOM_TABLE_UTIL );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Rechercher_utilsDB: Recuperation de tous les champs des utilisateurs                                   */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 struct UTILISATEURDB *Recuperer_utilsDB_suite( struct LOG *log, struct DB *db )
  { struct UTILISATEURDB *util;
  
    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    util = (struct UTILISATEURDB *)g_malloc0( sizeof(struct UTILISATEURDB) );
    if (!util) Info( log, DEBUG_MEM, "Recuperer_utilsDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( util->nom, db->row[0], sizeof(util->nom) );                   /* Recopie dans la structure */
       memcpy( util->commentaire, db->row[3], sizeof(util->commentaire) );
       util->id            = atoi(db->row[1]);
       util->changepass    = atoi(db->row[2]);
       util->actif         = atoi(db->row[4]);
       util->date_creation = atoi(db->row[5]);
       util->expire        = atoi(db->row[6]);
       util->date_expire   = atoi(db->row[7]);
       util->cansetpass    = atoi(db->row[8]);
       util->date_modif    = atoi(db->row[9]);
     }
    return( util );
  }
/**********************************************************************************************************/
/* Rechercher_utilDB: Recuperation de tous les champs des utilisateurs                                    */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 struct UTILISATEURDB *Rechercher_utilisateurDB( struct LOG *log, struct DB *db, gint id )
  { gchar requete[200];
    struct UTILISATEURDB *util;
  
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT name,changepass,comment,enable,"
                "date_create,enable_expire,date_expire,cansetpass,date_modif "
                "FROM %s WHERE id=%d", NOM_TABLE_UTIL, id );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Info_n( log, DEBUG_DB, "Rechercher_utilisateurDB: Utilisateur non trouvé dans la BDD", id );
       Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    util = (struct UTILISATEURDB *)g_malloc0( sizeof(struct UTILISATEURDB) );
    if (!util) Info( log, DEBUG_MEM, "Rechercher_utilisateurDB: Erreur allocation mémoire" );
    else
     { memcpy( util->nom, db->row[0], sizeof(util->nom) );                   /* Recopie dans la structure */
       memcpy( util->commentaire, db->row[2], sizeof(util->commentaire) );
       util->id            = id;
       util->changepass    = atoi(db->row[1]);
       util->actif         = atoi(db->row[3]);
       util->date_creation = atoi(db->row[4]);
       util->expire        = atoi(db->row[5]);
       util->date_expire   = atoi(db->row[6]);
       util->cansetpass    = atoi(db->row[7]);
       util->date_modif    = atoi(db->row[8]);
     }
    Liberer_resultat_SQL ( log, db );
    if (util) Groupe_get_groupe_utilDB ( log, db, util->id, (guint *)&util->gids );
    return( util );
  }
/*--------------------------------------------------------------------------------------------------------*/
