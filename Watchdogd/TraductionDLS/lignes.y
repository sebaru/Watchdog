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

%union { gint val;
         gdouble valf;
         gchar *chaine;
         GList *gliste;
         struct OPTION *option;
         struct ACTION *action;
         struct ALIAS *t_alias;
         struct CONDITION *t_condition;
         struct INSTRUCTION *t_instruction;
       };

%token <val>    T_ERROR PVIRGULE VIRGULE T_DPOINTS DONNE EQUIV T_MOINS T_POUV T_PFERM T_EGAL T_PLUS ET BARRE T_FOIS
%token <val>    T_SWITCH T_ACCOUV T_ACCFERM T_PIPE T_DIFFERE
%token <val>    T_DEFINE T_LINK

%token <val>    T_TOP_ALERTE T_TOP_ALERTE_FUGITIVE
%token <val>    T_BUS T_HOST T_TECH_ID T_TAG T_TARGET

%token <val>    T_MODE T_COLOR CLIGNO T_RESET T_RATIO T_MULTI T_LIBELLE T_ETIQUETTE T_GROUPE T_UNITE T_FORME
%token <val>    T_PID T_KP T_KI T_KD T_INPUT
%token <val>    T_DAA T_DMINA T_DMAXA T_DAD T_RANDOM T_UPDATE T_CONSIGNE T_ALIAS

%token <val>    T_TYPE T_INFO T_ATTENTE T_DEFAUT T_ALARME T_VEILLE T_ALERTE T_DERANGEMENT T_DANGER
%type  <val>    type_msg

%token <val>    INF SUP INF_OU_EGAL SUP_OU_EGAL T_TRUE T_FALSE T_NOP
%type  <val>    ordre

%token <val>    HEURE APRES AVANT LUNDI MARDI MERCREDI JEUDI VENDREDI SAMEDI DIMANCHE
%type  <val>    modulateur jour_semaine

%token <val>    T_BI T_MONO T_ENTREE SORTIE T_ANALOG_OUTPUT T_TEMPO T_HORLOGE
%token <val>    T_MSG T_VISUEL T_CPT_H T_CPT_IMP T_ANALOG_INPUT T_START T_REGISTRE T_DIGITAL_OUTPUT T_WATCHDOG
%type  <val>    alias_classe

%token <val>    T_ROUGE T_VERT T_BLEU T_JAUNE T_NOIR T_BLANC T_ORANGE T_GRIS T_KAKI T_CYAN
%type  <chaine>  couleur

%token <val>    T_EDGE_UP T_EDGE_DOWN T_IN_RANGE

%token <val>    T_CADRAN T_MIN T_MAX T_SEUIL_NTB T_SEUIL_NB T_SEUIL_NH T_SEUIL_NTH T_DECIMAL

%token <chaine> T_CHAINE
%token <chaine> ID
%token <val>    ENTIER
%token <valf>   T_VALF

%type  <val>         barre
%type  <gliste>      liste_options options
%type  <option>      une_option
%type  <t_condition>   unite facteur expr expr_2
%type  <chaine>      listeCase listeInstr
%type  <t_instruction> une_instr
%type  <action>      liste_action une_action
%type  <t_alias>     un_alias

%%
fichier: listeDefinitions listeInstr {{ if($2) { Emettre( $2 ); g_free($2); } }}
                        ;

/*************************************************** Gestion des alias ********************************************************/
listeDefinitions:
                  une_definition listeDefinitions
                | {{ }}
                ;

une_definition: T_DEFINE ID EQUIV alias_classe liste_options PVIRGULE
                {{ if ( Get_alias_par_acronyme(NULL, $2) )                                                   /* Deja defini ? */
                        { Emettre_erreur_new( "'%s' is already defined", $2 );
                          Liberer_options($5);
                        }
                   else { New_alias(NULL, $2, $4, $5); }
                   g_free($2);
                }}
                | T_LINK ID T_DPOINTS ID liste_options PVIRGULE
                {{ if ($2 && $4)
                    { if ( Get_alias_par_acronyme($2, $4) )                                                  /* Deja defini ? */
                       { Emettre_erreur_new( "'%s:%s' is already defined", $2, $3 );
                         Liberer_options($5);
                       }
                      else { New_external_alias($2, $4, $5); }
                    }
                   if ($2) g_free($2);
                   if ($4) g_free($4);
                }}
                ;

