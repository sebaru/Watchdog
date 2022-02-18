 document.addEventListener('DOMContentLoaded', Load_syn, false);

/************************************ Controle de saisie avant envoi **********************************************************/
 function Synoptique_set_controle_page ( page_initiale )
  { FormatPage = RegExp(/^[a-zA-Z0-9_ ]+$/);
    table = $('#idTableSyn').DataTable();
    input = $('#idModalSynEditPage');
    if ( FormatPage.test(input.val())==false )
     { input.addClass("bg-danger");    $('#idModalSynEditValider').attr("disabled", true);
       Popover_show ( input, 'Caractères autorisés', 'lettres, chiffres, espaces et _' );
     }
    else
     { input.removeClass("bg-danger"); $('#idModalSynEditValider').attr("disabled", false);
       Popover_hide( input );
     }
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Synoptique_set ( syn_id )
  {
    var json_request =
       { parent_id: parseInt($('#idModalSynEditPPage').val()),
         page     : $('#idModalSynEditPage').val(),
         libelle  : $('#idModalSynEditLibelle').val(),
         access_level: parseInt($('#idModalSynEditAccessLevel').val()),
         mode_affichage: parseInt($('#idModalSynEditAffichage').val())
       };
    if (syn_id>0) json_request.syn_id = parseInt(syn_id);                                               /* Ajout ou édition ? */
    else json_request.image = "syn_maison.png";

    Send_to_API ( "POST", "/api/syn/set", JSON.stringify(json_request), function(Response)
     { $('#idTableSyn').DataTable().ajax.reload(null, false);
     }, null );

/*    if (syn_id>0 && fichierSelectionne != null)
     { var reader = new FileReader();
       reader.onloadend = function()
        { Send_to_API ( "POSTFILE", "/api/upload?filename=syn_"+syn_id+"&type="+fichierSelectionne.type+"&thumb=100", reader.result, function(Response)
           { $('#idTableSyn').DataTable().ajax.reload(null, false);
           }, null );
        };
       reader.readAsArrayBuffer(fichierSelectionne);
     }*/
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Syn_Add ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.syn_id==syn_id } )[0];
    $('#idModalSynEditTitre').text ( "Ajouter un synoptique" );
    $('#idModalSynEditPPage').empty();
    $.each ( table.ajax.json().synoptiques.sort( function(a, b)
                                                  { if (a.page<b.page) return(-1);
                                                    if (a.page>b.page) return(1);
                                                    return(0);
                                                  } ),
             function ( i, syn )
              { $('#idModalSynEditPPage').append("<option value='"+syn.syn_id+"'>"+syn.page+"</option>"); } );
    if (syn_id>0) $('#idModalSynEditPPage').val ( syn_id );
    $('#idModalSynEditPage').val("");
    $('#idModalSynEditPage').attr("oninput", "Synoptique_set_controle_page(null)");
    Synoptique_set_controle_page (null)
    $('#idModalSynEditAffichage').val("0");
    $('#idModalSynEditLibelle').val("");
    $('#idModalSynEditAccessLevel').attr("max", localStorage.getItem("access_level") ).val(0);
    $('#idModalSynEditValider').attr( "onclick", "Synoptique_set('0')" );
    $('#idModalSynEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Syn_Edit ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.syn_id==syn_id } )[0];
    $('#idModalSynEditTitre').text ( "Modifier le synoptique " + selection.page );
    $('#idModalSynEditPPage').empty();
    $.each ( table.ajax.json().synoptiques.sort( function(a, b)
                                                  { if (a.page<b.page) return(-1);
                                                    if (a.page>b.page) return(1);
                                                    return(0);
                                                  } ),
             function ( i, syn )
              { $('#idModalSynEditPPage').append("<option value='"+syn.syn_id+"'>"+syn.page+"</option>"); } );
    $('#idModalSynEditPPage').val ( selection.pid );
    if (syn_id==1) $('#idModalSynEditPPage').attr("disabled", true );
              else $('#idModalSynEditPPage').attr("disabled", false );
    $('#idModalSynEditPage').val( selection.page );
    $('#idModalSynEditPage').attr("oninput", "Synoptique_set_controle_page('"+selection.page+"')");
    Synoptique_set_controle_page (selection.page)
    $('#idModalSynEditAffichage').val( selection.mode_affichage );
    $('#idModalSynEditLibelle').val( selection.libelle );
    $('#idModalSynEditAccessLevel').attr("max", localStorage.getItem("access_level") )
                                   .val( selection.access_level );
    $('#idModalSynEditValider').attr( "onclick", "Synoptique_set('"+selection.syn_id+"')" );
    $('#idModalSynEdit').modal("show");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valide_del_synoptique ( syn_id )
  { var json_request = JSON.stringify( { syn_id: syn_id } );
    Send_to_API ( "DELETE", "/api/syn/del", json_request, function(Response)
     { $('#idTableSyn').DataTable().ajax.reload(null, false);
     }, null );;
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Syn_Del ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.syn_id==syn_id } )[0];
    Show_modal_del ( "Détruire le synoptique ?",
                     "Etes-vous sur de vouloir supprimer le synoptique suivant et toutes ses dépendances (DLS, mnémoniques, ...) ?",
                     selection.page+" - "+selection.libelle,
                     function () { Valide_del_synoptique(syn_id); } );
  }

