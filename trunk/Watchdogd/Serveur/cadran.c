/******************************************************************************************************************************/
/* Watchdogd/Serveur/cadran.c                Formatage des cadrans Watchdog                                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         ven. 10 déc. 2010 17:13:43 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * cadran.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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
/******************************************************************************************************************************/
/* Tester_update_cadran renvoie TRUE si le cadran doit etre updaté sur le client                                            */
/* Entrée: un cadran                                                                                                         */
/* Sortie: une structure prete à l'envoie                                                                                     */
/******************************************************************************************************************************/
 gboolean Tester_update_cadran( struct CADRAN *cadran )
  { if (!cadran) return(FALSE);
    if (cadran->bit_controle!=-1)                                                                   /* Ancienne mode statique */
     { switch(cadran->type)
        { case MNEMO_ENTREE:
               return( cadran->val_ech != E(cadran->bit_controle) );
          case MNEMO_BISTABLE:
               return( cadran->val_ech != B(cadran->bit_controle) );
          case MNEMO_ENTREE_ANA:
               return( TRUE );
          case MNEMO_CPTH:
               return( cadran->val_ech != Partage->ch[cadran->bit_controle].confDB.valeur );
          case MNEMO_CPT_IMP:
               return( cadran->val_ech != Partage->ci[cadran->bit_controle].confDB.valeur );
          case MNEMO_REGISTRE:
               return( cadran->val_ech != Partage->registre[cadran->bit_controle].val );
          default: return(FALSE);
        }
     }

    switch(cadran->type)
     { case MNEMO_ENTREE:
       case MNEMO_BISTABLE:
        { gboolean valeur;
          valeur = Dls_data_get_bool ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
          return( cadran->val_ech != valeur );
        }
       case MNEMO_ENTREE_ANA:
            return( TRUE );
       case MNEMO_CPT_IMP:
        { gint valeur;
          valeur = Dls_data_get_CI ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
          return( cadran->val_ech != valeur );
        }
       case MNEMO_CPTH:
       case MNEMO_REGISTRE:
       default: return(FALSE);
     }
  }