alias_classe:     T_BI             {{ $$=MNEMO_BISTABLE;       }}
                | T_MONO           {{ $$=MNEMO_MONOSTABLE;     }}
                | T_ENTREE         {{ $$=MNEMO_ENTREE;         }}
                | SORTIE           {{ $$=MNEMO_SORTIE;         }}
                | T_MSG            {{ $$=MNEMO_MSG;            }}
                | T_TEMPO          {{ $$=MNEMO_TEMPO;          }}
                | T_VISUEL         {{ $$=MNEMO_VISUEL;         }}
                | T_CPT_H          {{ $$=MNEMO_CPTH;           }}
                | T_CPT_IMP        {{ $$=MNEMO_CPT_IMP;        }}
                | T_ANALOG_INPUT   {{ $$=MNEMO_ENTREE_ANA;     }}
                | T_ANALOG_OUTPUT  {{ $$=MNEMO_SORTIE_ANA;     }}
                | T_DIGITAL_OUTPUT {{ $$=MNEMO_DIGITAL_OUTPUT; }}
                | T_REGISTRE       {{ $$=MNEMO_REGISTRE;       }}
                | T_HORLOGE        {{ $$=MNEMO_HORLOGE;        }}
                | T_BUS            {{ $$=MNEMO_BUS;            }}
                | T_WATCHDOG       {{ $$=MNEMO_WATCHDOG;       }}
                ;

/**************************************************** Gestion des instructions ************************************************/
listeInstr:     une_instr listeInstr
                {{ if ($1 && $1->condition->is_bool == FALSE && $2)
                    { gint taille = $1->condition->taille + $1->actions->taille_alors + 256;
                      $$ = New_chaine( taille + strlen($2) );
                      g_snprintf( $$, taille,
                                  "vars->num_ligne = %d; /* une_instr FLOAT-------*/\n"
                                  " { gdouble local_result=%s;\n"
                                  "   %s\n"
                                  " }\n %s\n", DlsScanner_get_lineno(), $1->condition->chaine, $1->actions->alors, $2 );
                    }
                   else if ($1 && $1->condition->is_bool == TRUE && $2)
                    { gint taille = $1->condition->taille + $1->actions->taille_alors + $1->actions->taille_sinon + strlen($2)+256;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille,
                                  "vars->num_ligne = %d; /* une_instr BOOL--------*/\n"
                                  " if (%s)\n {\n %s\n }\n else\n {\n %s\n }\n %s\n",
                                  DlsScanner_get_lineno(), $1->condition->chaine, $1->actions->alors, $1->actions->sinon, $2 );
                    } else $$=NULL;
/*else if ($2 && $6)
                    { gchar *alors = Liste_action_to_string_alors ( $6 );
                      gchar *sinon = Liste_action_to_string_sinon ( $6 );
                      gint taille = strlen($2->alors)+strlen(alors)+strlen(sinon)+1024;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille,
                                  "vars->num_ligne = %d; * une_instr différée----------*\n"
                                  " { static gboolean counting_on=FALSE;\n"
                                  "   static gboolean counting_off=FALSE;\n"
                                  "   static gint top;\n"
                                  "   if(%s)\n"
                                  "    { counting_off=FALSE;\n"
                                  "      if (counting_on==FALSE)\n"
                                  "       { counting_on=TRUE; top = Dls_get_top(); }\n"
                                  "      else\n"
                                  "       { if ( Dls_get_top() - top >= %d )\n"
                                  "          { %s\n"
                                  "          }\n"
                                  "       }\n"
                                  "    }\n"
                                  "   else\n"
                                  "    { counting_on = FALSE;\n"
                                  "      if (counting_off==FALSE)\n"
                                  "       { counting_off=TRUE; top = Dls_get_top(); }\n"
                                  "      else\n"
                                  "       { if ( Dls_get_top() - top >= %d )\n"
                                  "          { %s\n"
                                  "          }\n"
                                  "       }\n"
                                  "    }\n"
                                  " }\n\n",
                                  DlsScanner_get_lineno(), $2->alors,
                                  Get_option_entier($4, T_DAA, 0), alors,
                                  Get_option_entier($4, T_DAD, 0), sinon );
                       g_free(alors);
                       g_free(sinon);
                     } else $$=NULL;
*/
                   Del_instruction($1);
                   if ($2) g_free($2);
                }}
