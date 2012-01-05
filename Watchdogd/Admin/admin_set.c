/**********************************************************************************************************/
/* Watchdogd/Admin/admin_set.c        Gestion des connexions Admin SET au serveur watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                    jeu. 05 janv. 2012 23:24:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_set.c
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
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Admin_set: Gere une commande 'admin set' depuis une connexion admin                                    */
/* Entrée: le client et la ligne de commande                                                              */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_set ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion, "  -- Watchdog ADMIN -- Help du mode 'SET'\n" );
       Write_admin ( client->connexion, "  ch num val            - Set CH[num] = val\n" );
       Write_admin ( client->connexion, "  ci num val            - Set CI[num] = val\n" );
       Write_admin ( client->connexion, "  help                  - This help\n" );
     } else
    if ( ! strcmp ( commande, "ch" ) )
     { int num, val;
       sscanf ( ligne, "%s %d %d", commande, &num, &val );           /* Découpage de la ligne de commande */
       if (num<NBR_COMPTEUR_H)
        { Partage->ch[num].cpthdb.valeur = val;
          g_snprintf( chaine, sizeof(chaine), " CH%03d = %d\n", num, val );
        } else
        { g_snprintf( chaine, sizeof(chaine), " CH -> num '%d' out of range\n", num ); }
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "ci" ) )
     { int num, val;
       sscanf ( ligne, "%s %d %d", commande, &num, &val );           /* Découpage de la ligne de commande */
       if (num<NBR_COMPTEUR_IMP)
        { Partage->ci[num].cpt_impdb.valeur = val;
          g_snprintf( chaine, sizeof(chaine), " CI%03d = %d\n", num, val );
        } else
        { g_snprintf( chaine, sizeof(chaine), " CI -> num '%d' out of range\n", num ); }
       Write_admin ( client->connexion, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
