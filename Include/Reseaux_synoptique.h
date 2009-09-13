/**********************************************************************************************************/
/* Include/Reseaux_atelier.h:   Sous_tag de l'atelier pour watchdog 2.0 par lefevre Sebastien             */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 10:44:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_ATELIER_H_
 #define _RESEAUX_ATELIER_H_

 #include "Cst_atelier.h"

 struct CMD_SHOW_SYNOPTIQUE
  { guint id;                                                      /* Numero du message dans la structure */
    gchar  libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar  mnemo[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    guint  groupe;
  };
 struct CMD_EDIT_SYNOPTIQUE
  { guint id;                                                      /* Numero du message dans la structure */
    gchar  libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar  mnemo[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    guint  groupe;
  };

 struct CMD_ADD_SYNOPTIQUE
  { gchar  libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar  mnemo[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    guint  groupe;
  };

 struct CMD_ID_SYNOPTIQUE
  { guint id;                                                      /* Numero du message dans la structure */
    gchar libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
  };


/************************************************* Tag de communication ***********************************/
 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE,                      /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FIN,                  /* Ajout d'un groupe dans la liste cliente */
    SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE,
    SSTAG_CLIENT_WANT_GROUPE_FOR_SYNOPTIQUE,        /* Affichage des groupes pour appartenance synoptique */
    SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_SYNOPTIQUE,      /* Envoi des groupes pour l'edition utilisateur */
    SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_SYNOPTIQUE_FIN,                      /* Le transfert est terminé */
    SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_PROPRIETE_SYNOPTIQUE,  /* Envoi des groupes pour l'edition motif */
    SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_PROPRIETE_SYNOPTIQUE_FIN,            /* Le transfert est terminé */
    SSTAG_CLIENT_ADD_SYNOPTIQUE,                           /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_SYNOPTIQUE_OK,                                   /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_SYNOPTIQUE,                                    /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_SYNOPTIQUE_OK,                                   /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_SYNOPTIQUE,                              /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_SYNOPTIQUE_OK,          /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_SYNOPTIQUE,                         /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_SYNOPTIQUE_OK,                   /* Le serveur valide les nouvelles données */      

    SSTAG_CLIENT_ATELIER_SYNOPTIQUE,                 /* Le client desire editer par atelier le synoptique */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF,         /* Le serveur envoi des motifs dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF_FIN,     /* Le serveur envoi des motifs dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT,     /* Le serveur envoi des comments dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT_FIN, /* Le serveur envoi des comments dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS,            /* Le serveur envoi des pass dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS_FIN,        /* Le serveur envoi des pass dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE,     /* Le serveur envoi des palettes dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE_FIN, /* Le serveur envoi des palettes dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR,     /* Le serveur envoi des capteurs dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR_FIN, /* Le serveur envoi des capteurs dans l'atelier client */

    SSTAG_CLIENT_ATELIER_DEL_MOTIF,                  /* Le client desire editer par atelier le synoptique */
    SSTAG_SERVEUR_ATELIER_DEL_MOTIF_OK,              /* Le client desire editer par atelier le synoptique */

    SSTAG_CLIENT_ATELIER_ADD_MOTIF,                    /* Le client desire ajouter un motif au synoptique */
    SSTAG_SERVEUR_ATELIER_ADD_MOTIF_OK,                                          /* L'ajout est un succes */

    SSTAG_CLIENT_ATELIER_EDIT_MOTIF,                /* Le client envoi les propriétés du motif au serveur */

    SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE,              /* Le client veut un mnemonique du bit TYPE-NUM (B001) */
    SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE,                                     /* Le serveur repond au client */

    SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_EA,                          /* Le client desire un mnémonique EAxxx */
    SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE_EA,
    
    SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_PASS,                         /* Le client desire un mnémonique Ixxx */
    SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE_PASS,

    SSTAG_CLIENT_WANT_PAGE_CLASSE_FOR_ATELIER,        /* Le client veut les données classe pour l'atelier */
    SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FOR_ATELIER, /* Le serveur envoie les données classes pour l'atelier */
    SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FOR_ATELIER_FIN,
    SSTAG_CLIENT_WANT_PAGE_ICONE_FOR_ATELIER,          /* Le client veut les données icone pour l'atelier */
    SSTAG_SERVEUR_ADDPROGRESS_ICONE_FOR_ATELIER,   /* Le serveur envoie les données icones pour l'atelier */
    SSTAG_SERVEUR_ADDPROGRESS_ICONE_FOR_ATELIER_FIN,

    SSTAG_CLIENT_ATELIER_ADD_COMMENT,  /* Le client veut ajouter un commentaire au syn en cours d'edition */
    SSTAG_SERVEUR_ATELIER_ADD_COMMENT_OK,                   /* Le serveur valide l'ajout d'un commentaire */

    SSTAG_CLIENT_ATELIER_DEL_COMMENT,                           /* Le client veut detruire un commentaire */
    SSTAG_SERVEUR_ATELIER_DEL_COMMENT_OK,            /* Le client desire editer par atelier le synoptique */

    SSTAG_CLIENT_ATELIER_EDIT_COMMENT,            /* Le client envoi les propriétés du comment au serveur */

    SSTAG_CLIENT_ATELIER_ADD_PASS,            /* Le client veut ajouter un pass au syn en cours d'edition */
    SSTAG_SERVEUR_ATELIER_ADD_PASS_OK,                             /* Le serveur valide l'ajout d'un pass */

    SSTAG_CLIENT_ATELIER_DEL_PASS,                                     /* Le client veut detruire un pass */
    SSTAG_SERVEUR_ATELIER_DEL_PASS_OK,               /* Le client desire editer par atelier le synoptique */

    SSTAG_CLIENT_ATELIER_EDIT_PASS,                  /* Le client envoi les propriétés du pass au serveur */

    SSTAG_CLIENT_ATELIER_ADD_PALETTE,      /* Le client veut ajouter un palette au syn en cours d'edition */
    SSTAG_SERVEUR_ATELIER_ADD_PALETTE_OK,                       /* Le serveur valide l'ajout d'un palette */

    SSTAG_CLIENT_ATELIER_DEL_PALETTE,                               /* Le client veut detruire un palette */
    SSTAG_SERVEUR_ATELIER_DEL_PALETTE_OK,            /* Le client desire editer par atelier le synoptique */

    SSTAG_CLIENT_ATELIER_EDIT_PALETTE,            /* Le client envoi les propriétés du palette au serveur */

    SSTAG_CLIENT_ATELIER_ADD_CAPTEUR,      /* Le client veut ajouter un capteur au syn en cours d'edition */
    SSTAG_SERVEUR_ATELIER_ADD_CAPTEUR_OK,                       /* Le serveur valide l'ajout d'un capteur */

    SSTAG_CLIENT_ATELIER_DEL_CAPTEUR,                               /* Le client veut detruire un capteur */
    SSTAG_SERVEUR_ATELIER_DEL_CAPTEUR_OK,                                             /* Destruction OKAY */

    SSTAG_CLIENT_ATELIER_EDIT_CAPTEUR,            /* Le serveur envoi les propriétés du capteur au client */

    SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER,    /* Le client veut les données classe pour l'atelier */
    SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER,
    SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_FIN,

    SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER_PALETTE,     /* noms des syn pour les choix de palettes */
    SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_PALETTE, 
    SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_PALETTE_FIN,

    SSTAG_CLIENT_TYPE_NUM_MNEMO_CLIC,                    /* Envoi des informations mnemonique à l'atelier */
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC,                   /* Envoi des informations mnemonique à l'atelier */
    SSTAG_CLIENT_TYPE_NUM_MNEMO_CTRL,
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_CTRL,
    SSTAG_CLIENT_TYPE_NUM_MNEMO_CLIC2,
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC2,
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