/****************************************************** Partie SWITCH *********************************************************/
                | T_SWITCH listeCase listeInstr
                {{ gint taille;
                   if ($2)
                    { taille = strlen($2);
                      if (taille==0)
                       { Emettre_erreur_new( "Switch sans action" ); }
                      else
                       { taille += strlen($3) + 100;
                         $$ = New_chaine( taille );
                         g_snprintf( $$, taille, "/* Ligne %d (CASE BEGIN)------------*/\n"
                                                 "%s\n"
                                                 "/* Ligne %d (CASE END)--------------*/\n %s\n",
                                                 DlsScanner_get_lineno(), $2, DlsScanner_get_lineno(), ($3 ? $3 : "") );
                       }
                    } else $$=NULL;
                   if ($2) g_free($2);
                   if ($3) g_free($3);
                }}
                | {{ $$=NULL; }}
                ;

une_instr:      T_MOINS expr DONNE liste_action PVIRGULE
                {{ $$=New_instruction ( $2, NULL, $4 ); }}
                | T_MOINS expr T_DIFFERE options DONNE liste_action PVIRGULE
                {{ $$=New_instruction ( $2, $4, $6 ); }}
/*                | T_MOINS expr DONNE T_ACCOUV listeInstr T_ACCFERM
                {{ int taille;
                   if ($2 && $2->is_bool == FALSE)
                    { Emettre_erreur_new( "Boolean is left mandatory" ); }
                   else if ($2 && $5)
                    { taille = strlen($2->alors)+strlen($5)+100;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille,
                                  "* Ligne %d une_instr if----------*\nif(%s)\n { %s }\n\n",
                                     DlsScanner_get_lineno(), $2->alors, $5 );
                    } else $$=NULL;
                   if ($5) g_free($5);
                   Del_condition($2);
                }}*/
                ;

listeCase:      T_PIPE une_instr listeCase
                {{ if ($2 && $2->condition && $2->condition->is_bool == FALSE)
                    { Emettre_erreur_new( "Boolean is left mandatory" ); $$=NULL; }
                   else if ($2)
                    { gint taille = $2->actions->taille_alors+$2->actions->taille_sinon+$2->condition->taille+256 + strlen($3);
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille,
                                  "/* Ligne %d (CASE INSIDE)----------*/\n"
                                  "if(%s)\n { %s }\nelse\n { %s\n%s }\n",
                                  DlsScanner_get_lineno(), $2->condition->chaine, $2->actions->alors,
                                  ($2->actions->sinon ? $2->actions->sinon : ""), ($3 ? $3 : "") );
                    }
                   Del_instruction($2);
                   if ($3) g_free($3);
                }}
                | T_PIPE T_MOINS DONNE liste_action PVIRGULE
                {{ if ($4)
                    { gint taille = $4->taille_alors+100;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille,
                                  "/* Ligne %d (CASE INSIDE DEFAULT)--*/\n"
                                  "  %s", DlsScanner_get_lineno(), $4->alors );
                    } else $$=NULL;
                   Del_actions($4);
                }}
                | {{ $$=NULL; }}
                ;
