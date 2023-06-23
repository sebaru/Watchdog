/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_archive.c  Gestion des archives                                                                      */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    27.11.2022 18:09:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_archive.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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

 #include <glib.h>
 #include <fcntl.h>
 #include <string.h>
 #include <signal.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <semaphore.h>
 #include <locale.h>
 #include <math.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_run_archivage: Gere l'archivage des bits internes le necessitant                                                       */
/* Entrée : le plugin a traiter                                                                                               */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 void Dls_run_archivage ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { if (!plugin) return;
    GSList *liste = plugin->Dls_data_AI;
    while ( liste )
     { struct DLS_AI *bit = liste->data;
       if ( (bit->archivage == 1 && bit->last_arch + 50     <= Partage->top) ||                      /* Toutes les 5 secondes */
            (bit->archivage == 2 && bit->last_arch + 600    <= Partage->top) ||                         /* Toutes les minutes */
            (bit->archivage == 3 && bit->last_arch + 36000  <= Partage->top) ||                          /* Toutes les heures */
            (bit->archivage == 4 && bit->last_arch + 864000 <= Partage->top) || bit->last_arch == 0         /* Tous les jours */
          )
        { Ajouter_arch( bit->tech_id, bit->acronyme, (bit->in_range ? bit->valeur : 0.0) );            /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_AO;
    while ( liste )
     { struct DLS_AO *bit = liste->data;
       if ( (bit->archivage == 1 && bit->last_arch + 50     <= Partage->top) ||                      /* Toutes les 5 secondes */
            (bit->archivage == 2 && bit->last_arch + 600    <= Partage->top) ||                         /* Toutes les minutes */
            (bit->archivage == 3 && bit->last_arch + 36000  <= Partage->top) ||                          /* Toutes les heures */
            (bit->archivage == 4 && bit->last_arch + 864000 <= Partage->top) || bit->last_arch == 0         /* Tous les jours */
          )
        { Ajouter_arch( bit->tech_id, bit->acronyme, bit->valeur );                                    /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_CI;
    while ( liste )
     { struct DLS_CI *bit = liste->data;
       if ( (bit->archivage == 1 && bit->last_arch + 50     <= Partage->top) ||                      /* Toutes les 5 secondes */
            (bit->archivage == 2 && bit->last_arch + 600    <= Partage->top) ||                         /* Toutes les minutes */
            (bit->archivage == 3 && bit->last_arch + 36000  <= Partage->top) ||                          /* Toutes les heures */
            (bit->archivage == 4 && bit->last_arch + 864000 <= Partage->top) || bit->last_arch == 0         /* Tous les jours */
          )
        { Ajouter_arch( bit->tech_id, bit->acronyme, bit->valeur*1.0 );                                /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_CH;
    while ( liste )
     { struct DLS_CH *bit = liste->data;
       if ( (bit->archivage == 1 && bit->last_arch + 50     <= Partage->top) ||                      /* Toutes les 5 secondes */
            (bit->archivage == 2 && bit->last_arch + 600    <= Partage->top) ||                         /* Toutes les minutes */
            (bit->archivage == 3 && bit->last_arch + 36000  <= Partage->top) ||                          /* Toutes les heures */
            (bit->archivage == 4 && bit->last_arch + 864000 <= Partage->top) || bit->last_arch == 0         /* Tous les jours */
          )
        { Ajouter_arch( bit->tech_id, bit->acronyme, bit->valeur*1.0 );                                /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_REGISTRE;
    while ( liste )
     { struct DLS_REGISTRE *bit = liste->data;
       if ( (bit->archivage == 1 && bit->last_arch + 50     <= Partage->top) ||                      /* Toutes les 5 secondes */
            (bit->archivage == 2 && bit->last_arch + 600    <= Partage->top) ||                         /* Toutes les minutes */
            (bit->archivage == 3 && bit->last_arch + 36000  <= Partage->top) ||                          /* Toutes les heures */
            (bit->archivage == 4 && bit->last_arch + 864000 <= Partage->top) || bit->last_arch == 0         /* Tous les jours */
          )
        { Ajouter_arch( bit->tech_id, bit->acronyme, bit->valeur );                                    /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
