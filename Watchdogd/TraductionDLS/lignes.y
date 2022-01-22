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
         struct COMPARATEUR *comparateur;
         struct ALIAS *t_alias;
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

%token <chaine> ID T_CHAINE
%token <val>    ENTIER
%token <valf>   T_VALF

%type  <val>         barre
%type  <gliste>      liste_options options
%type  <option>      une_option
%type  <chaine>      unite facteur expr suffixe unSwitch listeCase une_instr listeInstr
%type  <action>      action une_action
%type  <chaine>      calcul_expr calcul_expr2 calcul_expr3
%type  <t_alias>     un_alias calcul_alias_result
%type  <comparateur> comparateur

%%
fichier: ligne_source_dls;

ligne_source_dls:         listeDefinitions listeInstr {{ if($2) { Emettre( $2 ); g_free($2); } }}
                        | listeDefinitions
                        | listeInstr {{ if($1) { Emettre( $1 ); g_free($1); } }}
                        |
                        ;

/*************************************************** Gestion des alias ********************************************************/
listeDefinitions:
                  une_definition listeDefinitions
                | une_definition
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
                | T_VISUEL         {{ $$=MNEMO_VISUEL;          }}
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
                {{ if ($1 && $2)
                    { int taille = strlen($1) + strlen($2) + 1;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille, "%s%s", $1, $2 );
                    } else $$ = NULL;
                   if ($1) g_free($1);
                   if ($2) g_free($2);
                }}
                | une_instr
                {{ $$=$1; }}
                ;

une_instr:      T_MOINS expr DONNE action PVIRGULE
                {{ int taille;
                   if ($2 && $4)
                    { taille = strlen($2)+strlen($4->alors)+100;
                      if ($4->sinon)
                       { taille += (strlen($4->sinon) + 10);
                         $$ = New_chaine( taille );
                         g_snprintf( $$, taille,
                                     "vars->num_ligne = %d; /* une_instr-------------*/\nif(%s)\n { %s }\nelse\n { %s }\n\n",
                                     DlsScanner_get_lineno(), $2, $4->alors, $4->sinon );
                       }
                      else
                       { $$ = New_chaine( taille );
                         g_snprintf( $$, taille, "vars->num_ligne = %d;/* une_instr-------------*/\nif(%s)\n { %s }\n\n",
                                     DlsScanner_get_lineno(), $2, $4->alors );
                       }
                    } else $$=NULL;
                   if ($4)
                    { if ($4->sinon) g_free($4->sinon);
                      g_free($4->alors);
                      g_free($4);
                    }
                   if ($2) g_free($2);
                }}
                | T_MOINS expr T_DIFFERE options DONNE action PVIRGULE
                {{ int taille;
                   if ($2 && $6)
                    { taille = strlen($2)+strlen($6->alors)+1024;
                      if ($6->sinon) taille += strlen($6->sinon);
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille,
                                  "vars->num_ligne = %d; /* une_instr différée----------*/\n"
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
                                  DlsScanner_get_lineno(), $2,
                                  Get_option_entier($4, T_DAA, 0), $6->alors,
                                  Get_option_entier($4, T_DAD, 0),($6->sinon ? $6->sinon : "") );
                     } else $$=NULL;
                   if ($6 && $6->sinon) g_free($6->sinon);
                   if ($6 && $6->alors) g_free($6->alors);
                   if ($6) g_free($6);
                   Liberer_options($4);
                   if ($2) g_free($2);
                }}
                | T_MOINS expr T_MOINS T_POUV calcul_expr T_PFERM DONNE calcul_alias_result PVIRGULE
                {{ int taille;
                   if ($2 && $5 && $8)
                    { taille = strlen($5);
                      taille+= strlen($2);
                      taille+= strlen($8->tech_id);
                      taille+= strlen($8->acronyme);
                      taille+= 256;
                      $$ = New_chaine( taille );
                      if ($8->classe==MNEMO_SORTIE_ANA)
                       { g_snprintf( $$, taille,
                                     "vars->num_ligne = %d; /* une_instr-------------*/\n"
                                     "if(%s)\n { Dls_data_set_AO ( vars, \"%s\", \"%s\", &_%s_%s, \n    %s );\n }\n",
                                     DlsScanner_get_lineno(), $2, $8->tech_id, $8->acronyme, $8->tech_id, $8->acronyme, $5 );
                       }
                      else if ($8->classe==MNEMO_REGISTRE)
                       { g_snprintf( $$, taille,
                                     "vars->num_ligne = %d; /* une_instr-------------*/\n"
                                     "if(%s)\n { Dls_data_set_REGISTRE ( vars, \"%s\", \"%s\", &_%s_%s, \n    %s );\n }\n",
                                     DlsScanner_get_lineno(), $2, $8->tech_id, $8->acronyme, $8->tech_id, $8->acronyme, $5 );
                       }
                      else
                       { Emettre_erreur_new( "'%s:%s' is unknown", $8->tech_id, $8->acronyme ); }
                    } else $$=g_strdup("/* test ! */");
                   if ($2) g_free($2);
                   if ($5) g_free($5);
                   /* $8 est un alias, et ne doit pas etre g_freer */
                }}
                | T_MOINS expr DONNE T_ACCOUV listeInstr T_ACCFERM
                {{ int taille;
                   if ($2 && $5)
                    { taille = strlen($2)+strlen($5)+100;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille,
                                  "/* Ligne %d une_instr if----------*/\nif(%s)\n { %s }\n\n",
                                     DlsScanner_get_lineno(), $2, $5 );
                    } else $$=NULL;
                   if ($5) g_free($5);
                   if ($2) g_free($2);
                }}
                | unSwitch {{ $$=$1; }}
                ;

