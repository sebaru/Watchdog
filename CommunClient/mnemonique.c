/**********************************************************************************************************/
/* CommunClient/mnemonique.c        Fonctions communes mnemoniques de Watchdog v2.0                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 05 déc 2010 14:20:57 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <gnome.h>
 #include "Cst_mnemoniques.h"

 GdkColor COULEUR[NBR_TYPE_MNEMO]=
  { { 0x0, 0xAFFF, 0xAFFF, 0xAFFF },
    { 0x0, 0x9FFF, 0x0,    0x9FFF },
    { 0x0, 0x0,    0x7FFF, 0x7FFF },
    { 0x0, 0xCFFF, 0x4FFF, 0x0    },
    { 0x0, 0xAFFF, 0xAFFF, 0x0    },
    { 0x0, 0x0,    0xAFFF, 0x0    },
    { 0x0, 0x0,    0x0,    0xFFFF },
    { 0x0, 0x0,    0x7FFF, 0xCFFF },
    { 0x0, 0x0,    0x7FFF, 0x0    },
    { 0x0, 0xFFFF, 0xFFFF, 0x0    },
  };
 gchar *TYPE_BIT_INTERNE[ NBR_TYPE_MNEMO ]=          /* Type des différents bits internes utilisés */
  { "Bistable      B",
    "Monostable    M",
    "Temporisation TR",
    "Entree TOR    E",
    "Sortie TOR    A",
    "Entree ANA    EA",
    "Sortie ANA    AA",
    "Icone         I",
    "Compteur H    CH",
    "Camera        CAM",
  };
 gchar *TYPE_BIT_INTERNE_COURT[ NBR_TYPE_MNEMO ]=    /* Type des différents bits internes utilisés */
  { "B",
    "M",
    "TR",
    "E",
    "A",
    "EA",
    "AA",
    "I",
    "CH",
    "CAM",
  };
/********************************* Définitions des prototypes programme ***********************************/

/**********************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                          */
/* Entrée: le numero du type                                                                              */
/* Sortie: le type                                                                                        */
/**********************************************************************************************************/
 GdkColor *Couleur_bit_interne ( gint num )
  { if (num >= NBR_TYPE_MNEMO)
     return( &COULEUR[0] );
    return( &COULEUR[num] );
  }
/**********************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                          */
/* Entrée: le numero du type                                                                              */
/* Sortie: le type                                                                                        */
/**********************************************************************************************************/
 gchar *Type_bit_interne ( gint num )
  { if (num >= NBR_TYPE_MNEMO)
     return("Type_bit_interne: Erreur interne");
    return( TYPE_BIT_INTERNE[num] );
  }
/**********************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                          */
/* Entrée: le numero du type                                                                              */
/* Sortie: le type                                                                                        */
/**********************************************************************************************************/
 gint Type_bit_interne_int ( gchar *type )
  { gint cpt;
    for (cpt=0; cpt<NBR_TYPE_MNEMO; cpt++)
     { if ( !strcmp(type, TYPE_BIT_INTERNE[cpt]) ) return cpt; }
    printf("Type_bit_interne_int: pas trouvé\n");
    return(0);
  }
/**********************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                          */
/* Entrée: le numero du type                                                                              */
/* Sortie: le type                                                                                        */
/**********************************************************************************************************/
 gchar *Type_bit_interne_court ( gint num )
  { if (num >= NBR_TYPE_MNEMO)
     return("Type_bit_interne_court: Erreur interne");
    return( TYPE_BIT_INTERNE_COURT[num] );
  }
/*--------------------------------------------------------------------------------------------------------*/
