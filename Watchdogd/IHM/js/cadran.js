/******************************************************************************************************************************/
/* Creer_cadran: Ajoute un cadran sur la page du synoptique                                                                   */
/******************************************************************************************************************************/
 function Creer_cadran ( cadran )
  { var card = $('<div></div>').addClass("row bg-transparent m-1")
               .append( $('<div></div>').addClass("col text-center")
                        .append( $('<span></span>').addClass("text-white").text( cadran.libelle ) )
                      )
               .append( $('<div></div>').addClass('w-100') );

    if (cadran.forme=="simple")
     { /**/
     }
    else if (cadran.forme=="progress")
     { var barres = $('<div></div>').addClass("col");
       barres.append( $('<div></div>').addClass("progress")
                      .append( $('<div></div>').addClass("progress-bar")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                    );
       card.append(barres).append( $('<div></div>').addClass('w-100') );
     }
    else if (cadran.forme=="progress-rovor")
     { var barres = $('<div></div>').addClass("col");
       barres.append( $('<div></div>').addClass("progress")
                      .append( $('<div></div>').addClass("progress-bar bg-danger")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre1")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                      .append( $('<div></div>').addClass("progress-bar bg-warning")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre2")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                      .append( $('<div></div>').addClass("progress-bar bg-success")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre3")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                      .append( $('<div></div>').addClass("progress-bar bg-warning")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre4")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                      .append( $('<div></div>').addClass("progress-bar bg-danger")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre5")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                    );
       card.append(barres).append( $('<div></div>').addClass('w-100') );
     }
    else if (cadran.forme=="progress-vor")
     { var barres = $('<div></div>').addClass("col");
       barres.append( $('<div></div>').addClass("progress")
                      .append( $('<div></div>').addClass("progress-bar bg-success")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre1")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                      .append( $('<div></div>').addClass("progress-bar bg-warning")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre2")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                      .append( $('<div></div>').addClass("progress-bar bg-danger")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre3")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                    );
       card.append(barres).append( $('<div></div>').addClass('w-100') );
     }
    else if (cadran.forme=="progress-rov")
     { var barres = $('<div></div>').addClass("col");
       barres.append( $('<div></div>').addClass("progress")
                      .append( $('<div></div>').addClass("progress-bar bg-danger")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre1")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                      .append( $('<div></div>').addClass("progress-bar bg-warning")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre2")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                      .append( $('<div></div>').addClass("progress-bar bg-success")
                               .attr("id", "wtd-cadran-"+cadran.tech_id+"-"+cadran.acronyme+"-barre3")
                               .attr("role", "progressbar" )
                               .attr("aria-valuemin", cadran.minimum )
                               .attr("aria-valuemax", cadran.maximum )
                             )
                    );
       card.append(barres).append( $('<div></div>').addClass('w-100') );
     }

    card.append( $('<div></div>').addClass("col text-center")
                 .append( $('<h4></h4>').addClass("text-white").text( "Loading" )
                          .attr("id", "wtd-cadran-texte-"+cadran.tech_id+"-"+cadran.acronyme)
                        )
               );

    return(card);
  }
/******************************************************************************************************************************/
/* Changer_etat_cadran: Appeler par la websocket pour changer un visuel d'un cadran                                           */
/******************************************************************************************************************************/
 function Changer_etat_cadran ( etat )
  { if (Synoptique==null) return;
    cadrans = Synoptique.cadrans.filter( function (item) { return(item.tech_id==etat.tech_id && item.acronyme==etat.acronyme); });
    if (cadrans.length!=1) return;
    cadran = cadrans[0];
/*    console.debug(etat);*/
    var minimum = parseFloat(cadran.minimum);
    var maximum = parseFloat(cadran.maximum);
    var valeur  = etat.valeur;
    if (cadran.forme.startsWith("progress"))
     { if (valeur<minimum) valeur=minimum;
       if (valeur>maximum) valeur=maximum;
       var position = 100*(valeur-minimum)/(maximum-minimum);
     }

/*console.log("Changer_etat_cadran valeur="+etat.valeur+" seuils = ntb="+cadran.seuil_ntb+" nb="+cadran.seuil_nb+" nh="+cadran.seuil_nh+" nth="+cadran.seuil_nth);*/

    if (cadran.forme=="progress")
     { var idcadranforme = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre";
       $('#'+idcadranforme).css("width", position+"%").attr("aria-valuenow", position);
     }
    else if (cadran.forme=="progress-rovor")
     { var idcadranbarre1 = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre1";
       var idcadranbarre2 = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre2";
       var idcadranbarre3 = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre3";
       var idcadranbarre4 = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre4";
       var idcadranbarre5 = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre5";
       if (valeur<=cadran.seuil_ntb)
        { $('#'+idcadranbarre1).css("width", position+"%").attr("aria-valuenow", position);
          $('#'+idcadranbarre2).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre3).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre4).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre5).css("width", "0%").attr("aria-valuenow", 0);
        }
       else if (valeur<=cadran.seuil_nb)
        { $('#'+idcadranbarre1).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre2).css("width", position+"%").attr("aria-valuenow", position);
          $('#'+idcadranbarre3).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre4).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre5).css("width", "0%").attr("aria-valuenow", 0);
        }
       else if (valeur<=cadran.seuil_nh)
        { $('#'+idcadranbarre1).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre2).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre3).css("width", position+"%").attr("aria-valuenow", position);
          $('#'+idcadranbarre4).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre5).css("width", "0%").attr("aria-valuenow", 0);
        }
       else if (valeur<=cadran.seuil_nth)
        { $('#'+idcadranbarre1).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre2).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre3).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre4).css("width", position+"%").attr("aria-valuenow", position);
          $('#'+idcadranbarre5).css("width", "0%").attr("aria-valuenow", 0);
        }
       else
        { $('#'+idcadranbarre1).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre2).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre3).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre4).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre5).css("width", position+"%").attr("aria-valuenow", position);
        }
     }
    else if (cadran.forme=="progress-rov")
     { var idcadranbarre1 = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre1";
       var idcadranbarre2 = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre2";
       var idcadranbarre3 = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre3";
            if (valeur<=cadran.seuil_ntb)
        { $('#'+idcadranbarre1).css("width", position+"%").attr("aria-valuenow", position);
          $('#'+idcadranbarre2).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre3).css("width", "0%").attr("aria-valuenow", 0);
        }
       else if (valeur<=cadran.seuil_nb)
        { $('#'+idcadranbarre1).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre2).css("width", position+"%").attr("aria-valuenow", position);
          $('#'+idcadranbarre3).css("width", "0%").attr("aria-valuenow", 0);
        }
       else
        { $('#'+idcadranbarre1).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre2).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre3).css("width", position+"%").attr("aria-valuenow", position);
        }
     }
    else if (cadran.forme=="progress-vor")
     { var idcadranbarre1 = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre1";
       var idcadranbarre2 = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre2";
       var idcadranbarre3 = "wtd-cadran-"+etat.tech_id+"-"+etat.acronyme+"-barre3";
            if (valeur<=cadran.seuil_nh)
        { $('#'+idcadranbarre1).css("width", position+"%").attr("aria-valuenow", position);
          $('#'+idcadranbarre2).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre3).css("width", "0%").attr("aria-valuenow", 0);
        }
       else if (valeur<=cadran.seuil_nth)
        { $('#'+idcadranbarre1).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre2).css("width", position+"%").attr("aria-valuenow", position);
          $('#'+idcadranbarre3).css("width", "0%").attr("aria-valuenow", 0);
        }
       else
        { $('#'+idcadranbarre1).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre2).css("width", "0%").attr("aria-valuenow", 0);
          $('#'+idcadranbarre3).css("width", position+"%").attr("aria-valuenow", position);
        }
     }

    var idcadrantexte = "wtd-cadran-texte-"+etat.tech_id+"-"+etat.acronyme;
    texte = etat.valeur.toFixed(cadran.nb_decimal);                                            /* Affiche la valeur non capée */
    $('#'+idcadrantexte).text( texte + " " + etat.unite );
  }