/******************************************************************************************************************************/
 function Valide_edit_image ( syn_id, image_name )
  { var json_request = { syn_id: syn_id, image: image_name };
    console.debug ( json_request );
    Send_to_API ( "POST", "/api/syn/set", JSON.stringify(json_request), function(Response)
     { $('#idTableSyn').DataTable().ajax.reload(null, false);
     }, null );
    $('#idSynEditImage').modal("hide");
  }

 function Show_modal_edit_image ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.syn_id==syn_id } )[0];
    $('#idSynEditImageTitre').text ( "Modifier l'image pour " + htmlEncode(selection.libelle) );

    images = [ "syn_maison.png", "syn_communication.png", "syn_reseau.png",
               "syn_buanderie.png", "syn_camera.png", "syn_chambre_double.png", "syn_chambre_simple.png",
               "syn_cuisine.png", "syn_garage.png", "syn_cour.png", "syn_jardin.png", "syn_piscine.png",
               "syn_salle_de_bain.png", "syn_salon.png", "syn_jeux.png", "syn_tele.png",
               "syn_ouvrants.png", "syn_volets.png",
               "syn_luminaires.png", "syn_spot.png", "syn_sonorisation.png",
               "syn_maintenance.png", "syn_parametres.png", "syn_horloge.png",
               "syn_confort.png", "syn_vmc.png", "syn_energie.png", "syn_chaudiere.png", "syn_electricite.png",
               "syn_ups.png", "syn_panneau_solaire.png"
             ];

    liste = $("#idSynEditImageListe");
    liste.empty();
    images.forEach ( function (element)
                      { liste.append( $("<img>").addClass("wtd-synoptique-preview m-1")
                             .attr("name", element).attr("src", "/img/"+element)
                             .click ( function () { Valide_edit_image(syn_id, element); } ) );
                      } );
    $('#idSynEditImage').modal("show");
  }


/********************************************* Appelé au chargement de la page ************************************************/
 function Load_syn ()
  { console.log ("in load synoptique !");
    $('#idTableSyn').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/syn/list",	type : "GET", dataSrc: "synoptiques",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "syn_id",
         columns:
          [ { "data": "syn_id", "title": "#", "className": "align-middle text-center" },
            { "data": null, "title":"Aperçu", "className": "align-middle text-center",
              "render": function (item)
                { target = "/img/"+item.image;
                  return( "<img src='"+target+"' class='wtd-synoptique-preview' loading=lazy alt='No Image !' onclick='Show_modal_edit_image("+item.syn_id+")' />" ); }
            },
            { "data": null, "title":"<i class='fas fa-star'></i> Level", "className": "align-middle text-center",
              "render": function (item)
                { return( Badge_Access_level ( item.access_level ) ); }
            },
            { "data": null, "title":"Affichage Full", "className": "align-middle text-center",
              "render": function (item)
                { if (item.mode_affichage==1) return( "Mode Full" ); else return ("Mode Simple"); }
            },
            { "data": null, "title": "Parent", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/"+item.ppage, "Voir le synoptique "+item.plibelle, item.ppage ) ); },
            },
            { "data": null, "title": "Page", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/"+item.page, "Voir le synoptique "+item.libelle, item.page ) ); },
            },
            { "data": null, "title": "Description", "className": "align-middle ",
              "render": function (item)
                { return( Lien ( "/"+item.page, "Voir le synoptique "+item.libelle, item.libelle ) ); },
            },
            { "data": "dls_count", "title": "#dls", "className": "align-middle text-center" },
            { "data": "subsyn_count", "title": "#SubSyn", "className": "align-middle text-center" },
            { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  /*boutons += Bouton_actions_add ( "outline-primary", "Ouvrir l'atelier", "Redirect", '/tech/atelier/'+item.syn_id, "image", null );*/
                  boutons += Bouton_actions_add ( "outline-primary", "Configurer", "Show_Modal_Syn_Edit", item.syn_id, "pen", null );
                  boutons += Bouton_actions_add ( "outline-success", "Ajouter un synoptique fils", "Show_Modal_Syn_Add", item.syn_id, "plus", null );
                  boutons += Bouton_actions_add ( "outline-primary", "Voir les tableaux", "Redirect", '/tech/tableau?syn_id='+item.syn_id, "chart-line", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le synoptique", "Show_Modal_Syn_Del", item.syn_id, "trash", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            }
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );
  }
