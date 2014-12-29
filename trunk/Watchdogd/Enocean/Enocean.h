/**********************************************************************************************************/
/* Watchdogd/Enocean/Enocean.h        Déclaration structure internes des communication ENOCEAN             */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 28 déc. 2014 15:43:41 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Enocean.h
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
 
#ifndef _ENOCEAN_H_
 #define _ENOCEAN_H_

 #define DEFAUT_PORT_ENOCEAN             "/dev/watchdog_ENOCEAN"

 #define NOM_THREAD                 "enocean"
 #define NOM_TABLE_MODULE_ENOCEAN   "enocean"
 #define NBR_CARAC_LIBELLE_ENOCEAN  128
 #define TAILLE_ENTETE_ENOCEAN      6

 struct TRAME_ENOCEAN                                                   /* Definition d'une trame ENOCEAN */
  { guchar sync;
    guchar data_length_msb;
    guchar data_length_lsb;
    guchar optional_length;
    guchar packet_type;
    guchar crc_header;
    guchar data[80];
  };

 struct ENOCEANDB
  { guint id;                                                                   /* ID unique de la enocean */
    guchar type;                                                                   /* Numéro de la enocean */
    guchar sous_type;                                                              /* Numéro de la enocean */
    guchar id1;                                                                    /* Numéro de la enocean */
    guchar id2;                                                                    /* Numéro de la enocean */
    guchar id3;                                                                    /* Numéro de la enocean */
    guchar id4;                                                                    /* Numéro de la enocean */
    guchar housecode;                                                              /* Numéro de la enocean */
    guchar unitcode;                                                               /* Numéro de la enocean */
    gint e_min, ea_min, a_min;
    gchar libelle[NBR_CARAC_LIBELLE_ENOCEAN];                                      /* Libelle de la enocean */
  };

 struct MODULE_ENOCEAN
  { struct ENOCEANDB enocean;
    guint date_last_view;
  };

 struct ENOCEAN_CONFIG
  { struct LIBRAIRIE *lib;
    gchar port[80];
    gint fd;                                               /* File descripteur de la connexion au ENOCEAN */
    gboolean enable;                                                           /* Thread enable at boot ? */
    gboolean reload;
    GSList *Modules_ENOCEAN;                                                /* Listes des modules ENOCEAN */
    GSList *Liste_sortie;                                              /* Liste des sorties a positionner */
 } Cfg_enocean;

/*************************************** Définitions des prototypes ***************************************/
 extern gboolean Enocean_Lire_config ( void );
 extern gboolean Retirer_enoceanDB ( gint id );
 extern gint Ajouter_enoceanDB ( struct ENOCEANDB *enocean );
 extern gboolean Modifier_enoceanDB( struct ENOCEANDB *enocean );

#endif
/*--------------------------------------------------------------------------------------------------------*/