/******************************************************************************************************************************/
/* Formater_cadran: Formate la structure dédiée cadran pour envoi au client                                                 */
/* Entrée: un cadran                                                                                                         */
/* Sortie: une structure prete à l'envoie                                                                                     */
/******************************************************************************************************************************/
 struct CMD_ETAT_BIT_CADRAN *Formater_cadran( struct CADRAN *cadran )
  { struct CMD_ETAT_BIT_CADRAN *etat_cadran;

    if (!cadran) return(NULL);

    if (cadran->bit_controle==-1 && cadran->dls_data == NULL)
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
       else cadran->bit_controle = 0;
     }
    etat_cadran = (struct CMD_ETAT_BIT_CADRAN *)g_try_malloc0( sizeof(struct CMD_ETAT_BIT_CADRAN) );
    if (!etat_cadran) return(NULL);

    etat_cadran->type         = cadran->type;
    etat_cadran->bit_controle = cadran->bit_controle;
    g_snprintf( etat_cadran->tech_id, sizeof(etat_cadran->tech_id), "%s", cadran->tech_id );
    g_snprintf( etat_cadran->acronyme, sizeof(etat_cadran->acronyme), "%s", cadran->acronyme );

    switch(cadran->type)
     { case MNEMO_BISTABLE:
            if (cadran->bit_controle!=-1)
             { g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle),
                           "B%04d = %d", cadran->bit_controle, B(cadran->bit_controle)
                         );
               cadran->val_ech = B(cadran->bit_controle);
             }
            else
             { gboolean valeur;
               valeur = Dls_data_get_bool ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
               g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle), "B = %d", valeur );
               cadran->val_ech = valeur;
             }
            return(etat_cadran);
       case MNEMO_ENTREE:
            if (cadran->bit_controle!=-1)
             { g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle),
                           "E%04d = %d", cadran->bit_controle, E(cadran->bit_controle)
                         );
               cadran->val_ech = B(cadran->bit_controle);
             }
            else
             { gboolean valeur;
               valeur = Dls_data_get_bool ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
               g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle), "E = %d", valeur );
               cadran->val_ech = valeur;
             }
            return(etat_cadran);
       case MNEMO_ENTREE_ANA:
            if(cadran->bit_controle!=-1)
             { cadran->val_ech = Partage->ea[cadran->bit_controle].val_ech; }
            else
             { cadran->val_ech = Dls_data_get_AI(cadran->tech_id, cadran->acronyme, &cadran->dls_data ); }
            if ( cadran->bit_controle!=-1 && EA_inrange(cadran->bit_controle))
             { if(-1000000.0<cadran->val_ech && cadran->val_ech<1000000.0)
                { g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle),
                             "%6.2f %s", cadran->val_ech,
                              Partage->ea[cadran->bit_controle].confDB.unite
                            );
                }
               else
                { g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle),
                             "%8.0f %s", cadran->val_ech,
                              Partage->ea[cadran->bit_controle].confDB.unite
                            );
                }
             }
            else if ( cadran->bit_controle==-1 )
             { if(-1000000.0<cadran->val_ech && cadran->val_ech<1000000.0)
                { g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle),
                             "%6.2f %s", cadran->val_ech,
                              ((struct ANALOG_INPUT *)cadran->dls_data)->confDB.unite
                            );
                }
               else
                { g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle),
                             "%8.0f %s", cadran->val_ech,
                              ((struct ANALOG_INPUT *)cadran->dls_data)->confDB.unite
                            );
                }
             }
            else
             { g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle),
                           " - pb comm - "
                         );
             }
            return(etat_cadran);
       case MNEMO_CPTH:
             { time_t valeur;
               valeur = (time_t)Partage->ch[cadran->bit_controle].confDB.valeur;
               if (valeur < 60)
                { g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle), "%02d min", (int)valeur ); }
                else
                { g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle), "%05dh", (int)valeur/60 ); }
               cadran->val_ech = Partage->ch[cadran->bit_controle].confDB.valeur;
             }
            break;
       case MNEMO_CPT_IMP:
             { gfloat valeur;
               gchar *format;
               if(cadran->bit_controle==-1) break;
               valeur = Partage->ci[cadran->bit_controle].confDB.valeur;
               valeur = valeur * (gfloat)Partage->ci[cadran->bit_controle].confDB.multi;                 /* Multiplication ! */
               switch (Partage->ci[cadran->bit_controle].confDB.type)
                { case CI_TOTALISATEUR: format = "%8.0f %s"; break;
                  default:              format = "%8.2f %s"; break;
                }
               g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle), format, valeur,
                           Partage->ci[cadran->bit_controle].confDB.unite
                         );
               cadran->val_ech = Partage->ci[cadran->bit_controle].confDB.valeur;
             }
            break;
       case MNEMO_REGISTRE:
            if(cadran->bit_controle==-1) break;
            cadran->val_ech = Partage->registre[cadran->bit_controle].val;
            if(-1000000.0<cadran->val_ech && cadran->val_ech<1000000.0)
             { g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle), "%6.2f %s", cadran->val_ech,
                           Partage->registre[cadran->bit_controle].confDB.unite
                         );
             }
            else
             { g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle), "%8.0f %s", cadran->val_ech,
                           Partage->registre[cadran->bit_controle].confDB.unite
                         );
             }
            return(etat_cadran);
       case MNEMO_TEMPO:
            if(cadran->bit_controle!=-1) break;
            Dls_data_get_tempo ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
            struct DLS_TEMPO *tempo = cadran->dls_data;
            if (!tempo) break;

            if (tempo->status)
             { gint src, heure, minute, seconde;
               src = (tempo->date_off - tempo->date_on)/10;
               heure = src / 3600;
               minute = (src % 3600) / 60;
               seconde = (src % (3600*60));
               g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle), "%02d:%02d:%02d", heure, minute, seconde );
             } 
            else
             { gint src, heure, minute, seconde;
               if (tempo->confDB.delai_on) src = tempo->confDB.delai_on/10;
               else src = tempo->confDB.min_on/10;
               heure = src / 3600;
               minute = (src % 3600) / 60;
               seconde = (src % (3600*60));
               g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle), "%02d:%02d:%02d", heure, minute, seconde );
             } 
            return(etat_cadran);
       default:
            g_snprintf( etat_cadran->libelle, sizeof(etat_cadran->libelle), "unknown" );
            break;
      }
     return(etat_cadran);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
