/**********************************************************************************************************/
/* Watchdogd/Mnemonique/Mnemonique.c        Déclaration des fonctions pour la gestion des mnemoniques     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 19 avr 2009 15:15:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

 #include "watchdogd.h"
 #include "Erreur.h"
 #include "Mnemonique_DB.h"

/**********************************************************************************************************/
/* Retirer_mnemoDB: Elimination d'un mnemo                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_mnemoDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MNEMONIQUE *mnemo )
  { gchar requete[200];
    struct MNEMONIQUEDB *mnemo_a_virer;

    mnemo_a_virer = Rechercher_mnemoDB ( log, db, mnemo->id );
    if (mnemo_a_virer)
     { switch (mnemo_a_virer->type)
        { case MNEMO_ENTREE_ANA:
               g_snprintf( requete, sizeof(requete),                                          /* Requete SQL */
               "DELETE FROM %s WHERE id_mnemo=%d", NOM_TABLE_ENTREEANA, mnemo_a_virer->id );
               Lancer_requete_SQL ( log, db, requete );
               break;
          case MNEMO_CPT_IMP:
               g_snprintf( requete, sizeof(requete),                                          /* Requete SQL */
               "DELETE FROM %s WHERE id_mnemo=%d", NOM_TABLE_CPT_IMP, mnemo_a_virer->id );
               Lancer_requete_SQL ( log, db, requete );
               break;
          default:
               break;
        }
       g_free(mnemo_a_virer);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MNEMO, mnemo->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_mnemoDB: Ajout ou edition d'un mnemo                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure mnemo                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_mnemoDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MNEMONIQUE *mnemo )
  { gchar requete[200];
    gchar *libelle, *objet, *acro;
    gint last_id;

    libelle = Normaliser_chaine ( log, mnemo->libelle );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_SERVEUR, "Ajouter_mnemoDB: Normalisation impossible" );
       return(-1);
     }
    objet = Normaliser_chaine ( log, mnemo->objet );                     /* Formatage correct des chaines */
    if (!objet)
     { Info( log, DEBUG_SERVEUR, "Ajouter_mnemoDB: Normalisation impossible" );
       g_free(libelle);
       return(-1);
     }
    acro = Normaliser_chaine ( log, mnemo->acronyme );                   /* Formatage correct des chaines */
    if (!acro)
     { Info( log, DEBUG_SERVEUR, "Ajouter_mnemoDB: Normalisation impossible" );
       g_free(objet);
       g_free(libelle);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(type,num,objet,acronyme,libelle) VALUES "
                "(%d,%d,'%s','%s','%s')", NOM_TABLE_MNEMO, mnemo->type,
                mnemo->num, objet, acro, libelle );
    g_free(libelle);
    g_free(acro);
    g_free(objet);

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }

    last_id = Recuperer_last_ID_SQL( log, db );

    switch (mnemo->type)
     { case MNEMO_ENTREE_ANA:
            g_snprintf( requete, sizeof(requete),                                          /* Requete SQL */
                        "INSERT INTO %s(min,max,unite,id_mnemo) VALUES "
                        "('%f','%f','%d','%d')", NOM_TABLE_ENTREEANA, 0.0, 100.0, 0, last_id );
            Lancer_requete_SQL ( log, db, requete );
            break;
       case MNEMO_CPT_IMP:
            g_snprintf( requete, sizeof(requete),                                          /* Requete SQL */
                        "INSERT INTO %s(val,unite,id_mnemo,type_ci) VALUES "
                        "('%d','%d','%d','%d')", NOM_TABLE_CPT_IMP, 0, 0, last_id, 0 );
            Lancer_requete_SQL ( log, db, requete );
            break;
       default:
            break;
     }

    return( last_id );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_mnemoDB: Recupération de la liste des ids des mnemos                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_mnemoDB ( struct LOG *log, struct DB *db )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,type,num,objet,acronyme,libelle"
                " FROM %s ORDER BY objet,type,num", NOM_TABLE_MNEMO );          /* order by test 25/01/06 */

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_mnemoDB: Recupération de la liste des ids des mnemos                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MNEMONIQUEDB *Recuperer_mnemoDB_suite( struct LOG *log, struct DB *db )
  { struct MNEMONIQUEDB *mnemo;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    mnemo = (struct MNEMONIQUEDB *)g_malloc0( sizeof(struct MNEMONIQUEDB) );
    if (!mnemo) Info( log, DEBUG_SERVEUR, "Recuperer_mnemoDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &mnemo->libelle,  db->row[5], sizeof(mnemo->libelle ) );      /* Recopie dans la structure */
       memcpy( &mnemo->objet,    db->row[3], sizeof(mnemo->objet   ) );      /* Recopie dans la structure */
       memcpy( &mnemo->acronyme, db->row[4], sizeof(mnemo->acronyme) );      /* Recopie dans la structure */
       mnemo->id          = atoi(db->row[0]);
       mnemo->type        = atoi(db->row[1]);
       mnemo->num         = atoi(db->row[2]);
     }
    return(mnemo);
  }
