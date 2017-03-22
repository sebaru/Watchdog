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
 #include <time.h>

 #include "watchdogd.h"
/**********************************************************************************************************/
/* Admin_print_user : Affiche le user en parametre sur la console d'admin CLI                             */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_print_user ( struct CONNEXION *connexion, struct CMD_TYPE_UTILISATEUR *util )
  { gchar date_expire[20], date_creation[20], date_modif[20];
    gchar chaine[1024];
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

    time = util->date_creation;
    temps = localtime( (time_t *)&time );
    if (temps) { strftime  ( date_creation, sizeof(date_creation), "%F %T", temps ); }
    else       { g_snprintf( date_creation, sizeof(date_creation), "Erreur" );    }

    g_snprintf( chaine, sizeof(chaine),
              " [%03d]%12s -> enable=%d, expire=%d, date_expire = %s, mustchangepwd = %s, cansetpwd = %s\n"
              "   |               -> date_creation = %s, date_modif = %s\n"
              "   |               -> sms_enable  = %d, sms_allow_cde  = %d, sms_phone = %s\n"
              "   |               -> imsg_enable = %d, imsg_allow_cde = %d, imsg_jabber_id = %s\n"
              "   |               -> imsg_available = %d\n"
              "   |               -> ssrv_bit_presence = B%04d (=%d)\n"
              "   |               -> salt=%s\n"
              "   |               -> hash=%s\n"
              "   |----------------> %s\n",
                util->id, util->nom, util->enable, util->expire, date_expire, (util->mustchangepwd ? "TRUE" : "FALSE"),
                (util->cansetpwd ? "TRUE" : "FALSE"), date_creation, date_modif,
                util->sms_enable, util->sms_allow_cde, util->sms_phone,
                util->imsg_enable, util->imsg_allow_cde, util->imsg_jabberid,
                util->imsg_available,
                util->ssrv_bit_presence, B(util->ssrv_bit_presence),
                util->salt, util->hash, util->commentaire
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
       return;
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
  { gchar commande[128], chaine[256];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'running'\n" );
       Admin_write ( connexion, "  add $name $comment         - Add user $name with $commment\n" );
       Admin_write ( connexion, "  del $name                  - Erase user $name\n" );
       Admin_write ( connexion, "  enable $name               - Allow user $name to connect\n" );
       Admin_write ( connexion, "  disable $name              - Deny user $id to connect $name\n" );
       Admin_write ( connexion, "  cansetpwd $name $bool      - Set CanSetPwd flag to true or false\n" );
       Admin_write ( connexion, "  musttchangepwd $name $bool - Set CanSetPwd flag to true or false\n" );
       Admin_write ( connexion, "  list                       - Liste les users Watchdog\n" );
       Admin_write ( connexion, "  passwd $name $pwd          - Set password $pwd to user $name\n" );
       Admin_write ( connexion, "  show $name                 - Show user $name\n" );
       Admin_write ( connexion, "  help                       - This help\n" );
     } else
    if ( ! strcmp ( commande, "list" ) )
     { Admin_user_list ( connexion );
     } else
    if ( ! strcmp ( commande, "show" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80];

       sscanf ( ligne, "%s %s", commande, name );                    /* Découpage de la ligne de commande */

       util = Rechercher_utilisateurDB_by_name ( name );
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " User %s not found in Database\n", name );
          Admin_write ( connexion, chaine );
        }
       else
        { Admin_print_user ( connexion, util );
          g_free(util);
        }
     } else
    if ( ! strcmp ( commande, "del" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80];
       sscanf ( ligne, "%s %s", commande, name );                    /* Découpage de la ligne de commande */
       util = Rechercher_utilisateurDB_by_name( name );          /* suppression par nom plutot que par id */
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " User %s not found in Database\n", name );
          Admin_write ( connexion, chaine );
        }
       else
        { if (Retirer_utilisateurDB ( util ) == FALSE)
           { g_snprintf( chaine, sizeof(chaine), " User %s couldn't be removed\n", name );
             Admin_write ( connexion, chaine );
           }
          else
           { g_snprintf( chaine, sizeof(chaine), " User %s (id=%d) removed\n", util->nom, util->id );
             Admin_write ( connexion, chaine );
           }
          g_free(util);
        }
     } else
    if ( ! strcmp ( commande, "enable" ) )
     { struct CMD_TYPE_UTILISATEUR util;
       sscanf ( ligne, "%s %s", commande, util.nom );                /* Découpage de la ligne de commande */
       util.id = -1;                                            /* suppression par nom plutot que par id */
       util.enable = TRUE;
       if (Set_enable_utilisateurDB ( &util ) == FALSE)
        { g_snprintf( chaine, sizeof(chaine), " User %s couldn't be enabled\n", util.nom );
          Admin_write ( connexion, chaine );
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " User %s enabled\n", util.nom );
          Admin_write ( connexion, chaine );
        }
     } else
    if ( ! strcmp ( commande, "disable" ) )
     { struct CMD_TYPE_UTILISATEUR util;
       sscanf ( ligne, "%s %s", commande, util.nom );                /* Découpage de la ligne de commande */
       util.id = -1;                                            /* suppression par nom plutot que par id */
       util.enable = FALSE;
       if (Set_enable_utilisateurDB ( &util ) == FALSE)
        { g_snprintf( chaine, sizeof(chaine), " User %s couldn't be disabled\n", util.nom );
          Admin_write ( connexion, chaine );
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " User %s disabled\n", util.nom );
          Admin_write ( connexion, chaine );
        }
     } else
    if ( ! strcmp ( commande, "add" ) )
     { struct CMD_TYPE_UTILISATEUR util;
       memset ( &util, 0, sizeof( struct CMD_TYPE_UTILISATEUR ) );
       sscanf ( ligne, "%s %s %[^\n]", commande, util.nom, util.commentaire );/* Découpage de la ligne de commande */
       if (Ajouter_utilisateurDB ( &util ) == -1)
        { g_snprintf( chaine, sizeof(chaine), " User %s couldn't be added in Database\n", util.nom );
          Admin_write ( connexion, chaine );
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " User %s added. UID = %d\n", util.nom, util.id );
          Admin_write ( connexion, chaine );
        }

     } else
    if ( ! strcmp ( commande, "cansetpwd" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80], cansetpwd[80];

       sscanf ( ligne, "%s %s %s", commande, name, cansetpwd );      /* Découpage de la ligne de commande */

       util = Rechercher_utilisateurDB_by_name ( name );
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " User %s not found in Database\n", name );
          Admin_write ( connexion, chaine );
        }
       else
        { if (!strcmp( cansetpwd, "true" ) ) util->cansetpwd = 1;
                                        else util->cansetpwd = 0;
          if( Modifier_utilisateurDB_set_cansetpwd( util ) )
           { g_snprintf( chaine, sizeof(chaine), " Flag CanSetPwd set to %d for user %s\n",
                         util->cansetpwd, util->nom ); }
          else
           { g_snprintf( chaine, sizeof(chaine), " Error while setting CanSetPwd\n" ); }
          g_free(util);
          Admin_write ( connexion, chaine );
        }
     } else
    if ( ! strcmp ( commande, "mustchangepwd" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80], mustchangepwd[80];

       sscanf ( ligne, "%s %s %s", commande, name, mustchangepwd );  /* Découpage de la ligne de commande */

       util = Rechercher_utilisateurDB_by_name ( name );
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " User %s not found in Database\n", name );
          Admin_write ( connexion, chaine );
        }
       else
        { if (!strcmp( mustchangepwd, "true" ) ) util->mustchangepwd = 1;
                                            else util->mustchangepwd = 0;
          if( Modifier_utilisateurDB_set_mustchangepwd( util ) )
           { g_snprintf( chaine, sizeof(chaine), " Flag MustChangePwd set to %d for user %s\n",
                         util->mustchangepwd, util->nom ); }
          else
           { g_snprintf( chaine, sizeof(chaine), " Error while setting MustChangePwd\n" ); }
          g_free(util);
          Admin_write ( connexion, chaine );
        }
     } else
    if ( ! strcmp ( commande, "passwd" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80], pwd[80];

       sscanf ( ligne, "%s %s %s", commande, name, pwd );            /* Découpage de la ligne de commande */

       util = Rechercher_utilisateurDB_by_name ( name );
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " User %s not found in Database\n", name );
          Admin_write ( connexion, chaine );
        }
       else
        { if( Modifier_utilisateurDB_set_password( util, pwd ) )
           { g_snprintf( chaine, sizeof(chaine), " Password set for user %s\n", util->nom ); }
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
