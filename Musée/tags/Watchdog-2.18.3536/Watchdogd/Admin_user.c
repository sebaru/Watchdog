/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_running.c        Gestion des connexions Admin RUNNING au serveur watchdog                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         mer. 17 nov. 2010 20:00:45 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include <time.h>

 #include "watchdogd.h"
/******************************************************************************************************************************/
/* Admin_print_user : Affiche le user en parametre sur la console d'admin CLI                                                 */
/* Entrée: Le buffer de reponse et l'utilisateur a printer                                                                    */
/* Sortie: Le buffer de reponse complété                                                                                      */
/******************************************************************************************************************************/
 static gchar *Admin_print_user ( gchar *response, struct CMD_TYPE_UTILISATEUR *util )
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
              " |---------------------------------------------------------------------------------------\n"
              " [%03d]%12s -> enable=%d, expire=%d, date_expire = %s, mustchangepwd = %s, cansetpwd = %s\n"
              " |               -> date_creation = %s, date_modif = %s\n"
              " |               -> sms_enable  = %d, sms_allow_cde  = %d, sms_phone = %s\n"
              " |               -> imsg_enable = %d, imsg_allow_cde = %d, imsg_jabber_id = %s\n"
              " |               -> imsg_available = %d\n"
              " |               -> ssrv_bit_presence = B%04d (=%d)\n"
              " |               -> salt=%s\n"
              " |               -> hash=%s\n"
              " |----------------> %s\n",
                util->id, util->nom, util->enable, util->expire, date_expire, (util->mustchangepwd ? "TRUE" : "FALSE"),
                (util->cansetpwd ? "TRUE" : "FALSE"), date_creation, date_modif,
                util->sms_enable, util->sms_allow_cde, util->sms_phone,
                util->imsg_enable, util->imsg_allow_cde, util->imsg_jabberid,
                util->imsg_available,
                util->ssrv_bit_presence, B(util->ssrv_bit_presence),
                util->salt, util->hash, util->commentaire
              );
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_user_list : liste les utilisateurs de Watchdog et leurs privilèges                                                   */
/* Entrée: La connexion connexion ADMIN                                                                                       */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                                           */
/******************************************************************************************************************************/
 static gchar *Admin_user_list ( gchar *response )
  { struct CMD_TYPE_UTILISATEUR *util;
    gchar chaine[80];
    struct DB *db;

    if ( ! Recuperer_utilisateurDB( &db ) )                                               /* Chargement de la base de données */
     { g_snprintf( chaine, sizeof(chaine), " | - Error : DB Connexion failed" );
       response = Admin_write ( response, chaine );
       return(response);
     }                                                                                               /* Si pas de histos (??) */

    g_snprintf( chaine, sizeof(chaine), " | -------- Users List ------------" );
    response = Admin_write ( response, chaine );
    for ( ; ; )
     { util = Recuperer_utilisateurDB_suite( &db );
       if (!util) return(response);                                                                /* Fin, ou erreur, on sort */
       response = Admin_print_user ( response, util );
       g_free(util);
     }
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_running: Appellée lorsque l'admin envoie une commande en mode run dans la ligne de commande                          */
/* Entrée: La connexion connexione et la ligne de commande, et le buffer de sortie                                            */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_user ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[256];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, " |  -- Watchdog ADMIN -- Help du mode 'running'" );
       response = Admin_write ( response, " | - add $name $comment         - Add user $name with $commment" );
       response = Admin_write ( response, " | - del $name                  - Erase user $name" );
       response = Admin_write ( response, " | - enable $name               - Allow user $name to connect" );
       response = Admin_write ( response, " | - disable $name              - Deny user $id to connect $name" );
       response = Admin_write ( response, " | - cansetpwd $name $bool      - Set CanSetPwd flag to true or false" );
       response = Admin_write ( response, " | - mustchangepwd $name $bool  - Set CanSetPwd flag to true or false" );
       response = Admin_write ( response, " | - list                       - Liste les users Watchdog" );
       response = Admin_write ( response, " | - passwd $name $pwd          - Set password $pwd to user $name" );
       response = Admin_write ( response, " | - show $name                 - Show user $name" );
       response = Admin_write ( response, " | - help                       - This help" );
     } else
    if ( ! strcmp ( commande, "list" ) )
     { return(Admin_user_list ( response ) );
     } else
    if ( ! strcmp ( commande, "show" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80];

       sscanf ( ligne, "%s %s", commande, name );                    /* Découpage de la ligne de commande */

       util = Rechercher_utilisateurDB_by_name ( name );
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " | - User %s not found in Database", name );
          response = Admin_write ( response, chaine );
        }
       else
        { response = Admin_print_user ( response, util );
          g_free(util);
        }
     } else
    if ( ! strcmp ( commande, "del" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80];
       sscanf ( ligne, "%s %s", commande, name );                    /* Découpage de la ligne de commande */
       util = Rechercher_utilisateurDB_by_name( name );          /* suppression par nom plutot que par id */
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " | - User %s not found in Database", name );
          response = Admin_write ( response, chaine );
        }
       else
        { if (Retirer_utilisateurDB ( util ) == FALSE)
           { g_snprintf( chaine, sizeof(chaine), " | - User %s couldn't be removed", name );
             response = Admin_write ( response, chaine );
           }
          else
           { g_snprintf( chaine, sizeof(chaine), " | - User %s (id=%d) removed", util->nom, util->id );
             response = Admin_write ( response, chaine );
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
        { g_snprintf( chaine, sizeof(chaine), " | - User %s couldn't be enabled", util.nom );
          response = Admin_write ( response, chaine );
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " | - User %s enabled", util.nom );
          response = Admin_write ( response, chaine );
        }
     } else
    if ( ! strcmp ( commande, "disable" ) )
     { struct CMD_TYPE_UTILISATEUR util;
       sscanf ( ligne, "%s %s", commande, util.nom );                                    /* Découpage de la ligne de commande */
       util.id = -1;                                                                 /* suppression par nom plutot que par id */
       util.enable = FALSE;
       if (Set_enable_utilisateurDB ( &util ) == FALSE)
        { g_snprintf( chaine, sizeof(chaine), " | - User %s couldn't be disabled", util.nom );
          response = Admin_write ( response, chaine );
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " | - User %s disabled", util.nom );
          response = Admin_write ( response, chaine );
        }
     } else
    if ( ! strcmp ( commande, "add" ) )
     { struct CMD_TYPE_UTILISATEUR util;
       memset ( &util, 0, sizeof( struct CMD_TYPE_UTILISATEUR ) );
       sscanf ( ligne, "%s %s %[^]", commande, util.nom, util.commentaire );             /* Découpage de la ligne de commande */
       if (Ajouter_utilisateurDB ( &util ) == -1)
        { g_snprintf( chaine, sizeof(chaine), " | - User %s couldn't be added in Database", util.nom );
          response = Admin_write ( response, chaine );
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " | - User %s added. UID = %d", util.nom, util.id );
          response = Admin_write ( response, chaine );
        }
     } else
    if ( ! strcmp ( commande, "cansetpwd" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80], cansetpwd[80];

       sscanf ( ligne, "%s %s %s", commande, name, cansetpwd );                          /* Découpage de la ligne de commande */

       util = Rechercher_utilisateurDB_by_name ( name );
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " | - User %s not found in Database", name );
          response = Admin_write ( response, chaine );
        }
       else
        { if (!strcmp( cansetpwd, "true" ) ) util->cansetpwd = 1;
                                        else util->cansetpwd = 0;
          if( Modifier_utilisateurDB_set_cansetpwd( util ) )
           { g_snprintf( chaine, sizeof(chaine), " | - Flag CanSetPwd set to %d for user %s",
                         util->cansetpwd, util->nom ); }
          else
           { g_snprintf( chaine, sizeof(chaine), " | - Error while setting CanSetPwd" ); }
          g_free(util);
          response = Admin_write ( response, chaine );
        }
     } else
    if ( ! strcmp ( commande, "mustchangepwd" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80], mustchangepwd[80];

       sscanf ( ligne, "%s %s %s", commande, name, mustchangepwd );                      /* Découpage de la ligne de commande */

       util = Rechercher_utilisateurDB_by_name ( name );
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " | - User %s not found in Database", name );
          response = Admin_write ( response, chaine );
        }
       else
        { if (!strcmp( mustchangepwd, "true" ) ) util->mustchangepwd = 1;
                                            else util->mustchangepwd = 0;
          if( Modifier_utilisateurDB_set_mustchangepwd( util ) )
           { g_snprintf( chaine, sizeof(chaine), " | - Flag MustChangePwd set to %d for user %s",
                         util->mustchangepwd, util->nom ); }
          else
           { g_snprintf( chaine, sizeof(chaine), " | - Error while setting MustChangePwd" ); }
          g_free(util);
          response = Admin_write ( response, chaine );
        }
     } else
    if ( ! strcmp ( commande, "passwd" ) )
     { struct CMD_TYPE_UTILISATEUR *util;
       gchar name[80], pwd[80];

       sscanf ( ligne, "%s %s %s", commande, name, pwd );                                /* Découpage de la ligne de commande */

       util = Rechercher_utilisateurDB_by_name ( name );
       if (!util)
        { g_snprintf( chaine, sizeof(chaine), " | - User %s not found in Database", name );
          response = Admin_write ( response, chaine );
        }
       else
        { if( Modifier_utilisateurDB_set_password( util, pwd ) )
           { g_snprintf( chaine, sizeof(chaine), " | - Password set for user %s", util->nom ); }
          else
           { g_snprintf( chaine, sizeof(chaine), " | - Error while setting password" ); }
          g_free(util);
          response = Admin_write ( response, chaine );
        }
     } else
     { g_snprintf( chaine, sizeof(chaine), " | - Unknown command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
