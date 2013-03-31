/**********************************************************************************************************/
/* Watchdogd/Master/Master.h        Déclaration structure internes des MASTER                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                    lun. 18 févr. 2013 20:06:29 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * master.h
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
 
#ifndef _MASTER_H_
 #define _MASTER_H_
 #include <libsoup/soup.h>

 #define NOM_TABLE_SLAVES  "slaves"

 struct SLAVEDB
  { guint id;                                                                    /* ID unique de la rs485 */
    guint bit_comm;                         /* Bit bistable correspondant au bon fonctionnement du module */
    gboolean enable;                                                            /* Module Start at boot ? */
    gint ea_min, ea_max;
    gint e_min, e_max;
    gint s_min, s_max;
    gint sa_min, sa_max;
    gchar libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8];                              /* Libelle de la rs485 */
    gchar ip[32];                                                                  /* Adresse IP du slave */
    guint port;
  };

 struct SLAVE
  { struct SLAVEDB slave;

    gboolean started;                                                           /* Module Start at boot ? */
    guint  nbr_query;
    guint  last_request;
  };

 struct MASTER_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean enable;                              /* True si la config indique que le thread doit tourner */
    gboolean master_enable;                                            /* True si le thread est un master */
    GSList *Liste_message;                                         /* liste de struct MSGDB msg a envoyer */
    GSList *Liste_sortie;                                          /* liste de struct MSGDB msg a envoyer */
    GSList *Slaves;                                                /* Leste des clients (slave) connectés */
    gint port;
    GMainContext *context;
    SoupServer *server;
 } Cfg_master;

/*************************************** Définitions des prototypes ***************************************/
#endif
/*--------------------------------------------------------------------------------------------------------*/
