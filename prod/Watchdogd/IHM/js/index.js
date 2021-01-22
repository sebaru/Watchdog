 document.addEventListener('DOMContentLoaded', Load_page, false);

 var Synoptique;                                                     /* Toutes les données du synoptique en cours d'affichage */
 var Messages_loaded;                                                                      /* true si le datatable a été créé */

/********************************************* Affichage des vignettes ********************************************************/
 function Changer_img_src ( id, target )
  { $('#'+id).fadeOut("slow", function()
     { $('#'+id).attr("src", target).fadeIn("slow"); } );
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
 function Charger_messages ( page )
  { if (page == null || page == "") { param = ""; } else { param = "page="+page; }
    if (Messages_loaded==true)
     { $('#idTableMessages').DataTable().ajax.page = page;
       $('#idTableMessages').DataTable().ajax.url("/api/histo/alive?"+param).load();
     }
    else $('#idTableMessages').DataTable(
        { pageLength : 25,
          fixedHeader: true, searching: false, paging:false,
          ajax: {	url: "/api/histo/alive?"+param, type : "GET", dataSrc: "enregs",
                  error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
                },
          rowId: "id",
          columns:
           [ { "data": null, "title":"Type", "className": "align-middle text-center bg-dark",
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
             { "data": "dls_shortname", "title":"Objet", "className": "text-center bg-dark d-none d-sm-table-cell" },
             { "data": "libelle", "title":"Message", "className": "text-center bg-dark" },
             { "data": null, "title":"Acquit", "className": "align-middle text-center bg-dark d-none d-sm-table-cell",
               "render": function (item)
                 { if (item.nom_ack!="None") return(item.nom_ack);
                   else return( Bouton ( "primary", "Acquitter le message", "Msg_acquitter", item.id, "Acquitter" ) );
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
 function Change_page ( page )
  { $('#bodycard').fadeOut("fast", function ()
     { $('#bodycard').empty();
       Send_to_API ( "GET", "/api/syn/show", (page === "" ? null : "page="+page), function(Response)
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
          $('#bodycard').fadeIn("slow");
        }, null );
     });

    $('#idTableMessages').fadeOut("fast", function ()
     { Charger_messages ( page );
       $('#idTableMessages').fadeIn("slow");
     }, null);

  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Creer_card ( Response )
  {

    var card = $('<div></div>');
    card.addClass("card bg-transparent m-1");

    var card_header = $('<div></div>');
    card_header.addClass("card-header text-center");

    var activite = $('<img>');
    activite.attr("id", "idVignetteActivite_"+Response.id);
    activite.addClass("wtd-vignette");
    card_header.append(activite);
    card.append(card_header);

    var secu_bien = $('<img>');
    secu_bien.attr("id", "idVignetteSecuBien_"+Response.id);
    secu_bien.addClass("wtd-vignette");
    card_header.append(secu_bien);

    var secu_pers = $('<img>');
    secu_pers.attr("id", "idVignetteSecuPers_"+Response.id);
    secu_pers.addClass("wtd-vignette");
    card_header.append(secu_pers);
    card_header.append(" "+Response.page);

    var card_body = $('<div></div>');
    card_body.addClass("card-body text-center");

    var img = $('<img>');
    if (Response.image=="custom") { img.attr("src", "/upload/syn_"+Response.id+".jpg"); }
                             else { img.attr("src", "/img/syn_"+Response.image+".png"); }
    img.attr("onclick", "Change_page('"+Response.page+"')");
    img.addClass("wtd-synoptique");
    card_body.append(img);
    card.append(card_body);

    var card_footer = $('<div></div>');
    card_footer.addClass("card-footer text-center");
    card_footer.append("<p>"+Response.libelle+"</p>");
    card.append(card_footer);

    return(card);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    Change_page ( vars[1] );

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
       console.debug (Response);
       if (!Synoptique) return;
            if (Response.zmq_tag == "DLS_HISTO")
        { if (Response.syn_id == Synoptique.id)                                     /* S'agit-il d'un message de notre page ? */
           { Charger_messages ( Synoptique.syn_page ); }
        }
       else if (Response.zmq_tag == "SET_SYN_VARS")
        { $.each ( Response.syn_vars, function (i, item) { Set_syn_vars ( item.id, item ); } ); }
       else console.log("zmq_tag: " + Response.zmq_tag + " not known");
     }

  }
/*----------------------------------------------------------------------------------------------------------------------------*/
