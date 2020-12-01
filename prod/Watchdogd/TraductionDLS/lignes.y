/******************************************************************************************************************************/
/* Watchdogd/TraductionDLS/ligne.y        Définitions des ligne dls DLS                                                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        jeu. 24 juin 2010 19:37:44 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * lignes.y
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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

%{
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "watchdogd.h"
%}

%union { int val;
         float valf;
         char *chaine;
         GList *gliste;
         struct OPTION *option;
         struct ACTION *action;
         struct COMPARATEUR *comparateur;
         struct ALIAS *t_alias;
       };

%token <val>    T_ERROR PVIRGULE VIRGULE T_DPOINTS DONNE EQUIV T_MOINS T_POUV T_PFERM T_EGAL T_PLUS ET BARRE T_FOIS
%token <val>    T_SWITCH T_ACCOUV T_ACCFERM T_PIPE
%token <val>    T_DEFINE

%token <val>    T_SBIEN_VEILLE T_SBIEN_ALE T_SBIEN_ALEF T_SBIEN_ALE_FUGITIVE T_TOP_ALERTE T_TOP_ALERTE_FUGITIVE
%token <val>    T_SPERS_DER T_SPERS_DERF T_SPERS_DAN T_SPERS_DANF T_SPERS_OK T_OSYN_ACQ
%token <val>    T_ACT_COMOUT T_ACT_DEF T_ACT_ALA T_ACT_DEFF T_ACT_ALAF  T_ACT_OK
%token <val>    T_BUS T_HOST T_THREAD T_TAG T_PARAM1

%token <val>    MODE COLOR CLIGNO RESET RATIO T_LIBELLE T_ETIQUETTE T_UNITE T_FORME
%token <val>    T_DAA T_DMINA T_DMAXA T_DAD T_RANDOM T_UPDATE T_CONSIGNE

%token <val>    T_TYPE T_INFO T_ATTENTE T_DEFAUT T_ALARME T_VEILLE T_ALERTE T_DERANGEMENT T_DANGER
%type  <val>    type_msg

%token <val>    INF SUP INF_OU_EGAL SUP_OU_EGAL T_TRUE T_FALSE
%type  <val>    ordre

%token <val>    HEURE APRES AVANT LUNDI MARDI MERCREDI JEUDI VENDREDI SAMEDI DIMANCHE
%type  <val>    modulateur jour_semaine

%token <val>    T_BI T_MONO ENTREE SORTIE T_ANALOG_OUTPUT T_TEMPO T_HORLOGE T_DYN_STRING
%token <val>    T_MSG T_ICONE T_CPT_H T_CPT_IMP EANA T_START T_REGISTRE T_DIGITAL_OUTPUT T_WATCHDOG
%type  <val>    alias_bit

%token <val>    ROUGE VERT BLEU JAUNE NOIR BLANC ORANGE GRIS KAKI T_EDGE_UP T_EDGE_DOWN T_IN_RANGE
%type  <val>    couleur

%token <chaine> ID T_CHAINE
%token <val>    ENTIER
%token <valf>   VALF

%type  <val>         barre
%type  <gliste>      liste_options options
%type  <option>      une_option dyn_string
%type  <chaine>      unite facteur expr suffixe unSwitch listeCase une_instr listeInstr
%type  <action>      action une_action
%type  <comparateur> comparateur
%type  <chaine>      calcul_expr calcul_expr2 calcul_expr3
%type  <t_alias>     calcul_ea_result

%%
fichier: ligne_source_dls;

ligne_source_dls:         listeAlias listeInstr {{ if($2) { Emettre( $2 ); g_free($2); } }}
                        | listeAlias
                        | listeInstr {{ if($1) { Emettre( $1 ); g_free($1); } }}
                        |
                        ;

/*************************************************** Gestion des alias ********************************************************/
listeAlias:     un_alias listeAlias
                | un_alias
                ;

un_alias:       T_DEFINE ID EQUIV alias_bit liste_options PVIRGULE
                {{ if ( New_alias(NULL, $2, $4, $5) == FALSE )                    /* Deja defini ? */
                    { Emettre_erreur_new( "'%s' is already defined", $2 ); }
                   g_free($2);
                }}
                ;
alias_bit:        T_BI        {{ $$=MNEMO_BISTABLE;   }}
                | T_MONO      {{ $$=MNEMO_MONOSTABLE; }}
                | ENTREE      {{ $$=MNEMO_ENTREE;     }}
                | SORTIE      {{ $$=MNEMO_SORTIE;     }}
                | T_MSG       {{ $$=MNEMO_MSG;        }}
                | T_TEMPO     {{ $$=MNEMO_TEMPO;      }}
                | T_ICONE     {{ $$=MNEMO_MOTIF;      }}
                | T_CPT_H     {{ $$=MNEMO_CPTH;       }}
                | T_CPT_IMP   {{ $$=MNEMO_CPT_IMP;    }}
                | EANA        {{ $$=MNEMO_ENTREE_ANA; }}
                | T_ANALOG_OUTPUT {{ $$=MNEMO_SORTIE_ANA; }}
                | T_DIGITAL_OUTPUT {{ $$=MNEMO_DIGITAL_OUTPUT; }}
                | T_REGISTRE  {{ $$=MNEMO_REGISTRE;   }}
                | T_HORLOGE   {{ $$=MNEMO_HORLOGE;    }}
                | T_BUS       {{ $$=MNEMO_BUS;        }}
                | T_WATCHDOG  {{ $$=MNEMO_WATCHDOG;   }}
                ;
