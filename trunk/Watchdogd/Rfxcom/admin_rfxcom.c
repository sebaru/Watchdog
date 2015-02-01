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
 #include <unistd.h>
 #include "watchdogd.h"
 #include "Rfxcom.h"

/**********************************************************************************************************/
/* Admin_rfxcom_print : Affiche en details les infos d'un onduleur en parametre                           */
/* Entrée: La connexion connexion ADMIN et l'onduleur                                                     */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_rfxcom_print ( struct CONNEXION *connexion, struct MODULE_RFXCOM *module )
  { gchar chaine[1024];
    g_snprintf( chaine, sizeof(chaine),
                " RFXCOM[%02d] ------> date_last_view = %03ds - %s\n"
                "  | - type = %02d (0x%02X), sous_type = %02d (0x%02X)\n"
                "  | - IDs  = %03d %03d %03d %03d\n"
                "  | - housecode = %03d, unitcode=%03d\n"
                "  | - map_E = %03d, map_EA = %03d, map_A = %03d\n"
                "  -\n",
                module->rfxcom.id, (Partage->top - module->date_last_view)/10, module->rfxcom.libelle,
                module->rfxcom.type, module->rfxcom.type,
                module->rfxcom.sous_type, module->rfxcom.sous_type,
                module->rfxcom.id1, module->rfxcom.id2, module->rfxcom.id3, module->rfxcom.id4, 
                module->rfxcom.housecode, module->rfxcom.unitcode,
                module->rfxcom.map_E, module->rfxcom.map_EA, module->rfxcom.map_A
              );
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_rfxcom_list: Liste l'ensemble des capteurs rfxcom présent dans la conf                           */
/* Entrée: le connexion                                                                                   */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rfxcom_list ( struct CONNEXION *connexion )
  { GSList *liste_modules;
    pthread_mutex_lock ( &Cfg_rfxcom.lib->synchro );
    liste_modules = Cfg_rfxcom.Modules_RFXCOM;
    while ( liste_modules )
     { struct MODULE_RFXCOM *module;
       module = (struct MODULE_RFXCOM *)liste_modules->data;
       Admin_rfxcom_print ( connexion, module );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_rfxcom.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_rfxcom_list: Liste l'ensemble des capteurs rfxcom présent dans la conf                           */
/* Entrée: le connexion                                                                                   */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rfxcom_show ( struct CONNEXION *connexion, gint id )
  { GSList *liste_modules;
    pthread_mutex_lock ( &Cfg_rfxcom.lib->synchro );
    liste_modules = Cfg_rfxcom.Modules_RFXCOM;
    while ( liste_modules )
     { struct MODULE_RFXCOM *module;
       module = (struct MODULE_RFXCOM *)liste_modules->data;
       if (module->rfxcom.id == id)
        { Admin_rfxcom_print ( connexion, module );
          break;
        }
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_rfxcom.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_rfxcom_del: Retire le capteur/module rfxcom dont l'id est en parametre                           */
/* Entrée: le connexion et l'id                                                                           */
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
/* Entrée: le connexion et la structure de reference du capteur                                           */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rfxcom_set ( struct CONNEXION *connexion, gchar *ligne )
  { gchar id_char[16], param[32], valeur_char[64], chaine[128];
    struct MODULE_RFXCOM *module = NULL;
    GSList *liste_modules;
    guint id, valeur;
    gint retour;

    if ( ! strcmp ( ligne, "list" ) )
     { Admin_write ( connexion, " | Parameter can be:\n" );
       Admin_write ( connexion, " | - type, sous_type, id1, id2, id3, id4, housecode, unitcode,\n" );
       Admin_write ( connexion, " | - map_E, map_EA, map_A, libelle\n" );
       Admin_write ( connexion, " -\n" );
       return;
     }

    sscanf ( ligne, "%s %s %s", id_char, param, valeur_char );
    id     = atoi ( id_char     );
    valeur = atoi ( valeur_char );

    pthread_mutex_lock( &Cfg_rfxcom.lib->synchro );                     /* Recherche du module en mémoire */
    liste_modules = Cfg_rfxcom.Modules_RFXCOM;
    while ( liste_modules )
     { module = (struct MODULE_RFXCOM *)liste_modules->data;
       if (module->rfxcom.id == id) break;
       liste_modules = liste_modules->next;                                  /* Passage au module suivant */
     }
    pthread_mutex_unlock( &Cfg_rfxcom.lib->synchro );

    if (!module)                                                                         /* Si non trouvé */
     { Admin_write( connexion, " Module not found\n");
       return;
     }

    if ( ! strcmp( param, "type" ) )
     { module->rfxcom.type = valeur; }
    else if ( ! strcmp( param, "bit" ) )
     { module->rfxcom.sous_type = valeur; }
    else if ( ! strcmp( param, "id1" ) )
     { module->rfxcom.id1 = valeur; }
    else if ( ! strcmp( param, "id2" ) )
     { module->rfxcom.id2 = valeur; }
    else if ( ! strcmp( param, "id3" ) )
     { module->rfxcom.id3 = valeur; }
    else if ( ! strcmp( param, "id4" ) )
     { module->rfxcom.id4 = valeur; }
    else if ( ! strcmp( param, "housecode" ) )
     { module->rfxcom.housecode = valeur; }
    else if ( ! strcmp( param, "unitcode" ) )
     { module->rfxcom.unitcode = valeur; }
    else if ( ! strcmp( param, "map_E" ) )
     { module->rfxcom.map_E = valeur; }
    else if ( ! strcmp( param, "map_EA" ) )
     { module->rfxcom.map_EA = valeur; }
    else if ( ! strcmp( param, "map_A" ) )
     { module->rfxcom.map_A = valeur; }
    else if ( ! strcmp( param, "libelle" ) )
     { g_snprintf( module->rfxcom.libelle, sizeof(module->rfxcom.libelle), "%s", valeur_char ); }
    else
     { g_snprintf( chaine, sizeof(chaine),
                 " Parameter %s not known for RFXCOM id %s ('rfxcom set list' can help)\n", param, id_char );
       Admin_write ( connexion, chaine );
       return;
     }

    retour = Modifier_rfxcomDB ( &module->rfxcom );
    if (retour)
     { Admin_write ( connexion, " ERROR : RFXCOM module NOT set\n" ); }
    else
     { Admin_write ( connexion, " RFXCOM module parameter set\n" ); }
  }
/**********************************************************************************************************/
/* Admin_rfxcom: Fonction gerant les différentes commandes possible pour l'administration rfxcom          */
/* Entrée: le connexion d'admin et la ligne de commande                                                   */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "add" ) )
     { struct RFXCOMDB rfxcom;
       memset( &rfxcom, 0, sizeof(struct RFXCOMDB) );                /* Découpage de la ligne de commande */
       sscanf ( ligne, "%s %d,%d,(%d,%d,%d,%d),%d,%d,%d,%d,%d,%[^\n]", commande,
                (gint *)&rfxcom.type, (gint *)&rfxcom.sous_type,
                (gint *)&rfxcom.id1, (gint *)&rfxcom.id2, (gint *)&rfxcom.id3, (gint *)&rfxcom.id4,
                (gint *)&rfxcom.housecode,(gint *)&rfxcom.unitcode,
                &rfxcom.map_E, &rfxcom.map_EA, &rfxcom.map_A, rfxcom.libelle );
       Admin_rfxcom_add ( connexion, &rfxcom );
     }
    else if ( ! strcmp ( commande, "set" ) )
     { Admin_rfxcom_set ( connexion, ligne+4 );
     }
    else if ( ! strcmp ( commande, "light" ) )
     { gchar trame_send_AC[] = { 0x07, 0x10, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
       gint housecode, unitcode, cmd, retour;
       gchar proto[32];

       sscanf ( ligne, "%s %s,%d,%d,%d", commande,                   /* Découpage de la ligne de commande */
                proto, &housecode, &unitcode, &cmd );

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
        { g_snprintf( chaine, sizeof(chaine), " Sending Proto %d, housecode %d, unitcode %d, cmd %d OK\n",
                      trame_send_AC[2], housecode, unitcode, cmd );
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " Sending Proto %d, housecode %d, unitcode %d, cmd %d Failed !\n",
                      trame_send_AC[2], housecode, unitcode, cmd );
        }
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "light_ac" ) )
     { gchar trame_send_AC[] = { 0x0B, 0x11, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
       gint id1, id2, id3, id4, unitcode, cmd, retour;

       sscanf ( ligne, "%s (%d,%d,%d,%d),%d,%d", commande,           /* Découpage de la ligne de commande */
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
        { g_snprintf( chaine, sizeof(chaine), " Sending Proto AC ids=%d-%d-%d-%d, unitcode %d, cmd %d OK\n",
                      id1, id2, id3, id4, unitcode, cmd );
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " Sending Proto AC ids=%d-%d-%d-%d, unitcode %d, cmd %d Failed\n",
                      id1, id2, id3, id4, unitcode, cmd );
        }
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "del" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_rfxcom_del ( connexion, num );
     }
    else if ( ! strcmp ( commande, "show" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_rfxcom_show ( connexion, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_rfxcom_list ( connexion );
     }
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { if (Admin_dbcfg_thread ( connexion, NOM_THREAD, ligne+6 ) == TRUE)   /* Si changement de parametre */
        { gboolean retour;
          retour = Rfxcom_Lire_config();
          g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s\n",
                      (retour ? "Success" : "Failed") );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'RFXCOM'\n" );
       Admin_write ( connexion, "  dbcfg ...                              - Get/Set Database Parameters\n" );
       Admin_write ( connexion, "  add type,sstype,(id1,id2,id3,id4),housecode,unitcode,map_E,map_EA,map_A,libelle\n"
                     "                                         - Ajoute un module\n" );
       Admin_write ( connexion, "  set ID,type,sstype,(id1,id2,id3,id4),housecode,unitcode,map_E,map_EA,map_A,libelle\n"
                     "                                         - Edite le module ID\n" );
       Admin_write ( connexion, "  del ID                                 - Retire le module ID\n" );
       Admin_write ( connexion, "  light proto,housecode,unitcode,cmdnumber\n" );
       Admin_write ( connexion, "                                         - Envoie une commande RFXCOM proto = x10 or arc\n" );
       Admin_write ( connexion, "  light_ac (id1,id2,id3,id4),unitcode,cmdnumber,level\n" );
       Admin_write ( connexion, "                                         - Envoie une commande RFXCOM Protocol AC, HomeEasy EU, ANSLUT\n" );
       Admin_write ( connexion, "  list                                   - Affiche les status des equipements RFXCOM\n" );
       Admin_write ( connexion, "  show $id                               - Affiche les status de l'equipements RFXCOM $id\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown RFXCOM command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
