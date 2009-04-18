/**********************************************************************************************************/
/* Watchdogd/Db/Synoptiques/motifs.c       Ajout/retrait de motifs dans les motifs                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mer 05 mai 2004 12:11:21 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * motifs.c
 * This file is part of <program name>
 *
 * Copyright (C) 2009 - sebastien
 *
 * <program name> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "Erreur.h"
 #include "Synoptiques_DB.h"

/**********************************************************************************************************/
/* Retirer_msgDB: Elimination d'un motif                                                                  */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_motifDB ( struct LOG *log, struct DB *db, struct CMD_ID_MOTIF *motif )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Retirer_motifDB: recherche failed: query=null" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MOTIF, motif->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Retirer_motifDB: elimination failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_motifDB: elimination ok" );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Max_id_motifsDB: Renvoie l'id maximum utilisé + 1 des motifs                                 */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_motifDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_motifDB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_motifDB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_MOTIF );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_motifDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_motifDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr!=0)
     { SQLFetch( hquery );
       id = atoi(id_from_sql) + 1;
     }
    else id = 0;
    EndQueryDB( log, db, hquery );
    return( id );
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_motifDB ( struct LOG *log, struct DB *db, struct CMD_ADD_MOTIF *motif )
  { gchar requete[512];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *libelle;
    gint id;

    id = Max_id_motifDB( log, db );
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_motifDB: recherche failed: query=null" );
       return(-1);
     }

    libelle = Normaliser_chaine ( log, motif->libelle );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_motifDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,icone,syn,libelle,gid,bitctrl,bitclic,posx,posy,larg,haut,angle,"
                "dialog,gestion,rouge,vert,bleu,bitclic2) VALUES "
                "(%d,%d,%d,'%s',%d,%d,%d,%d,%d,%f,%f,%f,%d,%d,%d,%d,%d,%d)", NOM_TABLE_MOTIF,
                id, motif->icone_id, motif->syn_id, libelle, motif->gid,
                motif->bit_controle, motif->bit_clic,
                motif->position_x, motif->position_y, motif->largeur, motif->hauteur, motif->angle,
                motif->type_dialog, motif->type_gestion,
                motif->rouge0, motif->vert0, motif->bleu0, motif->bit_clic2 );
    g_free(libelle);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Ajouter_motifDB: ajout failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Ajouter_motifDB: ajout ok" );

    EndQueryDB( log, db, hquery );
    return(id);
  }
