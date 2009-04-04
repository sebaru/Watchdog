/**********************************************************************************************************/
/* Include/Reseaux_atelier.h:   Sous_tag de l'atelier pour watchdog 2.0 par lefevre Sebastien             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
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

 struct CMD_SHOW_MOTIF
  { gint    id;                                                               /* Id du motif dans la base */
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gint    icone_id;                                                       /* Correspond au fichier .gif */
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
    guint   gid;                                                 /* Nom du groupe d'appartenance du motif */
    gint    bit_controle;                                                                   /* Ixxx, Cxxx */
    gint    bit_clic;                         /* Bit à activer quand on clic avec le bouton gauche souris */
    gint    bit_clic2;                        /* Bit à activer quand on clic avec le bouton gauche souris */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    gfloat  largeur;                                               /* Taille de l'image sur le synoptique */
    gfloat  hauteur;
    gfloat  angle;
    gchar   type_dialog;                         /* Type de la boite de dialogue pour le clic de commande */
    guchar  rouge0;
    guchar  vert0;
    guchar  bleu0;
    gchar   type_gestion;                                                  /* Statique/dynamique/cyclique */
  };

 struct CMD_ADD_MOTIF
  { gint    icone_id;                                                       /* Correspond au fichier .gif */
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
    guint   gid;                                                 /* Nom du groupe d'appartenance du motif */
    gint    bit_controle;                                                                   /* Ixxx, Cxxx */
    gint    bit_clic;                         /* Bit à activer quand on clic avec le bouton gauche souris */
    gint    bit_clic2;                        /* Bit à activer quand on clic avec le bouton gauche souris */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    gfloat  largeur;                                               /* Taille de l'image sur le synoptique */
    gfloat  hauteur;
    gfloat  angle;
    gchar   type_dialog;                         /* Type de la boite de dialogue pour le clic de commande */
    guchar  rouge0;
    guchar  vert0;
    guchar  bleu0;
    gchar   type_gestion;                                                  /* Statique/dynamique/cyclique */
  };

 struct CMD_EDIT_MOTIF
  { gint    id;                                                             /* Correspond a l'id du motif */
    gint    icone_id;                                                       /* Correspond au fichier .gif */
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
    guint   gid;                                                 /* Nom du groupe d'appartenance du motif */
    gint    bit_controle;                                                                   /* Ixxx, Cxxx */
    gint    bit_clic;                         /* Bit à activer quand on clic avec le bouton gauche souris */
    gint    bit_clic2;                        /* Bit à activer quand on clic avec le bouton gauche souris */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    gfloat largeur;                                               /* Taille de l'image sur le synoptique */
    gfloat hauteur;
    gfloat angle;
    gchar   type_dialog;                         /* Type de la boite de dialogue pour le clic de commande */
    guchar  rouge0;
    guchar  vert0;
    guchar  bleu0;
    gchar   type_gestion;                                                  /* Statique/dynamique/cyclique */
  };

 struct CMD_ID_MOTIF
  { gint    id;                                                             /* Correspond au fichier .gif */
    gint    syn_id;
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
  };


