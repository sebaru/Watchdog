/**********************************************************************************************************/
/* Watchdogd/Imsg/admin_imsg.c        Gestion des connexions Admin IMSG au serveur watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                  sam. 28 juil. 2012 16:35:09 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_imsg.c
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
 
 #include "watchdogd.h"
 #include "Imsg.h"

/**********************************************************************************************************/
/* Admin_imsg_reload: Demande le rechargement des conf IMSG                                               */
/* Entrée: le connexion                                                                                   */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_imsg_reload ( struct CONNEXION *connexion )
  { if (Cfg_imsg.lib->Thread_run == FALSE)
     { Admin_write ( connexion, " Thread IMSG is not running\n" );
       return;
     }
    
    Cfg_imsg.reload = TRUE;
    Admin_write ( connexion, " IMSG Reloading in progress\n" );
    while (Cfg_imsg.reload) sched_yield();
    Admin_write ( connexion, " IMSG Reloading done\n" );
  }
/**********************************************************************************************************/
/* Admin_print_contact : Affiche le parametre sur la console d'admin CLI                                     */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_print_contact ( struct CONNEXION *connexion, struct IMSG_CONTACT *contact )
  { gchar chaine[256];

    g_snprintf( chaine, sizeof(chaine),
              " Contact_IMSG[%03d] -> enable=%d, receive_imsg=%d, send_command=%d, bit_presence=B%03d, Name=%s, jabberid=%s is %s\n",
                contact->imsg.id, contact->imsg.enable, contact->imsg.receive_imsg, contact->imsg.send_command,
                contact->imsg.bit_presence, contact->imsg.nom, contact->imsg.jabber_id,
                (contact->available ? "available" : "UNavailable") 
                 );
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_imsg_list : L'utilisateur admin lance la commande "list" en mode imsg                            */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_imsg_list ( struct CONNEXION *connexion )
  { gchar chaine[256];
    GSList *liste;

    g_snprintf( chaine, sizeof(chaine), " -- Liste des Contacts IMSG\n" );
    Admin_write ( connexion, chaine );

    pthread_mutex_lock ( &Cfg_imsg.lib->synchro );
    liste = Cfg_imsg.Contacts;
    while(liste)
     { struct IMSG_CONTACT *contact;
       contact  =(struct IMSG_CONTACT *)liste->data;
       Admin_print_contact ( connexion, contact );
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Cfg_imsg.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_Imsg_Modifier_enable : Modifie le flag "enable" du contact IMSG passé en parametre               */
/* Entrée: La connexion connexion ADMIN, le IMSG Contact et le nouveau status                             */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_Imsg_Modifier_enable ( struct CONNEXION *connexion, struct IMSG_CONTACT *orig_contact, 
                                          gboolean new_status )
  { struct IMSG_CONTACT *contact;
    GSList *liste_contact;
    gchar chaine[256];
    
    pthread_mutex_lock ( &Cfg_imsg.lib->synchro );
    liste_contact = Cfg_imsg.Contacts;
    while ( liste_contact )
     { contact = (struct IMSG_CONTACT *)liste_contact->data;
       if ( orig_contact->imsg.id == contact->imsg.id ) break;
       liste_contact = liste_contact->next;
     }

    if (liste_contact)
     { contact->imsg.enable = new_status;
       Admin_print_contact( connexion, contact );
     }
    else
     { g_snprintf( chaine, sizeof(chaine),
                   " Contact_IMSG[%03d] -> Not found\n", orig_contact->imsg.id );
       Admin_write ( connexion, chaine );
     }
    pthread_mutex_unlock ( &Cfg_imsg.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                 */
/* Entrée: Le connexion d'admin, la ligne a traiter                                                       */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "send" ) )
     { gchar to[256];
       sscanf ( ligne, "%s %s", commande, to );                      /* Découpage de la ligne de commande */
       Imsg_Envoi_message_to ( to, ligne + strlen (to) + 6 );
       g_snprintf( chaine, sizeof(chaine), " Message '%s' send to %s\n", ligne + strlen (to) + 6, to );
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_imsg_list ( connexion ); }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_imsg_reload ( connexion ); }
    else if ( ! strcmp ( commande, "status" ) )
     { gchar chaine[128];
       if (Cfg_imsg.connection)
        { LmConnectionState state;
          state =  lm_connection_get_state ( Cfg_imsg.connection );
          switch (state)
           { case LM_CONNECTION_STATE_CLOSED:
                  g_snprintf( chaine, sizeof(chaine), " Connexion closed.\n"); break;
             case LM_CONNECTION_STATE_OPENING:
                  g_snprintf( chaine, sizeof(chaine), " Connexion is opening.\n"); break;
             case LM_CONNECTION_STATE_OPEN:
                  g_snprintf( chaine, sizeof(chaine), " Connexion opened.\n"); break;
             case LM_CONNECTION_STATE_AUTHENTICATING:
                  g_snprintf( chaine, sizeof(chaine), " Connexion is authenticating.\n"); break;
             case LM_CONNECTION_STATE_AUTHENTICATED:
                  g_snprintf( chaine, sizeof(chaine), " Connexion authenticated (OK).\n"); break;
             default:
                  g_snprintf( chaine, sizeof(chaine), " Connexion Status Unknown.\n"); break;
           }
          Admin_write ( connexion, chaine );
          if (Cfg_imsg.date_retente)
           { g_snprintf( chaine, sizeof(chaine), " Re-trying in %03ds.\n",
                         (Cfg_imsg.date_retente - Partage->top)/10);
             Admin_write ( connexion, chaine );
           }
        }
       else 
        { g_snprintf( chaine, sizeof(chaine), " No connexion ... strange ! \n" );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "presence" ) )
     { g_snprintf( Cfg_imsg.new_status, sizeof(Cfg_imsg.new_status), "%s", commande + 9 );
       Cfg_imsg.set_status = TRUE;
       g_snprintf( chaine, sizeof(chaine), " Presence Status changed to %s! \n", Cfg_imsg.new_status );
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "add" ) )
     { struct IMSGDB imsg;
       gint retour;
       sscanf ( ligne, "%s %d,%d,%d,%d,%[^,],%[^\n]", commande,    /* Découpage de la ligne de commande */
                &imsg.enable, &imsg.send_command, &imsg.receive_imsg, &imsg.bit_presence,
                imsg.jabber_id, imsg.nom
              );
       retour = Ajouter_imsgDB ( &imsg );
       if (retour == -1)
        { Admin_write ( connexion, "Error, IMSG not added\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " IMSG %s added. New ID=%d\n", imsg.jabber_id, retour );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "set" ) )
     { struct IMSGDB imsg;
       gint retour;
       sscanf ( ligne, "%s %d,%d,%d,%d,%d,%[^,],%[^\n]", commande, /* Découpage de la ligne de commande */
                &imsg.id, &imsg.enable, &imsg.send_command, &imsg.receive_imsg, &imsg.bit_presence,
                imsg.jabber_id, imsg.nom
              );
       retour = Modifier_imsgDB ( &imsg );
       if (retour == FALSE)
        { Admin_write ( connexion, "Error, IMSG not changed\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " IMSG %s changed\n", imsg.jabber_id );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "del" ) )
     { struct IMSGDB imsg;
       gboolean retour;
       sscanf ( ligne, "%s %d", commande, &imsg.id );                 /* Découpage de la ligne de commande */
       retour = Retirer_imsgDB ( &imsg );
       if (retour == FALSE)
        { Admin_write ( connexion, "Error, IMSG not erased\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " IMSG %d erased\n", imsg.id );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "enable" ) )
     { struct IMSG_CONTACT contact;
       sscanf ( ligne, "%s %d", commande, &contact.imsg.id );        /* Découpage de la ligne de commande */
       Admin_Imsg_Modifier_enable( connexion, &contact, TRUE );
     }
    else if ( ! strcmp ( commande, "disable" ) )
     { struct IMSG_CONTACT contact;
       sscanf ( ligne, "%s %d", commande, &contact.imsg.id );        /* Découpage de la ligne de commande */
       Admin_Imsg_Modifier_enable( connexion, &contact, FALSE );
     }
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { if (Admin_dbcfg_thread ( connexion, NOM_THREAD, ligne+6 ) == TRUE)   /* Si changement de parametre */
        { gboolean retour;
          retour = Imsg_Lire_config();
          g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s\n",
                      (retour ? "Success" : "Failed") );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'IMSG'\n" );
       Admin_write ( connexion, "  dbcfg ...                              - Get/Set Database Parameters\n" );
       Admin_write ( connexion, "  send user@domain/resource message      - Send a message to user\n" );
       Admin_write ( connexion, "  reload                                 - Reload contacts from Database\n" );
       Admin_write ( connexion, "  list                                   - List contact and availability\n" );
       Admin_write ( connexion, "  add enable,send_command,receive_imsg,bit_presence,jabber_id,nom\n");
       Admin_write ( connexion, "                                         - Add a IMSG contact\n" );
       Admin_write ( connexion, "  set id,enable,send_command,receive_imsg,bit_presence,jabber_id,nom\n" );
       Admin_write ( connexion, "                                         - Change IMSG id\n" );
       Admin_write ( connexion, "  del id                                 - Delete IMSG id\n" );
       Admin_write ( connexion, "  enable id                              - Enable IMSG Contact id\n" );
       Admin_write ( connexion, "  disable id                             - Disable IMSG Contact id\n" );
       Admin_write ( connexion, "  presence new_status                    - Change Presence to 'new_status'\n" );
       Admin_write ( connexion, "  status                                 - See connexion status\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown IMSG command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
