 document.addEventListener('DOMContentLoaded', Load_page, false);

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
  {      if (syn_vars.bit_comm == false) { activite_coul = "kaki"; activite_cligno = true; }
    else if (syn_vars.bit_alarme == true) { activite_coul = "rouge"; activite_cligno = true; }
    else if (syn_vars.bit_defaut == true) { activite_coul = "orange"; activite_cligno = true; }
    else if (syn_vars.bit_alarme_fixe == true) { activite_coul = "rouge"; activite_cligno = false; }
    else if (syn_vars.bit_defaut_fixe == true) { activite_coul = "orange"; activite_cligno = false; }
    else { activite_coul = "vert"; activite_cligno = false; }
    Set_vignette ( "idVignetteActivite_"+syn_id, "activite", activite_coul, activite_cligno );
    if (syn_id == Synoptique.id)
     { Set_vignette ( "idMasterVignetteActivite", "activite", activite_coul, activite_cligno ); }

         if (syn_vars.bit_comm == false) { secu_bien_coul = "kaki"; secu_bien_cligno = true; }
    else if (syn_vars.bit_alerte == true) { secu_bien_coul = "rouge"; secu_bien_cligno = true; }
    else if (syn_vars.bit_alerte_fixe == true) { secu_bien_coul = "rouge"; secu_bien_cligno = true; }
    else if (syn_vars.bit_veille_totale == true) { secu_bien_coul = "vert"; secu_bien_cligno = false; }
    else if (syn_vars.bit_veille_partielle == true) { secu_bien_coul = "orange"; secu_bien_cligno = false; }
    else { secu_bien_coul = "blanc"; secu_bien_cligno = false; }
    Set_vignette ( "idVignetteSecuBien_"+syn_id, "secu_bien", secu_bien_coul, secu_bien_cligno );
    if (syn_id == Synoptique.id)
     { Set_vignette ( "idMasterVignetteSecuBien", "secu_bien", secu_bien_coul, secu_bien_cligno ); }

         if (syn_vars.bit_comm == false) { secu_pers_coul = "kaki"; secu_pers_cligno = true; }
    else if (syn_vars.bit_danger == true) { secu_pers_coul = "rouge"; secu_pers_cligno = true; }
    else if (syn_vars.bit_derangement == true) { secu_pers_coul = "orange"; secu_pers_cligno = true; }
    else if (syn_vars.bit_danger_fixe == true) { secu_pers_coul = "rouge"; secu_pers_cligno = false; }
    else if (syn_vars.bit_derangement_fixe == true) { secu_pers_coul = "orange"; secu_pers_cligno = false; }
    else { secu_pers_coul = "vert"; secu_pers_cligno = false; }
    Set_vignette ( "idVignetteSecuPers_"+syn_id, "secu_pers", secu_pers_coul, secu_pers_cligno );
    if (syn_id == Synoptique.id)
     { Set_vignette ( "idMasterVignetteSecuPers", "secu_pers", secu_pers_coul, secu_pers_cligno ); }
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
                  error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
                },
          rowId: "id",
          columns:
           [ { "data": null, "title":"-", "className": "align-middle text-center bg-dark",
               "render": function (item)
                 {      if (item.typologie==0) { cligno = false; img = "info.svg"; } /* etat */
                   else if (item.typologie==1) { cligno = true;  img = "bouclier_rouge.svg"; } /* alerte */
                   else if (item.typologie==2) { cligno = true;  img = "pignon_jaune.svg"; } /* defaut */
                   else if (item.typologie==3) { cligno = true;  img = "pignon_orange.svg"; } /* alarme */
                   else if (item.typologie==4) { cligno = false; img = "bouclier_vert.svg"; } /* veille */
                   else if (item.typologie==5) { cligno = false; img = "info.svg"; } /* attente */
                   else if (item.typologie==6) { cligno = true;  img = "croix_rouge_rouge.svg"; } /* danger */
                   else if (item.typologie==7) { cligno = true;  img = "croix_rouge_orange.svg"; } /* derangement */
                   else img = "info.svg";
                   if (cligno==true) classe="wtd-cligno"; else classe="";
                   return("<img class='wtd-vignette "+classe+"' src='/img/"+img+"'>");
                 }
             },
             { "data": "date_create", "title":"Apparition", "className": "text-center bg-dark d-none d-sm-table-cell" },
             { "data": "dls_shortname", "title":"Objet", "className": "text-center bg-dark " },
             { "data": "libelle", "title":"Message", "className": "text-center bg-dark" },
             { "data": null, "title":"Acquit", "className": "align-middle text-center bg-dark d-none d-sm-table-cell",
               "render": function (item)
                 { if (item.typologie==0) return("-");                                                      /* Si INFO, pas de ACK */
                   if (item.nom_ack!="None") return(item.nom_ack);
                   return( Bouton ( "primary", "Acquitter le message", "Msg_acquitter", item.id, "Acquitter" ) );
                 }
             },
 /*            { "data": null, "title":"Titre", "className": "align-middle ",
               "render": function (item)
                 { return( Input ( "text", "idTableauTitre_"+item.id,
                                   "Tableau_Set('"+item.id+"')",
                                   "Quel est le titre du tableau ?",
                                   item.titre )
                         );
                 }
             },
             { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
               "render": function (item)
                 { boutons = Bouton_actions_start ();
                   boutons += Bouton_actions_add ( "primary", "Voir le tableau", "Redirect", "/home/tableau?id="+item.id, "chart-line", null );
                   boutons += Bouton_actions_add ( "primary", "Editer les courbes", "Redirect", "/tech/tableau_map/"+item.id, "pen", null );
                   boutons += Bouton_actions_add ( "danger", "Supprimer ce tableau", "Tableau_Delete", item.id, "trash", null );
                   boutons += Bouton_actions_end ();
                   return(boutons);
                 },
             }*/
           ],
          /*order: [ [0, "desc"] ],*/
          responsive: false,
        });
     Messages_loaded = true;
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Change_page ( syn_id )
  { $('#bodycard').fadeOut("fast", function ()
     { $('#bodycard').empty();
       Send_to_API ( "GET", "/api/syn/show", "syn_id="+syn_id, function(Response)
        { console.log(Response);
          Synoptique = Response;
          $('#idPageTitle').text(Response.libelle);
          $.each ( Response.child_syns, function (i, syn)
                    { $('#bodycard').append ( Creer_card ( syn ) );
                      Set_syn_vars ( syn.id, Response.syn_vars.filter ( function(ssitem) { return ssitem.id==syn.id } )[0] );
                    }
                 );
          Set_syn_vars ( Response.id, Response.syn_vars.filter ( function(ssitem) { return ssitem.id==Response.id } )[0] );
          if (Response.image=="custom") { Changer_img_src ( 'idMenuImgAccueil', "/upload/syn_"+Response.id+".jpg" ); }
                                   else { Changer_img_src ( 'idMenuImgAccueil', "/img/syn_"+Response.image+".png" ); }
          $.each ( Response.visuels, function (i, visuel)
                    { $('#bodycard').append ( Creer_visuel ( visuel ) );
                    }
                 );
          $.each ( Response.etat_visuels, function (i, etat_visuel)
                    { Changer_etat_visuel ( etat_visuel );
                    }
                 );
          $('#bodycard').fadeIn("slow");
        }, null );
     });

    $('#idTableMessages').fadeOut("fast", function ()
     { Charger_messages ( syn_id );
       $('#idTableMessages').fadeIn("slow");
     }, null);

  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Creer_card ( Response )
  { var card = $('<div></div>').addClass("card bg-transparent m-1")
               .append( $('<div></div>').addClass("card-header text-center")
                        .append($('<img>').attr("id", "idVignetteActivite_"+Response.id).addClass("wtd-vignette") )
                        .append($('<img>').attr("id", "idVignetteSecuBien_"+Response.id).addClass("wtd-vignette") )
                        .append($('<img>').attr("id", "idVignetteSecuPers_"+Response.id).addClass("wtd-vignette") )
                        .append( " "+Response.page )
                      )
               .append( $('<div></div>').addClass("card-body text-center")
                        .append( $('<img>').attr("src", (Response.image=="custom" ? "/upload/syn_"+Response.id+".jpg"
                                                                                  : "/img/syn_"+Response.image+".png") )
                                 .attr("onclick", "Change_page("+Response.id+")")
                                 .addClass("wtd-synoptique")
                               )
                      )
               .append( $('<div></div>').addClass("card-footer text-center")
                        .append( "<p>"+Response.libelle+"</p>" )
                      );

    return(card);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Changer_etat_visuel ( etat )
  { if (Synoptique==null) return;
    var idvisuel = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme;
    var idimage  = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-img";
    var idheader = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-header-text";
    var idfooter = "wtd-visu-"+etat.tech_id+"-"+etat.acronyme+"-footer-text";
    visuels = Synoptique.visuels.filter( function (item) { return(item.tech_id==etat.tech_id && item.acronyme==etat.acronyme); });
    if (visuels.length!=1) return;
    visuel = visuels[0];
/*-------------------------------------------------- Visuel si pas de comm ---------------------------------------------------*/
         if (etat.color=="darkgreen")
     { Changer_img_src ( idimage, "/img/"+visuel.forme+"."+visuel.extension);
       $("#"+idvisuel).css("border", "medium dashed darkgreen" );
       $("#"+idheader).css("background-color", "darkgreen" );
       $("#"+idfooter).css("background-color", "darkgreen" );
     }
/*-------------------------------------------------- Visuel mode cadre -------------------------------------------------------*/
    else if (visuel.mode_affichage=="cadre")
     { Changer_img_src ( idimage, "/img/"+visuel.forme+"."+visuel.extension);
       style = "none";
       $("#"+idvisuel).css("border", "medium solid "+etat.color );
       $("#"+idheader).css("background-color", "transparent" );
       $("#"+idfooter).css("background-color", "transparent" );
     }
    if (etat.cligno) $("#"+idimage).addClass("wtd-cligno");
                else $("#"+idimage).removeClass("wtd-cligno");
    $("#"+idvisuel).css("border-radius", "30px" );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Creer_visuel ( Response )
  { var id = "wtd-visu-"+Response.tech_id+"-"+Response.acronyme;
    var card = $('<div></div>').addClass("row bg-transparent m-1")
               .append( $('<div></div>').addClass("col mt-2 text-center text-white")
                        .append( $('<p></p>').text (Response.dls_shortname )
                                 .attr ( "id", id+"-header-text" )
                               )
                      )
               .append( $('<div></div>').addClass("w-100") )
               .append( $('<div></div>').addClass("col text-center")
                        .append( $('<img>') /*.attr("src", "/img/"+Response.forme+"."+Response.extension)*/
                                 /*.attr("onclick", "Change_page('"+Response.page+"')")*/
                                 .addClass("wtd-visuel")
                                 /*.addClass("wtd-cligno")*/
                                 .attr ( "id", id+"-img" )
                               )
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
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Change_page (1);

    var WTDWebSocket = new WebSocket("wss://"+window.location.hostname+":"+window.location.port+"/api/live-msgs", "live-msgs");
    WTDWebSocket.onopen = function (event)
     { var json_request = JSON.stringify( { wtd_session: localStorage.getItem("wtd_session") } );
       this.send ( json_request );
     }
    WTDWebSocket.onerror = function (event)
     { $('#idAlertConnexionLost').show();
       console.log("Error au websocket !");
       console.debug(event);
     }
    WTDWebSocket.onclose = function (event)
     { console.log("Close au websocket !");
       console.debug(event);
     }
    WTDWebSocket.onmessage = function (event)
     { var Response = JSON.parse(event.data);                                               /* Pointe sur <synoptique a=1 ..> */
       if (!Synoptique) return;
            if (Response.zmq_tag == "DLS_HISTO" && Messages_loaded==true)
        { if (Response.syn_id == Synoptique.id)                                     /* S'agit-il d'un message de notre page ? */
           { $('#idTableMessages').DataTable().ajax.reload( null, false ); }
        }
       else if (Response.zmq_tag == "SET_SYN_VARS")
        { $.each ( Response.syn_vars, function (i, item) { Set_syn_vars ( item.id, item ); } ); }
       else if (Response.zmq_tag == "pulse")
        { }
       else console.log("zmq_tag: " + Response.zmq_tag + " not known");
     }

  }
/*----------------------------------------------------------------------------------------------------------------------------*/
