/**********************************************************************************************************/
/* Include/Reseaux_connexion.h:   Sous_tag de connexion utilisé pour watchdog 2.0   par lefevre Sebastien */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_connexion.h
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

#ifndef _RESEAUX_CONNEXION_H_
 #define _RESEAUX_CONNEXION_H_

 #include "Reseaux_utilisateur.h"

 struct REZO_CLI_IDENT
  { gchar  nom [ NBR_CARAC_LOGIN_UTF8 + 1 ];
    gchar  passwd  [24];
    gchar  version [32];
    gint   icone_version;
  };

 struct REZO_SRV_IDENT
  { gchar comment[80];
  };

 enum 
  { SSTAG_CLIENT_IDENT,                                               /* Le client s'identifie au serveur */

    SSTAG_SERVEUR_AUTORISE,                                       /* Le serveur autorise ou non le client */
    SSTAG_SERVEUR_REFUSE,                                         /* Le serveur autorise ou non le client */
    SSTAG_SERVEUR_ACCOUNT_DISABLED,                                /* Le compte utilisateur est desactivé */
    SSTAG_SERVEUR_ACCOUNT_EXPIRED,                                    /* Le compte utilisateur est expiré */
    SSTAG_SERVEUR_NEEDCHANGEPWD,                               /* L'utilisateur doit changer son password */
    SSTAG_CLIENT_SETPASSWORD,
    SSTAG_SERVEUR_CANNOTCHANGEPWD,                  /* L'utilisateur ne peut pas changer son mot de passe */
    SSTAG_SERVEUR_PWDCHANGED,                            /* L'utilisateur a bien modifié son mot de passe */
    SSTAG_SERVEUR_CLI_VALIDE,                       /* Le client est passé valide du point de vue serveur */
    SSTAG_CLIENT_OFF,                                                          /* Le client se deconnecte */
    SSTAG_SERVEUR_OFF,                                                             /* Le serveur se coupe */
    SSTAG_SERVEUR_PULSE,                                                            /* Keep alive serveur */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

