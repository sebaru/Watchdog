/**********************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.c  Gestion des modules UPS Watchdgo 2.0                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 10 nov. 2009 15:56:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Onduleur.c
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
 
 #include <stdio.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <upsclient.h>
 #include <locale.h>

/********************************************* Headers ****************************************************/
 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 #include "Onduleur.h"

/**********************************************************************************************************/
/* Ups_Lire_config : Lit la config Watchdog et rempli la structure mémoire                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Ups_Lire_config ( void )
  { GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Ups_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_ups.lib->Thread_debug = g_key_file_get_boolean ( gkf, "UPS", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */

    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Ups_Liberer_config : Libere la mémoire allouer précédemment pour lire la config imsg                  */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Ups_Liberer_config ( void )
  {
  }
/**********************************************************************************************************/
/* Retirer_upsDB: Elimination d'un ups                                                                    */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_upsDB ( struct UPSDB *ups )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "Retirer_upsDB: Database Connection Failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_UPS, ups->id );

    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( Config.log, &db );
    Cfg_ups.reload = TRUE;                         /* Rechargement des modules RS en mémoire de travaille */
    return(retour);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_upsDB: Recupération de la liste des ids des upss                                    */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Recuperer_upsDB ( struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,host,ups,bit_comm,enable,ea_min,libelle,e_min,a_min,username,password "
                " FROM %s ORDER BY host,ups", NOM_TABLE_UPS );

    return ( Lancer_requete_SQL ( Config.log, db, requete ) );             /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_upsDB: Recupération de la liste des ids des upss                                    */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct UPSDB *Recuperer_upsDB_suite( struct DB *db )
  { struct UPSDB *ups;

    Recuperer_ligne_SQL (Config.log, db);                              /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( Config.log, db );
       return(NULL);
     }

    ups = (struct UPSDB *)g_try_malloc0( sizeof(struct UPSDB) );
    if (!ups) Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "Recuperer_upsDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &ups->host,     db->row[1],  sizeof(ups->host   ) );
       memcpy( &ups->ups,      db->row[2],  sizeof(ups->ups    ) );
       memcpy( &ups->libelle,  db->row[6],  sizeof(ups->libelle) );
       memcpy( &ups->username, db->row[9],  sizeof(ups->username) );
       memcpy( &ups->password, db->row[10], sizeof(ups->password) );
       ups->id                = atoi(db->row[0]);
       ups->bit_comm          = atoi(db->row[3]);
       ups->enable            = atoi(db->row[4]);
       ups->ea_min            = atoi(db->row[5]);
       ups->e_min             = atoi(db->row[7]);
       ups->a_min             = atoi(db->row[8]);
     }
    return(ups);
  }
