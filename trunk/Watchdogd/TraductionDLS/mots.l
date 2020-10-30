/******************************************************************************************************************************/
/* Watchdogd/TraductionDLS/mots.l        Définitions des mots DLS                                                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        jeu. 24 juin 2010 19:37:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
%option 8bit noyywrap nounput noinput
/* On garde le numéro de ligne lors du scan des tokens */
%option yylineno
 /*%option debug*/

/* Expressions rationnelles de detection */
digit         [0-9]+
digitf        [-]?[0-9]+[\.[0-9]+]?
id            [a-zA-Z][a-zA-Z_0-9]+[a-zA-Z0-9]
chaine        [^"]*
blanc         [\t ]+
newline       [\n\r]+

 /* Noms des environnements */
%x code_en_c
%x chaine_directe
%x comment

%%
{blanc}
{newline}

 /******************************************** Methodes ***************************************************/
"_EA"                      { DlsScanner_lval.val = EANA;            return EANA;            }
"_AO"                      { DlsScanner_lval.val = T_ANALOG_OUTPUT; return T_ANALOG_OUTPUT; }
"_DO"                      { DlsScanner_lval.val = T_DIGITAL_OUTPUT;return T_DIGITAL_OUTPUT;  }
"_E"                       { DlsScanner_lval.val = ENTREE;          return ENTREE;          }
"_A"                       { DlsScanner_lval.val = SORTIE;          return SORTIE;          }
"_I"                       { DlsScanner_lval.val = T_ICONE;         return T_ICONE;         }
"_B"                       { DlsScanner_lval.val = T_BI;            return T_BI;            }
"_M"                       { DlsScanner_lval.val = T_MONO;          return T_MONO;          }
"_T"                       { DlsScanner_lval.val = T_TEMPO;         return T_TEMPO;         }
"_R"                       { DlsScanner_lval.val = T_REGISTRE;      return T_REGISTRE;      }
"_CH"                      { DlsScanner_lval.val = T_CPT_H;         return T_CPT_H;         }
"_CI"                      { DlsScanner_lval.val = T_CPT_IMP;       return T_CPT_IMP;       }
"_HORLOGE"                 { DlsScanner_lval.val = T_HORLOGE;       return T_HORLOGE;       }
"_MSG"                     { DlsScanner_lval.val = T_MSG;           return T_MSG;           }
"_START"                   { DlsScanner_lval.val = T_START;         return T_START;         }
"_TRUE"                    { DlsScanner_lval.val = T_TRUE;          return T_TRUE;          }
"_FALSE"                   { DlsScanner_lval.val = T_FALSE;         return T_FALSE;         }
"_TOP_ALERTE"              { DlsScanner_lval.val = T_TOP_ALERTE;    return T_TOP_ALERTE;    }
"_TOP_ALERTE_FUGITIVE"     { DlsScanner_lval.val = T_TOP_ALERTE_FUGITIVE;return T_TOP_ALERTE_FUGITIVE;}
"_BUS"                     { DlsScanner_lval.val = T_BUS;           return T_BUS;           }

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


 /**************************************************** Status des vignette ****************************************************/
"_MEMSA_COMOUT"            { DlsScanner_lval.val = T_ACT_COMOUT; return T_ACT_COMOUT; }
"_MEMSA_DEFAUT_FIXE"       { DlsScanner_lval.val = T_ACT_DEFF;   return T_ACT_DEFF;   }
"_MEMSA_DEFAUT"            { DlsScanner_lval.val = T_ACT_DEF;    return T_ACT_DEF;    }
"_MEMSA_ALARME_FIXE"       { DlsScanner_lval.val = T_ACT_ALAF;   return T_ACT_ALAF;   }
"_MEMSA_ALARME"            { DlsScanner_lval.val = T_ACT_ALA;    return T_ACT_ALA;    }
"_MEMSA_OK"                { DlsScanner_lval.val = T_ACT_OK;     return T_ACT_OK;     }
"_MEMSSB_VEILLE"           { DlsScanner_lval.val = T_SBIEN_VEILLE;return T_SBIEN_VEILLE; }
"_MEMSSB_ALERTE_FIXE"      { DlsScanner_lval.val = T_SBIEN_ALEF; return T_SBIEN_ALEF; }
"_MEMSSB_ALERTE_FUGITIVE"  { DlsScanner_lval.val = T_SBIEN_ALE_FUGITIVE; return T_SBIEN_ALE_FUGITIVE; }
"_MEMSSB_ALERTE"           { DlsScanner_lval.val = T_SBIEN_ALE;  return T_SBIEN_ALE;  }
"_MEMSSP_DERANGEMENT_FIXE" { DlsScanner_lval.val = T_SPERS_DERF; return T_SPERS_DERF; }
"_MEMSSP_DERANGEMENT"      { DlsScanner_lval.val = T_SPERS_DER;  return T_SPERS_DER;  }
"_MEMSSP_DANGER_FIXE"      { DlsScanner_lval.val = T_SPERS_DANF; return T_SPERS_DANF; }
"_MEMSSP_DANGER"           { DlsScanner_lval.val = T_SPERS_DAN;  return T_SPERS_DAN;  }
"_MEMSSP_OK"               { DlsScanner_lval.val = T_SPERS_OK;   return T_SPERS_OK;   }
"_OSYN_ACQUIT"             { DlsScanner_lval.val = T_OSYN_ACQ;   return T_OSYN_ACQ;   }

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
"in_range"                 { DlsScanner_lval.val = T_IN_RANGE;  return T_IN_RANGE;  }
"edge_up"                  { DlsScanner_lval.val = T_EDGE_UP;   return T_EDGE_UP;   }
"libelle"                  { DlsScanner_lval.val = T_LIBELLE;   return T_LIBELLE;   }
"unite"                    { DlsScanner_lval.val = T_UNITE;     return T_UNITE;     }
"random"                   { DlsScanner_lval.val = T_RANDOM;    return T_RANDOM;    }
"update"                   { DlsScanner_lval.val = T_UPDATE;    return T_UPDATE;    }
"etiquette"                { DlsScanner_lval.val = T_ETIQUETTE; return T_ETIQUETTE; }
"daa"                      { DlsScanner_lval.val = T_DAA;       return T_DAA;       }
"dma"                      { DlsScanner_lval.val = T_DMINA;     return T_DMINA;     }
"dMa"                      { DlsScanner_lval.val = T_DMAXA;     return T_DMAXA;     }
"dad"                      { DlsScanner_lval.val = T_DAD;       return T_DAD;       }
"type"                     { DlsScanner_lval.val = T_TYPE;      return T_TYPE;      }
"info"                     { DlsScanner_lval.val = T_INFO;      return T_INFO;      }
"attente"                  { DlsScanner_lval.val = T_ATTENTE;   return T_ATTENTE;   }
"defaut"                   { DlsScanner_lval.val = T_DEFAUT;    return T_DEFAUT;    }
"alarme"                   { DlsScanner_lval.val = T_ALARME;    return T_ALARME;    }
"veille"                   { DlsScanner_lval.val = T_VEILLE;    return T_VEILLE;    }
"alerte"                   { DlsScanner_lval.val = T_ALERTE;    return T_ALERTE;    }
"derangement"              { DlsScanner_lval.val = T_DERANGEMENT;return T_DERANGEMENT;}
"danger"                   { DlsScanner_lval.val = T_DANGER;    return T_DANGER;    }
"host"                     { DlsScanner_lval.val = T_HOST;      return T_HOST;      }
"thread"                   { DlsScanner_lval.val = T_THREAD;    return T_THREAD;    }
"tag"                      { DlsScanner_lval.val = T_TAG;       return T_TAG;       }
"param1"                   { DlsScanner_lval.val = T_PARAM1;    return T_PARAM1;    }
"forme"                    { DlsScanner_lval.val = T_FORME;     return T_FORME;     }
 /*********************************************** Le reste ************************************************/
"#define"                  { DlsScanner_lval.val = T_DEFINE;    return T_DEFINE;    }
"dyn_string"               { DlsScanner_lval.val = T_DYN_STRING;return T_DYN_STRING;}
"switch"                   { DlsScanner_lval.val = T_SWITCH;    return T_SWITCH;    }
"."                        { DlsScanner_lval.val = ET;          return ET;          }
"+"                        { DlsScanner_lval.val = T_PLUS;      return T_PLUS;      }
"/"                        { DlsScanner_lval.val = BARRE;       return BARRE;       }
"*"                        { DlsScanner_lval.val = T_FOIS;      return T_FOIS;      }
"|"                        { DlsScanner_lval.val = T_PIPE;      return T_PIPE;      }
"-"                        { DlsScanner_lval.val = T_MOINS;     return T_MOINS;     }
";"                        { DlsScanner_lval.val = PVIRGULE;    return PVIRGULE;    }
","                        { DlsScanner_lval.val = VIRGULE;     return VIRGULE;     }
":"                        { DlsScanner_lval.val = T_DPOINTS;   return T_DPOINTS;   }
"("                        { DlsScanner_lval.val = T_POUV;      return T_POUV;      }
")"                        { DlsScanner_lval.val = T_PFERM;     return T_PFERM;     }
"{"                        { DlsScanner_lval.val = T_ACCOUV;    return T_ACCOUV;    }
"}"                        { DlsScanner_lval.val = T_ACCFERM;   return T_ACCFERM;   }
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

"\""                       { BEGIN(chaine_directe); }
<chaine_directe>"\""       { DlsScanner_text[yyleng-1]=0; DlsScanner_lval.chaine = (char *)strdup(DlsScanner_text); BEGIN(INITIAL); return T_CHAINE; }
<chaine_directe>.          { yymore(); }

{id}                       { DlsScanner_lval.chaine = (char *)strdup(DlsScanner_text); return ID;       }
{digit}                    { DlsScanner_lval.val    = atoi(DlsScanner_text);           return ENTIER;   }
{digitf}                   { DlsScanner_lval.valf   = atof(DlsScanner_text);           return VALF;     }

.                          { DlsScanner_lval.val = T_ERROR; return T_ERROR; }
 /*------------------------------------------------------------------------------------------------------*/
