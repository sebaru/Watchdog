 var Synoptique;                                                     /* Toutes les données du synoptique en cours d'affichage */
 var Messages_loaded;                                                                      /* true si le datatable a été créé */

/********************************************* Affichage des vignettes ********************************************************/
 function Msg_acquitter ( id )
  { table = $('#idTableMessages').DataTable();
    selection = table.ajax.json().enregs.filter( function(item) { return (item.id==id) } )[0];
    var json_request = JSON.stringify(
       { tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
       }
     );
    Send_to_API ( 'POST', "/api/histo/ack", json_request, function ()
     { $('#idTableMessages').DataTable().ajax.reload( null, false );
     }, null);
  }
/********************************************* Clic sur visuel ****************************************************************/
 function Envoyer_clic_visuel ( tech_id, acronyme )
  { console.log ("Envoyer_clic_visuel: "+tech_id+":"+acronyme );
    var json_request = JSON.stringify(
       { tech_id  : tech_id,
         acronyme : acronyme,
       }
     );
    Send_to_API ( 'POST', "/api/syn/clic", json_request, null, null );
  }
/********************************************* Affichage des vignettes ********************************************************/
 function Set_vignette ( id, type, couleur, cligno )
  { var src_actuelle = $('#'+id).attr("src");
    var src_cible;
    if (type == "activite")       { src_cible = "/img/pignon_"+couleur+".svg"; }
    else if (type == "secu_bien") { src_cible = "/img/bouclier_"+couleur+".svg"; }
    else if (type == "secu_pers") { src_cible = "/img/croix_rouge_"+couleur+".svg"; }
    else console.log( "Set_vignette : type inconnu" );

    if (src_actuelle != src_cible) { Changer_img_src ( id, src_cible ); }
    if (cligno) $('#'+id).addClass("wtd-cligno");
           else $('#'+id).removeClass("wtd-cligno");
  }
/******************************************************************************************************************************/
 function Set_syn_vars ( syn_id, syn_vars )
  { var vignette = $('#idVignette_'+syn_id);

    if (syn_vars.bit_comm == false)
     { $('#idImgSyn_'+syn_id).addClass("wtd-img-grayscale");
       Changer_img_src ( "idVignetteComm_"+syn_id, "/img/syn_communication.png" );
       $('#idVignetteComm_'+syn_id).addClass("wtd-cligno");
     }
    else
     { $('#idImgSyn_'+syn_id).removeClass("wtd-img-grayscale");
       $('#idVignetteComm_'+syn_id).removeClass("wtd-cligno").slideUp();
     }

    if (syn_vars.bit_danger == true)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/croix_rouge_rouge.svg" );
       vignette.addClass("wtd-cligno");
     }
    else if (syn_vars.bit_alerte == true)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/bouclier_rouge.svg" );
       vignette.addClass("wtd-cligno");
     }
    else if (syn_vars.bit_alarme == true)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/pignon_rouge.svg" );
       vignette.addClass("wtd-cligno");
     }
    else if (syn_vars.bit_defaut == true)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/pignon_jaune.svg" );
       vignette.addClass("wtd-cligno");
     }
    else if (syn_vars.bit_derangement == true)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/croix_rouge_orange.svg" );
       vignette.addClass("wtd-cligno");
     }
    else if (syn_vars.bit_alerte_fixe == true)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/bouclier_rouge.svg" );
       vignette.removeClass("wtd-cligno");
     }
    else if (syn_vars.bit_alarme_fixe == true)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/pignon_rouge.svg" );
       vignette.removeClass("wtd-cligno");
     }
    else if (syn_vars.bit_defaut_fixe == true)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/pignon_jaune.svg" );
       vignette.removeClass("wtd-cligno");
     }
    else if (syn_vars.bit_danger_fixe == true)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/croix_rouge_rouge.svg" );
       vignette.removeClass("wtd-cligno");
     }
    else if (syn_vars.bit_derangement_fixe == true)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/croix_rouge_orange.svg" );
       vignette.removeClass("wtd-cligno");
     }
    else if (syn_vars.bit_veille_totale == true)
     { vignette.slideUp().removeClass("wtd-cligno"); }
    else if (syn_vars.bit_veille_partielle == true)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/bouclier_jaune.svg" );
       vignette.removeClass("wtd-cligno");
     }
    else if (syn_vars.bit_veille_partielle == false)
     { Changer_img_src ( "idVignette_"+syn_id, "/img/bouclier_blanc.svg" );
       vignette.removeClass("wtd-cligno");
     }
    else
     { /*Changer_img_src ( "idVignette_"+syn_id, "/img/pignon_vert.svg" );*/
       vignette.removeClass("wtd-cligno").slideUp();
     }
  }
