/**********************************************************************************************************/
/* Watchdogd/Admin/admin_eana.c        Gestion des connexions Admin EANA au serveur watchdog              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim. 01 nov. 2009 09:28:26 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_eana.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
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

 #include "Admin.h"
 #include "EntreeANA_DB.h"
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Admin_eana_reload: Demande le rechargement des conf EANA                                               */
/* Entrée: le client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_eana_reload ( struct CLIENT_ADMIN *client )
  { Charger_eana();
    Write_admin ( client->connexion, " EANA Reloading done\n" );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_eana_list ( struct CLIENT_ADMIN *client )
  { struct CMD_TYPE_ENTREEANA *entree;
    GList *liste_eana;
    gchar chaine[128];
    struct DB *db;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_eana_list: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    if (!Recuperer_entreeANADB_simple( Config.log, db ))
     { Libere_DB_SQL( Config.log, &db );
       return;
     }                                                                           /* Si pas de histos (??) */

    for( ; ; )
     { entree = Recuperer_entreeANADB_simple_suite( Config.log, db );
       if (!entree)
        { Libere_DB_SQL( Config.log, &db );
          return;
        }

       g_snprintf( chaine, sizeof(chaine),
                   " EANA[%03d] -> unite=%s,min=%f,max=%f\n",
                   entree->num, entree->unite, entree->min, entree->max
                 );
       g_free(entree);
     }
  }
#ifdef bouh
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gint Admin_eana_add ( struct CLIENT_ADMIN *client, gint id, gint ea_min, gint ea_max,
                               gint e_min, gint e_max, gint ec_min, gint ec_max,gint s_min, gint s_max,
                               gint sa_min,gint sa_max )
  { gchar requete[128];
    struct DB *db;
    gint retour;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_eana_add: impossible d'ouvrir la Base de données",
               Config.db_database );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(id,actif,ea_min,ea_max,e_min,e_max,ec_min,ec_max,s_min,s_max,sa_min,sa_max) "
                " VALUES (%d,FALSE,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
                NOM_TABLE_MODULE_RS485,id,ea_min,ea_max,e_min,e_max,ec_min,ec_max,s_min,s_max,sa_min,sa_max
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return(-1);
     }
    retour = Recuperer_last_ID_SQL ( Config.log, db );
    Libere_DB_SQL( Config.log, &db );

    while (Partage->com_eana.admin_add) sched_yield();
    Partage->com_eana.admin_add = id;
    return(retour);
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_eana_del ( struct CLIENT_ADMIN *client, gint id )
  { gchar requete[128], chaine[128];
    struct DB *db;

    while (Partage->com_eana.admin_del) sched_yield();
    Partage->com_eana.admin_del = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_eana_del: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "DELETE FROM %s WHERE id = %d",
                NOM_TABLE_MODULE_RS485, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }

    Libere_DB_SQL( Config.log, &db );
    g_snprintf( chaine, sizeof(chaine), "Module RS485 %d deleted\n", id );
    Write_admin ( client->connexion, chaine );
  }
#endif
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_eana ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

     if ( ! strcmp ( commande, "ea" ) )
     { Admin_eana_list ( client );
     }
#ifdef bouh
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_eana_reload(client);
     }
    else if ( ! strcmp ( commande, "add" ) )
     { gint id, ea_min, ea_max, e_min, e_max, ec_min, ec_max, s_min, s_max, sa_min, sa_max;
       gchar chaine[128];
       sscanf ( ligne, "%s %d %d %d %d %d %d %d %d %d %d %d", commande, &id,
                &ea_min, &ea_max, &e_min, &e_max, &ec_min, &ec_max, &s_min, &s_max, &sa_min, &sa_max
              );                                                     /* Découpage de la ligne de commande */
       if ( ea_min < -1 || ea_min> NBR_BIT_DLS )
        { Write_admin ( client->connexion, " ea_min should be < NBR_BIT_DLS\n" ); }
       else if ( ea_max < -1 || ea_max> NBR_BIT_DLS )
        { Write_admin ( client->connexion, " ea_min should be < NBR_BIT_DLS\n" ); }
       else if ( e_min < -1 || e_min> NBR_BIT_DLS )
        { Write_admin ( client->connexion, " e_min should be < NBR_BIT_DLS\n" ); }
       else if ( e_max < -1 || e_max> NBR_BIT_DLS )
        { Write_admin ( client->connexion, " e_max should be < NBR_BIT_DLS\n" ); }
       else if ( ec_min < -1 || ec_min> NBR_BIT_DLS )
        { Write_admin ( client->connexion, " ec_min should be < NBR_BIT_DLS\n" ); }
       else if ( ea_max < -1 || ec_max> NBR_BIT_DLS )
        { Write_admin ( client->connexion, " ec_max should be < NBR_BIT_DLS\n" ); }
       else if ( s_min < -1 || s_min> NBR_BIT_DLS )
        { Write_admin ( client->connexion, " s_min should be < NBR_BIT_DLS\n" ); }
       else if ( s_max < -1 || s_max> NBR_BIT_DLS )
        { Write_admin ( client->connexion, " s_max should be < NBR_BIT_DLS\n" ); }
       else if ( sa_min < -1 || sa_min> NBR_BIT_DLS )
        { Write_admin ( client->connexion, " sa_min should be < NBR_BIT_DLS\n" ); }
       else if ( sa_max < -1 || sa_max> NBR_BIT_DLS )
        { Write_admin ( client->connexion, " sa_max should be < NBR_BIT_DLS\n" ); }
       else
        { int retour;
          retour = Admin_eana_add ( client, id, ea_min, ea_max, e_min, e_max, ec_min, ec_max,
                                     s_min, s_max, sa_min, sa_max );
          if (id != -1) { g_snprintf( chaine, sizeof(chaine), "Module RS485 %d added\n", retour ); }
          else          { g_snprintf( chaine, sizeof(chaine), "Module RS485 NOT added\n" ); }
          Write_admin ( client->connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "delete" ) )
     { guint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_eana_del ( client, num );
     }
#endif
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'RS485'\n" );
       Write_admin ( client->connexion,
                     "  add id ea_min ea_max e_min e_max ec_min ec_max s_min s_max sa_min sa_max - Ajoute un module RS485\n" );
       Write_admin ( client->connexion,
                     "  delete id                              - Supprime le module id\n" );
       Write_admin ( client->connexion,
                     "  start id                               - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  stop id                                - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  ea                                     - Affiche les configs EANA\n" );
       Write_admin ( client->connexion,
                     "  reload                                 - Recharge la configuration\n" );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