/****************************************************** Partie SWITCH *********************************************************/
unSwitch:       T_SWITCH listeCase
                {{ gint taille;
                   if ($2)
                    { taille = strlen($2);
                      if (taille==0)
                       { Emettre_erreur_new( "Switch sans action" ); }
                      else
                       { taille += 100;
                         $$ = New_chaine( taille );
                         g_snprintf( $$, taille, "/* Ligne %d (CASE BEGIN)------------*/\n"
                                                 "%s\n"
                                                 "/* Ligne %d (CASE END)--------------*/\n",
                                                 DlsScanner_get_lineno(), $2, DlsScanner_get_lineno() );
                       }
                    } else $$=NULL;
                   if ($2) g_free($2);
                }}
                ;

listeCase:      T_PIPE T_MOINS expr DONNE action PVIRGULE listeCase
                {{ int taille;
                   if ($3 && $5 && $7)
                    { taille = strlen($3)+strlen($5->alors)+strlen($7)+100;
                      if ($5->sinon) taille+=strlen($5->sinon);
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille,
                                  "/* Ligne %d (CASE INSIDE)----------*/\n"
                                  "if(%s)\n { %s }\nelse\n { %s\n%s }\n",
                                  DlsScanner_get_lineno(), $3, $5->alors, ($5->sinon ? $5->sinon : ""), $7 );
                    } else $$=NULL;
                   if ($5 && $5->sinon) g_free($5->sinon);
                   if ($5 && $5->alors) g_free($5->alors);
                   if ($5) g_free($5);
                   if ($3) g_free($3);
                   if ($7) g_free($7);
                }}
                | T_PIPE T_MOINS DONNE action PVIRGULE
                {{ int taille;
                   if ($4)
                    { taille = strlen($4->alors)+100;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille,
                                  "/* Ligne %d (CASE INSIDE DEFAULT)--*/\n"
                                 "  %s", DlsScanner_get_lineno(), $4->alors );
                    } else $$=NULL;
                   if ($4 && $4->sinon) g_free($4->sinon);
                   if ($4 && $4->alors) g_free($4->alors);
                   if ($4) g_free($4);
                }}
                | {{ $$=strdup(""); }}
                ;
/****************************************************** Partie CALCUL *********************************************************/
calcul_expr:    calcul_expr T_PLUS calcul_expr2
                {{ int taille;
                   if ($1 && $3)
                    { taille = strlen($1) + strlen($3) + 4;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille, "(%s+%s)", $1, $3 );
                    } else $$ = NULL;
                   if ($1) g_free($1);
                   if ($3) g_free($3);
                }}
                | calcul_expr T_MOINS calcul_expr2
                {{ int taille;
                   if ($1 && $3)
                    { taille = strlen($1) + strlen($3) + 4;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille, "(%s-%s)", $1, $3 );
                    } else $$ = NULL;
                   if ($1) g_free($1);
                   if ($3) g_free($3);
                }}
                | calcul_expr2
                ;
calcul_expr2:   calcul_expr2 T_FOIS calcul_expr3
                {{ int taille;
                   if ($1 && $3)
                    { taille = strlen($1) + strlen($3) + 4;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille, "(%s*%s)", $1, $3 );
                    } else $$ = NULL;
                   if ($1) g_free($1);
                   if ($3) g_free($3);
                }}
                | calcul_expr2 BARRE calcul_expr3
                {{ int taille;
                   if ($1 && $3)
                    { taille = strlen($1) + 2*strlen($3) + 40;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille, "((gdouble)%s==0.0 ? 1.0 : ((gdouble)%s/%s))", $3, $1, $3 );
                    } else $$ = NULL;
                   if ($1) g_free($1);
                   if ($3) g_free($3);
                }}
                | calcul_expr3
                ;
