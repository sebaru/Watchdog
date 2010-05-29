/**********************************************************************************************************/
/* Include/Cst_message.h        Déclaration des constantes dediées aux messages                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 11 aoû 2003 10:17:39 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Cst_message.h
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

#ifndef _CST_MESSAGE_H_
 #define _CST_MESSAGE_H_

 #define NBR_CARAC_LIBELLE_MSG    100                                                 /* Attention au SMS */
 #define NBR_CARAC_LIBELLE_MSG_UTF8  (6*NBR_CARAC_LIBELLE_MSG)
 #define NBR_CARAC_OBJET_MSG      30
 #define NBR_CARAC_OBJET_MSG_UTF8    (6*NBR_CARAC_OBJET_MSG)
 enum
  { MSG_ETAT,                                                        /* Definitions des types de messages */
    MSG_ALERTE,
    MSG_DEFAUT,
    MSG_ALARME,
    MSG_VEILLE,
    NBR_TYPE_MSG
  };
#endif
/*--------------------------------------------------------------------------------------------------------*/
