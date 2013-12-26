/**********************************************************************************************************/
/* Watchdogd/Admin/admin_running.c        Gestion des connexions Admin RUNNING au serveur watchdog        */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 17 nov. 2010 20:00:45 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_running.c
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
 #include <unistd.h>                                                                  /* Pour gethostname */
 #include <openssl/rand.h>
 #include <time.h>

 #include "watchdogd.h"
/**********************************************************************************************************/
/* Admin_print_user : Affiche le user en parametre sur la console d'admin CLI                             */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_print_user ( struct CONNEXION *connexion, struct CMD_TYPE_UTILISATEUR *util )
  { gchar date_expire[20], date_creation[20], date_modif[20];
    gchar chaine[256];
    struct tm *temps;
    time_t time;

    time = util->date_expire;
    temps = localtime( (time_t *)&time );
    if (temps) { strftime  ( date_expire, sizeof(date_expire), "%F %T", temps ); }
    else       { g_snprintf( date_expire, sizeof(date_expire), "Erreur" );    }

    time = util->date_modif;
    temps = localtime( (time_t *)&time );
    if (temps) { strftime  ( date_modif, sizeof(date_modif), "%F %T", temps ); }
    else       { g_snprintf( date_modif, sizeof(date_modif), "Erreur" );    }

    time = util->date_expire;
    temps = localtime( (time_t *)&time );
    if (temps) { strftime  ( date_creation, sizeof(date_creation), "%F %T", temps ); }
    else       { g_snprintf( date_creation, sizeof(date_creation), "Erreur" );    }

    g_snprintf( chaine, sizeof(chaine),
              " [%03d]%20s -> enable=%d, expire=%d, date_expire=%s, changepass=%d, cansetpass=%d\n"
              "   |                       -> date_creation=%s, date_modif=%s\n"
              "   |                       -> salt=%20s, hash=%20s\n"
              "   |---------> %s\n",
                util->id, util->nom, util->enable, util->expire, date_expire, util->changepass,
                util->cansetpass, date_creation, date_modif, util->salt, util->hash, util->commentaire
              );
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_user_list : liste les utilisateurs de Watchdog et leurs privilèges                               */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_user_list ( struct CONNEXION *connexion )
  { struct CMD_TYPE_UTILISATEUR *util;
    gchar chaine[80];
    struct DB *db;

    if ( ! Recuperer_utilisateurDB( &db ) )                           /* Chargement de la base de données */
     { g_snprintf( chaine, sizeof(chaine), " Error : DB Connexion failed\n" );
       Admin_write ( connexion, chaine );
     }                                                                           /* Si pas de histos (??) */

    g_snprintf( chaine, sizeof(chaine), " -------- Users List ------------\n" );
    Admin_write ( connexion, chaine );
    for ( ; ; )
     { util = Recuperer_utilisateurDB_suite( &db );
       if (!util) return;                                                      /* Fin, ou erreur, on sort */
       Admin_print_user ( connexion, util );
       g_free(util);
     }
  }
/**********************************************************************************************************/
/* Admin_running: Appellée lorsque l'admin envoie une commande en mode run dans la ligne de commande      */
/* Entrée: La connexion connexione et la ligne de commande, et le buffer de sortie                        */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_user ( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'running'\n" );
       Admin_write ( connexion, "  list                    - Liste les users Watchdog\n" );
       Admin_write ( connexion, "  setpassword $name $pwd - Set password $pwd to name\n" );
       Admin_write ( connexion, "  help                    - This help\n" );
     } else
    if ( ! strcmp ( commande, "list" ) )
     { Admin_user_list ( connexion );
     } else
    if ( ! strcmp ( commande, "setpassword" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80], pwd[80];

       sscanf ( ligne, "%s %s %s", commande, name, pwd );            /* Découpage de la ligne de commande */

       util = Rechercher_utilisateurDB_by_name ( name );
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " User %s not found in Database\n", name );
          Admin_write ( connexion, chaine );
        }
       else
        { EVP_MD_CTX *mdctx;
          guint md_len;
          memset ( util->salt, 0, sizeof(util->salt) );
          memset ( util->hash, 0, sizeof(util->hash) );
          RAND_pseudo_bytes( (guchar *)util->salt, sizeof(util->salt)-1 ); /* Récupération d'un nouveau SALT */

          mdctx = EVP_MD_CTX_create();
          EVP_DigestInit_ex (mdctx, EVP_sha512(), NULL);
          EVP_DigestUpdate  (mdctx, util->salt, sizeof(util->salt));
          EVP_DigestUpdate  (mdctx, pwd,  strlen(pwd));
          EVP_DigestFinal_ex(mdctx, (guchar *)util->hash, &md_len);
          EVP_MD_CTX_destroy(mdctx);

          if( Modifier_utilisateurDB_set_password( util ) )
           { g_snprintf( chaine, sizeof(chaine), " Password set to %s\n", util->hash ); }
          else
           { g_snprintf( chaine, sizeof(chaine), " Error while setting password\n" ); }
          g_free(util);
          Admin_write ( connexion, chaine );
        }
     } else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
