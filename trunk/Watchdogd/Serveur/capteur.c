/**********************************************************************************************************/
/* Watchdogd/Serveur/capteur.c                Formatage des capteurs Watchdog                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                     ven. 10 déc. 2010 17:13:43 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * capteur.c
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
 
 #include <glib.h>
 #include "Module_dls.h"                                                                /* Acces à E et B */

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Tester_update_capteur renvoie TRUE si le capteur doit etre updaté sur le client                        */
/* Entrée: un capteur                                                                                     */
/* Sortie: une structure prete à l'envoie                                                                 */
/**********************************************************************************************************/
 gboolean Tester_update_capteur( struct CAPTEUR *capteur )
  { if (!capteur) return(FALSE);
    switch(capteur->type)
     { case MNEMO_ENTREE:
            return( capteur->val_ech != E(capteur->bit_controle) );
       case MNEMO_BISTABLE:
            return( capteur->val_ech != B(capteur->bit_controle) );
       case MNEMO_ENTREE_ANA:
            return( TRUE );
       case MNEMO_CPTH:
            return( capteur->val_ech != Partage->ch[capteur->bit_controle].cpthdb.valeur );
       case MNEMO_CPT_IMP:
            return( capteur->val_ech != Partage->ci[capteur->bit_controle].confDB.valeur );
       default: return(FALSE);
     }
  }
/**********************************************************************************************************/
/* Formater_capteur: Formate la structure dédiée capteur pour envoi au client                             */
/* Entrée: un capteur                                                                                     */
/* Sortie: une structure prete à l'envoie                                                                 */
/**********************************************************************************************************/
 struct CMD_ETAT_BIT_CAPTEUR *Formater_capteur( struct CAPTEUR *capteur )
  { struct CMD_ETAT_BIT_CAPTEUR *etat_capteur;

    if (!capteur) return(NULL);

    etat_capteur = (struct CMD_ETAT_BIT_CAPTEUR *)g_try_malloc0( sizeof(struct CMD_ETAT_BIT_CAPTEUR) );
    if (!etat_capteur) return(NULL);

    etat_capteur->type         = capteur->type;
    etat_capteur->bit_controle = capteur->bit_controle;

    switch(capteur->type)
     { case MNEMO_BISTABLE:
            g_snprintf( etat_capteur->libelle, sizeof(etat_capteur->libelle),
                        "B%04d = %d", capteur->bit_controle, B(capteur->bit_controle)
                      );
            capteur->val_ech = B(capteur->bit_controle);
            return(etat_capteur);
       case MNEMO_ENTREE:
            g_snprintf( etat_capteur->libelle, sizeof(etat_capteur->libelle),
                        "E%04d = %d", capteur->bit_controle, E(capteur->bit_controle)
                      );
            capteur->val_ech = B(capteur->bit_controle);
            return(etat_capteur);
       case MNEMO_ENTREE_ANA:
            capteur->val_ech = Partage->ea[capteur->bit_controle].val_ech;
            if (EA_inrange(capteur->bit_controle))
             { if(-1000000.0<capteur->val_ech && capteur->val_ech<1000000.0)
                { g_snprintf( etat_capteur->libelle, sizeof(etat_capteur->libelle),
                             "%6.2f %s", capteur->val_ech,
                              Partage->ea[capteur->bit_controle].confDB.unite
                            );
                }
               else
                { g_snprintf( etat_capteur->libelle, sizeof(etat_capteur->libelle),
                             "%8.0f %s", capteur->val_ech,
                              Partage->ea[capteur->bit_controle].confDB.unite
                            );
                }
             }
            else
             { g_snprintf( etat_capteur->libelle, sizeof(etat_capteur->libelle),
                           " - pb comm - "
                         );
             }
            return(etat_capteur);
       case MNEMO_CPTH:
             { time_t valeur;
               valeur = (time_t)Partage->ch[capteur->bit_controle].cpthdb.valeur / 60;
               g_snprintf( etat_capteur->libelle, sizeof(etat_capteur->libelle),
                           "%05dh", (int)valeur
                         );
               capteur->val_ech = Partage->ch[capteur->bit_controle].cpthdb.valeur;
             }
            break;
       case MNEMO_CPT_IMP:
             { gfloat valeur;
               gchar *format;
               valeur = Partage->ci[capteur->bit_controle].confDB.valeur;
               valeur = valeur * (gfloat)Partage->ci[capteur->bit_controle].confDB.multi;      /* Multiplication ! */
               switch (Partage->ci[capteur->bit_controle].confDB.type)
                { case CI_TOTALISATEUR: format = "%8.0f %s"; break;
                  default:              format = "%8.2f %s"; break;
                }
               g_snprintf( etat_capteur->libelle, sizeof(etat_capteur->libelle),
                           format, valeur,
                           Partage->ci[capteur->bit_controle].confDB.unite
                         );
               capteur->val_ech = Partage->ci[capteur->bit_controle].confDB.valeur;
             }
            break;
       default:
            g_snprintf( etat_capteur->libelle, sizeof(etat_capteur->libelle),
                        "unknown"
                      );
            break;
      }
     return(etat_capteur);
  }
/*--------------------------------------------------------------------------------------------------------*/
