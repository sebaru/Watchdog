/**********************************************************************************************************/
/* Client/Config_cli/Config_cli_mots.l        Définitions des mots de config du client Watchdog           */
/* Projet WatchDog version 1.7       Gestion d'habitat                      sam 07 jun 2003 14:44:47 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

%{
#include <stdio.h>
#include <string.h>
#include "Config_cli_lignes.h"
 int ligne=1;

%}

digit         [0-9]+
chaine        [a-zA-Z_/][a-zA-Z0-9_/.]+
blanc         [\t ]+

%x comment
%%
{blanc}                    ;

\n                         { ligne++; }
"true"                     { yylval.val = 1;
                             return ENTIER;
                           }
"false"                    { yylval.val = 0;
                             return ENTIER;
                           }
"serveur"                  { yylval.val = SERVEUR;
                             return SERVEUR;
                           }
"SSL_crypt"                { yylval.val = SSL_CRYPT;
                             return SSL_CRYPT;
                           }
"port"                     { yylval.val = PORT;
                             return PORT;
                           }
"taille_bloc_reseau"       { yylval.val = TAILLE_BLOC_RESEAU;
                             return TAILLE_BLOC_RESEAU;
                           }
"debug"                    { yylval.val = DEBUG;
                             return(DEBUG);

                           }
"CONFIG"                   { yylval.val = D_CONFIG;
                             return(D_CONFIG);
                           }
"SIGNAUX"                  { yylval.val = D_SIGNAUX;
                             return(D_SIGNAUX);
                           }
"DB"                       { yylval.val = D_DB;
                             return(D_DB);
                           }
"MEM"                      { yylval.val = D_MEM;
                             return(D_MEM);
                           }
"USER"                     { yylval.val = D_USER;
                             return(D_USER);
                           }
"CRYPTO"                   { yylval.val = D_CRYPTO;
                             return(D_CRYPTO);
                           }
"INFO"                     { yylval.val = D_INFO;
                             return(D_INFO);
                           }
"ALL"                      { yylval.val = D_ALL;
                             return(D_ALL);
                           }
"CDG"                      { yylval.val = D_CDG;
                             return(D_CDG);
                           }
"NETWORK"                  { yylval.val = D_NETWORK;
                             return(D_NETWORK);
                           }
"FORK"                     { yylval.val = D_FORK;
                             return(D_FORK);
                           }
"CONNEXION"                { yylval.val = D_CONNEXION;
                             return(D_CONNEXION);
                           }
"="                        { yylval.val = EGAL;
                             return EGAL;
                           }                        
","                        { yylval.val = VIRGULE;
                             return VIRGULE;
                           }                        
"#"                        { BEGIN(comment); }
<comment>\n                { ligne++; BEGIN(INITIAL); }
<comment>.                 ;

{chaine}                   { yylval.chaine = (char *)strdup(yytext);
                             return CHAINE;
                           }
{digit}                    { yylval.val = atoi(yytext);
                             return ENTIER;
                           }
.                          { return 0; }

%%
