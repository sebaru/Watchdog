/******************************************************************************************************************************/
/* Changer_etat_visuel: Appeler par la websocket pour changer un visuel d'etat                                                */
/******************************************************************************************************************************/
 function Changer_etat_visuel_by_mode ( visuel, etat )
  { var idvisuel = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme;
    var idimage  = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-img";
    var idfooter = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-footer-text";
    visuels = Synoptique.visuels.filter( function (item) { return(item.tech_id==etat.tech_id && item.acronyme==etat.acronyme); });
    if (visuels.length!=1) return;
    visuel = visuels[0];
    console.log("Changer_etat_visuel_by_mode " + etat.tech_id + ":" + etat.acronyme + " -> mode = "+etat.mode +" couleur="+etat.color );
    console.debug(visuel);
/*-------------------------------------------------- Visuel si pas de comm ---------------------------------------------------*/
    if (etat.mode=="hors_comm" || etat.mode=="disabled")
     { etat.cligno = false;
       $("#"+idimage).addClass("wtd-img-grayscale");
     }
/*-------------------------------------------------- Visuel si pas de comm ---------------------------------------------------*/
    else if (etat.mode=="default")                                                                         /* si mode inconnu */
     { etat.cligno = false;
       target = "https://static.abls-habitat.fr/img/question.png";
       Changer_img_src ( idimage, target );
     }
/*-------------------------------------------------- Visuel mode inline ------------------------------------------------------*/
    else
     { target = "https://static.abls-habitat.fr/img/"+visuel.forme+"_"+etat.mode+"."+visuel.extension;
       Changer_img_src ( idimage, target );
       $("#"+idimage).removeClass("wtd-img-grayscale");
     }
/*-------------------------------------------------- Visuel commun -----------------------------------------------------------*/
    if (etat.cligno) $("#"+idimage).addClass("wtd-cligno");
                else $("#"+idimage).removeClass("wtd-cligno");
    $("#"+idfooter).addClass("text-white").text(etat.libelle);

  }
/******************************************************************************************************************************/
/* Changer_etat_visuel: Appeler par la websocket pour changer un visuel d'etat                                                */
/******************************************************************************************************************************/
 function Changer_etat_visuel_by_color ( visuel, etat )
  { var idvisuel = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme;
    var idimage  = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-img";
    var idfooter = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-footer-text";
    visuels = Synoptique.visuels.filter( function (item) { return(item.tech_id==etat.tech_id && item.acronyme==etat.acronyme); });
    if (visuels.length!=1) return;
    visuel = visuels[0];
    console.log("Changer_etat_1_visuel_by_color " + etat.tech_id + ":" + etat.acronyme + " -> mode = "+etat.mode +" couleur="+etat.color );
    console.debug(visuel);
/*-------------------------------------------------- Visuel si pas de comm ---------------------------------------------------*/
    if (etat.mode=="hors_comm" || etat.mode=="disabled")
     { etat.cligno = false;
       $("#"+idimage).addClass("wtd-img-grayscale");
     }
/*-------------------------------------------------- Visuel mode inline ------------------------------------------------------*/
    else
     { target = "https://static.abls-habitat.fr/img/"+visuel.forme+"_"+etat.color+"."+visuel.extension;
       Changer_img_src ( idimage, target );
       $("#"+idimage).removeClass("wtd-img-grayscale");
     }
/*-------------------------------------------------- Visuel commun -----------------------------------------------------------*/
    if (etat.cligno) $("#"+idimage).addClass("wtd-cligno");
                else $("#"+idimage).removeClass("wtd-cligno");
    $("#"+idfooter).addClass("text-white").text(etat.libelle);
  }
/******************************************************************************************************************************/
/* Changer_etat_visuel: Appeler par la websocket pour changer un visuel d'etat                                                */
/******************************************************************************************************************************/
 function Changer_etat_visuel_by_mode_color ( visuel, etat )
  { var idvisuel = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme;
    var idimage  = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-img";
    var idfooter = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-footer-text";
    visuels = Synoptique.visuels.filter( function (item) { return(item.tech_id==etat.tech_id && item.acronyme==etat.acronyme); });
    if (visuels.length!=1) return;
    visuel = visuels[0];
    console.log("Changer_etat_visuel_by_mode_color " + etat.tech_id + ":" + etat.acronyme + " -> mode = "+etat.mode +" couleur="+etat.color );
    console.debug(visuel);
/*-------------------------------------------------- Visuel si pas de comm ---------------------------------------------------*/
    if (etat.mode=="hors_comm" || etat.mode=="disabled")
     { etat.cligno = false;
       $("#"+idimage).addClass("wtd-img-grayscale");
     }
/*-------------------------------------------------- Visuel si pas de comm ---------------------------------------------------*/
    else if (etat.mode=="default")                                                                         /* si mode inconnu */
     { etat.cligno = false;
       target = "https://static.abls-habitat.fr/img/question.png";
       Changer_img_src ( idimage, target );
     }
/*-------------------------------------------------- Visuel mode inline ------------------------------------------------------*/
    else
     { target = "https://static.abls-habitat.fr/img/"+visuel.forme+"_"+etat.mode+"_"+etat.color+"."+visuel.extension;
       Changer_img_src ( idimage, target );
       $("#"+idimage).removeClass("wtd-img-grayscale");
     }
/*-------------------------------------------------- Visuel commun -----------------------------------------------------------*/
    if (etat.cligno) $("#"+idimage).addClass("wtd-cligno");
                else $("#"+idimage).removeClass("wtd-cligno");
    $("#"+idfooter).addClass("text-white").text(etat.libelle);
  }
/******************************************************************************************************************************/
/* Changer_etat_visuel: Appeler par la websocket pour changer un visuel d'etat                                                */
/******************************************************************************************************************************/
 function Changer_etat_visuel_static ( visuel, etat )
  { var idvisuel = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme;
    var idimage  = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-img";
    var idfooter = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-footer-text";
    visuels = Synoptique.visuels.filter( function (item) { return(item.tech_id==etat.tech_id && item.acronyme==etat.acronyme); });
    if (visuels.length!=1) return;
    visuel = visuels[0];
    console.log("Changer_etat_visuel_static " + etat.tech_id + ":" + etat.acronyme + " -> mode = "+etat.mode +" couleur="+etat.color );
    console.debug(visuel);

/*-------------------------------------------------- Visuel si pas de comm ---------------------------------------------------*/
    if (etat.mode=="hors_comm" || etat.mode=="disabled")
     { etat.cligno = false;
       $("#"+idimage).addClass("wtd-img-grayscale");
     }
/*-------------------------------------------------- Visuel mode inline ------------------------------------------------------*/
    else
     { target = "https://static.abls-habitat.fr/img/"+visuel.forme+"."+visuel.extension;
       Changer_img_src ( idimage, target );
       $("#"+idimage).removeClass("wtd-img-grayscale");
     }

/*-------------------------------------------------- Visuel commun -----------------------------------------------------------*/
    if (etat.cligno) $("#"+idimage).addClass("wtd-cligno");
                else $("#"+idimage).removeClass("wtd-cligno");
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