/**************************************************** Gestion des instructions ************************************************/
listeInstr:     une_instr listeInstr
                {{ int taille = ($1 ? strlen($1) : 0) + ($2 ? strlen($2) : 0) + 1;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "%s%s", $1, $2 );
                   if ($1) g_free($1);
                   if ($2) g_free($2);
                }}
                | une_instr
                {{ $$=$1; }}
                ;

une_instr:      T_MOINS expr DONNE action PVIRGULE
                {{ int taille;
                   taille = strlen($2)+strlen($4->alors)+100;
                   if ($4->sinon)
                    { taille += (strlen($4->sinon) + 10);
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille,
                                  "/* Ligne %d une_instr-------------*/\nif(%s)\n { %s }\nelse\n { %s }\n\n",
                                  DlsScanner_get_lineno(), $2, $4->alors, $4->sinon );
                    }
                   else
                    { $$ = New_chaine( taille );
                      g_snprintf( $$, taille, "/* Ligne %d une_instr-------------*/\nif(%s)\n { %s }\n\n",
                                  DlsScanner_get_lineno(), $2, $4->alors );
                    }

                   if ($4->sinon) g_free($4->sinon);
                   g_free($4->alors);
                   g_free($4);
                   g_free($2);
                }}
                | T_MOINS expr T_MOINS T_POUV calcul_expr T_PFERM DONNE calcul_ea_result PVIRGULE
                {{ int taille;
                   if ($8)
                    { taille = strlen($5)+strlen($2)+strlen($8->tech_id)+strlen($8->acronyme)+100;
                      $$ = New_chaine( taille );
                      if ($8->classe==MNEMO_SORTIE_ANA)
                       { g_snprintf( $$, taille,
                                     "if(%s) { Dls_data_set_AO ( vars, \"%s\", \"%s\", &_%s_%s, %s ); }\n",
                                     $2, $8->tech_id, $8->acronyme, $8->tech_id, $8->acronyme, $5 );
                       }
                      else if ($8->classe==MNEMO_REGISTRE)
                       { g_snprintf( $$, taille,
                                     "if(%s) { Dls_data_set_R ( vars, \"%s\", \"%s\", &_%s_%s, %s ); }\n",
                                     $2, $8->tech_id, $8->acronyme, $8->tech_id, $8->acronyme, $5 );
                       }
                      else
                       { Emettre_erreur_new( "'%s:%s' is unknown", $8->tech_id, $8->acronyme ); }
                    } else $$=g_strdup("/* test ! */");
                   g_free($2);
                   g_free($5);
                }}
                | T_MOINS expr DONNE T_ACCOUV listeInstr T_ACCFERM
                {{ int taille;
                   taille = strlen($2)+strlen($5)+100;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille,
                               "/* Ligne %d une_instr if----------*/\nif(%s)\n { %s }\n\n",
                                  DlsScanner_get_lineno(), $2, $5 );
                   g_free($5);
                   g_free($2);
                }}
                | unSwitch {{ $$=$1; }}
                ;

/****************************************************** Partie SWITCH *********************************************************/
unSwitch:       T_SWITCH listeCase
                {{ gint taille;
                   taille = strlen($2)+100;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "/* Ligne %d (CASE BEGIN)------------*/\n"
                                           "%s\n"
                                           "/* Ligne %d (CASE END)--------------*/\n",
                                           DlsScanner_get_lineno(), $2, DlsScanner_get_lineno() );
                   g_free($2);
                }}
                ;

listeCase:      T_PIPE T_MOINS expr DONNE action PVIRGULE listeCase
                {{ int taille;
                   taille = strlen($3)+strlen($5->alors)+strlen($7)+100;
                   if ($5->sinon) taille+=strlen($5->sinon);
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille,
                               "/* Ligne %d (CASE INSIDE)----------*/\n"
                               "if(%s)\n { %s }\nelse\n { %s\n%s }\n",
                               DlsScanner_get_lineno(), $3, $5->alors, ($5->sinon ? $5->sinon : ""), $7 );

                   if ($5->sinon) g_free($5->sinon);
                   g_free($5->alors); g_free($5);
                   g_free($3);
                   g_free($7);
                }}
                | T_PIPE T_MOINS DONNE action PVIRGULE
                {{ int taille;
                   taille = strlen($4->alors)+100;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille,
                               "/* Ligne %d (CASE INSIDE DEFAULT)--*/\n"
                               "  %s", DlsScanner_get_lineno(), $4->alors );
                   if ($4->sinon) g_free($4->sinon);
                   g_free($4->alors); g_free($4);
                }}
                | {{ $$=strdup(""); }}
                ;