/**********************************************************************************************************/
/* Modifier_upsDB: Modification d'un ups Watchdog                                                         */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 static gint Ajouter_modifier_upsDB( struct UPSDB *ups, gboolean ajout )
  { gchar *host, *name, *libelle,*username,*password;
    gboolean retour_sql;
    gchar requete[2048];
    struct DB *db;
    gint retour;

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_upsDB: Database Connection Failed" );
       return(-1);
     }

    host = Normaliser_chaine ( Config.log, ups->host );                  /* Formatage correct des chaines */
    if (!host)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_upsDB: Normalisation host impossible" );
       return(-1);
     }
    name = Normaliser_chaine ( Config.log, ups->ups );                   /* Formatage correct des chaines */
    if (!name)
     { g_free(host);
       Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_upsDB: Normalisation name impossible" );
       return(-1);
     }
    libelle = Normaliser_chaine ( Config.log, ups->libelle );            /* Formatage correct des chaines */
    if (!libelle)
     { g_free(host);
       g_free(name);
       Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_upsDB: Normalisation libelle impossible" );
       return(-1);
     }
    username = Normaliser_chaine ( Config.log, ups->username );          /* Formatage correct des chaines */
    if (!username)
     { g_free(host);
       g_free(name);
       g_free(libelle);
       Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_upsDB: Normalisation username impossible" );
       return(-1);
     }
    password = Normaliser_chaine ( Config.log, ups->password );          /* Formatage correct des chaines */
    if (!password)
     { g_free(host);
       g_free(name);
       g_free(libelle);
       g_free(username);
       Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_upsDB: Normalisation password impossible" );
       return(-1);
     }

    if (ajout == TRUE)
     { g_snprintf( requete, sizeof(requete),
                   "INSERT INTO %s"
                   "(host,ups,libelle,bit_comm,enable,ea_min,e_min,a_min) "
                   "VALUES ('%s','%s','%s',%d,%d,%d,%d,%d,'%s','%s')",
                   NOM_TABLE_UPS, host, name, libelle, ups->bit_comm, ups->enable,
                   ups->ea_min, ups->e_min, ups->a_min, username,password
                 );
     }
    else
     { g_snprintf( requete, sizeof(requete),                                               /* Requete SQL */
                   "UPDATE %s SET "             
                   "host='%s',ups='%s',bit_comm=%d,enable=%d,"
                   "ea_min=%d,e_min=%d,a_min=%d,"
                   "libelle='%s',username='%s',password='%s' "
                   "WHERE id=%d",
                   NOM_TABLE_UPS, host, name, ups->bit_comm, ups->enable,
                   ups->ea_min, ups->e_min, ups->a_min,
                   libelle, username, password,
                   ups->id );
     }
    g_free(host);
    g_free(name);
    g_free(libelle);
    g_free(username);
    g_free(password);

    retour_sql = Lancer_requete_SQL ( Config.log, db, requete );               /* Lancement de la requete */
    if ( retour_sql == TRUE )                                                          /* Si pas d'erreur */
     { if (ajout==TRUE) retour = Recuperer_last_ID_SQL( Config.log, db );    /* Retourne le nouvel ID ups */
       else retour = 0;
     }
    else retour = -1;
    Libere_DB_SQL( Config.log, &db );
    Cfg_ups.reload = TRUE;                         /* Rechargement des modules RS en mémoire de travaille */
    return ( retour );                                            /* Pas d'erreur lors de la modification */
  }
/**********************************************************************************************************/
/* Modifier_upsDB: Modification d'un ups Watchdog                                                         */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gint Modifier_upsDB( struct UPSDB *ups )
  { return ( Ajouter_modifier_upsDB( ups, FALSE ) );
  }
/**********************************************************************************************************/
/* Modifier_upsDB: Modification d'un ups Watchdog                                                         */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gint Ajouter_upsDB( struct UPSDB *ups )
  { return ( Ajouter_modifier_upsDB( ups, TRUE ) );
  }
/**********************************************************************************************************/
/* Charger_tous_Requete la DB pour charger les modules ups                                                */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 struct MODULE_UPS *Chercher_module_ups_by_id ( gint id )
  { struct MODULE_UPS *module;
    GSList *liste;
    module = NULL;
    pthread_mutex_lock ( &Cfg_ups.lib->synchro );
    liste = Cfg_ups.Modules_UPS;
    while ( liste )
     { module = ((struct MODULE_UPS *)liste->data);
       if (module->ups.id == id) break;
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Cfg_ups.lib->synchro );
    if (liste) return(module);
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO, "Chercher_module_ups_by_id: UPS %d not found", id );
    return(NULL);
  }
/**********************************************************************************************************/
/* Charger_tous_ups: Requete la DB pour charger les modules ups                                           */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_tous_ups ( void  )
  { struct DB *db;
    gint cpt;

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "Charger_tous_ups: Database Connection Failed" );
       return(FALSE);
     }

/********************************************** Chargement des modules ************************************/
    if ( ! Recuperer_upsDB( db ) )
     { Libere_DB_SQL( Config.log, &db );
       Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "Charger_tous_ups: Recuperer_ups Failed" );
       return(FALSE);
     }

    Cfg_ups.Modules_UPS = NULL;
    cpt = 0;
    for ( ; ; )
     { struct MODULE_UPS *module;
       struct UPSDB *ups;

       ups = Recuperer_upsDB_suite( db );
       if (!ups) break;

       module = (struct MODULE_UPS *)g_try_malloc0( sizeof(struct MODULE_UPS) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR,
                   "Charger_tous_Erreur allocation mémoire struct MODULE_UPS" );
          g_free(ups);
          Libere_DB_SQL( Config.log, &db );
          return(FALSE);
        }
       memcpy( &module->ups, ups, sizeof(struct UPSDB) );
       g_free(ups);
       cpt++;                                              /* Nous avons ajouté un module dans la liste ! */
                                                                        /* Ajout dans la liste de travail */
       pthread_mutex_lock( &Cfg_ups.lib->synchro );
       Cfg_ups.Modules_UPS = g_slist_prepend ( Cfg_ups.Modules_UPS, module );
       pthread_mutex_unlock( &Cfg_ups.lib->synchro );
       Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO,
                "Charger_tous_ups: id = %03d, host=%s, name=%s",
                 module->ups.id, module->ups.host, module->ups.ups );
     }
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO, "Charger_tous_ups: %03d UPS found  !", cpt );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Rechercher_upsDB: Recupération du ups dont le num est en parametre                                     */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_un_UPS ( struct MODULE_UPS *module )
  { if (!module) return;
    pthread_mutex_lock( &Cfg_ups.lib->synchro );
    Cfg_ups.Modules_UPS = g_slist_remove ( Cfg_ups.Modules_UPS, module );
    g_free(module);
    pthread_mutex_unlock( &Cfg_ups.lib->synchro );
  }
