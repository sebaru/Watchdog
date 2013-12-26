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
 #include <openssl/rand.h> /* Pour l'utilisation du SALT */
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Admin_running: Appellée lorsque l'admin envoie une commande en mode run dans la ligne de commande      */
/* Entrée: La connexion connexione et la ligne de commande, et le buffer de sortie                           */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_user ( struct CONNEXION *connexion, gchar *ligne )
  { struct LIBRAIRIE *lib;
    GSList *liste;
    gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'running'\n" );
       Admin_write ( connexion, "  list                    - Liste les users Watchdog\n" );
       Admin_write ( connexion, "  setpassword $name $pwd - Set password $pwd to name\n" );
       Admin_write ( connexion, "  help                    - This help\n" );
     } else
    if ( ! strcmp ( commande, "setpassword" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80], pwd[80], salt[21];
       struct DB *db;

       sscanf ( ligne, "%s %s %s %s", command, name, pwd );                /* Découpage de la ligne de commande */

       util = Rechercher_utilisateurDB_by_name ( name );
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " User %s not found in Database\n", name );
          Admin_write ( connexion, chaine );
        }
       else
        { EVP_MD_CTX *mdctx;
          const EVP_MD *md;
          unsigned char md_value[EVP_MAX_MD_SIZE];
          int md_len, i;
          RAND_pseudo_bytes( &util->salt, sizeof(util->salt)-1 );
          salt[sizeof(util->salt)-1] = 0; /* Last value */
          /*OpenSSL_add_all_digests();*/

          mdctx = EVP_MD_CTX_create();
          EVP_DigestInit_ex (mdctx, EVP_sha512(), NULL);
          EVP_DigestUpdate  (mdctx, util->salt, strlen(util->salt));
          EVP_DigestUpdate  (mdctx, pwd,  strlen(pwd));
          EVP_DigestFinal_ex(mdctx, util->hash, &md_len);
          EVP_MD_CTX_destroy(mdctx);
          /*util.cansetpass = TRUE;
          util.setpassnow = TRUE;
          util.actif = TRUE;
          util.expire = FALSE;
          util.changepass = FALSE;*/
          if( Modifier_utilisateurDB( Config.crypto_key, &util ) )
           { g_snprintf( chaine, sizeof(chaine), " Password set to %s\n", util->hash ); }
          else
           { g_snprintf( chaine, sizeof(chaine), " Error while setting password\n" ); }
          Admin_write ( connexion, chaine );
        }
     } else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
