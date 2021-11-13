/******************************************************************************************************************************/
/* Watchdogd/Rfxcom/admin_rfxcom.c        Gestion des connexions Admin RFXCOM au serveur watchdog                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        mer. 13 juin 2012 23:02:08 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_rfxcom.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
 #include <unistd.h>
 #include "watchdogd.h"
 #include "Rfxcom.h"
 extern struct RFXCOM_CONFIG Cfg_rfxcom;
/******************************************************************************************************************************/
/* Admin_rfxcom: Fonction gerant les différentes commandes possible pour l'administration rfxcom                              */
/* Entrée: le connexion d'admin et la ligne de commande                                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_command( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "light" ) )
     { gchar trame_send_AC[] = { 0x07, 0x10, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
       gint housecode, unitcode, cmd, retour;
       gchar proto[32];

       if (sscanf ( ligne, "%s %[^,],%d,%d,%d", commande,                                /* Découpage de la ligne de commande */
                    proto, &housecode, &unitcode, &cmd ) != 5) return(response);

       trame_send_AC[0] = 0x07; /* Taille */
       trame_send_AC[1] = 0x10; /* lightning 1 */
       if ( ! strcmp( proto, "arc" ) )
        { trame_send_AC[2] = 0x01; /* ARC */ }
       else
        { trame_send_AC[2] = 0x00; /* X10 */ }
       trame_send_AC[3] = 0x01; /* Seqnbr */
       trame_send_AC[4] = housecode;
       trame_send_AC[5] = unitcode;
       trame_send_AC[6] = cmd;
       trame_send_AC[7] = 0x0; /* rssi */
       retour = write ( Cfg_rfxcom.fd, &trame_send_AC, trame_send_AC[0] + 1 );
       if (retour>0)
        { g_snprintf( chaine, sizeof(chaine), " Sending Proto %d, housecode %d, unitcode %d, cmd %d OK",
                      trame_send_AC[2], housecode, unitcode, cmd );
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " Sending Proto %d, housecode %d, unitcode %d, cmd %d Failed !",
                      trame_send_AC[2], housecode, unitcode, cmd );
        }
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "light_ac" ) )
     { gchar trame_send_AC[] = { 0x0B, 0x11, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
       gint id1, id2, id3, id4, unitcode, cmd, retour;

       sscanf ( ligne, "%s (%d,%d,%d,%d),%d,%d", commande,                               /* Découpage de la ligne de commande */
                &id1, &id2, &id3, &id4, &unitcode, &cmd );

       trame_send_AC[0]  = 0x0B; /* Taille */
       trame_send_AC[1]  = 0x11; /* lightning 2 */
       trame_send_AC[2]  = 0x00; /* AC */
       trame_send_AC[3]  = 0x01; /* Seqnbr */
       trame_send_AC[4]  = id1 << 6;
       trame_send_AC[5]  = id2;
       trame_send_AC[6]  = id3;
       trame_send_AC[7]  = id4;
       trame_send_AC[8]  = unitcode;
       trame_send_AC[9]  = cmd;
       trame_send_AC[10] = 0;/* level */
       trame_send_AC[11] = 0x0; /* rssi */
       retour = write ( Cfg_rfxcom.fd, &trame_send_AC, trame_send_AC[0] + 1 );
       if (retour>0)
        { g_snprintf( chaine, sizeof(chaine), " Sending Proto AC ids=%d-%d-%d-%d, unitcode %d, cmd %d OK",
                      id1, id2, id3, id4, unitcode, cmd );
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " Sending Proto AC ids=%d-%d-%d-%d, unitcode %d, cmd %d Failed",
                      id1, id2, id3, id4, unitcode, cmd );
        }
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "status" ) )
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " RFXCOM Status:" );
       response = Admin_write ( response, chaine );
       g_snprintf( chaine, sizeof(chaine), " | mode = %d", Cfg_rfxcom.mode );
       response = Admin_write ( response, chaine );
       g_snprintf( chaine, sizeof(chaine), " | fd = %d", Cfg_rfxcom.fd );
       response = Admin_write ( response, chaine );
       g_snprintf( chaine, sizeof(chaine), " | date_next_retry = in %02.1fs",
                   (Partage->top - Cfg_rfxcom.date_next_retry) / 10.0 );
       response = Admin_write ( response, chaine );
       response = Admin_write ( response, " -" );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'RFXCOM'" );
       response = Admin_write ( response, "  light proto,housecode,unitcode,cmdnumber" );
       response = Admin_write ( response, "                                         - Envoie une commande RFXCOM proto = x10 or arc" );
       response = Admin_write ( response, "  light_ac (id1,id2,id3,id4),unitcode,cmdnumber,level" );
       response = Admin_write ( response, "                                         - Envoie une commande RFXCOM Protocol AC, HomeEasy EU, ANSLUT" );
       response = Admin_write ( response, "  status                                 - Affiche le statut de la connexion" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown RFXCOM command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