/******************************************************* Partie LOGIQUE *******************************************************/
expr:           facteur T_PLUS facteur
                {{ if ($1 && $3)
                    { if ($1->is_bool != $3->is_bool)
                       { Emettre_erreur_new( "Mixing Bool and Float is forbidden" ); $$=NULL; }
                      else
                       { $$ = New_condition( $1->is_bool, $1->taille + $3->taille + 6 );
                         if ($$ && $1->is_bool)
                          { g_snprintf( $$->chaine, $$->taille, "(%s || %s)", $1->chaine, $3->chaine ); }
                        else
                          { g_snprintf( $$->chaine, $$->taille, "(%s+%s)", $1->chaine, $3->chaine ); }
                       }
                    } else $$=NULL;
                   Del_condition($1);
                   Del_condition($3);
                }}
                | facteur T_MOINS facteur
                {{ if ($1 && $3)
                    { if ($1->is_bool == TRUE || $3->is_bool == TRUE)
                       { Emettre_erreur_new( "Boolean not allowed within -" ); $$=NULL; }
                      else
                       { gint taille = $1->taille + $3->taille + 3;
                         $$ = New_condition( FALSE, taille );
                         if ($$)
                          { g_snprintf( $$->chaine, taille, "(%s-%s)", $1->chaine, $3->chaine ); }
                       }
                    } else $$=NULL;
                   Del_condition($1);
                   Del_condition($3);
                }}
                | facteur
                ;
facteur:        expr_2 ET expr_2
                {{ if ($1 && $3)
                    { if ($1->is_bool == FALSE || $3->is_bool == FALSE)
                       { Emettre_erreur_new( "Boolean mandatory in AND" ); $$=NULL; }
                      else
                       { $$ = New_condition( TRUE, $1->taille + $3->taille + 6 );
                         if ($$)
                          { g_snprintf( $$->chaine, $$->taille, "(%s && %s)", $1->chaine, $3->chaine ); }
                       }
                    } else $$=NULL;
                   Del_condition($1);
                   Del_condition($3);
                }}
                | expr_2 T_FOIS expr_2
                {{ if ($1 && $3)
                    { if ($1->is_bool == TRUE || $3->is_bool == TRUE)
                       { Emettre_erreur_new( "Float mandatory in *" ); $$=NULL; }
                      else
                       { $$ = New_condition( FALSE, $1->taille + $3->taille + 3 );
                         if ($$)
                          { g_snprintf( $$->chaine, $$->taille, "(%s*%s)", $1->chaine, $3->chaine ); }
                       }
                    } else $$=NULL;
                   Del_condition($1);
                   Del_condition($3);
                }}
                | expr_2 BARRE expr_2
                {{ if ($1 && $3)
                    { if ($1->is_bool == TRUE || $3->is_bool == TRUE)
                       { Emettre_erreur_new( "Boolean not allowed within /" ); $$=NULL; }
                      else
                       { gint taille = $1->taille + $3->taille + 36;
                         $$ = New_condition( FALSE, taille );
                         if ($$)
                          { g_snprintf( $$->chaine, taille, "(%s==0.0 ? 1.0 : (%s/%s))", $3->chaine, $1->chaine, $3->chaine ); }
                       }
                    } else $$=NULL;
                   Del_condition($1);
                   Del_condition($3);
                }}
                | expr_2
                ;


expr_2:         unite ordre unite
                {{ $$ = New_condition_comparaison ( $1, $2, $3 );
                   Del_condition($1);
                   Del_condition($3);
                }}
                | unite
                ;

