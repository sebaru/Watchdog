/**********************************************************************************************************/
/* Include/Reseaux_scenario.h:   Sous_tag de scenario pour watchdog 2.0 par lefevre Sebastien             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 03 aoû 2008 17:06:44 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_SCENARIO_H_
 #define _RESEAUX_SCENARIO_H_

 #include "Cst_scenario.h"

 struct CMD_SHOW_SCENARIO
  { guint  id;                                                                      /* Numero du compteur */
    guint  actif;                                                                   /* Valeur du compteur */
    guint  bit_m;                                                                   /* Valeur du compteur */
    gboolean ts_jour;                                                               /* Valeur du compteur */
    gboolean ts_mois;                                                               /* Valeur du compteur */
    gboolean lundi;                                                                 /* Valeur du compteur */
    gboolean mardi;                                                                 /* Valeur du compteur */
    gboolean mercredi;                                                              /* Valeur du compteur */
    gboolean jeudi;                                                                 /* Valeur du compteur */
    gboolean vendredi;                                                              /* Valeur du compteur */
    gboolean samedi;                                                                /* Valeur du compteur */
    gboolean dimanche;                                                              /* Valeur du compteur */
    gboolean janvier;                                                               /* Valeur du compteur */
    gboolean fevrier;                                                               /* Valeur du compteur */
    gboolean mars;                                                                  /* Valeur du compteur */
    gboolean avril;                                                                 /* Valeur du compteur */
    gboolean mai;                                                                   /* Valeur du compteur */
    gboolean juin;                                                                  /* Valeur du compteur */
    gboolean juillet;                                                               /* Valeur du compteur */
    gboolean aout;                                                                  /* Valeur du compteur */
    gboolean septembre;                                                             /* Valeur du compteur */
    gboolean octobre;                                                               /* Valeur du compteur */
    gboolean novembre;                                                              /* Valeur du compteur */
    gboolean decembre;                                                              /* Valeur du compteur */
    guint  heure;                                                                   /* Valeur du compteur */
    guint  minute;                                                                  /* Valeur du compteur */
    guchar libelle[NBR_CARAC_LIBELLE_SCENARIO_UTF8+1];
  };
 struct CMD_EDIT_SCENARIO
  { guint  id;                                                                      /* Numero du compteur */
    guint  actif;                                                                   /* Valeur du compteur */
    guint  bit_m;                                                                   /* Valeur du compteur */
    gboolean ts_jour;                                                               /* Valeur du compteur */
    gboolean ts_mois;                                                               /* Valeur du compteur */
    gboolean lundi;                                                                 /* Valeur du compteur */
    gboolean mardi;                                                                 /* Valeur du compteur */
    gboolean mercredi;                                                              /* Valeur du compteur */
    gboolean jeudi;                                                                 /* Valeur du compteur */
    gboolean vendredi;                                                              /* Valeur du compteur */
    gboolean samedi;                                                                /* Valeur du compteur */
    gboolean dimanche;                                                              /* Valeur du compteur */
    gboolean janvier;                                                               /* Valeur du compteur */
    gboolean fevrier;                                                               /* Valeur du compteur */
    gboolean mars;                                                                  /* Valeur du compteur */
    gboolean avril;                                                                 /* Valeur du compteur */
    gboolean mai;                                                                   /* Valeur du compteur */
    gboolean juin;                                                                  /* Valeur du compteur */
    gboolean juillet;                                                               /* Valeur du compteur */
    gboolean aout;                                                                  /* Valeur du compteur */
    gboolean septembre;                                                             /* Valeur du compteur */
    gboolean octobre;                                                               /* Valeur du compteur */
    gboolean novembre;                                                              /* Valeur du compteur */
    gboolean decembre;                                                              /* Valeur du compteur */
    guint  heure;                                                                   /* Valeur du compteur */
    guint  minute;                                                                  /* Valeur du compteur */
    guchar libelle[NBR_CARAC_LIBELLE_SCENARIO_UTF8+1];
  };
 struct CMD_ADD_SCENARIO
  { guint  id;                                                                      /* Numero du compteur */
    guint  actif;                                                                   /* Valeur du compteur */
    guint  bit_m;                                                                   /* Valeur du compteur */
    gboolean ts_jour;                                                               /* Valeur du compteur */
    gboolean ts_mois;                                                               /* Valeur du compteur */
    gboolean lundi;                                                                 /* Valeur du compteur */
    gboolean mardi;                                                                 /* Valeur du compteur */
    gboolean mercredi;                                                              /* Valeur du compteur */
    gboolean jeudi;                                                                 /* Valeur du compteur */
    gboolean vendredi;                                                              /* Valeur du compteur */
    gboolean samedi;                                                                /* Valeur du compteur */
    gboolean dimanche;                                                              /* Valeur du compteur */
    gboolean janvier;                                                               /* Valeur du compteur */
    gboolean fevrier;                                                               /* Valeur du compteur */
    gboolean mars;                                                                  /* Valeur du compteur */
    gboolean avril;                                                                 /* Valeur du compteur */
    gboolean mai;                                                                   /* Valeur du compteur */
    gboolean juin;                                                                  /* Valeur du compteur */
    gboolean juillet;                                                               /* Valeur du compteur */
    gboolean aout;                                                                  /* Valeur du compteur */
    gboolean septembre;                                                             /* Valeur du compteur */
    gboolean octobre;                                                               /* Valeur du compteur */
    gboolean novembre;                                                              /* Valeur du compteur */
    gboolean decembre;                                                              /* Valeur du compteur */
    guint  heure;                                                                   /* Valeur du compteur */
    guint  minute;                                                                  /* Valeur du compteur */
    guchar libelle[NBR_CARAC_LIBELLE_SCENARIO_UTF8+1];
  };
 struct CMD_ID_SCENARIO
  { guint id;                                                      /* Numero du scenario dans la structure */
    guchar libelle[NBR_CARAC_LIBELLE_SCENARIO_UTF8+1];
  };

 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_SCENARIO,                        /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_SCENARIO_FIN,                    /* Ajout d'un groupe dans la liste cliente */
    SSTAG_CLIENT_WANT_PAGE_SCENARIO,
    SSTAG_SERVEUR_CREATE_PAGE_SCENARIO_OK,                               /* Affichage de la page scenario */
    SSTAG_CLIENT_ADD_SCENARIO,                             /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_SCENARIO_OK,                                     /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_SCENARIO,                                      /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_SCENARIO_OK,                                     /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_SCENARIO,                                /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_SCENARIO_OK,            /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_SCENARIO,                           /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_SCENARIO_OK,                     /* Le serveur valide les nouvelles données */
   };

#endif
/*--------------------------------------------------------------------------------------------------------*/