calcul_expr3:   T_POUV calcul_expr T_PFERM {{ $$=$2; }}
                | T_VALF
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
                | T_PID liste_options
                {{ $$ = New_calcul_PID ( $2 );
                }}
                | ID suffixe
                {{ char *tech_id, *acro;
                   struct ALIAS *alias;
                   int taille;
                   if ($2) { tech_id = $1; acro = $2; }
                      else { tech_id = NULL; acro = $1; }
                   alias = Get_alias_par_acronyme(tech_id,acro);                                       /* On recupere l'alias */
                   if (!alias)
                    { alias = New_external_alias(tech_id,acro,NULL); }           /* Si dependance externe, on va chercher */

                   if (alias)
                    { switch(alias->classe)               /* On traite que ce qui peut passer en "condition" */
                       { case MNEMO_REGISTRE:
                          { taille = 256;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "Dls_data_get_REGISTRE(\"%s\",\"%s\",&_%s_%s)",
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
                          { taille = 256;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "Dls_data_get_MONO(\"%s\",\"%s\",&_%s_%s)",
                                        alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
                            break;
                          }
                         case MNEMO_BISTABLE:
                          { taille = 256;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "Dls_data_get_BI(\"%s\",\"%s\",&_%s_%s)",
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

calcul_alias_result: ID
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
                   if ($1 && $3)
                    { taille = strlen($1)+strlen($3)+7;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille, "(%s || %s)", $1, $3 );
                    } else $$ = NULL;
                   if ($1) g_free($1);
                   if ($3) g_free($3);
                }}

                | facteur
                ;
facteur:        facteur ET unite
                {{ int taille;
                   if ($1 && $3)
                    { taille = strlen($1)+strlen($3)+7;
                      $$ = New_chaine( taille );
                      g_snprintf( $$, taille, "(%s && %s)", $1, $3 );
                    } else $$=NULL;
                   if ($1) g_free($1);
                   if ($3) g_free($3);
                }}
                | unite {{ $$ = $1; }}
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
                | barre T_POUV expr T_PFERM
                {{ int taille;
                   if ($3)
                    { taille = strlen($3)+5;
                      $$ = New_chaine( taille );
                      if ($1) { g_snprintf( $$, taille, "!(%s)", $3 ); }
                      else    { g_snprintf( $$, taille, "(%s)", $3 ); }
                    } else $$=NULL;
                   if ($3) g_free($3);
                }}
/************************************** Partie Logique : gestion des comparaisons *********************************************/
                | barre ID suffixe liste_options comparateur                                            /* Gestion des comparaisons */
                {{ if ($5)
                    { if ($1)
                       { Emettre_erreur_new( "'/' interdit dans une comparaison" );
                         $$ = NULL;
                       }
                      else $$ = New_condition_comparateur ( $2, $3, $4, $5 );
                    }
                   else
                    { $$ = New_condition_simple ( $1, $2, $3, $4 ); }
                   g_free($2);                                                         /* On n'a plus besoin de l'identifiant */
                   if ($3) g_free($3);                                                   /* Libération du prefixe s'il existe */
                   Liberer_options($4);
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

une_action:     T_NOP
                  {{ $$=New_action(); $$->alors=g_strdup(""); }}
                | T_PID liste_options
                  {{ $$=New_action_PID($2);
                     Liberer_options($2);
                  }}
                | barre ID suffixe liste_options
                {{ struct ALIAS *alias;                                                   /* Definition des actions via alias */
                   gchar *tech_id, *acro;
                   int taille;
                   if ($3) { tech_id = $2; acro = $3; }
                      else { tech_id = NULL; acro = $2; }

                   alias = Get_alias_par_acronyme(tech_id,acro);                                       /* On recupere l'alias */
                   if (!alias)
                    { alias = New_external_alias(tech_id,acro, NULL); }          /* Si dependance externe, on va chercher */

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
                                 alias->classe==MNEMO_VISUEL ||
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
                         case MNEMO_VISUEL     : $$=New_action_visuel( alias, options );    break;
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

comparateur:    ordre ID suffixe
                {{ $$ = New_comparateur();
                   $$->ordre = $1;
                   $$->token_classe = ID;
                   if ($3)
                    { $$->has_tech_id = TRUE;
                      g_snprintf ( $$->tech_id, sizeof($$->tech_id), "%s", $2 );
                      g_snprintf ( $$->acronyme, sizeof($$->acronyme), "%s", $3 );
                    }
                   else
                    { $$->has_tech_id = FALSE;
                      g_snprintf ( $$->acronyme, sizeof($$->acronyme), "%s", $2 );
                    }
                   if ($3) g_free($3);
                   g_free($2);
                }}
                | ordre ENTIER
                {{ $$ = New_comparateur();
                   $$->ordre = $1;
                   $$->token_classe = T_VALF;
                   $$->valf = 1.0*$2;
                }}
                | ordre T_VALF
                {{ $$ = New_comparateur();
                   $$->ordre = $1;
                   $$->token_classe = T_VALF;
                   $$->valf = $2;
                }}
                | {{ $$=NULL; }}
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
                    { Emettre_erreur_new( "'%s:%s' is not defined", $1, $3 ); }
                   g_free($1);
                   g_free($3);
                }}
%%

/*----------------------------------------------------------------------------------------------------------------------------*/
