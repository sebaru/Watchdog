/**********************************************************************************************************/
/* Watchdogd/Rfxcom/admin_rfxcom.c        Gestion des connexions Admin RFXCOM au serveur watchdog         */
/* Projet WatchDog version 2.0       Gestion d'habitat                    mer. 13 juin 2012 23:02:08 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_rfxcom.c
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
 #include "watchdogd.h"
 #include "Rfxcom.h"

/**********************************************************************************************************/
/* Admin_rfxcom_list: Liste l'ensemble des capteurs rfxcom présent dans la conf                           */
/* Entrée: le connexion                                                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rfxcom_list ( struct CONNEXION *connexion )
  { GSList *liste_modules;
    gchar chaine[512];


    g_snprintf( chaine, sizeof(chaine), " -- Liste des modules/capteurs RFXCOM\n" );
    Admin_write ( connexion, chaine );

    pthread_mutex_lock ( &Cfg_rfxcom.lib->synchro );
    liste_modules = Cfg_rfxcom.Modules_RFXCOM;
    while ( liste_modules )
     { struct MODULE_RFXCOM *module;
       module = (struct MODULE_RFXCOM *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine),
                   " RFXCOM[%02d] -> type=%02d(0x%02X),sous_type=%02d(0x%02X),ids=%02d %02d %02d %02d housecode=%02d unitcode=%02d\n"
                   "               e_min=%03d,ea_min=%03d,a_min=%03d,libelle=%s\n"
                   "               date_last_view=%03ds\n",
                   module->rfxcom.id, module->rfxcom.type, module->rfxcom.type,
                   module->rfxcom.sous_type, module->rfxcom.sous_type,
                   module->rfxcom.id1, module->rfxcom.id2, module->rfxcom.id3, module->rfxcom.id4, 
                   module->rfxcom.housecode, module->rfxcom.unitcode,
                   module->rfxcom.e_min, module->rfxcom.ea_min, module->rfxcom.a_min,
                   module->rfxcom.libelle, 
                   (Partage->top - module->date_last_view)/10
                 );
       Admin_write ( connexion, chaine );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_rfxcom.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_rfxcom_del: Retire le capteur/module rfxcom dont l'id est en parametre                           */
