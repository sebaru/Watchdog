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
/* Admin_sms: Gere une commande 'admin sms' depuis une connexion admin                                    */
/* Entrée: le connexion et la ligne de commande                                                              */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command ( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'SMS'\n" );
       Admin_write ( connexion, "  sms smsbox message    - Send 'message' via smsbox\n" );
       Admin_write ( connexion, "  sms gsm    message    - Send 'message' via gsm\n" );
       Admin_write ( connexion, "  help                  - This help\n" );
     } else
    if ( ! strcmp ( commande, "gsm" ) )
     { gchar message[80];
       sscanf ( ligne, "%s %s", commande, message );                 /* Découpage de la ligne de commande */
       Envoyer_sms_gsm_text ( ligne + 4 ); /* On envoie le reste de la liste, pas seulement le mot suivant. */
       g_snprintf( chaine, sizeof(chaine), " Sms sent\n" );
       Admin_write ( connexion, chaine );
     } else
    if ( ! strcmp ( commande, "smsbox" ) )
     { gchar message[80];
       sscanf ( ligne, "%s %s", commande, message );                 /* Découpage de la ligne de commande */
       Envoyer_sms_smsbox_text ( ligne + 7 ); /* On envoie le reste de la liste, pas seulement le mot suivant. */
       g_snprintf( chaine, sizeof(chaine), " Sms sent\n" );
       Admin_write ( connexion, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
