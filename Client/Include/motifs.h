/**********************************************************************************************************/
/* Client/Include/motifs.h:   Chargement/Affichage d'un synoptique                                        */
/* Projet WatchDog version 1.7     Gestion d'habitat                         sam 27 sep 2003 15:08:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _MOTIFS_H_
 #define _MOTIFS_H_

 #include "Cst_synoptiques.h"
 #include "Cst_atelier.h"

 enum                                                            /* Diff�rents types de gestion de motifs */
  { TYPE_INERTE,
    TYPE_STATIQUE,
    TYPE_DYNAMIQUE,
    TYPE_CYCLIQUE,
    TYPE_PROGRESSIF,
    TYPE_INDICATEUR,
    TYPE_ANALOGIQUE,
    TYPE_BOUTON,
    TYPE_FOND
  };

 #define TAILLE_FONT                60                  /* Nombre de caractere pour la police d'affichage */
 #define COULEUR_FOND_SYN           "MidnightBlue"
 #define COULEUR_ACTIVE             200

 enum /* Diff�rente action associ�e � un item */
  { ACTION_SANS,
    ACTION_IMMEDIATE,
    ACTION_PROGRAMME,
    ACTION_NBR_ACTION                                                         /* nombre d'action possible */
  };
/**************************************** Qu'est-ce qu'un motif ?? ****************************************/
 struct MOTIF
  { gint   id;                                                      /* Numero du motif dans la DBWatchdog */
    gint   icone_id;       /* question: encore utilis� aujourdh'ui ??*/     /* Correspond au fichier .gif */
    gchar  libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                              /* "Vanne gaz chaudi�re" */
    guint  gid;                                                  /* Nom du groupe d'appartenance du motif */
    gint   bit_controle;                                                                    /* Ixxx, Cxxx */
    guint   bit_clic;                         /* Bit � activer quand on clic avec le bouton gauche souris */
    guint   bit_clic2;                        /* Bit � activer quand on clic avec le bouton gauche souris */
    gfloat position_x;                                                       /* en abscisses et ordonn�es */
    gfloat position_y;
    gint   gif_largeur;                                                /* Taille de l'image � l'echelle 1 */
    gint   gif_hauteur;
    gfloat largeur;                                               /* Taille de l'image sur le synoptique */
    gfloat hauteur;
    gfloat angle;
    gchar  type_dialog;                          /* Type de la boite de dialogue pour le clic de commande */
    guchar  rouge0;
    guchar  vert0;
    guchar  bleu0;
    gchar  type_gestion;                                                   /* Statique/dynamique/cyclique */
    gint   rafraich;
  };
/************************************** Qu'est-ce qu'une passerelle ?? ************************************/
 struct PASSERELLE
  { gint  id;
    gint  bit_controle;
    gint  bit_controle_1;
    gint  bit_controle_2;
    gchar libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];                                          /* "ChSeb" */
    gint  syn_cible_id;                                  /* Valeur autoritaire par rapport au mnemo_cible */
    gint  position_x;
    gint  position_y;
    gfloat angle;
  };
/************************************** Qu'est-ce qu'un commentaire ?? ************************************/
 struct COMMENTAIRE
  { gint   id;                                                /* Numero du commentaire dans la DBWatchdog */
    gchar  libelle [NBR_CARAC_LIBELLE_MOTIF_UTF8+1];
    guchar font[TAILLE_FONT+1];
    guchar rouge;
    guchar vert;
    guchar bleu;
    gint   position_x;
    gint   position_y;
    gfloat angle;
  };
/************************************** Qu'est-ce qu'une passerelle ?? ************************************/
 struct CAPTEUR
  { gint  id;
    gint  bit_controle;
    gint  type;
    gchar libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                                             /* "ChSeb" */
    gint  position_x;
    gint  position_y;
    gfloat angle;
  };
/**********************************************************************************************************/
#endif
/*--------------------------------------------------------------------------------------------------------*/