/* Entrée: le connexion et l'id                                                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rfxcom_del ( struct CONNEXION *connexion, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Suppression du module rfxcom %03d\n", id );
    Admin_write ( connexion, chaine );

    if ( Retirer_rfxcomDB( id ) )
     { g_snprintf( chaine, sizeof(chaine), " Module %03d erased.\n", id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error. Module %03d NOT erased.\n", id ); }
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_rfxcom_add: Ajoute un capteur/module RFXCOM                                                      */
/* Entrée: le connexion et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rfxcom_add ( struct CONNEXION *connexion, struct RFXCOMDB *rfxcom )
  { gchar chaine[128];
    gint last_id;

    g_snprintf( chaine, sizeof(chaine), " -- Ajout d'un module rfxcom\n" );
    Admin_write ( connexion, chaine );

    last_id = Ajouter_rfxcomDB( rfxcom );
    if ( last_id != -1 )
     { g_snprintf( chaine, sizeof(chaine), " Module added. New ID=%03d.\n", last_id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT added.\n" ); }
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_rfxcom_change: Modifie la configuration d'un capteur RFXCOM                                      */
/* Entrée: le connexion et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rfxcom_change ( struct CONNEXION *connexion, struct RFXCOMDB *rfxcom )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Modification du module rfxcom %03d\n", rfxcom->id );
    Admin_write ( connexion, chaine );

    if ( Modifier_rfxcomDB( rfxcom ) )
     { g_snprintf( chaine, sizeof(chaine), " Module %03d changed.\n", rfxcom->id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error. Module %03d NOT changed.\n", rfxcom->id ); }
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_rfxcom: Fonction gerant les différentes commandes possible pour l'administration rfxcom          */
/* Entrée: le connexion d'admin et la ligne de commande                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "add" ) )
     { struct RFXCOMDB rfxcom;
       memset( &rfxcom, 0, sizeof(struct RFXCOMDB) );                /* Découpage de la ligne de commande */
       sscanf ( ligne, "%s %d,%d,(%d,%d,%d,%d),%d,%d,%d,%d,%d,%[^\n]", commande,
                (gint *)&rfxcom.type, (gint *)&rfxcom.sous_type,
                (gint *)&rfxcom.id1, (gint *)&rfxcom.id2, (gint *)&rfxcom.id3, (gint *)&rfxcom.id4,
                (gint *)&rfxcom.housecode,(gint *)&rfxcom.unitcode,
                &rfxcom.e_min, &rfxcom.ea_min, &rfxcom.a_min, rfxcom.libelle );
       Admin_rfxcom_add ( connexion, &rfxcom );
     }
    else if ( ! strcmp ( commande, "change" ) )
     { struct RFXCOMDB rfxcom;
       memset( &rfxcom, 0, sizeof(struct RFXCOMDB) );                /* Découpage de la ligne de commande */
       sscanf ( ligne, "%s %d,%d,%d,(%d,%d,%d,%d),%d,%d,%d,%d,%d,%[^\n]", commande,
                &rfxcom.id, (gint *)&rfxcom.type, (gint *)&rfxcom.sous_type,
                (gint *)&rfxcom.id1, (gint *)&rfxcom.id2, (gint *)&rfxcom.id3, (gint *)&rfxcom.id4,
                (gint *)&rfxcom.housecode,(gint *)&rfxcom.unitcode,
                &rfxcom.e_min, &rfxcom.ea_min, &rfxcom.a_min, rfxcom.libelle );
       Admin_rfxcom_change ( connexion, &rfxcom );
     }
    else if ( ! strcmp ( commande, "light1" ) )
     { gchar trame_send_AC[] = { 0x07, 0x10, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
       gint proto, housecode, unitcode, cmd;

       sscanf ( ligne, "%s %d,%d,%d,%d", commande,             /* Découpage de la ligne de commande */
                &proto, &housecode, &unitcode, &cmd );

       trame_send_AC[0]  = 0x07; /* Taille */
       trame_send_AC[1]  = 0x10; /* lightning 1 */
       trame_send_AC[2]  = proto; /* ARC */
       trame_send_AC[3]  = 0x01; /* Seqnbr */
       trame_send_AC[4]  = housecode;
       trame_send_AC[5]  = unitcode;
       trame_send_AC[6] = cmd;
       trame_send_AC[7] = 0x0; /* rssi */
       write ( Cfg_rfxcom.fd, &trame_send_AC, trame_send_AC[0] + 1 );
     }
    else if ( ! strcmp ( commande, "light2" ) )
     { gchar trame_send_AC[] = { 0x0B, 0x11, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
       gint id1, id2, id3, id4, unitcode, cmd, level;

       sscanf ( ligne, "%s %d,%d,%d,%d,%d,%d", commande,             /* Découpage de la ligne de commande */
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
       trame_send_AC[10] = level;
       trame_send_AC[11] = 0x0; /* rssi */
       write ( Cfg_rfxcom.fd, &trame_send_AC, trame_send_AC[0] + 1 );
     }
    else if ( ! strcmp ( commande, "del" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_rfxcom_del ( connexion, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_rfxcom_list ( connexion );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'RFXCOM'\n" );
       Admin_write ( connexion, "  add type,sstype,(id1,id2,id3,id4),housecode,unitcode,e_min,ea_min,a_min,libelle\n"
                     "                                         - Ajoute un module\n" );
       Admin_write ( connexion, "  change ID,type,sstype,(id1,id2,id3,id4),housecode,unitcode,e_min,ea_min,a_min,libelle\n"
                     "                                         - Edite le module ID\n" );
       Admin_write ( connexion, "  del ID                                 - Retire le module ID\n" );
       Admin_write ( connexion, "  light1 proto,housecode,unitcode,cmdnumber\n" );
       Admin_write ( connexion, "                                         - Envoie une commande RFXCOM\n" );
       Admin_write ( connexion, "  light2 id1,id2,id3,id4,unitcode,cmdnumber,level\n" );
       Admin_write ( connexion, "                                         - Envoie une commande RFXCOM\n" );
       Admin_write ( connexion, "  list                                   - Affiche les status des equipements RFXCOM\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown RFXCOM command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
