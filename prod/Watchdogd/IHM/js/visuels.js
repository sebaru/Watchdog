/******************************************************************************************************************************/
/* Changer_etat_visuel: Appeler par la websocket pour changer un visuel d'etat                                                */
/******************************************************************************************************************************/
 function Changer_etat_visuel_simple ( visuel, etat )
  { var idvisuel = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme;
    var idimage  = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-img";
    var idfooter = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-footer-text";
    visuels = Synoptique.visuels.filter( function (item) { return(item.tech_id==etat.tech_id && item.acronyme==etat.acronyme); });
    if (visuels.length!=1) return;
    visuel = visuels[0];
    console.log("Changer_etat_1_visuel_simple " + etat.tech_id + ":" + etat.acronyme + " -> mode = "+etat.mode +" couleur="+etat.color );
    console.debug(visuel);
/*-------------------------------------------------- Visuel si pas de comm ---------------------------------------------------*/
         if (etat.mode=="hors_comm")
     { target = "/img/"+visuel.forme+"_default."+visuel.extension;
       etat.cligno = false;
       $("#"+idimage).addClass("wtd-img-grayscale");
       $("#"+idfooter).removeClass("text-white").addClass("text-warning");
       Changer_img_src ( idimage, target );
     }
/*-------------------------------------------------- Visuel mode inline ------------------------------------------------------*/
    else
     { target = "/img/"+visuel.forme+"_"+etat.mode+"."+visuel.extension;
       Changer_img_src ( idimage, target );
     }

    if (etat.mode!="hors_comm")
     { $("#"+idimage).removeClass("wtd-img-grayscale");
       $("#"+idfooter).addClass("text-white").removeClass("text-warning");
     }
/*-------------------------------------------------- Visuel commun -----------------------------------------------------------*/
    if (etat.cligno) $("#"+idimage).addClass("wtd-cligno");
                else $("#"+idimage).removeClass("wtd-cligno");
    /*$("#"+idvisuel).css("border-radius", "30px" );*/
  }
/******************************************************************************************************************************/
/* Changer_etat_visuel: Appeler par la websocket pour changer un visuel d'etat                                                */
/******************************************************************************************************************************/
 function Changer_etat_visuel_cadre ( visuel, etat )
  { var idvisuel = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme;
    var idimage  = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-img";
    var idfooter = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-footer-text";
    visuels = Synoptique.visuels.filter( function (item) { return(item.tech_id==etat.tech_id && item.acronyme==etat.acronyme); });
    if (visuels.length!=1) return;
    visuel = visuels[0];
    console.log("Changer_etat_cadre " + etat.tech_id + ":" + etat.acronyme + " -> mode = "+etat.mode +" couleur="+etat.color );
    console.debug(visuel);

/*-------------------------------------------------- Visuel si pas de comm ---------------------------------------------------*/
         if (etat.mode=="hors_comm")
     { etat.cligno = false;
       $("#"+idimage).addClass("wtd-img-grayscale");
       $("#"+idfooter).removeClass("text-white").addClass("text-warning");
     }
/*-------------------------------------------------- Visuel mode inline ------------------------------------------------------*/
    else
     { target = "/img/"+visuel.forme+"."+visuel.extension;
       Changer_img_src ( idimage, target );
       $("#"+idimage).removeClass("wtd-img-grayscale");
       $("#"+idfooter).addClass("text-white").removeClass("text-warning");
     }

/*-------------------------------------------------- Visuel commun -----------------------------------------------------------*/
    if (etat.cligno) $("#"+idimage).addClass("wtd-cligno");
                else $("#"+idimage).removeClass("wtd-cligno");
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
