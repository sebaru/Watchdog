/**********************************************************************************************************/
/* Watchdogd/Admin/admin_sms.c        Gestion des connexions Admin SET au serveur watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 19 mai 2012 11:03:52 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_sms.c
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
 
 #include <unistd.h>                                                                  /* Pour gethostname */
 #include "watchdogd.h"
 #include "Sms.h"

/**********************************************************************************************************/
/* Admin_sms_list : L'utilisateur admin lance la commande "list" en mode sms                              */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_sms_list ( struct CONNEXION *connexion )
  { GSList *liste_sms;
    gchar chaine[256];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des contacts SMS\n" );
    Admin_write ( connexion, chaine );

    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Admin_write ( connexion, chaine );
       
    pthread_mutex_lock ( &Cfg_sms.lib->synchro );
    liste_sms = Cfg_sms.Liste_contact_SMS;
    while ( liste_sms )
     { struct SMSDB *sms;
       sms = (struct SMSDB *)liste_sms->data;

       g_snprintf( chaine, sizeof(chaine),
                   " Contact_SMS[%03d] -> enable=%d, phone=%s, name=%s\n"
                   "                      phone_send_command=%d, phone_receive_sms=%d\n\n",
                   sms->id, sms->enable, sms->phone, sms->name,
                   sms->phone_send_command, sms->phone_receive_sms
                 );
       Admin_write ( connexion, chaine );
       liste_sms = liste_sms->next;
     }
    pthread_mutex_unlock ( &Cfg_sms.lib->synchro );
  }

/**********************************************************************************************************/
/* Admin_sms: Gere une commande 'admin sms' depuis une connexion admin                                    */
/* Entrée: le connexion et la ligne de commande                                                           */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command ( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    if (Cfg_sms.lib->Thread_run == FALSE)
     { Admin_write ( connexion, "\n" );
       Admin_write ( connexion, "  -- WARNING ----- Thread is not started -----\n");
       Admin_write ( connexion, "  -- WARNING -- Running config is not loaded !\n" );
       Admin_write ( connexion, "\n" );
     }              

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'SMS'\n" );
       Admin_write ( connexion, "  dbcfg ...             - Get/Set Database Parameters\n" );
       Admin_write ( connexion, "  reload                - Reload config from Database\n" );
       Admin_write ( connexion, "  sms smsbox message    - Send 'message' via smsbox\n" );
       Admin_write ( connexion, "  sms gsm    message    - Send 'message' via gsm\n" );

       Admin_write ( connexion, "  add enable,send_command,receive_sms,phone,name\n");
       Admin_write ( connexion, "                        - Add a SMS contact\n" );
       Admin_write ( connexion, "  set id,enable,send_command,receive_sms,phone,name\n");
       Admin_write ( connexion, "                        - Change SMS id\n" );
       Admin_write ( connexion, "  del id                - Delete SMS id\n" );
       Admin_write ( connexion, "  list                  - Liste les contacts SMS\n" );
       Admin_write ( connexion, "  help                  - This help\n" );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_sms_list ( connexion );
     }
    else if ( ! strcmp ( commande, "add" ) )
     { struct SMSDB sms;
       gint retour;
       sscanf ( ligne, "%s %d,%d,%d,%[^,],%[^\n]", commande,    /* Découpage de la ligne de commande */
                &sms.enable, &sms.phone_send_command, &sms.phone_receive_sms,
                sms.phone, sms.name
              );
       retour = Ajouter_smsDB ( &sms );
       if (retour == -1)
        { Admin_write ( connexion, "Error, SMS not added\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " SMS %s added. New ID=%d\n", sms.phone, retour );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "set" ) )
     { struct SMSDB sms;
       gint retour;
       sscanf ( ligne, "%s %d,%d,%d,%d,%[^,],%[^\n]", commande, /* Découpage de la ligne de commande */
                &sms.id, &sms.enable, &sms.phone_send_command, &sms.phone_receive_sms,
                sms.phone, sms.name
              );
       retour = Modifier_smsDB ( &sms );
       if (retour == FALSE)
        { Admin_write ( connexion, "Error, SMS not changed\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " SMS %s changed\n", sms.phone );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "del" ) )
     { struct SMSDB sms;
       gboolean retour;
       sscanf ( ligne, "%s %d", commande, &sms.id );                 /* Découpage de la ligne de commande */
       retour = Retirer_smsDB ( &sms );
       if (retour == FALSE)
        { Admin_write ( connexion, "Error, SMS not erased\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " SMS %d (%s)erased\n", sms.id, sms.phone );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { g_snprintf( chaine, sizeof(chaine), " Reloading Contacts List from Database\n" );
       Admin_write ( connexion, chaine );
       Cfg_sms.reload = TRUE;
     }
    else if ( ! strcmp ( commande, "gsm" ) )
     { gchar message[80];
       sscanf ( ligne, "%s %s", commande, message );                 /* Découpage de la ligne de commande */
       Envoyer_sms_gsm_text ( ligne + 4 ); /* On envoie le reste de la liste, pas seulement le mot suivant. */
       g_snprintf( chaine, sizeof(chaine), " Sms sent\n" );
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "smsbox" ) )
     { gchar message[80];
       sscanf ( ligne, "%s %s", commande, message );                 /* Découpage de la ligne de commande */
       Envoyer_sms_smsbox_text ( ligne + 7 ); /* On envoie le reste de la liste, pas seulement le mot suivant. */
       g_snprintf( chaine, sizeof(chaine), " Sms sent\n" );
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { if (Admin_dbcfg_thread ( connexion, NOM_THREAD, ligne+6 ) == TRUE)   /* Si changement de parametre */
        { gboolean retour;
          retour = Sms_Lire_config();
          g_snprintf( chaine, sizeof(chaine), " Reloading Sms Thread Parameters from Database -> %s\n",
                      (retour ? "Success" : "Failed") );
          Admin_write ( connexion, chaine );
        }
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