/******************************************************************************************************************************/
 function Charger_messages ( syn_id )
  { if (Messages_loaded==true)
     { $('#idTableMessages').DataTable().ajax.url("/api/histo/alive?syn_id="+syn_id).load();
     }
    else $('#idTableMessages').DataTable(
        { pageLength : 25,
          fixedHeader: true, searching: false, paging:false,
          ajax: {	url: "/api/histo/alive?syn_id="+syn_id, type : "GET", dataSrc: "enregs",
                  error: function ( xhr, status, error ) { /*Show_Error(xhr.statusText);*/ }
                },
          rowId: "id",
          createdRow: function( row, item, dataIndex )
           {      if (item.typologie==0) { classe="text-white"; } /* etat */
             else if (item.typologie==1) { classe="text-warning" } /* alerte */
             else if (item.typologie==2) { classe="text-warning"; } /* defaut */
             else if (item.typologie==3) { classe="text-danger"; } /* alarme */
             else if (item.typologie==4) { classe="text-success"; } /* veille */
             else if (item.typologie==5) { classe="text-white"; }   /* attente */
             else if (item.typologie==6) { classe="text-danger"; } /* danger */
             else if (item.typologie==7) { classe="text-warning"; } /* derangement */
             else classe="text-info";
             $(row).addClass( classe );
           },
          columns:
           [ { "data": null, "title":"-", "className": "align-middle text-center bg-dark",
               "render": function (item)
                 {      if (item.typologie==0) { cligno = false; img = "info.svg"; } /* etat */
                   else if (item.typologie==1) { cligno = true;  img = "bouclier_orange.svg"; } /* alerte */
                   else if (item.typologie==2) { cligno = true;  img = "pignon_orange.svg"; } /* defaut */
                   else if (item.typologie==3) { cligno = true;  img = "pignon_rouge.svg"; } /* alarme */
                   else if (item.typologie==4) { cligno = false; img = "bouclier_vert.svg"; } /* veille */
                   else if (item.typologie==5) { cligno = false; img = "info.svg"; } /* attente */
                   else if (item.typologie==6) { cligno = true;  img = "croix_rouge_rouge.svg"; } /* danger */
                   else if (item.typologie==7) { cligno = true;  img = "croix_rouge_orange.svg"; } /* derangement */
                   else img = "info.svg";
                   if (cligno==true) classe="wtd-cligno"; else classe="";
                   return("<img class='wtd-vignette "+classe+"' src='/img/"+img+"'>");
                 }
             },
             { "data": "date_create", "title":"Apparition", "className": "align-middle text-center bg-dark d-none d-sm-table-cell" },
             { "data": "dls_shortname", "title":"Objet", "className": "align-middle text-center bg-dark " },
             { "data": "libelle", "title":"Message", "className": "align-middle text-center bg-dark", },
             { "data": null, "title":"Acquit", "className": "align-middle text-center bg-dark d-none d-sm-table-cell",
               "render": function (item)
                 { if (item.typologie==0) return("-");                                                      /* Si INFO, pas de ACK */
                   if (item.nom_ack!=null) return(item.nom_ack);
                   return( Bouton ( "primary", "Acquitter le message", "Msg_acquitter", item.id, "Acquitter" ) );
                 }
             },
           ],
          /*order: [ [0, "desc"] ],*/
          responsive: false,
        });
     Messages_loaded = true;
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Charger_un_synoptique ( syn_id )
  { var bodymain = $('#bodymain');
    var tableaux = $('#tableaux');
    Send_to_API ( "GET", "/api/syn/show", "syn_id="+syn_id, function(Response)
     { console.log(Response);
       Synoptique = Response;
       $('#idPageTitle').text(Response.libelle);
       $.each ( Response.child_syns, function (i, syn)
                 { bodymain.append ( Creer_card ( syn ) );
                   Set_syn_vars ( syn.id, Response.syn_vars.filter ( function(ssitem) { return ssitem.id==syn.id } )[0] );
                 }
              );
       Set_syn_vars ( Response.id, Response.syn_vars.filter ( function(ssitem) { return ssitem.id==Response.id } )[0] );
       $.each ( Response.horloges, function (i, horloge)
                 { bodymain.append ( Creer_horloge ( horloge ) ); }
              );

       if (Response.image=="custom") { Changer_img_src ( 'idMenuImgAccueil', "/upload/syn_"+Response.id+".jpg" ); }
                                else { Changer_img_src ( 'idMenuImgAccueil', "/img/"+Response.image ); }
       $('#idMenuImgAccueil').on("click", function () { Charger_page_synoptique ( Response.id ); } );

       $.each ( Response.visuels, function (i, visuel)
                 { bodymain.append ( Creer_visuel ( visuel ) );
                 }
              );
       $.each ( Response.etat_visuels, function (i, etat_visuel)
                 { Changer_etat_visuel ( etat_visuel );
                 }
              );

       $.each ( Response.cadrans, function (i, cadran)
                 { bodymain.append( $('<div></div>').addClass('w-100') ).append ( Creer_cadran ( cadran ) );
                 }
              );

       if (Response.nbr_tableaux>0)
        { $.each ( Response.tableaux, function (i, tableau)
           { tableaux.append("<hr>");
             var id = "idTableau-"+tableau.id;
             tableaux.append( $("<div></div>").append("<canvas id='"+id+"'></canvas>").addClass("col wtd-courbe mx-1") );
             maps = Response.tableaux_map.filter ( function (item) { return(item.tableau_id==tableau.id) } );
             Charger_plusieurs_courbes ( id, maps, "HOUR" );
             $('#'+id).on("click", function () { Charger_page_tableau(tableau.id); } );
           });
        }

       Charger_messages ( syn_id );
       Slide_down_when_loaded ( "toplevel" );
     }, null );
 }

/********************************************* Appelé au chargement de la page ************************************************/
 function Charger_page_synoptique ( syn_id )
  {
    console.log("Charger_page_synoptique " + syn_id);
    Scroll_to_top();
    $('#toplevel').slideUp("normal", function ()
     { $('#toplevel').empty()
                     .append("<div id='bodymain' class='row row-cols-2 row-cols-sm-4 row-cols-md-5 row-cols-lg-6 row-cols-xl-6 justify-content-center'></div")
                     .append("<div id='tableaux' class='row mx-1 justify-content-center'></div");
       Charger_un_synoptique ( syn_id );
     });
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Creer_card ( Response )
  { var card = $('<div></div>').addClass("row bg-transparent")
               .append( $('<div></div>').addClass("col text-center")
                        .append( $('<div></div>').addClass("d-inline-block wtd-img-container")
                                 .append($('<img>').attr("src", (Response.image=="custom" ? "/upload/syn_"+Response.id+".jpg"
                                                                                           : "/img/"+Response.image) )
                                                   .attr("onclick", "Charger_page_synoptique("+Response.id+")")
                                                   .attr("id", "idImgSyn_"+Response.id)
		                              	                  .addClass("wtd-synoptique") )
                                 .append($('<img>').attr("id", "idVignetteComm_"+Response.id)
				                                               /*.attr("src","/img/pignon_vert.svg")*/
                                                   .addClass("wtd-vignette wtd-img-superpose-bas-droite").slideUp()
                                        )
                                 .append($('<img>').attr("id", "idVignette_"+Response.id)
				                                               /*.attr("src","")*/
                                                   .addClass("wtd-vignette wtd-img-superpose-haut-droite").slideUp()
                                        )
                               )
	                     )
               .append( $('<div></div>').addClass('w-100') )
               .append( $('<div></div>').addClass("col text-center mb-2")
                        .append( $('<span></span>').addClass("text-white").text(" "+Response.libelle) )
                      );

    return(card);
  }
/******************************************************************************************************************************/
/* Creer_cadran: Ajoute un cadran sur la page du synoptique                                                                   */
/******************************************************************************************************************************/
 function Creer_cadran ( cadran )
  { var card = $('<div></div>').addClass("row bg-transparent border border-info")
               .append( $('<div></div>').addClass("col text-center")
                        .append( $('<span></span>').addClass("text-white").text( "Cadran" )
                               )
                      )
               .append( $('<div></div>').addClass('w-100') )
               .append( $('<div></div>').addClass("col")
                        .append( $('<div></div>').addClass("progress")
                                 .append( $('<div></div>').addClass("progress-bar")
                                          .attr("wtd-cadran-forme", cadran.forme)
                                          .attr("role", "progressbar" )
                                          .attr("aria-valuemin", cadran.minimum )
                                          .attr("aria-valuemax", cadran.maximum )
                                          .attr("id", "wtd-cadran-forme-"+cadran.tech_id+"-"+cadran.acronyme)
                                        )
                               )
	                     )
               .append( $('<div></div>').addClass('w-100') )
               .append( $('<div></div>').addClass("col text-center")
                        .append( $('<h4></h4>').addClass("text-white").text( "Loading" )
                                 .attr("id", "wtd-cadran-texte-"+cadran.tech_id+"-"+cadran.acronyme)
                               )
                      )
               .append( $('<div></div>').addClass('w-100') )
               .append( $('<div></div>').addClass("col text-center mb-2")
                        .append( $('<span></span>').addClass("text-white").text( cadran.libelle ) )
                      );

    return(card);
  }
/******************************************************************************************************************************/
/* Changer_etat_cadran: Appeler par la websocket pour changer un visuel d'un cadran                                           */
/******************************************************************************************************************************/
 function Changer_etat_cadran ( etat )
  { if (Synoptique==null) return;
    var idcadranforme = "wtd-cadran-forme-"+etat.tech_id+"-"+etat.acronyme;
    var idcadrantexte = "wtd-cadran-texte-"+etat.tech_id+"-"+etat.acronyme;
    cadrans = Synoptique.cadrans.filter( function (item) { return(item.tech_id==etat.tech_id && item.acronyme==etat.acronyme); });
    if (cadrans.length!=1) return;
    cadran = cadrans[0];
    console.debug(etat);
    etat.valeur = etat.valeur.toFixed(cadran.nb_decimal);
    $('#'+idcadrantexte).text( etat.valeur + " " + etat.unite );
    if (cadran.forme=="progress")
     { var position = 100*(etat.valeur-cadran.minimum)/(cadran.maximum-cadran.minimum);
       $('#'+idcadranforme).css("width", position+"%").attr("aria-valuenow", position);
     }
  }
/******************************************************************************************************************************/
/* Changer_etat_visuel: Appeler par la websocket pour changer un visuel d'etat                                                */
/******************************************************************************************************************************/
 function Changer_etat_visuel ( etat )
  { if (Synoptique==null) return;
    var idvisuel = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme;
    var idimage  = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-img";
    var idheader = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-header-text";
    var idfooter = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-footer-text";
    visuels = Synoptique.visuels.filter( function (item) { return(item.tech_id==etat.tech_id && item.acronyme==etat.acronyme); });
    if (visuels.length!=1) return;
    visuel = visuels[0];
    console.log("Changer_etat_visuel " + etat.tech_id + ":" + etat.acronyme + " -> mode ="+etat.mode +" couleur="+etat.color );
    console.debug(visuel);
/*-------------------------------------------------- Visuel si pas de comm ---------------------------------------------------*/
         if (etat.color=="darkgreen")
     { Changer_img_src ( idimage, "/img/"+visuel.forme+"."+visuel.extension);
       $("#"+idvisuel).css("border", "medium dashed darkgreen" );
       $("#"+idheader).css("background-color", "darkgreen" );
       $("#"+idfooter).css("background-color", "darkgreen" );
     }
/*-------------------------------------------------- Visuel mode cadre -------------------------------------------------------*/
    else if (visuel.ihm_affichage=="cadre")
     { Changer_img_src ( idimage, "/img/"+visuel.forme+"."+visuel.extension);
       $("#"+idvisuel).css("border", "medium solid "+etat.color );
       $("#"+idheader).css("background-color", "transparent" );
       $("#"+idfooter).css("background-color", "transparent" );
     }
/*-------------------------------------------------- Visuel mode inline ------------------------------------------------------*/
    else if (visuel.ihm_affichage=="2_modes")
     { if (etat.mode>0) { target = "/img/"+visuel.forme+"_"+etat.mode+"."+visuel.extension; }
                  else  { target = "/img/"+visuel.forme+"."+visuel.extension; }
       Changer_img_src ( idimage, target );
       $("#"+idvisuel).css("border", "none" );
       $("#"+idheader).css("background-color", "transparent" );
       $("#"+idfooter).css("background-color", "transparent" );
     }
/*-------------------------------------------------- Visuel commun -----------------------------------------------------------*/
    if (etat.cligno) $("#"+idimage).addClass("wtd-cligno");
                else $("#"+idimage).removeClass("wtd-cligno");
    $("#"+idvisuel).css("border-radius", "30px" );
  }
/******************************************************************************************************************************/
/* Création d'un visuel sur la page de travail                                                                                */
/******************************************************************************************************************************/
 function Creer_visuel ( Response )
  { var id = "wtd-visu-"+Response.tech_id+"-"+Response.acronyme;
    var contenu;
/*-------------------------------------------------- Visuel mode cadre -------------------------------------------------------*/
         if (Response.ihm_affichage=="cadre")
     { contenu = $('<img>').addClass("wtd-visuel").attr ( "id", id+"-img" ); }
/*-------------------------------------------------- Visuel mode inline ------------------------------------------------------*/
    else if (Response.ihm_affichage=="2_modes")
     { contenu = $('<img>');
       $(contenu).addClass("wtd-visuel")
                 .attr ( "id", id+"-img" )
                 .attr ( "src", "/img/"+Response.forme+"."+Response.extension );

     }
    else
     { contenu = $('<img>').addClass("wtd-visuel").attr ( "id", id+"-img" ); }

    if (Response.ihm_reaction=="clic")
     { $(contenu).click( function () { Envoyer_clic_visuel( Response.tech_id, Response.acronyme+"_CLIC" ); } ); }

    var card = $('<div></div>').addClass("row bg-transparent mx-1")
               .append( $('<div></div>').addClass("col mt-2 text-center text-white")
                        .append( $('<p></p>').text (Response.dls_shortname )
                                             .attr ( "id", id+"-header-text" )
                               )
                      )
               .append( $('<div></div>').addClass("w-100") )
               .append( $('<div></div>').addClass("col text-center")
                        .append( contenu )
                      )
               .append( $('<div></div>').addClass("w-100") )
               .append( $('<div></div>').addClass("col mt-2 text-center text-white")
                        .append( $('<p></p>').text(Response.libelle)
                                             .attr ( "id", id+"-footer-text" )
                               )
                      )
               .attr ( "id", id );
    return(card);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
