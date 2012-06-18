/**********************************************************************************************************/
/* Watchdogd/Include/Rfxcom.h        Déclaration structure internes des communication RFXCOM              */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 27 mai 2012 13:02:55 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Rfxcom.h
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
 
#ifndef _RFXCOM_H_
 #define _RFXCOM_H_

 #define NOM_TABLE_MODULE_RFXCOM   "rfxcom"

 #define TAILLE_ENTETE_RFXCOM    1

 struct TRAME_RFXCOM                                                     /* Definition d'une trame RFXCOM */
  { unsigned char taille;
    unsigned char type;
    unsigned char sous_type;
    unsigned char seqno;
    unsigned char data[40];
  };

 struct RFXCOMDB
  { guint id;                                                                   /* ID unique de la rfxcom */
    guchar type;                                                                   /* Numéro de la rfxcom */
    guchar canal;                                                                  /* Numéro de la rfxcom */
    gint e_min, ea_min, a_min;
    gchar libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8];                              /* Libelle de la rfxcom */
  };

 struct MODULE_RFXCOM
  { struct RFXCOMDB rfxcom;
    time_t date_last_view;
  };


 struct COM_RFXCOM                                                           /* Communication vers RFXCOM */
  { pthread_t TID;                                                               /* Identifiant du thread */
    void *dl_handle;                                          /* handle de gestion de la librairie rfxcom */

    void (*Run_rfxcom)(void);                                 /* Fonction principale de gestion du thread */
    gint     (*Ajouter_rfxcomDB)   ( struct RFXCOMDB *rfxcom );
    gboolean (*Retirer_rfxcomDB)   ( gint id );
    gboolean (*Modifier_rfxcomDB)  ( struct RFXCOMDB *rfxcom );

    pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *Modules_RFXCOM;                                                   /* Listes des modules RFXCOM */
    struct RFXCOM_SEND_LEARN
     { gint id1, id2, id3, id4, unitcode, cmd, level;
     } learn;
    gboolean Thread_run;                /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean Thread_sigusr1;                                      /* TRUE si le thread doit gerer le USR1 */
    gboolean Thread_commande;                         /* TRUE si le thread doit enoyer une commande learn */
  };

/*************************************** Définitions des prototypes ***************************************/
#endif
/*--------------------------------------------------------------------------------------------------------*/
