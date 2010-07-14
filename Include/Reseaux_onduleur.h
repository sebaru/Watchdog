/**********************************************************************************************************/
/* Include/Reseaux_onduleur.h:   Sous_tag de onduleur pour watchdog 2.0 par lefevre Sebastien             */
/* Projet WatchDog version 2.0       Gestion d'habitat                   mer. 14 juil. 2010 19:01:54 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_onduleur.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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

#ifndef _RESEAUX_ONDULEUR_H_
 #define _RESEAUX_ONDULEUR_H_

 #define NBR_CARAC_HOST_ONDULEUR           32
 #define NBR_CARAC_HOST_ONDULEUR_UTF8      (6*NBR_CARAC_HOST_ONDULEUR)

 #define NBR_CARAC_UPS_ONDULEUR            32
 #define NBR_CARAC_UPS_ONDULEUR_UTF8       (6*NBR_CARAC_UPS_ONDULEUR)

 #define NBR_CARAC_LIBELLE_ONDULEUR        60
 #define NBR_CARAC_LIBELLE_ONDULEUR_UTF8   (6*NBR_CARAC_LIBELLE_ONDULEUR)

 struct CMD_TYPE_ONDULEUR
  { gint id;                                                  /* Numéro du module dans la base de données */
    gboolean actif;                                                        /* Le module doit-il tourner ? */
    gchar host[NBR_CARAC_HOST_ONDULEUR_UTF8+1];                         /* Adresses IP du module ONDULEUR */
    gchar ups[NBR_CARAC_UPS_ONDULEUR_UTF8+1];                                 /* Nom de l'UPS sur le HOST */
    gchar libelle[NBR_CARAC_LIBELLE_ONDULEUR_UTF8+1];                                  /* Libelle associé */
    guint bit_comm;                                  /* Bit interne B d'etat communication avec le module */
    guint ea_ups_load;                                                     /* Numéro de l'EA pour le load */
    guint ea_ups_real_power;                                         /* Numéro de l'EA pour le real power */
    guint ea_battery_charge;                                    /* Numéro de l'EA pour la charge batterie */
    guint ea_input_voltage;                                        /* Numéro de l'EA pour l'input voltage */
  };

 enum 
  { SSTAG_CLIENT_WANT_PAGE_ONDULEUR,
    SSTAG_SERVEUR_ADDPROGRESS_ONDULEUR,                         /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_ONDULEUR_FIN,                     /* Ajout d'un groupe dans la liste cliente */

    SSTAG_CLIENT_ADD_ONDULEUR,                              /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_ONDULEUR_OK,                                      /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_ONDULEUR,                                       /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_ONDULEUR_OK,                                      /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_ONDULEUR,                                 /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_ONDULEUR_OK,             /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_ONDULEUR,                            /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_ONDULEUR_OK,                      /* Le serveur valide les nouvelles données */

    SSTAG_CLIENT_TYPE_NUM_MNEMO_ONDULEUR,
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_ONDULEUR,
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