unite:          barre un_alias liste_options
                {{ $$ = New_condition_alias ( $1, $2, $3 );
                   if($$==NULL) Liberer_options($3);
                }}
                | T_VALF   {{ $$ = New_condition_valf ( $1 );   }}
                | ENTIER   {{ $$ = New_condition_entier ( $1 ); }}
                | HEURE T_POUV modulateur ENTIER T_DPOINTS ENTIER T_PFERM
                {{ if ($2>23) $2=23;
                   if ($2<0)  $2=0;
                   if ($4>59) $4=59;
                   if ($4<0)  $4=0;
                   $$ = New_condition( TRUE, 20 );
                   if ($$)
                    { switch ($1)
                       { case 0    : g_snprintf( $$->chaine, $$->taille, "Heure(%d,%d)", $2, $4 );
                                     break;
                         case APRES: g_snprintf( $$->chaine, $$->taille, "Heure_apres(%d,%d)", $2, $4 );
                                     break;
                         case AVANT: g_snprintf( $$->chaine, $$->taille, "Heure_avant(%d,%d)", $2, $4 );
                                     break;
                       }
                    }
                }}
                | jour_semaine
                {{ $$ = New_condition( TRUE, 18 );
                   if ($$) g_snprintf( $$->chaine, $$->taille, "Jour_semaine(%d)", $1 );
                }}
                | T_START
                {{ $$ = New_condition( TRUE, 20 );
                   if ($$) g_snprintf( $$->chaine, $$->taille, "(vars->resetted)" );
                }}
                | T_TRUE
                {{ $$ = New_condition( TRUE, 5 );
                   if ($$) g_snprintf( $$->chaine, $$->taille, "TRUE" );
                }}
                | T_FALSE
                {{ $$ = New_condition( TRUE, 5 );
                   if ($$) g_snprintf( $$->chaine, $$->taille, "FALSE" );
                }}
                | barre T_TOP_ALERTE
                {{ $$ = New_condition( TRUE, 25 );
                   if ($$)
                    { if ($1) g_snprintf( $$->chaine, $$->taille, "(!Dls_get_top_alerte())" );
                      else    g_snprintf( $$->chaine, $$->taille, "( Dls_get_top_alerte())" );
                    }
                }}
                | barre T_TOP_ALERTE_FUGITIVE
                {{ int taille;
                   taille = 35;
                   $$ = New_condition( TRUE, taille );
                   if ($$)
                    { if ($1) g_snprintf( $$->chaine, taille, "(!Dls_get_top_alerte_fugitive())" );
                      else    g_snprintf( $$->chaine, taille, "( Dls_get_top_alerte_fugitive())" );
                    }
                }}
                | barre T_POUV expr T_PFERM
                {{ if ($3)
                    { if ($1 && $3->is_bool == FALSE) Emettre_erreur_new( "'!' allow only with boolean" );
                      else
                       { $$ = New_condition( $3->is_bool, $3->taille+3 );
                         if ($1) { g_snprintf( $$->chaine, $$->taille, "!(%s)", $3->chaine ); }
                         else    { g_snprintf( $$->chaine, $$->taille, "(%s)", $3->chaine ); }
                       }
                    } else $$=NULL;
                   Del_condition($3);
                }}
/************************************** Partie Logique : gestion des comparaisons *********************************************/



/************************************************* Gestion des actions ********************************************************/
liste_action:   liste_action VIRGULE une_action
                {{ if ($1 && $3)
                    { $$ = New_action();
                      $$->alors = g_strconcat ( $1->alors, $3->alors, NULL );
                      if ($$->alors) $$->taille_alors = strlen($$->alors);
                      $$->sinon = g_strconcat ( $1->sinon, $3->sinon, NULL );
                      if ($$->sinon) $$->taille_sinon = strlen($$->sinon);
                    } else $$=NULL;
                   Del_actions ($1);
                   Del_actions ($3);
                }}
                | une_action
                {{ $$=$1;
                   if ($$->alors) $$->taille_alors = strlen($$->alors);
                   if ($$->sinon) $$->taille_sinon = strlen($$->sinon);
                }}
                ;

