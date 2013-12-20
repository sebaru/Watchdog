/**********************************************************************************************************/
/* Include/Reseaux_master_slave.h:   Sous_tag des communications Master/Slave par lefevre Sebastien       */
/* Projet WatchDog version 2.0       Gestion d'habitat                    lun. 18 févr. 2013 18:39:03 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_master_slave.h
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

#ifndef _RESEAUX_MASTER_SLAVE_H_
 #define _RESEAUX_MASTER_SLAVE_H_

 struct CMD_TYPE_MASTER_SLAVE
  { 
  };

/************************************************* Tag de communication ***********************************/
 enum 
  { SSTAG_SLAVE_IDENT,                                                            /* Le slave s'identifie */
    SSTAG_SLAVE_OFF,                                                            /* Le slave se deconnecte */
    SSTAG_SLAVE_GIVE_INPUT,                                  /* Le slave nous donne l'etat de ses entrées */
    SSTAG_MASTER_SET_OUTPUT,                                    /* Le master envoie les sorties aux slave */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

