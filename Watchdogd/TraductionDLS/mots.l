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
digit         [-]?[0-9]+
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
"_EA"                      { DlsScanner_lval.val = T_ANALOG_INPUT;  return T_ANALOG_INPUT;  }
"_AI"                      { DlsScanner_lval.val = T_ANALOG_INPUT;  return T_ANALOG_INPUT;  }
"_AO"                      { DlsScanner_lval.val = T_ANALOG_OUTPUT; return T_ANALOG_OUTPUT; }
"_E"                       { DlsScanner_lval.val = T_ENTREE;        return T_ENTREE;        }
"_DI"                      { DlsScanner_lval.val = T_ENTREE;        return T_ENTREE;        }
"_A"                       { DlsScanner_lval.val = SORTIE;          return SORTIE;          }
"_DO"                      { DlsScanner_lval.val = T_DIGITAL_OUTPUT;return T_DIGITAL_OUTPUT;}
"_I"                       { DlsScanner_lval.val = T_VISUEL;        return T_VISUEL;        }
"_VISUEL"                  { DlsScanner_lval.val = T_VISUEL;        return T_VISUEL;        }
"_B"                       { DlsScanner_lval.val = T_BI;            return T_BI;            }
"_M"                       { DlsScanner_lval.val = T_MONO;          return T_MONO;          }
"_T"                       { DlsScanner_lval.val = T_TEMPO;         return T_TEMPO;         }
"_TEMPO"                   { DlsScanner_lval.val = T_TEMPO;         return T_TEMPO;         }
"_R"                       { DlsScanner_lval.val = T_REGISTRE;      return T_REGISTRE;      }
"_REGISTRE"                { DlsScanner_lval.val = T_REGISTRE;      return T_REGISTRE;      }
"_CH"                      { DlsScanner_lval.val = T_CPT_H;         return T_CPT_H;         }
"_CI"                      { DlsScanner_lval.val = T_CPT_IMP;       return T_CPT_IMP;       }
"_HORLOGE"                 { DlsScanner_lval.val = T_HORLOGE;       return T_HORLOGE;       }
"_WATCHDOG"                { DlsScanner_lval.val = T_WATCHDOG;      return T_WATCHDOG;      }
"_MSG"                     { DlsScanner_lval.val = T_MSG;           return T_MSG;           }
"_START"                   { DlsScanner_lval.val = T_START;         return T_START;         }
"_TRUE"                    { DlsScanner_lval.val = T_TRUE;          return T_TRUE;          }
"_FALSE"                   { DlsScanner_lval.val = T_FALSE;         return T_FALSE;         }
"_TOP_ALERTE"              { DlsScanner_lval.val = T_TOP_ALERTE;    return T_TOP_ALERTE;    }
"_TOP_ALERTE_FUGITIVE"     { DlsScanner_lval.val = T_TOP_ALERTE_FUGITIVE;return T_TOP_ALERTE_FUGITIVE;}
"_BUS"                     { DlsScanner_lval.val = T_BUS;           return T_BUS;           }
"_PID"                     { DlsScanner_lval.val = T_PID;           return T_PID;           }
"_NOP"                     { DlsScanner_lval.val = T_NOP;           return T_NOP;           }

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
"rouge"                    { DlsScanner_lval.val = T_ROUGE;     return T_ROUGE;     }
"vert"                     { DlsScanner_lval.val = T_VERT;      return T_VERT;      }
"bleu"                     { DlsScanner_lval.val = T_BLEU;      return T_BLEU;      }
"cyan"                     { DlsScanner_lval.val = T_CYAN;      return T_CYAN;      }
"jaune"                    { DlsScanner_lval.val = T_JAUNE;     return T_JAUNE;     }
"noir"                     { DlsScanner_lval.val = T_NOIR;      return T_NOIR;      }
"orange"                   { DlsScanner_lval.val = T_ORANGE;    return T_ORANGE;    }
"gris"                     { DlsScanner_lval.val = T_GRIS;      return T_GRIS;      }
"blanc"                    { DlsScanner_lval.val = T_BLANC;     return T_BLANC;     }
"kaki"                     { DlsScanner_lval.val = T_KAKI;      return T_KAKI;      }
 /************************************************* Options ************************************************/
