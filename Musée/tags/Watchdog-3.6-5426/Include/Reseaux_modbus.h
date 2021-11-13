/**********************************************************************************************************/
/* Include/Reseaux_modbus.h:   Sous_tag de gestion des modbus pour watchdog 2.0 par lefevre Sebastien     */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 05 sept. 2010 12:07:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_modbus.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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

#ifndef _RESEAUX_MODBUS_H_
 #define _RESEAUX_MODBUS_H_

/********************************************* Gestion des modbus ******************************************/
 struct CMD_TYPE_MODBUS
  { guint id;                                                 /* Numéro du module dans la base de données */
    gboolean actif;                                                        /* Le module doit-il tourner ? */
    guint watchdog;                       /* Le module doit-il etre auto-supervisé ? en dixeme de seconde */
    guint bit;                                       /* Bit interne B d'etat communication avec le module */
    gchar ip[32];                                                         /* Adresses IP du module MODBUS */
    gchar libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8];                             /* Libelle de la modbus */
    guint min_e_tor;
    guint min_e_ana;
    guint min_s_tor;
    guint min_s_ana;
  };

/************************************************* Tag de communication ***********************************/
 enum 
  { SSTAG_CLIENT_WANT_PAGE_MODBUS,
    SSTAG_SERVEUR_CREATE_PAGE_MODBUS_OK,                                   /* Affichage de la page modbus */
    SSTAG_SERVEUR_ADDPROGRESS_MODBUS,                          /* Ajout d'un modbus dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_MODBUS_FIN,                /* Ajout du dernier modbus dans la liste cliente */

    SSTAG_CLIENT_ADD_MODBUS,                               /* Le client desire ajouter un modbus watchdog */
    SSTAG_SERVEUR_ADD_MODBUS_OK,                                       /* L'ajout du modbus est un succes */

    SSTAG_CLIENT_DEL_MODBUS,                                        /* Le client desire retirer un modbus */
    SSTAG_SERVEUR_DEL_MODBUS_OK,                                /* La suppression du modbus est un succes */

    SSTAG_CLIENT_EDIT_MODBUS,                                  /* Le client demande l'edition d'un modbus */
    SSTAG_SERVEUR_EDIT_MODBUS_OK,              /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_MODBUS,                             /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_MODBUS_OK,                       /* Le serveur valide les nouvelles données */

    SSTAG_CLIENT_TYPE_NUM_MNEMO_MODBUS,                    /* Le client souhaite le mnemonique bit_motion */
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_MODBUS,                          /* Le serveur répond avec le mnemonique */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