/**********************************************************************************************************/
/* Decharger_tous_Decharge l'ensemble des modules UPS                                                     */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Decharger_tous_UPS ( void  )
  { struct MODULE_UPS *module;
    while ( Cfg_ups.Modules_UPS )
     { module = (struct MODULE_UPS *)Cfg_ups.Modules_UPS->data;
       Decharger_un_UPS ( module );
     }
  }
/**********************************************************************************************************/
/* Deconnecter: Deconnexion du module                                                                     */
/* Entrée: un id                                                                                          */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Deconnecter_module ( struct MODULE_UPS *module )
  { gint num_ea;
    if (!module) return;

    if (module->started == TRUE)
     { upscli_disconnect( &module->upsconn );
       module->started = FALSE;
       module->nbr_deconnect++;
     }

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO, "Deconnecter_module %d", module->ups.id );
    SB( module->ups.bit_comm, 0 );                       /* Mise a zero du bit interne lié au module */

    num_ea = module->ups.ea_min;
    SEA_range( num_ea++, 0);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 0);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 0);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 0);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 0);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 0);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 0);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 0);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 0);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 0);                                             /* Numéro de l'EA pour la valeur */
  }
/**********************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                           */
/* Entrée: une nom et un password                                                                         */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                          */
/**********************************************************************************************************/
 static gboolean Connecter_ups ( struct MODULE_UPS *module )
  { gchar buffer[80];
    gint num_ea;
    int connexion;

    if ( (connexion = upscli_connect( &module->upsconn, module->ups.host,
                                      UPS_PORT_TCP, UPSCLI_CONN_TRYSSL)) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "Connecter_ups: connexion refused by module %d (%s)",
                 module->ups.id, (char *)upscli_strerror(&module->upsconn) );
       return(FALSE);
     }

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO, "Connecter_ups: %s", module->ups.host );

/********************************************* UPSDESC ****************************************************/
    g_snprintf( buffer, sizeof(buffer), "GET UPSDESC %s\n", module->ups.ups );
    if ( upscli_sendline( &module->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "Connecter_ups: Sending GET UPSDESC failed (%s)",
                (char *)upscli_strerror(&module->upsconn) );
     }
    else
     { if ( upscli_readline( &module->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                   "Connecter_ups: Reading GET UPSDESC failed (%s)",
                   (char *)upscli_strerror(&module->upsconn) );
        }
       else
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG, 
                   "Connecter_ups: Reading GET UPSDESC %s",
                   buffer + strlen(module->ups.ups) + 9 );
        }
     }

/********************************************* USERNAME ***************************************************/
    g_snprintf( buffer, sizeof(buffer), "USERNAME %s\n", module->ups.username );
    if ( upscli_sendline( &module->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "Connecter_ups: Sending USERNAME failed %s",
                (char *)upscli_strerror(&module->upsconn) );
     }
    else
     { if ( upscli_readline( &module->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                   "Connecter_ups: Reading USERNAME failed %s",
                   (char *)upscli_strerror(&module->upsconn) );
        }
       else
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                   "Connecter_ups: Reading USERNAME %s",
                    buffer );
        }
     }