"mode"                     { DlsScanner_lval.val = T_MODE;      return T_MODE;      }
"reset"                    { DlsScanner_lval.val = T_RESET;     return T_RESET;     }
"multi"                    { DlsScanner_lval.val = T_MULTI;     return T_MULTI;     }
"ratio"                    { DlsScanner_lval.val = T_RATIO;     return T_RATIO;     }
"color"                    { DlsScanner_lval.val = T_COLOR;     return T_COLOR;     }
"cligno"                   { DlsScanner_lval.val = CLIGNO;      return CLIGNO;      }
"in_range"                 { DlsScanner_lval.val = T_IN_RANGE;  return T_IN_RANGE;  }
"edge_up"                  { DlsScanner_lval.val = T_EDGE_UP;   return T_EDGE_UP;   }
"edge_down"                { DlsScanner_lval.val = T_EDGE_DOWN; return T_EDGE_DOWN; }
"libelle"                  { DlsScanner_lval.val = T_LIBELLE;   return T_LIBELLE;   }
"unite"                    { DlsScanner_lval.val = T_UNITE;     return T_UNITE;     }
"groupe"                   { DlsScanner_lval.val = T_GROUPE;    return T_GROUPE;    }
"random"                   { DlsScanner_lval.val = T_RANDOM;    return T_RANDOM;    }
"update"                   { DlsScanner_lval.val = T_UPDATE;    return T_UPDATE;    }
"etiquette"                { DlsScanner_lval.val = T_ETIQUETTE; return T_ETIQUETTE; }
"daa"                      { DlsScanner_lval.val = T_DAA;       return T_DAA;       }
"dma"                      { DlsScanner_lval.val = T_DMINA;     return T_DMINA;     }
"dMa"                      { DlsScanner_lval.val = T_DMAXA;     return T_DMAXA;     }
"dad"                      { DlsScanner_lval.val = T_DAD;       return T_DAD;       }
"consigne"                 { DlsScanner_lval.val = T_CONSIGNE;  return T_CONSIGNE;  }
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
"tech_id"                  { DlsScanner_lval.val = T_TECH_ID;   return T_TECH_ID;   }
"tag"                      { DlsScanner_lval.val = T_TAG;       return T_TAG;       }
"target"                   { DlsScanner_lval.val = T_TARGET;    return T_TARGET;    }
"forme"                    { DlsScanner_lval.val = T_FORME;     return T_FORME;     }
"cadran"                   { DlsScanner_lval.val = T_CADRAN;    return T_CADRAN;    }
"input"                    { DlsScanner_lval.val = T_INPUT;     return T_INPUT;     }
"kp"                       { DlsScanner_lval.val = T_KP;        return T_KP;        }
"ki"                       { DlsScanner_lval.val = T_KI;        return T_KI;        }
"kd"                       { DlsScanner_lval.val = T_KD;        return T_KD;        }
"min"                      { DlsScanner_lval.val = T_MIN;       return T_MIN;       }
"max"                      { DlsScanner_lval.val = T_MAX;       return T_MAX;       }
"seuil_ntb"                { DlsScanner_lval.val = T_SEUIL_NTB; return T_SEUIL_NTB; }
"seuil_nb"                 { DlsScanner_lval.val = T_SEUIL_NB;  return T_SEUIL_NB;  }
"seuil_nh"                 { DlsScanner_lval.val = T_SEUIL_NH;  return T_SEUIL_NH;  }
"seuil_nth"                { DlsScanner_lval.val = T_SEUIL_NTH; return T_SEUIL_NTH; }
"decimal"                  { DlsScanner_lval.val = T_DECIMAL;   return T_DECIMAL;   }

 /*********************************************** Le reste ************************************************/
"#define"                  { DlsScanner_lval.val = T_DEFINE;    return T_DEFINE;    }
"#link"                    { DlsScanner_lval.val = T_LINK;      return T_LINK;      }
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
"--"                       { DlsScanner_lval.val = T_DIFFERE;   return T_DIFFERE;   }
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
{digitf}                   { DlsScanner_lval.valf   = atof(DlsScanner_text);           return T_VALF;   }

.                          { DlsScanner_lval.val = T_ERROR; return T_ERROR; }
 /*------------------------------------------------------------------------------------------------------*/