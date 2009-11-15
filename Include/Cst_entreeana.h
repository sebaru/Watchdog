/**********************************************************************************************************/
/* Include/Cst_entreeana.h        Déclaration des constantes dediées aux entrees analogiques              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 25 jun 2004 13:42:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _CST_ENTREEANA_H_
 #define _CST_ENTREEANA_H_

 #define TAILLEBUF_HISTO_EANA         1600       /* 800 point de controle dans le client et buffer valana */
 #define NBR_VAL_INIT_COURBE          100                   /* Envoyé via 100 points à chaque paquet rezo */

 #define NBR_CARAC_LIBELLE_ENTREEANA  50
 #define NBR_CARAC_LIBELLE_ENTREEANA_UTF8  (6*NBR_CARAC_LIBELLE_ENTREEANA)

 #define COURBE_TEMPS_TOP             5  /* 1 point = 5 secondes sur la grille courbes */

 enum
  { ENTREEANA_NON_INTERP,
    ENTREEANA_4_20_MA_12BITS,
    NBR_TYPE_ENTREEANA
  };

 enum
  { UNITE_DEGRE_C,                                                   /* Definitions des types de messages */
    UNITE_DEGRE_K,
    UNITE_MILLIBAR,
    UNITE_BAR,
    UNITE_METRE,
    UNITE_KILOMETRE,
    UNITE_LITRE,
    UNITE_METRE_PAR_SECONDE,
    UNITE_KILOMETRE_PAR_HEURE,
    UNITE_POURCENT,
    UNITE_POURCENT_HR,
    UNITE_SECONDE,
    UNITE_HEURE,
    UNITE_DATE,
    NBR_TYPE_UNITE
  };

/*---------------------------- Déclarations des prototypes de fonctions ----------------------------------*/

 extern gchar *Unite_vers_string ( guint type );
 extern gint String_vers_unite ( guchar *unite );
 extern gchar *Type_ea_vers_string ( guint type );

#endif
/*--------------------------------------------------------------------------------------------------------*/