/********************************************* PASSWORD ***************************************************/
    g_snprintf( buffer, sizeof(buffer), "PASSWORD %s\n", module->ups.password );
    if ( upscli_sendline( &module->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "Connecter_ups: Sending PASSWORD failed %s",
                (char *)upscli_strerror(&module->upsconn) );
     }
    else
     { if ( upscli_readline( &module->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                   "Connecter_ups: Reading PASSWORD failed %s",
                   (char *)upscli_strerror(&module->upsconn) );
        }
       else
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                   "Connecter_ups: Reading PASSWORD %s",
                   buffer );
        }
     }

    module->date_retente = 0;
    module->started = TRUE;
    SB( module->ups.bit_comm, 1 );                         /* Mise a un du bit interne lié au module */
    num_ea = module->ups.ea_min;
    SEA_range( num_ea++, 1);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 1);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 1);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 1);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 1);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 1);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 1);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 1);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 1);                                             /* Numéro de l'EA pour la valeur */
    SEA_range( num_ea++, 1);                                             /* Numéro de l'EA pour la valeur */
    return(TRUE);
  }
/**********************************************************************************************************/
/* Onduleur_set_instcmd: Envoi d'une instant commande à l'ups                                             */
/* Entrée : l'ups, le nom de la commande                                                                  */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                      */
/**********************************************************************************************************/
 gboolean Onduleur_set_instcmd ( struct MODULE_UPS *module, gchar *nom_cmd )
  { gchar buffer[80];

    g_snprintf( buffer, sizeof(buffer), "INSTCMD %s %s\n", module->ups.ups, nom_cmd );
    if ( upscli_sendline( &module->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                 "Onduleur_set_instcmd: Sending INSTCMD failed (%s) error %s",
                 buffer, (char *)upscli_strerror(&module->upsconn) );
       return(FALSE);
     }

    if ( upscli_readline( &module->upsconn, buffer, sizeof(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "Onduleur_set_instcmd: Reading INSTCMD result failed (%s) error %s",
                 nom_cmd, (char *)upscli_strerror(&module->upsconn) );
       return(FALSE);
     }
    else
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                "Onduleur_set_instcmd: Sending INSTCMD OK",
                 nom_cmd, buffer );
     }
    return(TRUE);
  }

/**********************************************************************************************************/
/* Onduleur_get_var: Recupere une valeur de la variable en parametre                                      */
/* Entrée : l'ups, le nom de variable, la variable a renseigner                                           */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                      */
/**********************************************************************************************************/
 gboolean Onduleur_get_var ( struct MODULE_UPS *module, gchar *nom_var, gfloat *retour )
  { gchar buffer[80];

    g_snprintf( buffer, sizeof(buffer), "GET VAR %s %s\n", module->ups.ups, nom_var );
    if ( upscli_sendline( &module->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "Onduleur_get_var: Sending GET VAR failed (%s) error=%s",
                buffer, (char *)upscli_strerror(&module->upsconn) );
       return(FALSE);
     }

    if ( upscli_readline( &module->upsconn, buffer, sizeof(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "Onduleur_get_var: Reading GET VAR result failed (%s) error=%s",
                 nom_var, (char *)upscli_strerror(&module->upsconn) );
       if (upscli_upserror(&module->upsconn) != UPSCLI_ERR_VARNOTSUPP)        /* Variable non supportée ? */
        { return(FALSE); }
       *retour = 0.0;
       return(TRUE);                                     /* Variable not supported... is not an error ... */
     }

    if ( ! strncmp ( buffer, "VAR", 3 ) )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                "Onduleur_get_var: Reading GET VAR %s OK = %s", nom_var, buffer );
       *retour = atof ( buffer + 7 + strlen(module->ups.ups) + strlen(nom_var) );
       return(TRUE);
     }

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
             "Onduleur_get_var: Reading GET VAR %s NOK error %s", nom_var, buffer );
    return(FALSE);
  }
