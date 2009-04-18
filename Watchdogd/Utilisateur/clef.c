/**********************************************************************************************************/
/* Watchdogd/Db/Utilisateur/clef.c             Recuperation ID et clef utilisateur                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                     ven 03 avr 2009 20:31:01 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * clef.c
 * This file is part of <Watchdog
 *
 * Copyright (C) 2009 - sebastien
 *
 * <Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 #include <sql.h>                                             /* Entetes de gestion de la base de données */
 #include <sqlext.h> 
 #include <sqltypes.h>
 #include <string.h>
 #include <stdlib.h>
 #include <openssl/evp.h>                                                        /* Procedure de cryptage */

 #include "Erreur.h"
 #include "Utilisateur_DB.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "proto_util.h"

/**********************************************************************************************************/
/* Set_password: Correspond au changement de password de l'utilisateur                                    */
/* Entrées: un log, une db, un id utilisateur, une clef, un password                                      */
/* Sortie: FALSE si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Set_password( struct LOG *log, struct DB *db, gchar *clef, struct CMD_UTIL_SETPASSWORD *util )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *crypt, *code_crypt;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Set_password: recherche failed: query=null", util->id );
       return(FALSE);
     }

    crypt = Crypter( log, clef, util->code_en_clair );
    if (!crypt)
     { Info( log, DEBUG_DB, "Set_password: cryptage password impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    code_crypt = Normaliser_chaine ( log, crypt );
    g_free(crypt);
    if (!code_crypt)
     { Info( log, DEBUG_DB, "Set_password: Normalisation code crypte impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
 
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET crypt = '%s', changepass=false, login_failed='0' WHERE id=%d",
                NOM_TABLE_UTIL, code_crypt, util->id );
    g_free(code_crypt);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Set_password: update failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       g_free(crypt);
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Set_password: update ok", util->id );
    EndQueryDB( log, db, hquery );
    g_free(crypt);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_clef: essaie de recuperer la clef de l'utilisateur                                           */
/* Entrée: un log, une db, un nom, un entier accessible pour l'id                                         */
/* Sortie: la clef freeable ou null si pb                                                                 */
/**********************************************************************************************************/
 gchar *Recuperer_clef ( struct LOG *log, struct DB *db, gchar *nom, gint *id )
  { guchar crypt_from_sql[NBR_CARAC_CODE_CRYPTE+1];
    gchar requete[200];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    SQLINTEGER nbr;                                                         /* Nombre de solution retenue */
    gchar id_from_sql[20];
    gchar *crypt;

    hquery = NewQueryDB( log, db );
    if ( !hquery )
     { Info( log, DEBUG_DB, "Recuperer_clef: alloc hquery failed" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind ID */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_clef: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &crypt_from_sql, NBR_CARAC_CODE_CRYPTE+1, NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_clef: erreur bind du crypt" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),
                "SELECT id, crypt"
                " FROM %s WHERE name=\'%s\'",
                 NOM_TABLE_UTIL, nom );
    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Recuperer_clef: erreur execution requete SQL", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     } else Info_c( log, DEBUG_DB, "Recuperer_clef: execution requete SQL OK", requete );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Recuperer_clef: Clef non trouvé dans la BDD", *id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_c( log, DEBUG_DB, "Recuperer_clef: Multiple solution", nom );

    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );
    *id = atoi(id_from_sql);                                                      /* Conversion en entier */

    crypt = (gchar *)g_malloc0( NBR_CARAC_CODE_CRYPTE );
    if (!crypt)
     { Info( log, DEBUG_DB, "Recuperer_clef: out of memory" );
       return(NULL);
     }
    memcpy( crypt, crypt_from_sql, NBR_CARAC_CODE_CRYPTE );
    return(crypt);
  }
/**********************************************************************************************************/
/* Crypter: cryptage du password en parametre, selon la clef en parametre et l'algo blowfish              */
/* Entrées: un log, un password, la clef                                                                  */
/* Sortie: la chaine cryptée necessite d'etre frée                                                        */
/**********************************************************************************************************/
 gchar *Crypter( struct LOG *log, gchar *clef, gchar *pass )
  { guchar iv[] = "Watchdog";                                                         /* IV par défaut !! */
    guchar outbuf[NBR_CARAC_CODE_CRYPTE];                                             /* On prévoit large */
    guchar password[NBR_CARAC_LOGIN_UTF8+1];                             /* Pour le formatage du password */
    EVP_CIPHER_CTX ctx;                                                           /* Contexte de cryptage */
    gint outlen, tmplen;
    gchar *result;

    if (!g_utf8_strlen(pass, -1))
     { Info( log, DEBUG_CRYPTO, "Crypter: password should not be empty" );
       return( NULL );
     }
    if (!strlen(clef))
     { Info( log, DEBUG_CRYPTO, "Crypter: clef should not be empty" );
       return( NULL );
     }

    memset( password, 0, sizeof(password) );
    memcpy( password, pass, g_utf8_strlen(pass, -1) );

    EVP_CIPHER_CTX_init(&ctx);                                                   /* Initialisation du CTX */
    EVP_EncryptInit(&ctx, EVP_bf_cbc(), password, iv);                           /* Choix de l'algorithme */
                                                          /* Cryptage de la clef par le password, phase 1 */
    if(!EVP_EncryptUpdate(&ctx, outbuf, &outlen, clef, strlen(clef)))
     { Info( log, DEBUG_CRYPTO, "Crypter: Encrypt phase 1 failed" );
       return(NULL);
     }

    if(!EVP_EncryptFinal(&ctx, outbuf + outlen, &tmplen))                            /* Cryptage, phase 2 */
     { Info( log, DEBUG_CRYPTO, "Crypter: Encrypt phase 2 failed" );
       return(NULL);
     }
    outlen += tmplen;
    EVP_CIPHER_CTX_cleanup(&ctx);                                                    /* Cryptage effectué */

    if (outlen>NBR_CARAC_CODE_CRYPTE)
     { Info_n( log, DEBUG_CRYPTO, "Crypter: clef cryptée tronquée !!", outlen );
       outlen = NBR_CARAC_CODE_CRYPTE;
     } else Info_n( log, DEBUG_CRYPTO, "Crypter: taille clef cryptée !!", outlen );

    result = (gchar *)g_malloc0( outlen );
    if (!result)
     { Info( log, DEBUG_CRYPTO, "Crypter: erreur allocation mémoire" );
       return(NULL);
     }
    memcpy( result, outbuf, outlen );
    return( result );
  }
/*--------------------------------------------------------------------------------------------------------*/
