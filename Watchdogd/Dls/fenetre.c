/**********************************************************************************************************/
/* Watchdogd/Dls/fenetre.c  -> Gestion des acces type fenetres pour DLS                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 31 jui 2003 10:52:39 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include "Module_dls.h"                /* Inclusion des prototypes de fonctions de controle/commande DLS */
 #define ATTENTE_VEILLE_VOLET   900

/**********************************************************************************************************/
/* Gerer_fenetre: Gestion d'un acces type fenetre                                                         */
/* Entr�es: se r�f�rencer au dossier technique watchdog                                                   */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Gerer_fenetre ( int ePPF,                                              /* Position physique Fenetre */
                      int ePPV,                                               /* Position physique Volets */
                      int ePPVR,                                              /* Position physique verrou */
                      int ePPC,                                                 /* Position physique Choc */
                      int bMVF,                                            /* Memorisation veille fenetre */
                      int bMVV,                                              /* Memorisation veille volet */
                      int bMTEV,                                          /* Memorisation test etat volet */
                      int bMDG,                                                       /* Mem. Def g�n�ral */
                      int bMDEFEF,                                      /* Mem. Defaut effraction fenetre */
                      int bMDEFEV,                                        /* Mem. Defaut effraction volet */
                      int bMDEFCHOC,                                                   /* Mem Defaut choc */
                      int bMAA,                                                  /* Mem Activation Alarme */
                      int bMAVV,                                         /* Mem. Activation voyant veille */
                      int mOSVC,                                          /* Mem Voyant veille clignotant */
                      int mCADEF,                                /* Commande acquit defaut via synoptique */
                      int tTRAVV,                                        /* Tempo Activation veille volet */
                      int aACTV,                                              /* Activation voyant veille */
                      int AMFF,                                        /* Emission message fenetre ferm�e */
                      int AMFO,                                       /* Emission message fenetre ouverte */
                      int AMVF,                                         /* Emission message volets ferm�s */
                      int AMVO,                                        /* Emission message volets ouverts */
                      int AMVR,                                   /* Emission message fenetre verrouill�e */
                      int AMDVR,                                /* Emission message fenetre deverrouill�e */
                      int AMCHOC,                                      /* Emission message choc sur vitre */
                      int AMAFVOVO,  /* Emission msg anomalie fenetre verrouill�e ouverte et volet ouvert */
                      int AMEV,                                      /* Emission message effraction volet */
                      int AMEF,                                    /* Emission message effraction fenetre */
                      int AMVAV,                                 /* Emission message veille activ�e volet */
                      int AMVDV,                              /* Emission message veille desactiv�e volet */
                      int AMVAF,                               /* Emission message veille activ�e fenetre */
                      int AMVDF                             /* Emission message veille desactiv�e fenetre */
                    )
  {
    int PPF, PPC, PPV, PPVR, MVF, MVV, MTEV, MDEFEV, MDEFEF, MDEFCHOC, MAVV, MDG, CADEF, OSVC, TRAVV;
    int var1;

    CADEF    = M(mCADEF);
    OSVC     = M(mOSVC);

    MDG      = B(bMDG  );
    MAVV     = B(bMAVV);
    MDEFCHOC = B(bMDEFCHOC);
    MDEFEF   = B(bMDEFEF);
    MDEFEV   = B(bMDEFEV);
    MTEV     = B(bMTEV);
    MVV      = B(bMVV);
    MVF      = B(bMVF);

    PPVR     = E(ePPVR);
    PPV      = E(ePPV);
    PPC      = E(ePPC);
    PPF      = E(ePPF);

    TRAVV    = TR(tTRAVV);

    var1 = !PPF && !MVF;
    if ( !MDG && !PPVR && ( var1 || PPF ) )
     { SB(bMVF,0); MVF=0; SB(bMVV,0); MVV=0; SB(bMTEV,0); MTEV=0; }

    if ( var1 && PPVR && !MTEV )
     { if (!PPV) { SB(bMTEV,1); MTEV=1; }
            else { SB(bMVV,1);  MVV=1;
                   SB(bMTEV,1); MTEV=1;
                 }
     }

    if( PPF && PPVR && !MTEV )                       /* Veille fenetre */
     { SB(bMVF, 1); MVF = 1;
       STR( tTRAVV, ATTENTE_VEILLE_VOLET );
     }
    else STR( tTRAVV, 0 );

    if (TR(tTRAVV))
     { if (PPV) { SB(bMVV,1); MVV=1; }               /* Veille volet */
       SB(bMTEV, 1); MTEV=1;
     }

 /* Detection des defaut */
    if (PPVR && !PPC) { SB(bMDEFCHOC,1); MDEFCHOC=1; }
    if (!PPV && MVV)  { SB(bMDEFEV,1);   MDEFEV=1;   }
    if (!PPF && MVF)  { SB(bMDEFEF,1);   MDEFEF=1;   }


    MSG(AMFF, PPF);     /* Fenetre ferm�e */
    MSG(AMVR, PPVR);    /* Verrouill�e */
    MSG ( AMVAF, MVF ); /* Veille activ�e fenetre */

    MSG(AMVF, PPV);     /* Volet ferm� */
    MSG ( AMVAV, MVV ); /* veille activ�e volet */

    MSG(AMDVR, !PPVR);  /* Deverrouill�e */
    MSG ( AMVDF, !MVF); /* Veille desactiv�e fenetre */
    MSG ( AMVDV, !MVV); /* Veille desactiv�e volet */

    MSG(AMFO, !PPF);    /* Fenetre ouverte */
    MSG(AMVO, !PPV);    /* Volet ouvert */

    SB(bMAVV, MVV || MVF);
    MAVV = MVV || MVF;

    SM(mOSVC, 0); OSVC = 0; /* Simulation de la RAZ du monostable */

 /* Message d'alerte */
    if (!PPF && PPVR && !PPV) { MSG(AMAFVOVO,1); SM(mOSVC, 1); OSVC=1; }
                         else { MSG(AMAFVOVO,0); }
    if (MDEFEF) { MSG(AMEF,1); SB(bMDG,1); MDG=1; SM(mOSVC, 1); OSVC=1; }
           else { MSG(AMEF,0); }
    if (MDEFEV) { MSG(AMEV,1); SB(bMDG,1); MDG=1; SM(mOSVC, 1); OSVC=1; }
           else { MSG(AMEV,0); }
    if (MDEFCHOC) { MSG(AMCHOC,1); SB(bMDG,1); MDG=1; SM(mOSVC, 1); OSVC=1; }
             else { MSG(AMCHOC,0); }
    if (MDG) { SB(bMAA,1); }

    if(CADEF) /* Acquit def via synoptique */
     { SB(bMTEV,0); MTEV=0;
       SB(bMVV,0); MVV=0;
       SB(bMVF,0); MVF=0;
       SB(bMDG,0); MDG=0;
       SB(bMDEFEF,0); MDEFEF=0;
       SB(bMDEFEV,0); MDEFEV=0;
       SB(bMDEFCHOC,0); MDEFCHOC=0;
     }

    if(!OSVC)
     { SA(aACTV, MAVV); }
    else
     { SA(aACTV, B(4) ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
