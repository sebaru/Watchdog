/******************************************************************************************************************************/
/* Watchdogd/Serveur/cadran.c                Formatage des cadrans Watchdog                                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         ven. 10 déc. 2010 17:13:43 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * cadran.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

/************************************************ Prototypes de fonctions *****************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
 extern struct SSRV_CONFIG Cfg_ssrv;
/******************************************************************************************************************************/
/* Tester_update_cadran renvoie TRUE si le cadran doit etre updaté sur le client                                            */
/* Entrée: un cadran                                                                                                         */
/* Sortie: une structure prete à l'envoie                                                                                     */
/******************************************************************************************************************************/
 gboolean Tester_update_cadran( struct CMD_ETAT_BIT_CADRAN *cadran )
  { if (!cadran) return(FALSE);
    switch(cadran->type)
     { case MNEMO_ENTREE:
       case MNEMO_BISTABLE:
        { gboolean valeur;
          valeur = Dls_data_get_bool ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
          return( cadran->valeur != valeur );
        }
       case MNEMO_TEMPO:
            return( TRUE );
       case MNEMO_ENTREE_ANA:
        { gfloat valeur;
          valeur = Dls_data_get_AI ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
          return( cadran->valeur != valeur );
        }
       case MNEMO_CPT_IMP:
        { gint valeur;
          valeur = Dls_data_get_CI ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
          return( cadran->valeur != valeur );
        }
       case MNEMO_CPTH:
        { gint valeur;
          valeur = Dls_data_get_CH ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
          return( cadran->valeur != valeur );
        }
       case MNEMO_REGISTRE:
       default: return(FALSE);
     }
  }
/******************************************************************************************************************************/
/* Formater_cadran: Formate la structure dédiée cadran pour envoi au client                                                   */
/* Entrée: un cadran                                                                                                          */
/* Sortie: une structure prete à l'envoie                                                                                     */
/******************************************************************************************************************************/
 void Formater_cadran( struct CMD_ETAT_BIT_CADRAN *cadran )
  {
    if (!cadran) return;

    if (cadran->dls_data == NULL)
     { struct DB *db;
       if ( (db=Rechercher_CI ( cadran->tech_id, cadran->acronyme )) != NULL )
        { cadran->type = MNEMO_CPT_IMP;
          Libere_DB_SQL (&db);
        }
       else if ( (db=Rechercher_AI ( cadran->tech_id, cadran->acronyme )) != NULL )
        { cadran->type = MNEMO_ENTREE_ANA;
          Libere_DB_SQL (&db);
        }
       else if ( (db=Rechercher_Tempo ( cadran->tech_id, cadran->acronyme )) != NULL )
        { cadran->type = MNEMO_TEMPO;
          Libere_DB_SQL (&db);
        }
       else if ( (db=Rechercher_CH ( cadran->tech_id, cadran->acronyme )) != NULL )
        { cadran->type = MNEMO_CPTH;
          Libere_DB_SQL (&db);
        }
       else return;                                                                                       /* Si pas trouvé... */
     }

    switch(cadran->type)
     { case MNEMO_BISTABLE:
            cadran->in_range = TRUE;
            cadran->valeur = 1.0 * Dls_data_get_bool ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
            break;
       case MNEMO_ENTREE:
            cadran->in_range = TRUE;
            cadran->valeur = 1.0 * Dls_data_get_DI ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
            break;
       case MNEMO_ENTREE_ANA:
             { struct DLS_AI *ai;
               cadran->valeur = Dls_data_get_AI(cadran->tech_id, cadran->acronyme, &cadran->dls_data );
               if (!cadran->dls_data)                            /* si AI pas trouvée, on remonte le nom du cadran en libellé */
                { cadran->in_range = FALSE;
                  break;
                }
               ai = (struct DLS_AI *)cadran->dls_data;
               cadran->in_range = ai->inrange;
               cadran->valeur = ai->val_ech;
               g_snprintf( cadran->unite, sizeof(cadran->unite), "%s", ai->unite );
             }
            break;
       case MNEMO_CPTH:
             { cadran->in_range = TRUE;
               cadran->valeur = Dls_data_get_CH(cadran->tech_id, cadran->acronyme, &cadran->dls_data );
             }
            break;
       case MNEMO_CPT_IMP:
             { cadran->valeur = Dls_data_get_CI(cadran->tech_id, cadran->acronyme, &cadran->dls_data );
               struct DLS_CI *ci=cadran->dls_data;
               if (!ci)                                       /* si AI pas trouvée, on remonte le nom du cadran en libellé */
                { cadran->in_range = FALSE;
                  break;
                }
               cadran->in_range = TRUE;
               cadran->valeur *= ci->multi;                                                            /* Multiplication ! */
               g_snprintf( cadran->unite, sizeof(cadran->unite), "%s", ci->unite );
             }
            break;
       case MNEMO_REGISTRE:
            cadran->valeur = -1.0;
            g_snprintf( cadran->unite, sizeof(cadran->unite), "?" );
            cadran->in_range = TRUE;
            break;
       case MNEMO_TEMPO:
            Dls_data_get_tempo ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
            struct DLS_TEMPO *tempo = cadran->dls_data;
            if (!tempo)
             { cadran->in_range = FALSE;
               break;
             }
            cadran->in_range = FALSE;

            if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_ON)                     /* Temporisation Retard en train de compter */
             { cadran->valeur = (tempo->date_on - Partage->top); }
            else if (tempo->status == DLS_TEMPO_NOT_COUNTING)                  /* Tempo ne compte pas: on affiche la consigne */
             { cadran->valeur = tempo->delai_on; }
            break;
       default:
            cadran->in_range = FALSE;
            break;
      }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