/**********************************************************************************************************/
/* Rechercher_mnemoDB: Recupération du mnemo dont l'id est en parametre                                   */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MNEMONIQUEDB *Rechercher_mnemoDB ( struct LOG *log, struct DB *db, guint id )
  { gchar requete[200];
    struct MNEMONIQUEDB *mnemo;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle,acronyme,objet,type,num FROM %s WHERE id=%d ORDER BY acronyme",
                 NOM_TABLE_MNEMO, id
              );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_SERVEUR, "Rechercher_mnemoDB: Mnemo non trouvé dans la BDD", id );
       return(NULL);
     }

    mnemo = (struct MNEMONIQUEDB *)g_malloc0( sizeof(struct MNEMONIQUEDB) );
    if (!mnemo)
     { Info( log, DEBUG_SERVEUR, "Rechercher_mnemoDB: Mem error" ); }
    else
     { memcpy( &mnemo->libelle,  db->row[0], sizeof(mnemo->libelle ) );      /* Recopie dans la structure */
       memcpy( &mnemo->objet,    db->row[2], sizeof(mnemo->objet   ) );      /* Recopie dans la structure */
       memcpy( &mnemo->acronyme, db->row[1], sizeof(mnemo->acronyme) );      /* Recopie dans la structure */
       mnemo->id          = id;
       mnemo->type        = atoi(db->row[3]);
       mnemo->num         = atoi(db->row[4]);
     }
    return(mnemo);
  }
/**********************************************************************************************************/
/* Rechercher_mnemoDB: Recupération du mnemo dont l'id est en parametre                                   */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MNEMONIQUEDB *Rechercher_mnemoDB_type_num ( struct LOG *log, struct DB *db,
                                                    struct CMD_TYPE_NUM_MNEMONIQUE *critere )
  { gchar requete[200];
    struct MNEMONIQUEDB *mnemo;
    
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle,acronyme,objet,id FROM %s WHERE type=%d AND num=%d",
                NOM_TABLE_MNEMO, critere->type, critere->num );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info( log, DEBUG_SERVEUR, "Rechercher_mnemoDB_type_num: Mnemo non trouvé dans la BDD" );
       return(NULL);
     }

    mnemo = (struct MNEMONIQUEDB *)g_malloc0( sizeof(struct MNEMONIQUEDB) );
    if (!mnemo)
     { Info( log, DEBUG_SERVEUR, "Rechercher_mnemoDB_type_num: Mem error" ); }
    else
     { memcpy( &mnemo->libelle,  db->row[0], sizeof(mnemo->libelle ) );      /* Recopie dans la structure */
       memcpy( &mnemo->objet,    db->row[2], sizeof(mnemo->objet   ) );      /* Recopie dans la structure */
       memcpy( &mnemo->acronyme, db->row[1], sizeof(mnemo->acronyme) );      /* Recopie dans la structure */
       mnemo->id      = atoi(db->row[3]);
       mnemo->type    = critere->type;
       mnemo->num     = critere->num;
     }
    return(mnemo);
  }
/**********************************************************************************************************/
/* Modifier_mnemoDB: Modification d'un mnemo Watchdog                                                     */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_mnemoDB( struct LOG *log, struct DB *db, struct CMD_TYPE_MNEMONIQUE *mnemo )
  { gchar requete[1024];
    gchar *libelle, *objet, *acronyme;

    libelle = Normaliser_chaine ( log, mnemo->libelle );
    if (!libelle)
     { Info( log, DEBUG_SERVEUR, "Modifier_mnemoDB: Normalisation impossible" );
       return(FALSE);
     }

    objet = Normaliser_chaine ( log, mnemo->objet );
    if (!objet)
     { Info( log, DEBUG_SERVEUR, "Modifier_mnemoDB: Normalisation impossible" );
       g_free(libelle);
       return(FALSE);
     }

    acronyme = Normaliser_chaine ( log, mnemo->acronyme );
    if (!acronyme)
     { Info( log, DEBUG_SERVEUR, "Modifier_mnemoDB: Normalisation impossible" );
       g_free(objet);
       g_free(libelle);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s',acronyme='%s',objet='%s',type=%d,num=%d WHERE id=%d",
                NOM_TABLE_MNEMO, libelle, acronyme, objet,mnemo->type, mnemo->num, mnemo->id );
    g_free(libelle);
    g_free(acronyme);
    g_free(objet);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_mnemoDB: Recupération de la liste des ids des mnemos                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_mnemoDB_for_courbe ( struct LOG *log, struct DB *db )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,type,num,objet,acronyme,libelle"
                " FROM %s WHERE type=%d OR type=%d"
                " ORDER BY objet,type,num",
                NOM_TABLE_MNEMO, MNEMO_ENTREE, MNEMO_SORTIE
              );                                                                /* order by test 25/01/06 */

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_mnemoDB: Recupération de la liste des ids des mnemos                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MNEMONIQUEDB *Recuperer_mnemoDB_for_courbe_suite( struct LOG *log, struct DB *db )
  { struct MNEMONIQUEDB *mnemo;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    mnemo = (struct MNEMONIQUEDB *)g_malloc0( sizeof(struct MNEMONIQUEDB) );
    if (!mnemo) Info( log, DEBUG_SERVEUR, "Recuperer_mnemoDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &mnemo->libelle,  db->row[5], sizeof(mnemo->libelle ) );      /* Recopie dans la structure */
       memcpy( &mnemo->objet,    db->row[3], sizeof(mnemo->objet   ) );      /* Recopie dans la structure */
       memcpy( &mnemo->acronyme, db->row[4], sizeof(mnemo->acronyme) );      /* Recopie dans la structure */
       mnemo->id          = atoi(db->row[0]);
       mnemo->type        = atoi(db->row[1]);
       mnemo->num         = atoi(db->row[2]);
     }
    return(mnemo);
  }
/*--------------------------------------------------------------------------------------------------------*/
