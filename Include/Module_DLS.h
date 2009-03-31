/**********************************************************************************************************/
/* Include/Module_DLS.h: Fichier d'entete de tous les plugins DLS                                         */
/* Projet WatchDog version 1.5       Gestion d'habitat                       sam nov  2 17:04:57 CET 2002 */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 extern int Heure, Minute, Flag_heure;                             /* Gestion de l'heure pour modules DLS */

 extern int E( int num );
 extern int B( int num );
 extern int M( int num );
 extern int TR( int num );
 extern void SB( int num, int etat );
 extern void SM( int num, int etat );
 extern void SI( int num, int etat, unsigned char rouge, unsigned char vert, unsigned char bleu );
 extern void STR( int num, int etat );
 extern void SA( int num, int etat );
 extern void MSG( int num, int etat );
 extern void SVC( char *chaine, int cde );
 extern void Gerer_contacteur ( int ePPCT, int ePCMAN, int ePCSYS,                 /* Entrées de controle */
                         int aACTCT, int aDCTCT,                          /* Activation, desactivation CT */
                         int AMM, int AMA,                       /* Message de report position Contacteur */
                         int AMCSM, int AMCSS, int AMCSA,            /* Message de report position Commut */
                         int AMDTCCMC, int AMDTCCMH, int AMDTCCMS,/* Msg def travail cde commut, hor, sys */
                         int AMDRCCAC, int AMDRCCAH, int AMDRCCAS,            /* Les memes, sur def repos */
                         int mCACT, int mCDCT,               /* Clic souris pour Activer ou Desactiver CT */
                         int mAHGCT,                          /* Clic souris pour gerer le ct par horloge */
                         int mIRGCTH,    /* Clic souris Inhibition reprise gestion horloge ct à 6h du mat */
                         int mARGCTH,  /* Clic souris Autorisatoin reprise gestion horloge ct à 6h du mat */
                         int mCADCT,                                      /* Clic souris Acquit defaut CT */
                         int bMDGCT,                                          /* Bit de defaut general CT */
                         int bMHGCT,         /* Mémorisation de la gestion CT par horloge (!= de systeme) */
                         int bMDACT,    /* Etat théorique du contacteur apres action, pour gestion defaut */
                         int bRGCTH,                                    /* Reprise gestion CT par horloge */
                         int bAAUTIL,                         /* Attente action utilisateur sur défaut CT */
                         int bATJH,                           /* Activation tarif Jour horloge(impulsion) */
                         int bPPHOR,                                         /* Position physique HORLOGE */
                         int tTDDCT,                                     /* Tempo defaut desactivation CT */
                         int tTDACT,                                        /* Tempo defaut activation CT */
                         int iACCT,                                              /* Activation couleur CT */
                         int iACACQ,                                   /* Activation couleur AcquitDefaut */
                         int iACCSS,                             /* Activation couleur commut sur systeme */
                         int iACCSM,                                /* Activation couleur commut sur manu */
                         int iACCSA,                               /* Activation couleur commut sur arret */
                         int iACCDEM,                                    /* activation couleur CDE marche */
                         int iACCDEA,                                                       /* Idem Arret */
                         int iACCDEH,                                   /* Activation couleur CDE horloge */
                         int iACINDIC);      /* Indicateur du mode CT (manu avec/sans reprise sur horloge */
 extern void Gerer_fenetre ( int ePPF, int ePPV, int ePPVR, int ePPC,              /* Entrées de controle */
                             int bMVF,                                     /* Memorisation veille fenetre */
                             int bMVV,                                       /* Memorisation veille volet */
                             int bMTEV,                                   /* Memorisation test etat volet */
                             int bMDG,                                                /* Mem. Def général */
                             int bMDEFEF,                               /* Mem. Defaut effraction fenetre */
                             int bMDEFEV,                                 /* Mem. Defaut effraction volet */
                             int bMDEFCHOC,                                            /* Mem Defaut choc */
                             int bMAA,                                           /* Mem Activation Alarme */
                             int bMAVV,                                  /* Mem. Activation voyant veille */
                             int mOSVC,                                   /* Mem Voyant veille clignotant */
                             int mCADEF,                         /* Commande acquit defaut via synoptique */
                             int tTRAVV,                                 /* Tempo Activation veille volet */
                             int aACTV,                                       /* Activation voyant veille */
                             int AMFF,                                                  /* Fenetre fermée */
                             int AMFO,
                             int AMVF,                                                      /* Idem volet */
                             int AMVO,
                             int AMVR,                                             /* Fenetre verrouillée */
                             int AMDVR,
                             int AMCHOC,
                             int AMAFVOVO,
                             int AMEV,
                             int AMEF,
                             int AMVAV,
                             int AMVDV,
                             int AMVAF,
                             int AMVDF
                           );

/*--------------------------------------------------------------------------------------------------------*/
