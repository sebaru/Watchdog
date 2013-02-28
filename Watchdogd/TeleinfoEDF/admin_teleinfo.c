/**********************************************************************************************************/
/* Watchdogd/Teleinfo/admin_teleinfo.c        Gestion des connexions Admin TLEINFO au serveur watchdog    */
/* Projet WatchDog version 2.0       Gestion d'habitat                    mer. 13 juin 2012 23:02:08 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_teleinfo.c
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
 #include "Teleinfo.h"
#ifdef bouh
/**********************************************************************************************************/
/* Admin_teleinfo_list: Liste les sortieel'ensemble des capteurs teleinfo présent dans la conf                           */
/* Entrée: le client                                                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_teleinfo_list ( struct CLIENT *client )
  { GSList *liste_modules;
    gchar chaine[512];


    g_snprintf( chaine, sizeof(chaine), " -- Liste des modules/capteurs RFXCOM\n" );
    Admin_write ( client, chaine );

    pthread_mutex_lock ( &Cfg_teleinfo.lib->synchro );
    liste_modules = Cfg_teleinfo.Modules_RFXCOM;
    while ( liste_modules )
     { struct MODULE_RFXCOM *module;
       module = (struct MODULE_RFXCOM *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine),
                   " RFXCOM[%02d] -> type=%02d(0x%02X),sous_type=%02d(0x%02X),ids=%02d %02d %02d %02d housecode=%02d unitcode=%02d\n"
                   "               e_min=%03d,ea_min=%03d,a_min=%03d,libelle=%s\n"
                   "               date_last_view=%03ds\n",
                   module->teleinfo.id, module->teleinfo.type, module->teleinfo.type,
                   module->teleinfo.sous_type, module->teleinfo.sous_type,
                   module->teleinfo.id1, module->teleinfo.id2, module->teleinfo.id3, module->teleinfo.id4, 
                   module->teleinfo.housecode, module->teleinfo.unitcode,
                   module->teleinfo.e_min, module->teleinfo.ea_min, module->teleinfo.a_min,
                   module->teleinfo.libelle, 
                   (Partage->top - module->date_last_view)/10
                 );
       Admin_write ( client, chaine );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_teleinfo.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_teleinfo_del: Retire le capteur/module teleinfo dont l'id est en parametre                           */
/* Entrée: le client et l'id                                                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_teleinfo_del ( struct CLIENT *client, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Suppression du module teleinfo %03d\n", id );
    Admin_write ( client, chaine );

    if ( Retirer_teleinfoDB( id ) )
     { g_snprintf( chaine, sizeof(chaine), " Module %03d erased.\n", id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error. Module %03d NOT erased.\n", id ); }
    Admin_write ( client, chaine );
  }
/**********************************************************************************************************/
/* Admin_teleinfo_add: Ajoute un capteur/module RFXCOM                                                      */
/* Entrée: le client et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_teleinfo_add ( struct CLIENT *client, struct RFXCOMDB *teleinfo )
  { gchar chaine[128];
    gint last_id;

    g_snprintf( chaine, sizeof(chaine), " -- Ajout d'un module teleinfo\n" );
    Admin_write ( client, chaine );

    last_id = Ajouter_teleinfoDB( teleinfo );
    if ( last_id != -1 )
     { g_snprintf( chaine, sizeof(chaine), " Module added. New ID=%03d.\n", last_id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT added.\n" ); }
    Admin_write ( client, chaine );
  }
/**********************************************************************************************************/
/* Admin_teleinfo_change: Modifie la configuration d'un capteur RFXCOM                                      */
/* Entrée: le client et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_teleinfo_change ( struct CLIENT *client, struct RFXCOMDB *teleinfo )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Modification du module teleinfo %03d\n", teleinfo->id );
    Admin_write ( client, chaine );

    if ( Modifier_teleinfoDB( teleinfo ) )
     { g_snprintf( chaine, sizeof(chaine), " Module %03d changed.\n", teleinfo->id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error. Module %03d NOT changed.\n", teleinfo->id ); }
    Admin_write ( client, chaine );
  }
#endif
/**********************************************************************************************************/
/* Admin_command: Fonction gerant les différentes commandes possible pour l'administration teleinfo       */
/* Entrée: le client d'admin et la ligne de commande                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CLIENT *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */
#ifdef bouh
    if ( ! strcmp ( commande, "add" ) )
     { struct RFXCOMDB teleinfo;
       memset( &teleinfo, 0, sizeof(struct RFXCOMDB) );                /* Découpage de la ligne de commande */
       sscanf ( ligne, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%[^\n]", commande,
                (gint *)&teleinfo.type, (gint *)&teleinfo.sous_type,
                (gint *)&teleinfo.id1, (gint *)&teleinfo.id2, (gint *)&teleinfo.id3, (gint *)&teleinfo.id4,
                (gint *)&teleinfo.housecode,(gint *)&teleinfo.unitcode,
                &teleinfo.e_min, &teleinfo.ea_min, &teleinfo.a_min, teleinfo.libelle );
       Admin_teleinfo_add ( client, &teleinfo );
     }
    else if ( ! strcmp ( commande, "change" ) )
     { struct RFXCOMDB teleinfo;
       memset( &teleinfo, 0, sizeof(struct RFXCOMDB) );                /* Découpage de la ligne de commande */
       sscanf ( ligne, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%[^\n]", commande,
                &teleinfo.id, (gint *)&teleinfo.type, (gint *)&teleinfo.sous_type,
                (gint *)&teleinfo.id1, (gint *)&teleinfo.id2, (gint *)&teleinfo.id3, (gint *)&teleinfo.id4,
                (gint *)&teleinfo.housecode,(gint *)&teleinfo.unitcode,
                &teleinfo.e_min, &teleinfo.ea_min, &teleinfo.a_min, teleinfo.libelle );
       Admin_teleinfo_change ( client, &teleinfo );
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
       write ( Cfg_teleinfo.fd, &trame_send_AC, trame_send_AC[0] + 1 );
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
       write ( Cfg_teleinfo.fd, &trame_send_AC, trame_send_AC[0] + 1 );
     }
    else if ( ! strcmp ( commande, "del" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_teleinfo_del ( client, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_teleinfo_list ( client );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( client, "  -- Watchdog ADMIN -- Help du mode 'RFXCOM'\n" );
       Admin_write ( client, "  add type,sstype,id1,id2,id3,id4,housecode,unitcode,e_min,ea_min,a_min,libelle\n"
                     "                                         - Ajoute un module\n" );
       Admin_write ( client, "  change ID,type,sstype,id1,id2,id3,id4,housecode,unitcode,e_min,ea_min,a_min,libelle\n"
                     "                                         - Edite le module ID\n" );
       Admin_write ( client, "  del ID                                 - Retire le module ID\n" );
       Admin_write ( client, "  light1 proto,housecode,unitcode,cmdnumber\n" );
       Admin_write ( client, "                                         - Envoie une commande RFXCOM\n" );
       Admin_write ( client, "  light2 id1,id2,id3,id4,unitcode,cmdnumber,level\n" );
       Admin_write ( client, "                                         - Envoie une commande RFXCOM\n" );
       Admin_write ( client, "  list                                   - Affiche les status des equipements RFXCOM\n" );
     }
    else
#endif
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown Teleinfo command : %s\n", ligne );
       Admin_write ( client, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
