/**********************************************************************************************************/
/* Include/Cst_mnemoniques.h        Déclaration des constantes dediées aux synoptiques                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 sep 2003 16:40:16 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _CST_MNEMONIQUES_H_
 #define _CST_MNEMONIQUES_H_

 #define NBR_CARAC_LIBELLE_MNEMONIQUE       70
 #define NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8  (6*NBR_CARAC_LIBELLE_MNEMONIQUE)

 #define NBR_CARAC_OBJET_MNEMONIQUE       30
 #define NBR_CARAC_OBJET_MNEMONIQUE_UTF8  (6*NBR_CARAC_OBJET_MNEMONIQUE)
 
 #define NBR_CARAC_ACRONYME_MNEMONIQUE       14
 #define NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8  (6*NBR_CARAC_ACRONYME_MNEMONIQUE)

 enum
  { MNEMO_BISTABLE,                                               /* Definitions des types de mnemoniques */
    MNEMO_MONOSTABLE,
    MNEMO_TEMPO,
    MNEMO_ENTREE,
    MNEMO_SORTIE,
    MNEMO_ENTREE_ANA,
    MNEMO_SORTIE_ANA,
    MNEMO_MOTIF,
    MNEMO_CPTH,
    MNEMO_CAMERA,
    NBR_TYPE_MNEMO
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/
