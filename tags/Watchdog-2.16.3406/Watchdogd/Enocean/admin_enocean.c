/**********************************************************************************************************/
/* Watchdogd/Enocean/admin_enocean.c        Gestion des connexions Admin ENOCEAN au serveur watchdog      */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 28 d�c. 2014 15:45:35 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_enocean.c
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
 #include "Enocean.h"

 static gchar *ENOCEAN_COMM_STATUS[]=
  { "CONNECT",
    "WAIT_FOR_SYNC",
    "WAIT_FOR_HEADER",
    "WAIT_FOR_DATA",
    "DISCONNECT",
    "WAIT_BEFORE_RECONNECT",
  };

/**********************************************************************************************************/
/* Admin_enocean_print : Affiche en details les infos d'un onduleur en parametre                          */
/* Entr�e: La connexion connexion ADMIN et l'onduleur                                                     */
/* Sortie: Rien, tout est envoy� dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static gchar *Admin_enocean_status ( gchar *response )
  { gchar chaine[1024];
    g_snprintf( chaine, sizeof(chaine),
                " ENOCEAN status ------> %s (%02d) - %s"
                "  | - retry_connect  = in %03.1f s"
                "  | - date_last_view = %03.1f s ago"
                "  | - nbr_oct_lu     = %03d bytes"
                "  | - filedescriptor = %03d"
                "  -",
                ENOCEAN_COMM_STATUS[Cfg_enocean.comm_status], Cfg_enocean.comm_status, Cfg_enocean.port,
                (Cfg_enocean.date_retry_connect ? (Partage->top - Cfg_enocean.date_retry_connect)/10.0 : 0.0),
                (Cfg_enocean.date_last_view ? (Partage->top - Cfg_enocean.date_last_view)/10.0 : 0.0),
                Cfg_enocean.nbr_oct_lu, Cfg_enocean.fd
              );
    return(Admin_write ( response, chaine ));
  }
#ifdef bouh
/**********************************************************************************************************/
/* Admin_enocean_print : Affiche en details les infos d'un onduleur en parametre                          */
/* Entr�e: La connexion connexion ADMIN et l'onduleur                                                     */
/* Sortie: Rien, tout est envoy� dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_enocean_print ( gchar *response, struct MODULE_ENOCEAN *module )
  { gchar chaine[1024];
    g_snprintf( chaine, sizeof(chaine),
                " ENOCEAN[%02d] ------> date_last_view = %03ds - %s"
                "  | - type = %02d (0x%02X), sous_type = %02d (0x%02X)"
                "  | - IDs  = %03d %03d %03d %03d"
                "  | - housecode = %03d, unitcode=%03d"
                "  | - e_min = %03d, ea_min = %03d, a_min = %03d"
                "  -",
                module->enocean.id, (Partage->top - module->date_last_view)/10, module->enocean.libelle,
                module->enocean.type, module->enocean.type,
                module->enocean.sous_type, module->enocean.sous_type,
                module->enocean.id1, module->enocean.id2, module->enocean.id3, module->enocean.id4, 
                module->enocean.housecode, module->enocean.unitcode,
                module->enocean.e_min, module->enocean.ea_min, module->enocean.a_min
              );
    response = Admin_write ( response, chaine );
  }
/**********************************************************************************************************/
/* Admin_enocean_list: Liste l'ensemble des capteurs enocean pr�sent dans la conf                         */
/* Entr�e: le connexion                                                                                   */
/* Sortie: n�ant                                                                                          */
/**********************************************************************************************************/
 static void Admin_enocean_list ( gchar *response )
  { GSList *liste_modules;
    pthread_mutex_lock ( &Cfg_enocean.lib->synchro );
    liste_modules = Cfg_enocean.Modules_ENOCEAN;
    while ( liste_modules )
     { struct MODULE_ENOCEAN *module;
       module = (struct MODULE_ENOCEAN *)liste_modules->data;
       Admin_enocean_print ( connexion, module );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_enocean.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_enocean_list: Liste l'ensemble des capteurs enocean pr�sent dans la conf                         */
/* Entr�e: le connexion                                                                                   */
/* Sortie: n�ant                                                                                          */
/**********************************************************************************************************/
 static void Admin_enocean_show ( gchar *response, gint id )
  { GSList *liste_modules;
    pthread_mutex_lock ( &Cfg_enocean.lib->synchro );
    liste_modules = Cfg_enocean.Modules_ENOCEAN;
    while ( liste_modules )
     { struct MODULE_ENOCEAN *module;
       module = (struct MODULE_ENOCEAN *)liste_modules->data;
       if (module->enocean.id == id)
        { Admin_enocean_print ( connexion, module );
          break;
        }
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_enocean.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_enocean_del: Retire le capteur/module enocean dont l'id est en parametre                         */
/* Entr�e: le connexion et l'id                                                                           */
/* Sortie: n�ant                                                                                          */
/**********************************************************************************************************/
 static void Admin_enocean_del ( gchar *response, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Suppression du module enocean %03d", id );
    response = Admin_write ( response, chaine );

    if ( Retirer_enoceanDB( id ) )
     { g_snprintf( chaine, sizeof(chaine), " Module %03d erased.", id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error. Module %03d NOT erased.", id ); }
    response = Admin_write ( response, chaine );
  }
/**********************************************************************************************************/
/* Admin_enocean_add: Ajoute un capteur/module ENOCEAN                                                    */
/* Entr�e: le connexion et la structure de reference du capteur                                           */
/* Sortie: n�ant                                                                                          */
/**********************************************************************************************************/
 static void Admin_enocean_add ( gchar *response, struct ENOCEANDB *enocean )
  { gchar chaine[128];
    gint last_id;

    g_snprintf( chaine, sizeof(chaine), " -- Ajout d'un module enocean" );
    response = Admin_write ( response, chaine );

    last_id = Ajouter_enoceanDB( enocean );
    if ( last_id != -1 )
     { g_snprintf( chaine, sizeof(chaine), " Module added. New ID=%03d.", last_id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT added." ); }
    response = Admin_write ( response, chaine );
  }
/**********************************************************************************************************/
/* Admin_enocean_change: Modifie la configuration d'un capteur ENOCEAN                                    */
/* Entr�e: le connexion et la structure de reference du capteur                                           */
/* Sortie: n�ant                                                                                          */
/**********************************************************************************************************/
 static void Admin_enocean_set ( gchar *response, struct ENOCEANDB *enocean )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Modification du module enocean %03d", enocean->id );
    response = Admin_write ( response, chaine );

    if ( Modifier_enoceanDB( enocean ) )
     { g_snprintf( chaine, sizeof(chaine), " Module %03d changed.", enocean->id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error. Module %03d NOT changed.", enocean->id ); }
    response = Admin_write ( response, chaine );
  }
#endif
/**********************************************************************************************************/
/* Admin_enocean: Fonction gerant les diff�rentes commandes possible pour l'administration enocean        */
/* Entr�e: le connexion d'admin et la ligne de commande                                                   */
/* Sortie: n�ant                                                                                          */
/**********************************************************************************************************/
 gchar *Admin_command( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* D�coupage de la ligne de commande */

    if ( ! strcmp ( commande, "status" ) )
     { response = Admin_enocean_status ( response ); }
#ifdef bouh
    if ( ! strcmp ( commande, "add" ) )
     { struct ENOCEANDB enocean;
       memset( &enocean, 0, sizeof(struct ENOCEANDB) );                /* D�coupage de la ligne de commande */
       sscanf ( ligne, "%s %d,%d,(%d,%d,%d,%d),%d,%d,%d,%d,%d,%[^]", commande,
                (gint *)&enocean.type, (gint *)&enocean.sous_type,
                (gint *)&enocean.id1, (gint *)&enocean.id2, (gint *)&enocean.id3, (gint *)&enocean.id4,
                (gint *)&enocean.housecode,(gint *)&enocean.unitcode,
                &enocean.e_min, &enocean.ea_min, &enocean.a_min, enocean.libelle );
       Admin_enocean_add ( connexion, &enocean );
     }
    else if ( ! strcmp ( commande, "set" ) )
     { struct ENOCEANDB enocean;
       memset( &enocean, 0, sizeof(struct ENOCEANDB) );                /* D�coupage de la ligne de commande */
       sscanf ( ligne, "%s %d,%d,%d,(%d,%d,%d,%d),%d,%d,%d,%d,%d,%[^]", commande,
                &enocean.id, (gint *)&enocean.type, (gint *)&enocean.sous_type,
                (gint *)&enocean.id1, (gint *)&enocean.id2, (gint *)&enocean.id3, (gint *)&enocean.id4,
                (gint *)&enocean.housecode,(gint *)&enocean.unitcode,
                &enocean.e_min, &enocean.ea_min, &enocean.a_min, enocean.libelle );
       Admin_enocean_set ( connexion, &enocean );
     }
    else if ( ! strcmp ( commande, "light" ) )
     { gchar trame_send_AC[] = { 0x07, 0x10, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
       gint housecode, unitcode, cmd, retour;
       gchar proto[32];

       sscanf ( ligne, "%s %s,%d,%d,%d", commande,                   /* D�coupage de la ligne de commande */
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
       retour = write ( Cfg_enocean.fd, &trame_send_AC, trame_send_AC[0] + 1 );
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

       sscanf ( ligne, "%s (%d,%d,%d,%d),%d,%d", commande,           /* D�coupage de la ligne de commande */
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
       retour = write ( Cfg_enocean.fd, &trame_send_AC, trame_send_AC[0] + 1 );
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
    else if ( ! strcmp ( commande, "del" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* D�coupage de la ligne de commande */
       Admin_enocean_del ( connexion, num );
     }
    else if ( ! strcmp ( commande, "show" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* D�coupage de la ligne de commande */
       Admin_enocean_show ( connexion, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_enocean_list ( connexion );
     }
#endif
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction d�di�e � la gestion des parametres DB */
     { gboolean retour;
       response =  Admin_dbcfg_thread ( response, NOM_THREAD, ligne+6 );                        /* Si changement de parametre */
       retour = Enocean_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s",
                   (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'ENOCEAN'" );
       response = Admin_write ( response, "  dbcfg ...      - Get/Set Database Parameters" );
       response = Admin_write ( response, "  status         - Affiche les status de l'equipements ENOCEAN $id" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown ENOCEAN command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*--------------------------------------------------------------------------------------------------------*/
