/******************************************************************************************************************************/
/* Watchdogd/Archive/admin_arch.c  Gestion des connexions Admin du thread "Archive" de watchdog                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    17.03.2017 08:37:09 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_arch.c
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
 
 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Admin_arch_status: Print le statut du thread arch                                                                          */
/* Entrée: la connexion pour sortiee client et la ligne de commande                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_arch_status ( struct CONNEXION *connexion )
  { gchar chaine[128];
    gint save_nbr;

    pthread_mutex_lock( &Partage->com_arch.synchro );                                                        /* lockage futex */
    save_nbr = g_slist_length(Partage->com_arch.liste_arch);
    pthread_mutex_unlock( &Partage->com_arch.synchro );

    g_snprintf( chaine, sizeof(chaine), " | Length of Arch list : %d\n", save_nbr );
    g_snprintf( chaine, sizeof(chaine), " -\n");
    Admin_write ( connexion, chaine );
  }
/******************************************************************************************************************************/
/* Admin_command: Gere une commande liée au thread arch depuis une connexion admin                                            */
/* Entrée: le client et la ligne de commande                                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Admin_arch ( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "status" ) )
     { Admin_arch_status ( connexion ); }
    else if ( ! strcmp ( commande, "clear" ) )
     { gint nbr;
       nbr = Arch_Clear_list ();                            /* Clear de la list des archives à prendre en compte */
       g_snprintf( chaine, sizeof(chaine), " ArchiveList cleared (%d components)\n", nbr );
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'UPS'\n" );
       Admin_write ( connexion, "  status                                 - Get Status of Arch Thread\n");
       Admin_write ( connexion, "  clear                                  - Clear Archive List\n" );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