/**********************************************************************************************************/
/* Creer_db_motif: création des tables associées aux motifs                                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_motif ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_motif: Creation DB failed: query=null", NOM_TABLE_MOTIF );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id       INTEGER       PRIMARY KEY,"
                                          "  icone    INTEGER       NOT NULL,"
                                          "  syn      INTEGER       NOT NULL,"
                                          "  libelle  VARCHAR(%d)   NOT NULL,"
                                          "  gid      INTEGER       NOT NULL,"
                                          "  bitctrl  INTEGER       NOT NULL,"
                                          "  bitclic  INTEGER       NOT NULL,"
                                          "  bitclic2 INTEGER       NOT NULL,"
                                          "  posx     INTEGER       NOT NULL,"
                                          "  posy     INTEGER       NOT NULL,"
                                          "  larg     FLOAT         NOT NULL,"
                                          "  haut     FLOAT         NOT NULL,"
                                          "  angle    FLOAT         NOT NULL,"
                                          "  dialog   INTEGER       NOT NULL,"
                                          "  gestion  INTEGER       NOT NULL,"
                                          "  rouge    INTEGER       NOT NULL,"
                                          "  vert     INTEGER       NOT NULL,"
                                          "  bleu     INTEGER       NOT NULL"
                                          ");",
                                          NOM_TABLE_MOTIF, NBR_CARAC_LIBELLE_MOTIF_UTF8+1 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_motif: création table failed", NOM_TABLE_MOTIF );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_motif: succes création table", NOM_TABLE_MOTIF );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_motifDB ( struct LOG *log, struct DB *db, gint id_syn )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[512];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_motifDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,icone,syn,gid,bitctrl,bitclic,posx,posy,larg,haut,angle,"
                "dialog,gestion,rouge,vert,bleu,bitclic2"
                " FROM %s WHERE syn=%d ORDER BY id", NOM_TABLE_MOTIF, id_syn );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Recuperer_motifDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info_c( log, DEBUG_DB, "Recuperer_motifDB: recherche ok", requete );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MOTIFDB *Recuperer_motifDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id[10], icone[10], libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];
    gchar groupe[10], bitctrl[10], bitclic[10], syn[10], bitclic2[10];
    gchar posx[20], posy[20], larg[20], haut[20], angle[20];
    gchar dialog[10], gestion[10], rouge[10], vert[10], bleu[10];
    struct MOTIFDB *motif;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id, sizeof(id), NULL );                          /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &icone, sizeof(icone), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du icone" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &syn, sizeof(syn), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du syn" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &groupe, sizeof(groupe), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du groupe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &bitctrl, sizeof(bitctrl), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du bitctrl" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &bitclic, sizeof(bitclic), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du bitclic" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &posx, sizeof(posx), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du posx" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 9, SQL_C_CHAR, &posy, sizeof(posy), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du posy" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 10, SQL_C_CHAR, &larg, sizeof(larg), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du larg" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 11, SQL_C_CHAR, &haut, sizeof(haut), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du haut" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 12, SQL_C_CHAR, &angle, sizeof(angle), NULL );                   /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du angle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 13, SQL_C_CHAR, &dialog, sizeof(dialog), NULL );                 /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du dialog" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 14, SQL_C_CHAR, &gestion, sizeof(gestion), NULL );               /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du gestion" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 15, SQL_C_CHAR, &rouge, sizeof(rouge), NULL );                   /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du rouge" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 16, SQL_C_CHAR, &vert, sizeof(vert), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du vert" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 17, SQL_C_CHAR, &bleu, sizeof(bleu), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du bleu" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 18, SQL_C_CHAR, &bitclic2, sizeof(bitclic2), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du bitclic2" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    motif = (struct MOTIFDB *)g_malloc0( sizeof(struct MOTIFDB) );
    if (!motif) Info( log, DEBUG_MEM, "Recuperer_motifDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( motif->libelle, libelle, sizeof(motif->libelle) );            /* Recopie dans la structure */
       motif->id = atoi(id);
       motif->icone_id = atoi(icone);                                       /* Correspond au fichier .gif */
       motif->syn_id = atoi(syn);
       motif->gid = atoi(groupe);                                /* Nom du groupe d'appartenance du motif */
       motif->bit_controle = atoi(bitctrl);                                                 /* Ixxx, Cxxx */
       motif->bit_clic = atoi(bitclic);       /* Bit à activer quand on clic avec le bouton gauche souris */
       motif->bit_clic2 = atoi(bitclic2);     /* Bit à activer quand on clic avec le bouton gauche souris */
       motif->position_x = atoi(posx);                                       /* en abscisses et ordonnées */
       motif->position_y = atoi(posy);
       motif->largeur = atof(larg);                                /* Taille de l'image sur le synoptique */
       motif->hauteur = atof(haut);
       motif->angle = atof(angle);
       motif->type_dialog = atoi(dialog);        /* Type de la boite de dialogue pour le clic de commande */
       motif->rouge0 = atoi(rouge);
       motif->vert0 = atoi(vert);
       motif->bleu0 = atoi(bleu);
       motif->type_gestion = atoi(gestion);              
     }
    return(motif);
  }
