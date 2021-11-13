/**********************************************************************************************************/
/* Watchdogd/TraductionDLS/mots.l        Définitions des mots DLS                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                    jeu. 24 juin 2010 19:37:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * mots.l
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
 
%{
#include <stdio.h>
#include <string.h>
#include "Proto_traductionDLS.h"
#include "lignes.h"
%}

/* On n'utilise pas la fonction yywrap associée */
%option noyywrap nounput noinput
/* On garde le numéro de ligne lors du scan des tokens */
%option yylineno


/* Expressions rationnelles de detection */
digit         [0-9]+
digitf        [-]?[0-9]+[.[0-9]+]?
id            [a-zA-Z][a-zA-Z_0-9-]+[a-zA-Z0-9]
blanc         [\t ]+

 /* Noms des environnements */
%x code_en_c
%x comment
%%
{blanc}                    ;

"\n"                       { }

 /******************************************** Methodes ***************************************************/
"_E"                       { DlsScanner_lval.val = ENTREE;      return ENTREE;      }
"_EA"                      { DlsScanner_lval.val = EANA;        return EANA;        }
"_A"                       { DlsScanner_lval.val = SORTIE;      return SORTIE;      }
"_I"                       { DlsScanner_lval.val = ICONE;       return ICONE;       }
"_B"                       { DlsScanner_lval.val = BI;          return BI;          }
"_M"                       { DlsScanner_lval.val = MONO;        return MONO;        }
"_T"                       { DlsScanner_lval.val = T_TEMPO;     return T_TEMPO;     }
"_R"                       { DlsScanner_lval.val = T_REGISTRE;  return T_REGISTRE;  }
"_CH"                      { DlsScanner_lval.val = CPT_H;       return CPT_H;       }
"_CI"                      { DlsScanner_lval.val = T_CPT_IMP;   return T_CPT_IMP;   }
"_MSG"                     { DlsScanner_lval.val = T_MSG;       return T_MSG;       }
"_START"                   { DlsScanner_lval.val = T_START;     return T_START;     }
"_TRUE"                    { DlsScanner_lval.val = T_TRUE;      return T_TRUE;      }
"_FALSE"                   { DlsScanner_lval.val = T_FALSE;     return T_FALSE;     }

 /***************************************** Methodes speciales ********************************************/
"_HEURE"                   { DlsScanner_lval.val = HEURE;       return HEURE;       }
"_APRES"                   { DlsScanner_lval.val = APRES;       return APRES;       }
"_AVANT"                   { DlsScanner_lval.val = AVANT;       return AVANT;       }
"_LUNDI"                   { DlsScanner_lval.val = LUNDI;       return LUNDI;       }
"_MARDI"                   { DlsScanner_lval.val = MARDI;       return MARDI;       }
"_MERCREDI"                { DlsScanner_lval.val = MERCREDI;    return MERCREDI;    }
"_JEUDI"                   { DlsScanner_lval.val = JEUDI;       return JEUDI;       }
"_VENDREDI"                { DlsScanner_lval.val = VENDREDI;    return VENDREDI;    }
"_SAMEDI"                  { DlsScanner_lval.val = SAMEDI;      return SAMEDI;      }
"_DIMANCHE"                { DlsScanner_lval.val = DIMANCHE;    return DIMANCHE;    }

 /************************************************ Couleurs ***********************************************/
"rouge"                    { DlsScanner_lval.val = ROUGE;       return ROUGE;       }
"vert"                     { DlsScanner_lval.val = VERT;        return VERT;        }
"bleu"                     { DlsScanner_lval.val = BLEU;        return BLEU;        }
"jaune"                    { DlsScanner_lval.val = JAUNE;       return JAUNE;       }
"noir"                     { DlsScanner_lval.val = NOIR;        return NOIR;        }
"orange"                   { DlsScanner_lval.val = ORANGE;      return ORANGE;      }
"gris"                     { DlsScanner_lval.val = GRIS;        return GRIS;        }
"blanc"                    { DlsScanner_lval.val = BLANC;       return BLANC;       }
"kaki"                     { DlsScanner_lval.val = KAKI;        return KAKI;        }
 /************************************************* Options ************************************************/
"mode"                     { DlsScanner_lval.val = MODE;        return MODE;        }
"reset"                    { DlsScanner_lval.val = RESET;       return RESET;       }
"ratio"                    { DlsScanner_lval.val = RATIO;       return RATIO;       }
"color"                    { DlsScanner_lval.val = COLOR;       return COLOR;       }
"cligno"                   { DlsScanner_lval.val = CLIGNO;      return CLIGNO;      }
"edge_up"                  { DlsScanner_lval.val = T_EDGE_UP;   return T_EDGE_UP;   }
 /*********************************************** Le reste ************************************************/
"."                        { DlsScanner_lval.val = ET;          return ET;          }
"+"                        { DlsScanner_lval.val = OU;          return OU;          }
"/"                        { DlsScanner_lval.val = BARRE;       return BARRE;       }
"*"                        { DlsScanner_lval.val = T_FOIS;      return T_FOIS;      }
"-"                        { DlsScanner_lval.val = MOINS;       return MOINS;       }
";"                        { DlsScanner_lval.val = PVIRGULE;    return PVIRGULE;    }
","                        { DlsScanner_lval.val = VIRGULE;     return VIRGULE;     }
"("                        { DlsScanner_lval.val = T_POUV;      return T_POUV;      }
")"                        { DlsScanner_lval.val = T_PFERM;     return T_PFERM;     }
"="                        { DlsScanner_lval.val = T_EGAL;      return T_EGAL;      }
"<->"                      { DlsScanner_lval.val = EQUIV;       return EQUIV;       }
"->"                       { DlsScanner_lval.val = DONNE;       return DONNE;       }
"<"                        { DlsScanner_lval.val = INF;         return INF;         }
">"                        { DlsScanner_lval.val = SUP;         return SUP;         }
"<="                       { DlsScanner_lval.val = INF_OU_EGAL; return INF_OU_EGAL; }
">="                       { DlsScanner_lval.val = SUP_OU_EGAL; return SUP_OU_EGAL; }

"CodeC"                    { BEGIN(code_en_c); }
<code_en_c>\n              { BEGIN(INITIAL); Emettre(DlsScanner_text); }
<code_en_c>.               { Emettre(DlsScanner_text); }

"/*"                       { BEGIN(comment); }
<comment>"*/"              { BEGIN(INITIAL); }
<comment>\n                {  }
<comment>.                 ;

{id}                       { DlsScanner_lval.chaine = (char *)strdup(DlsScanner_text); return ID;     }
{digit}                    { DlsScanner_lval.val    = atoi(DlsScanner_text);           return ENTIER; }
{digitf}                   { DlsScanner_lval.valf   = atof(DlsScanner_text);           return VALF;   }
.                          {  }
%%