/****************************************************** Partie CALCUL *********************************************************/
calcul_expr:    calcul_expr T_PLUS calcul_expr2
                {{ int taille;
                   taille = strlen($1) + strlen($3) + 4;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s+%s)", $1, $3 );
                   g_free($1); g_free($3);
                }}
                | calcul_expr T_MOINS calcul_expr2
                {{ int taille;
                   taille = strlen($1) + strlen($3) + 4;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s-%s)", $1, $3 );
                   g_free($1); g_free($3);
                }}
                | calcul_expr2
                ;
calcul_expr2:   calcul_expr2 T_FOIS calcul_expr3
                {{ int taille;
                   taille = strlen($1) + strlen($3) + 4;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s*%s)", $1, $3 );
                   g_free($1); g_free($3);
                }}
                | calcul_expr2 BARRE calcul_expr3
                {{ int taille;
                   taille = strlen($1) + strlen($3) + 4;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s/%s)", $1, $3 );
                   g_free($1); g_free($3);
                }}
                | calcul_expr3
                ;
calcul_expr3:   VALF
                {{ int taille;
                   taille = 15;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "%f", $1 );
                }}
                | ENTIER
                {{ int taille;
                   taille = 15;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "%d", $1 );
                }}
                | T_POUV calcul_expr T_PFERM
                {{ $$=$2; }}
                | ID suffixe
                {{ char *tech_id, *acro;
                   struct ALIAS *alias;
                   int taille;
                   if ($2) { tech_id = $1; acro = $2; }
                      else { tech_id = NULL; acro = $1; }
                   alias = Get_alias_par_acronyme(tech_id,acro);                                       /* On recupere l'alias */
                   if (!alias)
                    { alias = Set_new_external_alias(tech_id,acro); }      /* Si dependance externe, on va chercher */

                   if (alias)
                    { switch(alias->classe)               /* On traite que ce qui peut passer en "condition" */
                       { case MNEMO_REGISTRE:
                          { taille = 256;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "Dls_data_get_R(\"%s\",\"%s\",&_%s_%s)",
                                        alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
                            break;
                          }
                         case MNEMO_ENTREE_ANA:
                          { taille = 256;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "Dls_data_get_AI(\"%s\",\"%s\",&_%s_%s)",
                                        alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
                            break;
                          }
                         case MNEMO_SORTIE_ANA:
                          { taille = 256;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "Dls_data_get_AO(\"%s\",\"%s\",&_%s_%s)",
                                        alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
                            break;
                          }
                         case MNEMO_CPTH:
                          { taille = 256;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "Dls_data_get_CH(\"%s\",\"%s\",&_%s_%s)",
                                        alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
                            break;
                          }
                         case MNEMO_CPT_IMP:
                          { taille = 256;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "Dls_data_get_CI(\"%s\",\"%s\",&_%s_%s)",
                                        alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
                            break;
                          }
                         case MNEMO_MONOSTABLE:
                         case MNEMO_BISTABLE:
                          { taille = 256;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "Dls_data_get_bool(\"%s\",\"%s\",&_%s_%s)",
                                        alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
                            break;
                          }
                         default:
                          { Emettre_erreur_new( "'%s:%s' ne peut s'utiliser dans un calcul", alias->tech_id, alias->acronyme );
                            $$=New_chaine(2);
                            g_snprintf( $$, 2, "0" );
                          }
                       }
                    }
                   else
                    { Emettre_erreur_new( "'%s' is not defined", $1 );
                      $$=New_chaine(2);
                      g_snprintf( $$, 2, "0" );
                    }
                   if ($2) g_free($2);                                                   /* Libération du prefixe s'il existe */
                   g_free($1);                                                         /* On n'a plus besoin de l'identifiant */
                }}
                ;

calcul_ea_result: ID
                {{ struct ALIAS *alias;
                   alias = Get_alias_par_acronyme(NULL,$1);                                            /* On recupere l'alias */
                   if (alias)
                    { switch(alias->classe)                              /* On traite que ce qui peut passer en "condition" */
                       { case MNEMO_REGISTRE:
                         case MNEMO_SORTIE_ANA:
                          { $$ = alias;
                            break;
                          }
                         default:
                          { Emettre_erreur_new( "'%s' ne peut s'utiliser dans un résultat de calcul", $1 );
                            $$=NULL;
                          }
                       }
                    }
                   else
                    { Emettre_erreur_new( "'%s' is not defined", $1 );
                      $$=NULL;
                    }
                   g_free($1);                                     /* On n'a plus besoin de l'identifiant */
                }}
                ;
