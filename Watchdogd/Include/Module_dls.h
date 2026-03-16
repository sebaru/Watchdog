/******************************************************************************************************************************/
/* Watchdogd/Include/Module_dls.h -> Déclaration des prototypes de fonctions                                                  */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                      jeu 31 jui 2003 11:49:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Module_dls.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

 #ifndef _MODULE_DLS_H_
 #define _MODULE_DLS_H_
 #include <glib.h>
 #include <json-glib/json-glib.h>

 enum                                                                                  /* différent statut des temporisations */
  { DLS_TEMPO_NOT_COUNTING,                                                                 /* La tempo ne compte pas du tout */
    DLS_TEMPO_WAIT_FOR_DELAI_ON,                                       /* La tempo compte, en attendant le delai de mise à un */
    DLS_TEMPO_WAIT_FOR_MIN_ON,                                         /* Delai de MAU dépassé, en attente du creneau minimum */
    DLS_TEMPO_WAIT_FOR_MAX_ON,                                      /* Creneau minimum atteint, en attente du creneau maximum */
    DLS_TEMPO_WAIT_FOR_DELAI_OFF,                                /* Creneau max atteint, en attente du delai de remise a zero */
    DLS_TEMPO_WAIT_FOR_COND_OFF                                            /* Attend que la condition soit tombée avant reset */
  };

 struct DLS_TEMPO                                                                           /* Définition d'une temporisation */
  { gchar   acronyme[64];
    gchar   tech_id[32];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gboolean init;                                   /* True si les données delai_on/off min_on/off ont bien été positionnées */
    guint status;                                                                               /* Statut de la temporisation */
    guint date_on;                                                              /* date a partir de laquelle la tempo sera ON */
    guint date_off;                                                            /* date a partir de laquelle la tempo sera OFF */
    gboolean state;
    guint delai_on;                                                     /* delai avant mise à un (fixé par option mnémonique) */
    guint delai_off;                                                  /* delai avant mise à zero (fixé par option mnémonique) */
    guint min_on;                            /* Durée minimale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
    guint max_on;                            /* Durée maximale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
    guint random;                                         /* Est-ce une tempo random ? si oui, est la dynamique max du random */
  };

 struct DLS_AI
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gchar   unite[32];                                                                                        /* Km, h, ° ... */
    gdouble valeur;
    guint   in_range;
    guint   archivage;
    guint   last_arch;                                                                         /* Date de la derniere archive */
   };

 struct DLS_AO
  { gchar   acronyme[64];
    gchar   tech_id[32];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gchar   unite[32];                                                                           /* Km, h, ° ... */
    gdouble valeur;
    guint   archivage;
    guint   last_arch;                                                                         /* Date de la derniere archive */
  };

 struct DLS_WATCHDOG
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gint    top;
  };

 struct DLS_HORLOGE
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
  };

 struct DLS_MONO
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gboolean etat;                                                                                      /* Etat actuel du bit */
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_BI
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gboolean etat;                                                                                      /* Etat actuel du bit */
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_DI
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
    guint   archivage;
    guint   last_arch;                                                                         /* Date de la derniere archive */
  };

 struct DLS_DO
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gboolean mono;
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
    guint   archivage;
    guint   last_arch;                                                                         /* Date de la derniere archive */
  };

 struct DLS_CI
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gint    valeur;
    gchar   unite[32];
    gboolean etat;
    gint    archivage;
    guint   last_arch;
  };

 struct DLS_CH
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    guint   valeur;
    gint    archivage;
    guint last_arch;                                                     /* Date de dernier enregistrement en base de données */
    guint old_top;                                                                         /* Date de debut du comptage du CH */
    gboolean etat;
  };

 struct DLS_VISUEL
  { gchar    forme[32];
    gchar    tech_id[32];
    gchar    acronyme[64];
    gchar   *libelle;
    gchar   *mode;
    gchar   *color;
    gchar   *badge;
    gdouble  valeur;
    gchar    unite[32];
    gboolean cligno;
    gboolean noshow;
    gboolean disable;
    gboolean changed;
    gint     next_send;
  };

 struct DLS_MESSAGE
  { JsonNode *source_node;
    gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle_converted[256];                                                      /* Le libelle converti selon les "$" */
    gboolean etat;                                                                          /* Etat avant execution du plugin */
    gboolean new_etat;                                                                      /* Etat après execution du plugin */
    gint new_etat_by_line;                                                    /* Numéro de ligne du dernier changement d'état */
    gint last_on;                                                                        /* Date du dernier changement d'état */
    gboolean libelle_is_dynamic;                                                       /* TRUE si le libelle dispose d'un "$" */
    gint next_top_check_libelle;                        /* Date a laquelle réaliser le prochain controle du libelle dynamique */
  };

 struct DLS_REGISTRE
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gdouble valeur;
    gchar   unite[32];
    gint    archivage;
    guint   last_arch;                                                   /* Date de dernier enregistrement en base de données */
    gdouble pid_somme_erreurs;                                                                                /* Calcul PID KI*/
    gdouble pid_prev_erreur;                                                                                 /* Calcul PID KD */
  };

 struct DLS_TO_PLUGIN                                                 /* structure dechange de données entre DLS et le plugin */
  { gboolean resetted;                                  /* 1 si les bits internes "start" du plugins doivent etre positionnés */
    gboolean debug;                                                 /* TRUE si le plugin doit logguer ses changements de bits */
    gint     num_ligne;                                                         /* N° de ligne du plugin en cours d'execution */
    struct DLS_MONO *dls_comm;
    struct DLS_MONO *dls_memsa_ok;
    struct DLS_MONO *dls_memsa_defaut;
    struct DLS_MONO *dls_memsa_defaut_fixe;
    struct DLS_MONO *dls_memsa_alarme;
    struct DLS_MONO *dls_memsa_alarme_fixe;
    struct DLS_MONO *dls_memssb_veille;
    struct DLS_MONO *dls_memssb_alerte;
    struct DLS_MONO *dls_memssb_alerte_fixe;
    struct DLS_MONO *dls_memssp_ok;
    struct DLS_MONO *dls_memssp_derangement;
    struct DLS_MONO *dls_memssp_derangement_fixe;
    struct DLS_MONO *dls_memssp_danger;
    struct DLS_MONO *dls_memssp_danger_fixe;
    struct DLS_DI   *dls_osyn_acquit;
    struct DLS_MESSAGE *dls_msg_comm_ok;
    struct DLS_MESSAGE *dls_msg_comm_hs;
  };

 extern struct DLS_BI *Dls_data_BI_lookup ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_BI_get        ( struct DLS_BI *bit );
 extern gboolean Dls_data_BI_get_up     ( struct DLS_BI *bit );
 extern gboolean Dls_data_BI_get_down   ( struct DLS_BI *bit );
 extern void     Dls_data_BI_set        ( struct DLS_TO_PLUGIN *vars, struct DLS_BI *bit, gboolean valeur );

 extern struct DLS_MONO *Dls_data_MONO_lookup ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_MONO_get      ( struct DLS_MONO *bit );
 extern gboolean Dls_data_MONO_get_up   ( struct DLS_MONO *bit );
 extern gboolean Dls_data_MONO_get_down ( struct DLS_MONO *bit );
 extern void     Dls_data_MONO_set      ( struct DLS_TO_PLUGIN *vars, struct DLS_MONO *bit, gboolean valeur );

 extern struct DLS_DI *Dls_data_DI_lookup ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_DI_get        ( struct DLS_DI *bit );
 extern gboolean Dls_data_DI_get_up     ( struct DLS_DI *bit );
 extern gboolean Dls_data_DI_get_down   ( struct DLS_DI *bit );
 extern void Dls_data_DI_set_pulse ( struct DLS_TO_PLUGIN *vars, struct DLS_DI *bit );

 extern struct DLS_DO *Dls_data_DO_lookup ( gchar *tech_id, gchar *acronyme );
 extern void     Dls_data_DO_set        ( struct DLS_TO_PLUGIN *vars, struct DLS_DO *bit, gboolean valeur );
 extern gboolean Dls_data_DO_get        ( struct DLS_DO *bit );
 extern gboolean Dls_data_DO_get_up     ( struct DLS_DO *bit );
 extern gboolean Dls_data_DO_get_down   ( struct DLS_DO *bit );

 extern struct DLS_AO *Dls_data_AO_lookup ( gchar *tech_id, gchar *acronyme );
 extern void     Dls_data_AO_set        ( struct DLS_TO_PLUGIN *vars, struct DLS_AO *bi, gdouble valeur );
 extern gdouble  Dls_data_AO_get        ( struct DLS_AO *bit );

 extern struct DLS_WATCHDOG *Dls_data_WATCHDOG_lookup ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_WATCHDOG_get ( struct DLS_WATCHDOG *bit );
 extern gint     Dls_data_WATCHDOG_get_time ( struct DLS_WATCHDOG *bit );
 extern void     Dls_data_WATCHDOG_set ( struct DLS_TO_PLUGIN *vars, struct DLS_WATCHDOG *bit, gint consigne );

 extern void Dls_data_set_bus ( struct DLS_TO_PLUGIN *vars, gchar *thread_tech_id, gchar *commande );

 extern struct DLS_AI *Dls_data_AI_lookup ( gchar *tech_id, gchar *acronyme );
 extern gdouble  Dls_data_AI_get        ( struct DLS_AI *bit );
 extern gboolean Dls_data_AI_get_inrange ( struct DLS_AI *bit );

 extern struct DLS_CI *Dls_data_CI_lookup ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_CI_set ( struct DLS_TO_PLUGIN *vars, struct DLS_CI *bit, gboolean etat );
 extern gint Dls_data_CI_get ( struct DLS_CI *bit );
 extern void Dls_data_CI_reset ( struct DLS_TO_PLUGIN *vars, struct DLS_CI *bit );

 extern struct DLS_CH *Dls_data_CH_lookup ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_CH_set ( struct DLS_TO_PLUGIN *vars, struct DLS_CH *bit, gboolean etat );
 extern gint Dls_data_CH_get ( struct DLS_CH *cpt_h );
 extern void Dls_data_CH_reset ( struct DLS_TO_PLUGIN *vars, struct DLS_CH *bit );

 extern struct DLS_REGISTRE *Dls_data_REGISTRE_lookup ( gchar *tech_id, gchar *acronyme );
 extern void    Dls_data_REGISTRE_set ( struct DLS_TO_PLUGIN *vars, struct DLS_REGISTRE *reg, gdouble valeur );
 extern gdouble Dls_data_REGISTRE_get ( struct DLS_REGISTRE *reg );

 extern struct DLS_VISUEL *Dls_data_VISUEL_lookup ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_VISUEL_set ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu,
                                   gdouble valeur, gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_badge ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, gchar *badge );
 extern void Dls_data_VISUEL_set_mode ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, gchar *mode );
 extern void Dls_data_VISUEL_set_color ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, gchar *color );
 extern void Dls_data_VISUEL_set_libelle ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, gchar *libelle );
 extern void Dls_data_VISUEL_set_for_WATCHDOG ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, struct DLS_WATCHDOG *src,
                                                gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_for_REGISTRE ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, struct DLS_REGISTRE *src,
                                                gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_for_TEMPO ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, struct DLS_TEMPO *src,
                                             gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_for_CI ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, struct DLS_CI *src,
                                          gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_for_CH ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, struct DLS_CH *src,
                                          gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_for_AI ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, struct DLS_AI *src,
                                          gboolean cligno, gboolean noshowe, gboolean disable );
 extern struct DLS_HORLOGE *Dls_data_HORLOGE_lookup ( gchar *tech_id, gchar *acronyme );
 extern gboolean HORLOGE_get ( struct DLS_HORLOGE *bit );

 extern struct DLS_MESSAGE *Dls_data_MESSAGE_lookup ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_MESSAGE_set ( struct DLS_TO_PLUGIN *vars, struct DLS_MESSAGE *msg );

 extern struct DLS_TEMPO *Dls_data_TEMPO_lookup ( gchar *tech_id, gchar *acronyme );
 extern void     Dls_data_TEMPO_set     ( struct DLS_TO_PLUGIN *vars, struct DLS_TEMPO *bit, gboolean etat,
                                          gint delai_on, gint min_on, gint max_on, gint delai_off, gint random);
 extern gboolean Dls_data_TEMPO_get     ( struct DLS_TEMPO *bit );
 extern gint     Dls_data_TEMPO_get_time ( struct DLS_TEMPO *bit );

 extern void Dls_PID_reset ( struct DLS_TO_PLUGIN *vars, struct DLS_REGISTRE *r_input );
 extern void Dls_PID ( struct DLS_TO_PLUGIN *vars, struct DLS_REGISTRE *input, struct DLS_REGISTRE *consigne,
                       struct DLS_REGISTRE *kp,struct DLS_REGISTRE *ki, struct DLS_REGISTRE *kd,
                       struct DLS_REGISTRE *outputmin, struct DLS_REGISTRE *outputmax, struct DLS_REGISTRE *output
                     );

 extern gint Dls_get_top( void );                                                                             /* donne le top */
 extern int Heure( int heure, int minute );                                                        /* Tester l'heure actuelle */
 extern int Heure_avant_egal( int heure, int minute );
 extern int Heure_apres_egal( int heure, int minute );
 extern int Heure_avant( int heure, int minute );
 extern int Heure_apres( int heure, int minute );
 extern int Jour_semaine( int jour );                                     /* Sommes nous le jour de la semaine en parametre ? */
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
