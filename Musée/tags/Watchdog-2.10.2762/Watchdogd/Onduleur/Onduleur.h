/**********************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.h   Header et constantes des modules UPS Watchdgo 2.0                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 10 nov. 2009 16:35:20 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Onduleur.h
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
 
#ifndef _UPS_H_
 #define _UPS_H_
 #include <upsclient.h>
 #include "Reseaux.h"

 #define NOM_THREAD      "ups"
 #define NOM_TABLE_UPS   "ups"

 #define UPS_PORT_TCP    3493                             /* Port de connexion TCP pour accès aux modules */
 #define UPS_RETRY       1800                          /* 3 minutes entre chaque retry si pb de connexion */
 #define UPS_POLLING      100                  /* Si tout va bien, on s'y connecte toutes les 10 secondes */

 struct UPS_CONFIG                                                    /* Communication entre DLS et l'UPS */
  { struct LIBRAIRIE *lib;
    GSList *Modules_UPS;
    gboolean enable;                                                           /* Thread enable at boot ? */
    gboolean reload;                                       /* Pour le rechargement des modules en mémoire */
    guint admin_start;                                                          /* Demande de deconnexion */
    guint admin_stop;                                                           /* Demande de deconnexion */
  } Cfg_ups;

 #define NBR_CARAC_HOST_UPS           32
 #define NBR_CARAC_HOST_UPS_UTF8      (2*NBR_CARAC_HOST_UPS)

 #define NBR_CARAC_UPS_UPS            32
 #define NBR_CARAC_UPS_UPS_UTF8       (2*NBR_CARAC_UPS_UPS)

 #define NBR_CARAC_LIBELLE_UPS        60
 #define NBR_CARAC_LIBELLE_UPS_UTF8   (2*NBR_CARAC_LIBELLE_UPS)

 #define NBR_CARAC_USERNAME_UPS        20
 #define NBR_CARAC_USERNAME_UPS_UTF8   (2*NBR_CARAC_USERNAME_UPS)

 #define NBR_CARAC_PASSWORD_UPS        20
 #define NBR_CARAC_PASSWORD_UPS_UTF8   (2*NBR_CARAC_PASSWORD_UPS)

 struct UPSDB
  { gint id;                                                  /* Numéro du module dans la base de données */
    gboolean enable;                                               /* Le module doit-il tourner au boot ? */
    gchar host[NBR_CARAC_HOST_UPS_UTF8+1];                                   /* Adresses IP du module UPS */
    gchar ups[NBR_CARAC_UPS_UPS_UTF8+1];                                      /* Nom de l'UPS sur le HOST */
    gchar username[NBR_CARAC_USERNAME_UPS_UTF8+1];                                    /* Username associé */
    gchar password[NBR_CARAC_PASSWORD_UPS_UTF8+1];                                    /* Password associé */
    guint bit_comm;                                  /* Bit interne B d'etat communication avec le module */
    guint map_EA;                                                    /* Numéro de la premiere EA impactée */
    guint map_E;                                                      /* Numéro de la premiere E impactée */
    guint map_A;                                                      /* Numéro de la premiere A impactée */
  };

 struct MODULE_UPS
  { struct UPSDB ups;
    gchar libelle[NBR_CARAC_LIBELLE_UPS_UTF8+1];                                       /* Libelle associé */

    UPSCONN_t upsconn;                                                           /* Connexion UPS à l'ups */
    gboolean started;                                                                  /* Est-il actif ?? */
    time_t date_next_connexion;
  };

/*********************************************** Déclaration des prototypes *******************************/
 extern gboolean Ups_Lire_config ( void );
 extern gboolean Retirer_upsDB ( struct UPSDB *ups );
 extern gint Ajouter_upsDB ( struct UPSDB *ups );
 extern gboolean Modifier_upsDB( struct UPSDB *ups );
 extern struct MODULE_UPS *Chercher_module_ups_by_id ( gint id );

#endif
/*--------------------------------------------------------------------------------------------------------*/