/******************************************************* Partie LOGIQUE *******************************************************/
expr:           expr T_PLUS facteur
                {{ int taille;
                   taille = strlen($1)+strlen($3)+7;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s || %s)", $1, $3 );
                   g_free($1); g_free($3);
                }}

                | facteur
                ;
facteur:        facteur ET unite
                {{ int taille;
                   taille = strlen($1)+strlen($3)+7;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s && %s)", $1, $3 );
                   g_free($1); g_free($3);
                }}
                | unite
                ;

unite:          modulateur ENTIER HEURE ENTIER
                {{ int taille;
                   taille = 20;
                   $$ = New_chaine(taille);
                   if ($2>23) $2=23;
                   if ($4>59) $4=59;
                   switch ($1)
                    { case 0    : g_snprintf( $$, taille, "Heure(%d,%d)", $2, $4 );
                                  break;
                      case APRES: g_snprintf( $$, taille, "Heure_apres(%d,%d)", $2, $4 );
                                  break;
                      case AVANT: g_snprintf( $$, taille, "Heure_avant(%d,%d)", $2, $4 );
                                  break;
                    }
                }}
                | jour_semaine
                {{ int taille;
                   taille = 18;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "Jour_semaine(%d)", $1 );
                }}
                | T_START
                {{ int taille;
                   taille = 20;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "(vars->resetted)" );
                }}
                | T_TRUE
                {{ int taille;
                   taille = 5;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "(1)" );
                }}
                | T_FALSE
                {{ int taille;
                   taille = 5;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "(0)" );
                }}
                | barre T_TOP_ALERTE
                {{ int taille;
                   taille = 25;
                   $$ = New_chaine(taille);
                   if ($1) g_snprintf( $$, taille, "(!Dls_get_top_alerte())" );
                   else    g_snprintf( $$, taille, "( Dls_get_top_alerte())" );
                }}
                | barre T_TOP_ALERTE_FUGITIVE
                {{ int taille;
                   taille = 35;
                   $$ = New_chaine(taille);
                   if ($1) g_snprintf( $$, taille, "(!Dls_get_top_alerte_fugitive())" );
                   else    g_snprintf( $$, taille, "( Dls_get_top_alerte_fugitive())" );
                }}
                | T_OSYN_ACQ
                {{ $$ = g_strdup("vars->bit_acquit");
                }}
                | barre T_ACT_OK
                {{ if ($1) $$ = g_strdup("!vars->bit_activite_ok");
                      else $$ = g_strdup("vars->bit_activite_ok");
                }}
                | barre T_SPERS_OK
                {{ if ($1) $$ = g_strdup("!vars->bit_secupers_ok");
                      else $$ = g_strdup("vars->bit_secupers_ok");
                }}
                | barre T_ACT_COMOUT
                  {{ $$=New_condition_vars( $1, "vars->bit_comm_out"); }}
                | barre T_ACT_DEF
                  {{ $$=New_condition_vars( $1, "vars->bit_defaut"); }}
                | barre T_ACT_DEFF
                  {{ $$=New_condition_vars( $1, "vars->bit_defaut_fixe"); }}
                | barre T_ACT_ALA
                  {{ $$=New_condition_vars( $1, "vars->bit_alarme"); }}
                | barre T_ACT_ALAF
                  {{ $$=New_condition_vars( $1, "vars->bit_alarme_fixe"); }}
                | barre T_SBIEN_VEILLE
                  {{ $$=New_condition_vars( $1, "vars->bit_veille"); }}
                | barre T_SBIEN_ALE
                  {{ $$=New_condition_vars( $1, "vars->bit_alerte"); }}
                | barre T_SBIEN_ALE_FUGITIVE
                  {{ $$=New_condition_vars( $1, "vars->bit_alerte_fugitive"); }}
                | barre T_SBIEN_ALEF
                  {{ $$=New_condition_vars( $1, "vars->bit_alerte_fixe"); }}
                | barre T_SPERS_DER
                  {{ $$=New_condition_vars( $1, "vars->bit_derangement"); }}
                | barre T_SPERS_DERF
                  {{ $$=New_condition_vars( $1, "vars->bit_derangement_fixe"); }}
                | barre T_SPERS_DAN
                  {{ $$=New_condition_vars( $1, "vars->bit_danger"); }}
                | barre T_SPERS_DANF
                  {{ $$=New_condition_vars( $1, "vars->bit_danger_fixe"); }}
                | barre T_POUV expr T_PFERM
                {{ int taille;
                   taille = strlen($3)+5;
                   $$ = New_chaine( taille );
                   if ($1) { g_snprintf( $$, taille, "!(%s)", $3 ); }
                   else    { g_snprintf( $$, taille, "(%s)", $3 ); }
                   g_free($3);
                }}
                | barre ID suffixe liste_options comparateur
                {{ struct ALIAS *alias;
                   char *tech_id, *acro;
                   int taille;

                   if ($3) { tech_id = $2; acro = $3; }
                      else { tech_id = NULL; acro = $2; }

                   alias = Get_alias_par_acronyme(tech_id,acro);                                       /* On recupere l'alias */
                   if (!alias)
                    { alias = Set_new_external_alias(tech_id,acro); }                /* Si dependance externe, on va chercher */

                   if (alias)
                    { if ($5 && (alias->classe==MNEMO_TEMPO ||                              /* Vérification des bits non comparables */
                                 alias->classe==MNEMO_ENTREE ||
                                 alias->classe==MNEMO_SORTIE ||
                                 alias->classe==MNEMO_BISTABLE ||
                                 alias->classe==MNEMO_MONOSTABLE ||
                                 alias->classe==MNEMO_DIGITAL_OUTPUT ||
                                 alias->classe==MNEMO_WATCHDOG ||
                                 alias->classe==MNEMO_HORLOGE)
                         )
                       { Emettre_erreur_new( "'%s' ne peut s'utiliser dans une comparaison", $3 );
                         $$=New_chaine(2);
                         g_snprintf( $$, 2, "0" );
                       } else
                      if (!$5 && (alias->classe==MNEMO_SORTIE_ANA ||
                                  alias->classe==MNEMO_REGISTRE ||
                                  alias->classe==MNEMO_CPT_IMP ||
                                  alias->classe==MNEMO_CPTH)
                         )
                       { Emettre_erreur_new( "'%s' ne peut s'utiliser qu'avec une comparaison", $3 );
                         $$=New_chaine(2);
                         g_snprintf( $$, 2, "0" );
                       }
                      else switch(alias->classe)                              /* On traite que ce qui peut passer en "condition" */
                       { case MNEMO_TEMPO :
                          { $$ = New_condition_tempo( $1, alias, $4 );
                            break;
                          }
                         case MNEMO_ENTREE:
                          { $$ = New_condition_entree( $1, alias, $4 );
                            break;
                          }
                         case MNEMO_BISTABLE:
                          { $$ = New_condition_bi( $1, alias, $4 );
                            break;
                          }
                         case MNEMO_MONOSTABLE:
                          { $$ = New_condition_mono( $1, alias, $4 );
                            break;
                          }
                         case MNEMO_HORLOGE:
                          { $$ = New_condition_horloge( $1, alias, $4 );
                            break;
                          }
                         case MNEMO_WATCHDOG:
                          { $$ = New_condition_WATCHDOG( $1, alias, $4 );
                            break;
                          }
                         case MNEMO_ENTREE_ANA:
                          { $$ = New_condition_entree_ana( $1, alias, $4, $5 );
                            break;
                          }
                         case MNEMO_SORTIE_ANA:
                          { $$ = New_condition_sortie_ana( $1, alias, $4, $5 );
                            break;
                          }
                         case MNEMO_REGISTRE:
                          { taille = 256;
                            $$ = New_chaine( taille );
                            switch( $5->type )
                             { case INF        : g_snprintf( $$, taille, "Dls_data_get_R(\"%s\",\"%s\",&_%s_%s)<%f",
                                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                                 break;
                               case SUP        : g_snprintf( $$, taille, "Dls_data_get_R(\"%s\",\"%s\",&_%s_%s)>%f",
                                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                                 break;
                               case INF_OU_EGAL: g_snprintf( $$, taille, "Dls_data_get_R(\"%s\",\"%s\",&_%s_%s)<=%f",
                                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                                 break;
                               case SUP_OU_EGAL: g_snprintf( $$, taille, "Dls_data_get_R(\"%s\",\"%s\",&_%s_%s)>=%f",
                                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                                 break;
                               case T_EGAL     : g_snprintf( $$, taille, "Dls_data_get_R(\"%s\",\"%s\",&_%s_%s)==%f",
                                                              alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                                 break;
                             }
                            break;
                           }
                         case MNEMO_CPT_IMP:
                          { taille = 256;
                            $$ = New_chaine( taille ); /* 10 caractÃ¨res max */
                            switch($5->type)
                             { case INF:
                                 g_snprintf( $$, taille, "Dls_data_get_CI(\"%s\",\"%s\",&_%s_%s)<%f",
                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                 break;
                               case SUP:
                                 g_snprintf( $$, taille, "Dls_data_get_CI(\"%s\",\"%s\",&_%s_%s)>%f",
                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                 break;
                               case INF_OU_EGAL:
                                 g_snprintf( $$, taille, "Dls_data_get_CI(\"%s\",\"%s\",&_%s_%s)<=%f",
                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                 break;
                               case SUP_OU_EGAL:
                                 g_snprintf( $$, taille, "Dls_data_get_CI(\"%s\",\"%s\",&_%s_%s)>=%f",
                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                 break;
                               case T_EGAL:
                                 g_snprintf( $$, taille, "Dls_data_get_CI(\"%s\",\"%s\",&_%s_%s)==%f",
                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                 break;
                             }
                            break;
                          }
                         case MNEMO_CPTH:
                          { taille = 256;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            switch($5->type)
                             { case INF:
                                 g_snprintf( $$, taille, "Dls_data_get_CH(\"%s\",\"%s\",&_%s_%s)<%f",
                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                 break;
                               case SUP:
                                 g_snprintf( $$, taille, "Dls_data_get_CH(\"%s\",\"%s\",&_%s_%s)>%f",
                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                 break;
                               case INF_OU_EGAL:
                                 g_snprintf( $$, taille, "Dls_data_get_CH(\"%s\",\"%s\",&_%s_%s)<=%f",
                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                 break;
                               case SUP_OU_EGAL:
                                 g_snprintf( $$, taille, "Dls_data_get_CH(\"%s\",\"%s\",&_%s_%s)>=%f",
                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                 break;
                               case T_EGAL:
                                 g_snprintf( $$, taille, "Dls_data_get_CH(\"%s\",\"%s\",&_%s_%s)==%f",
                                             alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, $5->valf );
                                 break;
                             }
                            break;
                          }
                         default:
                          { Emettre_erreur_new( "'%s' n'est pas une condition valide", acro );
                            $$=New_chaine(2);
                            g_snprintf( $$, 2, "0" );
                          }
                       }
                    }
                   else { if (tech_id) Emettre_erreur_new( "'%s:%s' is not defined", tech_id, acro );/* si l'alias n'existe pas */
                                  else Emettre_erreur_new( "'%s' is not defined", acro );/* si l'alias n'existe pas */
                          $$=New_chaine(2);
                          g_snprintf( $$, 2, "0" );
                        }
                   if ($3) g_free($3);                                                   /* Libération du prefixe s'il existe */
                   g_free($2);                                                         /* On n'a plus besoin de l'identifiant */
                   Liberer_options($4);
                   if ($5) g_free($5);                                               /* Libération du comparateur s'il existe */
                }}
                ;

suffixe:          T_DPOINTS ID {{ $$=$2; }}
                | {{ $$=NULL; }}
                ;
/************************************************* Gestion des actions ********************************************************/
action:         action VIRGULE une_action
                {{ int taille;
                   $$=New_action();
                   taille = strlen($1->alors)+strlen($3->alors)+1;
                   $$->alors = New_chaine( taille );
                   g_snprintf( $$->alors, taille, "%s%s", $1->alors, $3->alors );
                   taille = 1;
                   if ($1->sinon) taille += strlen($1->sinon);
                   if ($3->sinon) taille += strlen($3->sinon);
                   if (taille>1)
                    { $$->sinon = New_chaine( taille );
                      if ($1->sinon && $3->sinon)
                       { g_snprintf( $$->sinon, taille, "%s%s", $1->sinon, $3->sinon ); }
                      else if ($1->sinon)
                       { g_snprintf( $$->sinon, taille, "%s", $1->sinon ); }
                      else
                       { g_snprintf( $$->sinon, taille, "%s", $3->sinon ); }
                    }
                   g_free($1->alors); if ($1->sinon) { g_free($1->sinon); }
                   g_free($3->alors); if ($3->sinon) { g_free($3->sinon); }
                   g_free($1); g_free($3);
                }}
                | une_action {{ $$=$1; }}
                ;

une_action:     T_ACT_COMOUT
                  {{ $$=New_action_vars_mono("vars->bit_comm_out"); }}
                | T_ACT_DEF
                  {{ $$=New_action_vars_mono("vars->bit_defaut"); }}
                | T_ACT_DEFF
                  {{ $$=New_action_vars_mono("vars->bit_defaut_fixe"); }}
                | T_ACT_ALA
                  {{ $$=New_action_vars_mono("vars->bit_alarme"); }}
                | T_ACT_ALAF
                  {{ $$=New_action_vars_mono("vars->bit_alarme_fixe"); }}
                | T_SBIEN_VEILLE
                  {{ $$=New_action_vars_mono("vars->bit_veille"); }}
                | T_SBIEN_ALE
                  {{ $$=New_action_vars_mono("vars->bit_alerte"); }}
                | T_SBIEN_ALEF
                  {{ $$=New_action_vars_mono("vars->bit_alerte_fixe"); }}
                | T_SPERS_DER
                  {{ $$=New_action_vars_mono("vars->bit_derangement"); }}
                | T_SPERS_DERF
                  {{ $$=New_action_vars_mono("vars->bit_derangement_fixe"); }}
                | T_SPERS_DAN
                  {{ $$=New_action_vars_mono("vars->bit_danger"); }}
                | T_SPERS_DANF
                  {{ $$=New_action_vars_mono("vars->bit_danger_fixe"); }}
                | barre ID suffixe liste_options
                {{ struct ALIAS *alias;                                                   /* Definition des actions via alias */
                   gchar *tech_id, *acro;
                   int taille;
                   if ($3) { tech_id = $2; acro = $3; }
                      else { tech_id = NULL; acro = $2; }

                   alias = Get_alias_par_acronyme(tech_id,acro);                                       /* On recupere l'alias */
                   if (!alias)
                    { alias = Set_new_external_alias(tech_id,acro); }      /* Si dependance externe, on va chercher */

                   if (!alias)
                    { if ($3) Emettre_erreur_new( "'%s:%s' is not defined", $2, $3 );
                         else Emettre_erreur_new( "'%s' is not defined", $2 );

                      $$=New_action();
                      taille = 2;
                      $$->alors = New_chaine( taille );
                      g_snprintf( $$->alors, taille, " " );
                      $$->sinon = NULL;
                    }
                   else                                                           /* L'alias existe, vérifions ses parametres */
                    { GList *options, *options_g, *options_d;
                      options_g = g_list_copy( $4 );
                      options_d = g_list_copy( alias->options );
                      options = g_list_concat( options_g, options_d );                  /* Concaténation des listes d'options */
                      if ($1 && (alias->classe==MNEMO_TEMPO ||
                                 alias->classe==MNEMO_MSG ||
                                 alias->classe==MNEMO_BUS ||
                                 alias->classe==MNEMO_MOTIF ||
                                 alias->classe==MNEMO_DIGITAL_OUTPUT ||
                                 alias->classe==MNEMO_WATCHDOG ||
                                 alias->classe==MNEMO_MONOSTABLE)
                         )
                       { Emettre_erreur_new( "'/%s' ne peut s'utiliser", alias->acronyme );
                         $$=New_action();
                         taille = 2;
                         $$->alors = New_chaine( taille );
                         g_snprintf( $$->alors, taille, " " );
                         $$->sinon = NULL;
                       }
                      else switch(alias->classe)
                       { case MNEMO_TEMPO : $$=New_action_tempo( alias, options ); break;
                         case MNEMO_MSG   : $$=New_action_msg( alias, options );   break;
                         case MNEMO_BUS   : $$=New_action_bus( alias, options );   break;
                         case MNEMO_SORTIE: $$=New_action_sortie( alias, $1, options );  break;
                         case MNEMO_DIGITAL_OUTPUT: $$=New_action_digital_output( alias, options );  break;
                         case MNEMO_BISTABLE: $$=New_action_bi( alias, $1 ); break;
                         case MNEMO_MONOSTABLE: $$=New_action_mono( alias );              break;
                         case MNEMO_CPTH      : $$=New_action_cpt_h( alias, options );    break;
                         case MNEMO_CPT_IMP   : $$=New_action_cpt_imp( alias, options );  break;
                         case MNEMO_MOTIF     : $$=New_action_icone( alias, options );    break;
                         case MNEMO_WATCHDOG  : $$=New_action_WATCHDOG( alias, options ); break;
                         default: { Emettre_erreur_new( "'%s:%s' syntax error", alias->tech_id, alias->acronyme );
                                    $$=New_action();
                                    taille = 2;
                                    $$->alors = New_chaine( taille );
                                    g_snprintf( $$->alors, taille, " " );
                                    $$->sinon = NULL;
                                  }
                       }
                      g_list_free(options);
                    }
                   Liberer_options($4);                                                    /* On libére les options "locales" */
                   if ($3) g_free($3);
                   g_free($2);
                }}
                ;

comparateur:    ordre VALF
                {{ $$ = New_comparateur();
                   $$->type = $1;
                   $$->valf = $2;
                }}
                |     {{ $$=NULL; }}
                ;

barre:          BARRE {{ $$=1; }}
                |     {{ $$=0; }}
                ;
modulateur:     APRES        {{ $$=APRES;  }}
                | AVANT      {{ $$=AVANT;  }}
                |            {{ $$=0;      }}
                ;
jour_semaine:   LUNDI        {{ $$=1; }}
                | MARDI      {{ $$=2; }}
                | MERCREDI   {{ $$=3; }}
                | JEUDI      {{ $$=4; }}
                | VENDREDI   {{ $$=5; }}
                | SAMEDI     {{ $$=6; }}
                | DIMANCHE   {{ $$=0; }}
                ;
ordre:          INF | SUP | INF_OU_EGAL | SUP_OU_EGAL | T_EGAL
                ;
/**************************************************** Gestion des options *****************************************************/
liste_options:  T_POUV options T_PFERM   {{ $$ = $2;   }}
                |                        {{ $$ = NULL; }}
                ;

options:        options VIRGULE une_option
                {{ $$ = g_list_append( $1, $3 );
                }}
                | une_option {{ $$ = g_list_append( NULL, $1 ); }}
                ;

une_option:     MODE T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = MODE;
                   $$->entier = $3;
                }}
                | T_CONSIGNE T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = T_CONSIGNE;
                   $$->entier = $3;
                }}
                | COLOR T_EGAL couleur
                {{ $$=New_option();
                   $$->type = COLOR;
                   $$->entier = $3;
                }}
                | T_LIBELLE T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_LIBELLE;
                   $$->chaine = $3;
                }}
                | T_FORME T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_FORME;
                   $$->chaine = $3;
                }}
                | T_ETIQUETTE T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_ETIQUETTE;
                   $$->chaine = $3;
                }}
                | CLIGNO
                {{ $$=New_option();
                   $$->type = CLIGNO;
                   $$->entier = 1;
                }}
                | CLIGNO T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = CLIGNO;
                   $$->entier = $3;
                }}
                | T_UNITE T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_UNITE;
                   $$->chaine = $3;
                }}
                | RESET
                {{ $$=New_option();
                   $$->type = RESET;
                   $$->entier = 1;
                }}
                | RESET T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = RESET;
                   $$->entier = $3;
                }}
                | T_UPDATE
                {{ $$=New_option();
                   $$->type = T_UPDATE;
                   $$->entier = 1;
                }}
                | RATIO T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = RATIO;
                   $$->entier = $3;
                }}
                | T_EDGE_UP
                {{ $$=New_option();
                   $$->type = T_EDGE_UP;
                   $$->entier = 1;
                }}
                | T_EDGE_DOWN
                {{ $$=New_option();
                   $$->type = T_EDGE_DOWN;
                   $$->entier = 1;
                }}
                | T_IN_RANGE
                {{ $$=New_option();
                   $$->type = T_IN_RANGE;
                   $$->entier = 1;
                }}
                | T_DAA T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = T_DAA;
                   $$->entier = $3;
                }}
                | T_DMINA T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = T_DMINA;
                   $$->entier = $3;
                }}
                | T_DMAXA T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = T_DMAXA;
                   $$->entier = $3;
                }}
                | T_DAD T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = T_DAD;
                   $$->entier = $3;
                }}
                | T_RANDOM T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = T_RANDOM;
                   $$->entier = $3;
                }}
                | T_TYPE T_EGAL type_msg
                {{ $$=New_option();
                   $$->type = T_TYPE;
                   $$->entier = $3;
                }}
                | T_HOST T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_HOST;
                   $$->chaine = $3;
                }}
                | T_THREAD T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_THREAD;
                   $$->chaine = $3;
                }}
                | T_TAG T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_TAG;
                   $$->chaine = $3;
                }}
                | T_PARAM1 T_EGAL dyn_string
                {{ $$=$3;
                   $$->type = T_PARAM1;
                }}
                ;

