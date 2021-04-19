/******************************************************************************************************************************/
/* Client/supervision_cadran.c        Affichage des cadrans synoptique de supervision                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           mer 01 fév 2006 18:41:37 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * supervision_cadran.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sébastien Lefevre
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

 #include "Reseaux.h"
 #include "trame.h"
/***************************************** Définitions des prototypes programme ***********************************************/
 #include "protocli.h"

/******************************************************************************************************************************/
/* Clic_sur_camera_supervision: Appelé quand un evenement est capté sur une camera de supervision                             */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_cadran_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                    GdkEvent *event, struct TRAME_ITEM_CADRAN *trame_cadran )
  { if ( !(event->button.button == 1 &&                                                                     /* clic gauche ?? */
           event->type == GDK_BUTTON_PRESS)
       ) return;

    gint pid = fork();
    if (pid<0) return;
    else if (!pid)                                                                       /* Lancement de la ligne de commande */
     { gchar chaine[256];
       g_snprintf( chaine, sizeof(chaine),
                  "https://%s.abls-habitat.fr/archive/show/%s/%s/HOUR",
                   trame_cadran->page->client->hostname, trame_cadran->cadran->tech_id, trame_cadran->cadran->acronyme );
       execlp( "firefox", "firefox", chaine, NULL );
       printf("Lancement de firefox failed\n");
       _exit(0);
     }
  }
/******************************************************************************************************************************/
/* Met a jour le libelle d'un cadran                                                                                          */
/******************************************************************************************************************************/
 static void Updater_un_cadran ( struct TRAME_ITEM_CADRAN *trame_cadran, JsonNode *cadran )
  { gchar libelle[25];
    trame_cadran->valeur = Json_get_float ( cadran, "valeur" );
    switch(Json_get_int(cadran,"classe"))
     { case MNEMO_ENTREE:
       case MNEMO_BISTABLE:
            g_snprintf( libelle, sizeof(libelle), "%s", (trame_cadran->valeur ? "TRUE" : "FALSE") );
            break;
       case MNEMO_REGISTRE:
       case MNEMO_ENTREE_ANA:
            if (!Json_get_bool(cadran,"in_range")) g_snprintf(libelle, sizeof(libelle), "not in range" );
            else
             { gchar *digit, format[24];
               if(-1000000.0<trame_cadran->valeur && trame_cadran->valeur<1000000.0) digit = "%6"; else digit="%8";
               g_snprintf( format, sizeof(format), "%s.%df %%s", digit, trame_cadran->cadran->nb_decimal );
               g_snprintf( libelle, sizeof(libelle), format, trame_cadran->valeur, Json_get_string(cadran, "unite") );
             }
            break;
       case MNEMO_CPTH:
            if (trame_cadran->valeur < 3600)
             { g_snprintf( libelle, sizeof(libelle), "%02dm%02ds", (int)trame_cadran->valeur/60, ((int)trame_cadran->valeur%60) ); }
            else
             { g_snprintf( libelle, sizeof(libelle), "%04dh%02dm", (int)trame_cadran->valeur/3600, ((int)trame_cadran->valeur%3600)/60 ); }
            break;
       case MNEMO_CPT_IMP:
            g_snprintf( libelle, sizeof(libelle), "%8.2f %s", trame_cadran->valeur, Json_get_string(cadran, "unite") );
            break;
       case MNEMO_TEMPO:
             { gint src, heure, minute, seconde;
               src = trame_cadran->valeur/10;
               heure = src / 3600;
               minute = (src - heure*3600) / 60;
               seconde = src - heure*3600 - minute*60;
               g_snprintf( libelle, sizeof(libelle), "%02d:%02d:%02d", heure, minute, seconde );
             }
            break;
       default:
            g_snprintf( libelle, sizeof(libelle), "unknown" );
            break;
      }
    printf("%s: update cadran %s:%s to %s\n", __func__,
           trame_cadran->cadran->tech_id, trame_cadran->cadran->acronyme, libelle );
    g_object_set( trame_cadran->item_entry, "text", libelle, NULL );
  }
/******************************************************************************************************************************/
/* Proto_changer_etat_cadran: Rafraichissement du visuel cadran sur parametre                                                 */
/* Entrée: une reference sur le cadran                                                                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_les_cadrans( struct PAGE_NOTEBOOK *page, JsonNode *cadran )
  { struct TYPE_INFO_SUPERVISION *infos = page->infos;
    GList *liste_cadrans;
    gint cpt;

    liste_cadrans = infos->Trame->trame_items;                                     /* On parcours tous les cadrans de la page */
    while (liste_cadrans)
     { switch( *((gint *)liste_cadrans->data) )
        { case TYPE_CADRAN    :
           { cpt++;
             struct TRAME_ITEM_CADRAN *trame_cadran = liste_cadrans->data;
             if ( (!strcmp( Json_get_string(cadran,"tech_id"), trame_cadran->cadran->tech_id) &&
                   !strcmp( Json_get_string(cadran,"acronyme"), trame_cadran->cadran->acronyme))
                )
              { Updater_un_cadran ( trame_cadran, cadran ); }
             break;
           }
          default: break;
        }
       liste_cadrans=liste_cadrans->next;
     }
    if (!cpt)                                       /* Si nous n'avons rien mis à jour, alors nous demandons le desabonnement */
     { /*Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_CHANGE_CADRAN_UNKNOWN,
                      (gchar *)etat_cadran, sizeof(struct CMD_ETAT_BIT_CADRAN) );*/
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