/**********************************************************************************************************/
/* Rechercher_motifDB: Recupération du motif dont l'id est en parametre                                   */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MOTIFDB *Rechercher_motifDB ( struct LOG *log, struct DB *db, guint id )
  { gchar icone[10], libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];
    gchar groupe[10], bitctrl[10], bitclic[10], syn[10], bitclic2[10];
    gchar posx[20], posy[20], larg[20], haut[20], angle[20];
    gchar dialog[10], gestion[10], rouge[10], vert[10], bleu[10];
    struct MOTIFDB *motif;
    gchar requete[512];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_motifDB: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &icone, sizeof(icone), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du icone" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &syn, sizeof(syn), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du syn" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &groupe, sizeof(groupe), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du groupe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &bitctrl, sizeof(bitctrl), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du bitctrl" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &bitclic, sizeof(bitclic), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du bitclic" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &posx, sizeof(posx), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du posx" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &posy, sizeof(posy), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du posy" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 9, SQL_C_CHAR, &larg, sizeof(larg), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du larg" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 10, SQL_C_CHAR, &haut, sizeof(haut), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du haut" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 11, SQL_C_CHAR, &angle, sizeof(angle), NULL );                   /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du angle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 12, SQL_C_CHAR, &dialog, sizeof(dialog), NULL );                 /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du dialog" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 13, SQL_C_CHAR, &gestion, sizeof(gestion), NULL );               /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du gestion" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 14, SQL_C_CHAR, &rouge, sizeof(rouge), NULL );                   /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du rouge" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 15, SQL_C_CHAR, &vert, sizeof(vert), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du vert" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 16, SQL_C_CHAR, &bleu, sizeof(bleu), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du bleu" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 17, SQL_C_CHAR, &bitclic2, sizeof(bitclic2), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_motifDB_suite: erreur bind du bitclic2" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle,icone,syn,gid,bitctrl,bitclic,posx,posy,larg,haut,angle,"
                "dialog,gestion,rouge,vert,bleu,bitclic2 "
                "FROM %s WHERE id=%d", NOM_TABLE_MOTIF, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Rechercher_motifDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_motifDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_motifDB: Motif non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_motifDB: Multiple solution", id );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    motif = (struct MOTIFDB *)g_malloc0( sizeof(struct MOTIFDB) );
    if (!motif) Info( log, DEBUG_MEM, "Recuperer_motifDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( motif->libelle, libelle, sizeof(motif->libelle) );            /* Recopie dans la structure */
       motif->id = id;                                                              /* Id unique du motif */
       motif->icone_id = atoi(icone);                                       /* Correspond au fichier .gif */
       motif->syn_id = atoi(syn);
       motif->gid = atoi(groupe);                                /* Nom du groupe d'appartenance du motif */
       motif->bit_controle = atoi(bitctrl);                                                 /* Ixxx, Cxxx */
       motif->bit_clic = atoi(bitclic);       /* Bit à activer quand on clic avec le bouton gauche souris */
       motif->bit_clic2 = atoi(bitclic2);     /* Bit à activer quand on clic avec le bouton gauche souris */
       motif->position_x = atoi(posx);                                       /* en abscisses et ordonnées */
       motif->position_y = atoi(posy);
       motif->largeur = atof(larg);                                /* Taille de l'image sur le synoptique */
       motif->hauteur = atof(haut);
       motif->angle = atof(angle);
       motif->type_dialog = atoi(dialog);        /* Type de la boite de dialogue pour le clic de commande */
       motif->rouge0 = atoi(rouge);
       motif->vert0 = atoi(vert);
       motif->bleu0 = atoi(bleu);
       motif->type_gestion = atoi(gestion);              
     }
    return(motif);
  }
/**********************************************************************************************************/
/* Modifier_motifDB: Modification d'un motif Watchdog                                                     */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_motifDB( struct LOG *log, struct DB *db, struct CMD_EDIT_MOTIF *motif )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    gchar *libelle;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Modifier_motifDB: ajout failed: query=null", motif->id );
       return(FALSE);
     }

    libelle = Normaliser_chaine ( log, motif->libelle );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Modifier_motifDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "icone=%d,libelle='%s',gid=%d,bitctrl=%d,bitclic=%d,posx=%d,posy=%d,larg=%f,"
                "haut=%f,angle=%f,dialog=%d,gestion=%d,rouge=%d,vert=%d,bleu=%d,bitclic2=%d"
                " WHERE id=%d;", NOM_TABLE_MOTIF,
                motif->icone_id, libelle, motif->gid,
                motif->bit_controle, motif->bit_clic,
                motif->position_x, motif->position_y, motif->largeur, motif->hauteur, motif->angle,
                motif->type_dialog, motif->type_gestion,
                motif->rouge0, motif->vert0, motif->bleu0, motif->bit_clic2,
                motif->id );
    g_free(libelle);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Modifier_motifDB: Modif failed", motif->id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Modifier_motifDB: succes modif", motif->id );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
