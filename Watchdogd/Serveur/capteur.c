/**********************************************************************************************************/
/* Watchdogd/Serveur/capteur.c                Formatage des capteurs Watchdog                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       lun 20 fév 2006 18:07:44 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include "Module_dls.h"                                                                /* Acces à E et B */

 #include "watchdogd.h"
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

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
            return( capteur->val_ech != Partage->ea[capteur->bit_controle].val_ech );
       case MNEMO_CPTH:
            return( capteur->val_ech != Partage->ch[capteur->bit_controle].cpthdb.valeur );
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
    time_t valeur;

    if (!capteur) return(NULL);

    etat_capteur = (struct CMD_ETAT_BIT_CAPTEUR *)g_malloc0( sizeof(struct CMD_ETAT_BIT_CAPTEUR) );
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
            if (EA_inrange(capteur->bit_controle))
             { g_snprintf( etat_capteur->libelle, sizeof(etat_capteur->libelle),
                           "%8.2f %s", EA_ech(capteur->bit_controle),
                           Unite_vers_string(Partage->ea[capteur->bit_controle].unite)
                         );
             }
            else
             { g_snprintf( etat_capteur->libelle, sizeof(etat_capteur->libelle),
                           "- pb boucle -"
                         );
             }
            capteur->val_ech = Partage->ea[capteur->bit_controle].val_ech;
            return(etat_capteur);
       case MNEMO_CPTH:
            valeur = (time_t)Partage->ch[capteur->bit_controle].cpthdb.valeur / 60;
            g_snprintf( etat_capteur->libelle, sizeof(etat_capteur->libelle),
                        "%05dh", (int)valeur
                      );
            capteur->val_ech = Partage->ch[capteur->bit_controle].cpthdb.valeur;
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
