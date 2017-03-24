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
 int ligne_source_dls=1;
%}

 /* On n'utilise pas la fonction yywrap associée */
%option noyywrap

 /* Expressions rationnelles de detection */
digit         [0-9]+
digitf        [-]?[0-9]+[.[0-9]+]?
id            [a-zA-Z][a-zA-Z_0-9-]+[a-zA-Z_0-9]
blanc         [\t ]+

 /* Noms des environnements */
%x code_en_c
%x comment
%%
{blanc}                    ;

"\n"                       { ligne_source_dls++; }

 /******************************************** Methodes ***************************************************/
"_E"                       { Dls_lval.val = ENTREE;      return ENTREE;      }
"_EA"                      { Dls_lval.val = EANA;        return EANA;        }
"_A"                       { Dls_lval.val = SORTIE;      return SORTIE;      }
"_I"                       { Dls_lval.val = ICONE;       return ICONE;       }
"_B"                       { Dls_lval.val = BI;          return BI;          }
"_M"                       { Dls_lval.val = MONO;        return MONO;        }
"_T"                       { Dls_lval.val = T_TEMPO;     return T_TEMPO;     }
"_R"                       { Dls_lval.val = T_REGISTRE;  return T_REGISTRE;  }
"_CH"                      { Dls_lval.val = CPT_H;       return CPT_H;       }
"_CI"                      { Dls_lval.val = CPT_IMP;     return CPT_IMP;     }
"_MSG"                     { Dls_lval.val = T_MSG;       return T_MSG;       }
"_START"                   { Dls_lval.val = T_START;     return T_START;     }
"_TRUE"                    { Dls_lval.val = T_TRUE;      return T_TRUE;      }
"_FALSE"                   { Dls_lval.val = T_FALSE;     return T_FALSE;     }

 /***************************************** Methodes speciales ********************************************/
"_HEURE"                   { Dls_lval.val = HEURE;       return HEURE;       }
"_APRES"                   { Dls_lval.val = APRES;       return APRES;       }
"_AVANT"                   { Dls_lval.val = AVANT;       return AVANT;       }
"_LUNDI"                   { Dls_lval.val = LUNDI;       return LUNDI;       }
"_MARDI"                   { Dls_lval.val = MARDI;       return MARDI;       }
"_MERCREDI"                { Dls_lval.val = MERCREDI;    return MERCREDI;    }
"_JEUDI"                   { Dls_lval.val = JEUDI;       return JEUDI;       }
"_VENDREDI"                { Dls_lval.val = VENDREDI;    return VENDREDI;    }
"_SAMEDI"                  { Dls_lval.val = SAMEDI;      return SAMEDI;      }
"_DIMANCHE"                { Dls_lval.val = DIMANCHE;    return DIMANCHE;    }

 /************************************************ Couleurs ***********************************************/
"rouge"                    { Dls_lval.val = ROUGE;       return ROUGE;       }
"vert"                     { Dls_lval.val = VERT;        return VERT;        }
"bleu"                     { Dls_lval.val = BLEU;        return BLEU;        }
"jaune"                    { Dls_lval.val = JAUNE;       return JAUNE;       }
"noir"                     { Dls_lval.val = NOIR;        return NOIR;        }
"orange"                   { Dls_lval.val = ORANGE;      return ORANGE;      }
"gris"                     { Dls_lval.val = GRIS;        return GRIS;        }
"blanc"                    { Dls_lval.val = BLANC;       return BLANC;       }
"kaki"                     { Dls_lval.val = KAKI;        return KAKI;        }
 /************************************************* Options ************************************************/
"mode"                     { Dls_lval.val = MODE;        return MODE;        }
"reset"                    { Dls_lval.val = RESET;       return RESET;       }
"ratio"                    { Dls_lval.val = RATIO;       return RATIO;       }
"color"                    { Dls_lval.val = COLOR;       return COLOR;       }
"cligno"                   { Dls_lval.val = CLIGNO;      return CLIGNO;      }
 /*********************************************** Le reste ************************************************/
"."                        { Dls_lval.val = ET;          return ET;          }
"+"                        { Dls_lval.val = OU;          return OU;          }
"/"                        { Dls_lval.val = BARRE;       return BARRE;       }
"*"                        { Dls_lval.val = T_FOIS;      return T_FOIS;      }
"-"                        { Dls_lval.val = MOINS;       return MOINS;       }
";"                        { Dls_lval.val = PVIRGULE;    return PVIRGULE;    }
","                        { Dls_lval.val = VIRGULE;     return VIRGULE;     }
"("                        { Dls_lval.val = T_POUV;      return T_POUV;      }
")"                        { Dls_lval.val = T_PFERM;     return T_PFERM;     }
"="                        { Dls_lval.val = T_EGAL;      return T_EGAL;      }
"<->"                      { Dls_lval.val = EQUIV;       return EQUIV;       }
"->"                       { Dls_lval.val = DONNE;       return DONNE;       }
"<"                        { Dls_lval.val = INF;         return INF;         }
">"                        { Dls_lval.val = SUP;         return SUP;         }
"<="                       { Dls_lval.val = INF_OU_EGAL; return INF_OU_EGAL; }
">="                       { Dls_lval.val = SUP_OU_EGAL; return SUP_OU_EGAL; }

"CodeC"                    { BEGIN(code_en_c); }
<code_en_c>\n              { BEGIN(INITIAL); Emettre(Dls_text); ligne_source_dls++; }
<code_en_c>.               { Emettre(Dls_text); }

"/*"                       { BEGIN(comment); }
<comment>"*/"              { BEGIN(INITIAL); }
<comment>\n                { ligne_source_dls++; }
<comment>.                 ;

{id}                       { Dls_lval.chaine = (char *)strdup(Dls_text); return ID;     }
{digit}                    { Dls_lval.val = atoi(Dls_text);              return ENTIER; }
{digitf}                   { Dls_lval.valf = atof(Dls_text);             return VALF;   }
.                          {  }
%%