dyn_string:     T_CHAINE
                {{ gint taille_chaine;
                   $$=New_option();
                   $$->type = T_CHAINE;
                   taille_chaine = strlen($1)+100;
                   $$->chaine = g_try_malloc0(taille_chaine);
                   g_snprintf( $$->chaine, taille_chaine, "strdup(\"%s\")", $1 );
                   g_free($1);
                }}
                | T_DYN_STRING T_POUV T_CHAINE VIRGULE ID suffixe T_PFERM
                {{ gchar *tech_id, *acro;
                   struct ALIAS *alias;
                   gint taille_chaine;
                   if ($6) { tech_id = $5; acro = $6; }
                      else { tech_id = NULL; acro = $5; }
                   alias = Get_alias_par_acronyme(tech_id,acro);                                       /* On recupere l'alias */
                   if (!alias && $6) { alias = Set_new_external_alias(tech_id,acro); }/* Si dependance externe, on va chercher */
                   alias = Get_alias_par_acronyme(tech_id, acro);
                   if (!alias)
                    { if ($6) Emettre_erreur_new( "'%s:%s' is not defined", tech_id, acro );
                         else Emettre_erreur_new( "'%s' is not defined", acro );
                      $$=New_option();
                      $$->type = T_DYN_STRING;
                      $$->chaine = strdup("error");
                    }
                   else
                    { $$=New_option();
                      taille_chaine = strlen($3)+100;
                      $$->type = T_DYN_STRING;
                      $$->chaine = g_try_malloc0(taille_chaine);
                      g_snprintf( $$->chaine, taille_chaine, "Dls_dyn_string(\"%s\",%d,\"%s\",\"%s\", &_%s_%s)",
                                  $3, alias->classe, alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
                    }
                   g_free($3);
                   g_free($5);
                   if ($6) g_free($6);
                }}
                ;

couleur:        ROUGE | VERT | BLEU | JAUNE | NOIR | BLANC | GRIS | ORANGE | KAKI
                ;
type_msg:         T_INFO    {{ $$=MSG_ETAT; }}
                | T_ATTENTE {{ $$=MSG_ATTENTE; }}
                | T_DEFAUT  {{ $$=MSG_DEFAUT; }}
                | T_ALARME  {{ $$=MSG_ALARME; }}
                | T_VEILLE  {{ $$=MSG_VEILLE; }}
                | T_ALERTE  {{ $$=MSG_ALERTE; }}
                | T_DANGER  {{ $$=MSG_DANGER; }}
                | T_DERANGEMENT {{ $$=MSG_DERANGEMENT; }}
                ;

%%

/*----------------------------------------------------------------------------------------------------------------------------*/