une_action:     T_NOP
                  {{ $$=New_action(); $$->alors=g_strdup(""); }}
                | T_PID liste_options
                  {{ $$=New_action_PID($2);
                     Liberer_options($2);
                  }}
                | barre un_alias liste_options
                {{ struct ALIAS *alias;                                                   /* Definition des actions via alias */
                   alias = $2;                                       /* On recupere l'alias */
                   if (!alias) { $$ = NULL; }
                   else                                                           /* L'alias existe, vérifions ses parametres */
                    { GList *options, *options_g, *options_d;
                      options_g = g_list_copy( $3 );
                      options_d = g_list_copy( alias->options );
                      options = g_list_concat( options_g, options_d );                  /* Concaténation des listes d'options */
                      if ($1 && (alias->classe==MNEMO_TEMPO ||
                                 alias->classe==MNEMO_MSG ||
                                 alias->classe==MNEMO_BUS ||
                                 alias->classe==MNEMO_VISUEL ||
                                 alias->classe==MNEMO_DIGITAL_OUTPUT ||
                                 alias->classe==MNEMO_WATCHDOG ||
                                 alias->classe==MNEMO_MONOSTABLE)
                         )
                       { Emettre_erreur_new( "'/%s' ne peut s'utiliser", alias->acronyme );
                         $$ = NULL;
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
                         case MNEMO_VISUEL     : $$=New_action_visuel( alias, options );    break;
                         case MNEMO_WATCHDOG  : $$=New_action_WATCHDOG( alias, options ); break;
                         default: { Emettre_erreur_new( "'%s:%s' syntax error", alias->tech_id, alias->acronyme );
                                    $$=NULL;
                                  }
                       }
                      g_list_free(options);
                    }
                   Liberer_options($3);                                                    /* On libére les options "locales" */
                }}
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
                             {{ $$ = g_list_append( $1, $3 );   }}
                | une_option {{ $$ = g_list_append( NULL, $1 ); }}
                ;

une_option:     T_CONSIGNE T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_GROUPE T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_LIBELLE T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = $3;
                }}
                | T_ETIQUETTE T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = $3;
                }}
                | T_UNITE T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = $3;
                }}
                | T_EDGE_UP
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = 1;
                }}
                | T_EDGE_DOWN
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = 1;
                }}
                | T_IN_RANGE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = 1;
                }}

                | T_HOST T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = $3;
                }}
                | T_TECH_ID T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = $3;
                }}
                | T_TAG T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = $3;
                }}
                | T_TARGET T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = $3;
                }}
                | T_MODE T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_MODE T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = $3;
                }}
                | T_COLOR T_EGAL couleur
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = g_strdup($3);
                }}
                | T_COLOR T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = $3;
                }}
                | T_FORME T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = $3;
                }}
                | CLIGNO
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = 1;
                }}
                | CLIGNO T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                ;
                | T_RESET
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = 1;
                }}
                | T_RESET T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_RATIO T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_MULTI T_EGAL T_VALF
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = $3;
                }}
                | T_UPDATE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = 1;
                }}
                | T_TYPE T_EGAL type_msg
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_DAA T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_DMINA T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_DMAXA T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_DAD T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_RANDOM T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_CADRAN T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_CHAINE;
                   $$->chaine = $3;
                }}
                | T_MIN T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = 1.0*$3;
                }}
                | T_MIN T_EGAL T_VALF
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = $3;
                }}
                | T_MAX T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = 1.0*$3;
                }}
                | T_MAX T_EGAL T_VALF
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = $3;
                }}
                | T_SEUIL_NTB T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = 1.0*$3;
                }}
                | T_SEUIL_NTB T_EGAL T_VALF
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = $3;
                }}
                | T_SEUIL_NB T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = 1.0*$3;
                }}
                | T_SEUIL_NB T_EGAL T_VALF
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = $3;
                }}
                | T_SEUIL_NH T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = 1.0*$3;
                }}
                | T_SEUIL_NH T_EGAL T_VALF
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = $3;
                }}
                | T_SEUIL_NTH T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = 1.0*$3;
                }}
                | T_SEUIL_NTH T_EGAL T_VALF
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = T_VALF;
                   $$->val_as_double = $3;
                }}
                | T_DECIMAL T_EGAL ENTIER
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ENTIER;
                   $$->val_as_int = $3;
                }}
                | T_CONSIGNE T_EGAL un_alias
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ID;
                   $$->val_as_alias = $3;
                }}
                | T_INPUT T_EGAL ID
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ID;
                   $$->val_as_alias = Get_alias_par_acronyme ( NULL, $3 );
                   if (!$$->val_as_alias)
                    { Emettre_erreur_new( "'%s' is not defined", $3 ); }
                }}
                | T_KP T_EGAL ID
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ID;
                   $$->val_as_alias = Get_alias_par_acronyme ( NULL, $3 );
                   if (!$$->val_as_alias)
                    { Emettre_erreur_new( "'%s' is not defined", $3 ); }
                }}
                | T_KI T_EGAL ID
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ID;
                   $$->val_as_alias = Get_alias_par_acronyme ( NULL, $3 );
                   if (!$$->val_as_alias)
                    { Emettre_erreur_new( "'%s' is not defined", $3 ); }
                }}
                | T_KD T_EGAL ID
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ID;
                   $$->val_as_alias = Get_alias_par_acronyme ( NULL, $3 );
                   if (!$$->val_as_alias)
                    { Emettre_erreur_new( "'%s' is not defined", $3 ); }
                }}
                | T_MIN T_EGAL ID
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ID;
                   $$->val_as_alias = Get_alias_par_acronyme ( NULL, $3 );
                   if (!$$->val_as_alias)
                    { Emettre_erreur_new( "'%s' is not defined", $3 ); }
                }}
                | T_MAX T_EGAL ID
                {{ $$=New_option();
                   $$->token = $1;
                   $$->token_classe = ID;
                   $$->val_as_alias = Get_alias_par_acronyme ( NULL, $3 );
                   if (!$$->val_as_alias)
                    { Emettre_erreur_new( "'%s' is not defined", $3 ); }
                }}
                ;

