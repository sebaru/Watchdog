/******************************************************************************************************************************/
/* Watchdogd/Include/Module_dls.h -> Déclaration des prototypes de fonctions                                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          jeu 31 jui 2003 11:49:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Module_dls.h
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

 #ifndef _MODULE_DLS_H_
 #define _MODULE_DLS_H_
 #include <glib.h>

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
    guint   last_arch;                                                                         /* Date de la derniere archive */
    guint   in_range;
    guint   archivage;
    gboolean abonnement;
   };

 struct DLS_AO
  { gchar   acronyme[64];
    gchar   tech_id[32];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gdouble min; /* a virer */
    gdouble max;
    guint   type;                                                                                  /* Type de gestion de l'EA */
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
    gboolean etat;                                                                                      /* Etat actuel du bit */
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
    gboolean next_etat;                                                                       /*prochain etat calculé par DLS */
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_BI
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gint    groupe; /* Groupe 'radio' */
    gboolean etat;                                                                                      /* Etat actuel du bit */
    gboolean next_etat;                                                                       /*prochain etat calculé par DLS */
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
  };

 struct DLS_DO
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_CI
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gint    valeur;
    gint    val_en_cours1;                                                    /* valeur en cours pour le calcul via les ratio */
    gdouble ratio;
    gdouble multi;
    gchar   unite[32];
    gboolean etat;
    gint    archivage;
    guint   last_arch;
    gboolean abonnement;
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
    gboolean abonnement;
  };

 struct DLS_VISUEL
  { gchar    forme[32];
    gchar    tech_id[32];
    gchar    acronyme[64];
    gchar    libelle[128];                                                                                    /* Km, h, ° ... */
    gchar    mode[32];
    gchar    color[16];
    gboolean cligno;
    gboolean disable;
    gint     changes;
    gint     last_change_reset;
  };

 struct DLS_MESSAGE
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gboolean etat;
    gint groupe;
    gint last_change;
    gint last_on;
    gint changes;
  };

 struct DLS_REGISTRE
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gdouble valeur;
    gchar   unite[32];
    gint    archivage;
    guint   last_arch;                                                   /* Date de dernier enregistrement en base de données */
    gboolean abonnement;
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
    struct DLS_MONO *dls_memssb_alerte_fugitive;
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

 extern gboolean Dls_get_top_alerte ( void );
 extern gboolean Dls_get_top_alerte_fugitive ( void );

 extern struct DLS_BI *Dls_data_lookup_BI ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_get_BI        ( struct DLS_BI *bit );
 extern gboolean Dls_data_get_BI_up     ( struct DLS_BI *bit );
 extern gboolean Dls_data_get_BI_down   ( struct DLS_BI *bit );
 extern void     Dls_data_set_BI        ( struct DLS_TO_PLUGIN *vars, struct DLS_BI *bit, gboolean valeur );

 extern struct DLS_MONO *Dls_data_lookup_MONO ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_get_MONO      ( struct DLS_MONO *bit );
 extern gboolean Dls_data_get_MONO_up   ( struct DLS_MONO *bit );
 extern gboolean Dls_data_get_MONO_down ( struct DLS_MONO *bit );
 extern void     Dls_data_set_MONO      ( struct DLS_TO_PLUGIN *vars, struct DLS_MONO *bit, gboolean valeur );

 extern struct DLS_DI *Dls_data_lookup_DI ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_get_DI        ( struct DLS_DI *bit );
 extern gboolean Dls_data_get_DI_up     ( struct DLS_DI *bit );
 extern gboolean Dls_data_get_DI_down   ( struct DLS_DI *bit );

 extern struct DLS_DO *Dls_data_lookup_DO ( gchar *tech_id, gchar *acronyme );
 extern void     Dls_data_set_DO        ( struct DLS_TO_PLUGIN *vars, struct DLS_DO *bit, gboolean valeur );

 extern struct DLS_AO *Dls_data_lookup_AO ( gchar *tech_id, gchar *acronyme );
 extern void     Dls_data_set_AO        ( struct DLS_TO_PLUGIN *vars, struct DLS_AO *bi, gdouble valeur );
 extern gdouble  Dls_data_get_AO        ( struct DLS_AO *bit );


 extern struct DLS_WATCHDOG *Dls_data_lookup_WATCHDOG ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_get_WATCHDOG ( struct DLS_WATCHDOG *bit );
 extern void     Dls_data_set_WATCHDOG ( struct DLS_TO_PLUGIN *vars, struct DLS_WATCHDOG *bit, gint consigne );

 extern void Dls_data_set_bus ( gchar *tech_id, gchar *acronyme, gpointer *bus_p, gchar *target_tech_id, gchar *json_parametre );

 extern struct DLS_AI *Dls_data_lookup_AI ( gchar *tech_id, gchar *acronyme );
 extern gdouble  Dls_data_get_AI        ( struct DLS_AI *bit );
 extern gboolean Dls_data_get_AI_inrange ( struct DLS_AI *bit );

 extern struct DLS_CI *Dls_data_lookup_CI ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_set_CI ( struct DLS_TO_PLUGIN *vars, struct DLS_CI *bit, gboolean etat, gint reset, gint ratio );
 extern gint Dls_data_get_CI ( struct DLS_CI *bit );

 extern struct DLS_CH *Dls_data_lookup_CH ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_set_CH ( struct DLS_TO_PLUGIN *vars, struct DLS_CH *cpt_h, gboolean etat, gint reset );
 extern gint Dls_data_get_CH ( struct DLS_CH *cpt_h );

 extern struct DLS_REGISTRE *Dls_data_lookup_REGISTRE ( gchar *tech_id, gchar *acronyme );
 extern void    Dls_data_set_REGISTRE ( struct DLS_TO_PLUGIN *vars, struct DLS_REGISTRE *reg, gdouble valeur );
 extern gdouble Dls_data_get_REGISTRE ( struct DLS_REGISTRE *reg );
 extern void Dls_cadran_send_REGISTRE_to_API ( struct DLS_REGISTRE *bit );

 extern struct DLS_VISUEL *Dls_data_lookup_VISUEL ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_set_VISUEL ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu,
                                   gchar *mode, gchar *color, gboolean cligno, gchar *libelle, gboolean disable );

 extern struct DLS_HORLOGE *Dls_data_lookup_HORLOGE ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_get_HORLOGE ( struct DLS_HORLOGE *bit );

 extern struct DLS_MESSAGE *Dls_data_lookup_MESSAGE ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_set_MESSAGE ( struct DLS_TO_PLUGIN *vars, struct DLS_MESSAGE *msg, gboolean etat );

 extern struct DLS_TEMPO *Dls_data_lookup_TEMPO ( gchar *tech_id, gchar *acronyme );
 extern void     Dls_data_set_TEMPO     ( struct DLS_TO_PLUGIN *vars, struct DLS_TEMPO *bit, gboolean etat,
                                          gint delai_on, gint min_on, gint max_on, gint delai_off, gint random);
 extern gboolean Dls_data_get_TEMPO     ( struct DLS_TEMPO *bit );


 extern void Dls_PID_reset ( gchar *input_tech_id, gchar *input_acronyme, gpointer *r_input );
 extern gdouble Dls_PID ( gchar *input_tech_id, gchar *input_acronyme, gpointer *r_input,
                          gchar *consigne_tech_id, gchar *consigne_acronyme, gpointer *r_consigne,
                          gchar *kp_tech_id, gchar *kp_acronyme, gpointer *r_kp,
                          gchar *ki_tech_id, gchar *ki_acronyme, gpointer *r_ki,
                          gchar *kd_tech_id, gchar *kd_acronyme, gpointer *r_kd,
                          gchar *outputmin_tech_id, gchar *outputmin_acronyme, gpointer *r_outputmin,
                          gchar *outputmax_tech_id, gchar *outputmax_acronyme, gpointer *r_outputmax
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