/**********************************************************************************************************/
/* Envoyer_sortie_ups: Envoi des sorties/InstantCommand à l'ups                                           */
/* Entrée: identifiants des modules ups                                                                   */
/* Sortie: TRUE si pas de probleme, FALSE sinon                                                           */
/**********************************************************************************************************/
 static gboolean Envoyer_sortie_ups( struct MODULE_UPS *module )
  { gint num_a;

    num_a = module->ups.a_min;
    if (A(num_a)) { if (Onduleur_set_instcmd ( module, "load.off" ) == FALSE) return(FALSE); SA(num_a,0); }
    num_a++;
    if (A(num_a)) { if (Onduleur_set_instcmd ( module, "load.on" ) == FALSE) return(FALSE); SA(num_a,0); }
    num_a++;
    if (A(num_a)) { if (Onduleur_set_instcmd ( module, "outlet.1.load.off" ) == FALSE) return(FALSE); SA(num_a,0); }
    num_a++;
    if (A(num_a)) { if (Onduleur_set_instcmd ( module, "outlet.1.load.on" ) == FALSE) return(FALSE); SA(num_a,0); }
    num_a++;
    if (A(num_a)) { if (Onduleur_set_instcmd ( module, "outlet.2.load.off" ) == FALSE) return(FALSE); SA(num_a,0); }
    num_a++;
    if (A(num_a)) { if (Onduleur_set_instcmd ( module, "outlet.2.load.on" ) == FALSE) return(FALSE); SA(num_a,0); }
    num_a++;
    if (A(num_a)) { if (Onduleur_set_instcmd ( module, "test.battery.start.deep" ) == FALSE) return(FALSE); SA(num_a,0); }
    num_a++;
    if (A(num_a)) { if (Onduleur_set_instcmd ( module, "test.battery.start.quick" ) == FALSE) return(FALSE); SA(num_a,0); }
    num_a++;
    if (A(num_a)) { if (Onduleur_set_instcmd ( module, "test.battery.stop" ) == FALSE) return(FALSE); SA(num_a,0); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Interroger_ups: Interrogation d'un ups                                                                 */
/* Entrée: identifiants des modules ups                                                                   */
/* Sortie: TRUE si pas de probleme, FALSE sinon                                                           */
/**********************************************************************************************************/
 static gboolean Interroger_ups( struct MODULE_UPS *module )
  { gfloat valeur;
    gint num_ea;

    num_ea = module->ups.ea_min;

    if ( Onduleur_get_var ( module, "ups.load", &valeur ) == FALSE ) return(FALSE);
    SEA( num_ea++, valeur );                                             /* Numéro de l'EA pour la valeur */
    
    if ( Onduleur_get_var ( module, "ups.realpower", &valeur ) == FALSE ) return(FALSE);
    SEA( num_ea++, valeur );                                             /* Numéro de l'EA pour la valeur */

    if ( Onduleur_get_var ( module, "battery.charge", &valeur ) == FALSE ) return(FALSE);
    SEA( num_ea++, valeur );                                             /* Numéro de l'EA pour la valeur */

    if ( Onduleur_get_var ( module, "input.voltage", &valeur ) == FALSE ) return(FALSE);
    SEA( num_ea++, valeur );                                             /* Numéro de l'EA pour la valeur */

    if ( Onduleur_get_var ( module, "battery.runtime", &valeur ) == FALSE ) return(FALSE);
    SEA( num_ea++, valeur );                                             /* Numéro de l'EA pour la valeur */

    if ( Onduleur_get_var ( module, "battery.voltage", &valeur ) == FALSE ) return(FALSE);
    SEA( num_ea++, valeur );                                             /* Numéro de l'EA pour la valeur */

    if ( Onduleur_get_var ( module, "input.frequency", &valeur ) == FALSE ) return(FALSE);
    SEA( num_ea++, valeur );                                             /* Numéro de l'EA pour la valeur */

    if ( Onduleur_get_var ( module, "output.current", &valeur ) == FALSE ) return(FALSE);
    SEA( num_ea++, valeur );                                             /* Numéro de l'EA pour la valeur */

    if ( Onduleur_get_var ( module, "output.frequency", &valeur ) == FALSE ) return(FALSE);
    SEA( num_ea++, valeur );                                             /* Numéro de l'EA pour la valeur */

    if ( Onduleur_get_var ( module, "output.voltage", &valeur ) == FALSE ) return(FALSE);
    SEA( num_ea++, valeur );                                             /* Numéro de l'EA pour la valeur */

    return(TRUE);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du UPS                                                                       */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct MODULE_UPS *module;
    GSList *liste;

    prctl(PR_SET_NAME, "W-UPS", 0, 0, 0 );
    memset( &Cfg_ups, 0, sizeof(Cfg_ups) );                     /* Mise a zero de la structure de travail */
    Cfg_ups.lib = lib;                         /* Sauvegarde de la structure pointant sur cette librairie */
    Ups_Lire_config ();                                 /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );
    Cfg_ups.lib->Thread_run = TRUE;                                                 /* Le thread tourne ! */

    g_snprintf( Cfg_ups.lib->admin_prompt, sizeof(Cfg_ups.lib->admin_prompt), "ups" );
    g_snprintf( Cfg_ups.lib->admin_help,   sizeof(Cfg_ups.lib->admin_help),   "Manage UPS Modules" );

    Cfg_ups.Modules_UPS = NULL;                                  /* Init des variables du thread */

    if ( Charger_tous_ups() == FALSE )                                      /* Chargement des modules ups */
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "Run_thread: No module UPS found -> stop" );
       Ups_Liberer_config ();                           /* Lecture de la configuration logiciel du thread */
       lib->Thread_run = FALSE;                                             /* Le thread ne tourne plus ! */
       lib->TID        = 0;                               /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     }

    setlocale( LC_ALL, "C" );                        /* Pour le formattage correct des , . dans les float */
    while(lib->Thread_run == TRUE)                                    /* On tourne tant que l'on a besoin */
     { sleep(1);
       sched_yield();

       if (Cfg_ups.reload == TRUE)
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE, "Run_thread: Reloading conf" );
          Decharger_tous_UPS();
          Charger_tous_ups();
          Cfg_ups.reload = FALSE;
        }

       if (lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE, "Run_thread: SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

       if (Cfg_ups.admin_start)
        { module = Chercher_module_ups_by_id ( Cfg_ups.admin_start );
          if (module) module->started = TRUE;
          Cfg_ups.admin_start = 0;
        }

       if (Cfg_ups.admin_stop)
        { module = Chercher_module_ups_by_id ( Cfg_ups.admin_stop );
          if (module) { module->started = FALSE;
                        Deconnecter_module  ( module );
                        module->date_retente = 0;                            /* RAZ de la date de retente */
                      }
          Cfg_ups.admin_stop = 0;
        }

       if (Cfg_ups.Modules_UPS == NULL)                         /* Si pas de module référencés, on attend */
        { sleep(2); continue; }

       pthread_mutex_lock ( &Cfg_ups.lib->synchro );               /* Car utilisation de la liste chainée */
       liste = Cfg_ups.Modules_UPS;
       while (liste && (lib->Thread_run == TRUE))
        { module = (struct MODULE_UPS *)liste->data;
          if ( module->ups.enable != TRUE ||        /* si le module n'est pas enable, on ne le traite pas */
               Partage->top < module->date_retente )           /* Si attente retente, on change de module */
           { liste = liste->next;                      /* On prépare le prochain accès au prochain module */
             continue;
           }
/*********************************** Début de l'interrogation du module ***********************************/
          if ( ! module->started )                                           /* Communication OK ou non ? */
           { if ( ! Connecter_ups( module ) )                             /* Demande de connexion a l'ups */
              { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                         "Run_thread: Module %03d DOWN", module->ups.id );
                Deconnecter_module ( module );                     /* Sur erreur, on deconnecte le module */
                module->date_retente = Partage->top + UPS_RETRY;
              }
           }
          else
           { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                      "Run_thread: Envoi des sorties ups ID%03d", module->ups.id );
             if ( Envoyer_sortie_ups ( module ) == FALSE )
              { Deconnecter_module ( module );                     /* Sur erreur, on deconnecte le module */
                module->date_retente = Partage->top + UPS_RETRY;             /* On retente dans longtemps */
              }
             else
              { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                         "Run_thread: Interrogation ups ID%03d", module->ups.id );
                if ( Interroger_ups ( module ) == FALSE )
                 { Deconnecter_module ( module );
                   module->date_retente = Partage->top + UPS_RETRY;          /* On retente dans longtemps */
                 }
                else module->date_retente = Partage->top + UPS_POLLING;  /* Update toutes les xx secondes */
              }
           }
          liste = liste->next;                         /* On prépare le prochain accès au prochain module */
        }
       pthread_mutex_unlock ( &Cfg_ups.lib->synchro );             /* Car utilisation de la liste chainée */
     }

    Decharger_tous_UPS();
    Ups_Liberer_config ();                              /* Lecture de la configuration logiciel du thread */
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_ups.lib->TID = 0;                                 /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