/*********************************************** Gestion des commentaires *********************************/
 struct CMD_SHOW_COMMENT
  { gint    id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gchar   libelle[NBR_CARAC_LIBELLE_COMMENT_UTF8+1];                           /* "Vanne gaz chaudière" */
    gchar   font[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                                  /* police de caractère */
    guchar  rouge;
    guchar  vert;
    guchar  bleu;
    guint   position_x;
    guint   position_y;
    gfloat  angle;
  };

 struct CMD_ADD_COMMENT
  { gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
    gchar   font[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                                  /* police de caractère */
    guchar  rouge;
    guchar  vert;
    guchar  bleu;
    guint   position_x;
    guint   position_y;
    gfloat  angle;
  };

 struct CMD_EDIT_COMMENT
  { gint    id;
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
    gchar   font[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                                  /* police de caractère */
    guchar  rouge;
    guchar  vert;
    guchar  bleu;
    guint   position_x;
    guint   position_y;
    gfloat  angle;
  };

 struct CMD_ID_COMMENT
  { gint    id;                                                             /* Correspond au fichier .gif */
    gint    syn_id;
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
  };
/*********************************************** Gestion des passerelles **********************************/
 struct CMD_SHOW_PASSERELLE
  { gint    id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gint    syn_cible_id;                                                   /* Numéro du synoptique cible */
    gchar   libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];                    /* Libelle du synoptique cible */
    gint    bit_controle;                                                                   /* Ixxx, Cxxx */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    guint   bit_controle_1;                                       /* Numéro Ixxx du premier bit de retour */
    guint   bit_controle_2;                                        /* Numéro Ixxx du second bit de retour */
    gfloat  angle;
  };

 struct CMD_ADD_PASSERELLE
  { gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gint    syn_cible_id;                                                   /* Numéro du synoptique cible */
    gint    bit_controle;                                                                   /* Ixxx, Cxxx */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    guint   bit_controle_1;                                       /* Numéro Ixxx du premier bit de retour */
    guint   bit_controle_2;                                        /* Numéro Ixxx du second bit de retour */
    gchar   libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];                    /* Libelle du synoptique cible */
    gfloat  angle;
  };

 struct CMD_EDIT_PASSERELLE
  { gint    id;                                                             /* Correspond au fichier .gif */
    gint    syn_id_cible;                                                   /* Numéro du synoptique cible */
    gint    bit_controle;                                                                   /* Ixxx, Cxxx */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    guint   bit_controle_1;                                       /* Numéro Ixxx du premier bit de retour */
    guint   bit_controle_2;                                        /* Numéro Ixxx du second bit de retour */
    gchar   libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];                    /* Libelle du synoptique cible */
    gfloat  angle;
  };

 struct CMD_ID_PASSERELLE
  { gint    id;                                                             /* Correspond au fichier .gif */
    gint    syn_id;
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
  };
/*********************************************** Gestion des passerelles **********************************/
 struct CMD_SHOW_PALETTE
  { gint    id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gint    syn_cible_id;                                                   /* Numéro du synoptique cible */
    gchar   libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];                    /* Libelle du synoptique cible */
    guint   position;                                                        /* en abscisses et ordonnées */
  };

 struct CMD_ADD_PALETTE
  { gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gint    syn_cible_id;                                                   /* Numéro du synoptique cible */
    gchar   libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];                    /* Libelle du synoptique cible */
    guint   position;                                                        /* en abscisses et ordonnées */

  };

 struct CMD_EDIT_PALETTE
  { gint    id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gint    syn_cible_id;                                                   /* Numéro du synoptique cible */
    gchar   libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];                    /* Libelle du synoptique cible */
    guint   position;                                                        /* en abscisses et ordonnées */
  };

 struct CMD_ID_PALETTE
  { gint    id;                                                             /* Correspond au fichier .gif */
    gint    syn_id;
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
  };
/*********************************************** Gestion des capteurs ***************************************/
 struct CMD_SHOW_CAPTEUR
  { gint    id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                       /* Libelle du synoptique cible */
    gint    type;                                                              /* type du bit de controle */
    gint    bit_controle;                                                                   /* Ixxx, Cxxx */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    gfloat  angle;
  };

 struct CMD_ADD_CAPTEUR
  { gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                    /* Libelle du synoptique cible */
    gint    type;                                                              /* type du bit de controle */
    gint    bit_controle;                                                                   /* Ixxx, Cxxx */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    gfloat  angle;
  };

 struct CMD_EDIT_CAPTEUR
  { gint    id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                    /* Libelle du synoptique cible */
    gint    type;                                                              /* type du bit de controle */
    gint    bit_controle;                                                                   /* Ixxx, Cxxx */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    gfloat  angle;
  };

 struct CMD_ID_CAPTEUR
  { gint    id;                                                             /* Correspond au fichier .gif */
    gint    syn_id;
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
  };

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

