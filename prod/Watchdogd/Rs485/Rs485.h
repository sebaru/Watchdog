/**********************************************************************************************************/
/* Watchdogd/Include/Rs485.h   Header et constantes des modules rs485 Watchdgo 2.0                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 31 jui 2003 09:37:12 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Rs485.h
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
 
#ifndef _RS485_H_
 #define _RS485_H_

 #define DEFAUT_PORT_RS485              "/dev/watchdog_RS485"

 #define RS485_FCT_IDENT      0x01
 #define RS485_FCT_ENTRE_TOR  0x02
 #define RS485_FCT_ENTRE_ANA  0x03
 #define RS485_FCT_SORTIE_TOR 0x04
 #define RS485_FCT_PING       0xFF
 
 #define RS485_TEMPS_UPDATE_IO_ANA  50                              /* Get IO/ANA toutes les 5 secondes ! */

 #define RS485_TEMPS_SEUIL_DOWN     20                  /* Temps avant de considérer un module comme DOWN */

 #define NOM_THREAD                "rs485"
 #define NOM_TABLE_MODULE_RS485    "rs485"
 #define TAILLE_ENTETE  6
 #define TAILLE_DONNEES 10

 struct TRAME_RS485                                                       /* Definition d'une trame RS485 */
  { unsigned char dest;
    unsigned char source;
    unsigned char fonction;
    unsigned char taille;
    unsigned char donnees[TAILLE_DONNEES];
    unsigned char crc16_h;
    unsigned char crc16_l;
  };

 struct TRAME_RS485_IDENT
  { unsigned char version_major;
    unsigned char version_minor;
    unsigned char nbr_entre_ana;
    unsigned char nbr_entre_tor;
    unsigned char nbr_entre_choc;
    unsigned char nbr_sortie_tor;
  };

 struct RS485DB
  { guint id;                                                                    /* ID unique de la rs485 */
    guint num;                                                                      /* Numéro de la rs485 */
    guint bit_comm;                         /* Bit bistable correspondant au bon fonctionnement du module */
    gboolean enable;                                                            /* Module Start at boot ? */
    gint ea_min, ea_max;
    gint e_min, e_max, forced_e_min;
    gint s_min, s_max;
    gint sa_min, sa_max;
    gchar date_ajout[20];                                                 /* Date creation enregistrement */
    gchar libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8];                              /* Libelle de la rs485 */
  };

 struct MODULE_RS485
  { struct RS485DB rs485;

    gboolean started;                                                           /* Module Start at boot ? */
    guint  nbr_deconnect;
    guint  date_requete;
    guint  date_next_get_ana;
  };

 struct RS485_CONFIG                                                              /* Structure de travail */
  { struct LIBRAIRIE *lib;
    GSList *Modules_RS485;
    gboolean enable;                                                           /* Thread enable at boot ? */
    gint  fd;                                         /* File Descriptor de la connexion a la ligne RS485 */
    gchar port[80];                                                            /* Port d'accès a la RS485 */
  } Cfg_rs485;
/*********************************************** Déclaration des prototypes *******************************/
 extern gboolean Rs485_Lire_config ( void );
 extern gboolean Retirer_rs485DB ( struct RS485DB *rs485 );
 extern gint Ajouter_rs485DB ( struct RS485DB *rs485 );
 extern gboolean Modifier_rs485DB( struct RS485DB *rs485 );
 extern struct MODULE_RS485 *Chercher_module_rs485_by_id ( gint id );
#endif
/*--------------------------------------------------------------------------------------------------------*/