couleur:          T_ROUGE  {{ $$="red";       }}
                | T_VERT   {{ $$="green";     }}
                | T_BLEU   {{ $$="blue";      }}
                | T_JAUNE  {{ $$="yellow";    }}
                | T_NOIR   {{ $$="black";     }}
                | T_BLANC  {{ $$="white";     }}
                | T_GRIS   {{ $$="gray";      }}
                | T_ORANGE {{ $$="orange";    }}
                | T_KAKI   {{ $$="darkgreen"; }}
                | T_CYAN   {{ $$="lightblue"; }}
                ;
type_msg:         T_INFO        {{ $$=MSG_ETAT;        }}
                | T_ATTENTE     {{ $$=MSG_ATTENTE;     }}
                | T_DEFAUT      {{ $$=MSG_DEFAUT;      }}
                | T_ALARME      {{ $$=MSG_ALARME;      }}
                | T_VEILLE      {{ $$=MSG_VEILLE;      }}
                | T_ALERTE      {{ $$=MSG_ALERTE;      }}
                | T_DANGER      {{ $$=MSG_DANGER;      }}
                | T_DERANGEMENT {{ $$=MSG_DERANGEMENT; }}
                ;

un_alias:       ID
                {{ $$ = Get_alias_par_acronyme ( NULL, $1 );
                   if (!$$)
                    { Emettre_erreur_new( "'%s' is not defined", $1 ); }
                   g_free($1);
                }}
                | ID T_DPOINTS ID
                {{ $$ = Get_alias_par_acronyme ( $1, $3 );
                   if (!$$)
                    { $$ = New_external_alias( $1, $3, NULL ); }                          /* Si dependance externe, on va chercher */
                   if (!$$)
                    { Emettre_erreur_new( "'%s:%s' is not defined", $1, $3 ); }
                   g_free($1);
                   g_free($3);
                }}
%%

/*----------------------------------------------------------------------------------------------------------------------------*/
