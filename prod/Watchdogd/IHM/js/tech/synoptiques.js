 document.addEventListener('DOMContentLoaded', Load_syn, false);

/************************************ Controle de saisie avant envoi **********************************************************/
 function Synoptique_set_controle_ppage ( )
  { input = $('#idModalSynEditPPage');
    if (table.ajax.json().synoptiques.filter( function(item) { return item.ppage==input.val() } )[0] !== undefined)
     { input.removeClass("bg-danger"); $('#idModalSynEditValider').removeAttr("disabled"); }
    else
     { input.addClass("bg-danger");    $('#idModalSynEditValider').attr("disabled", "on");
       return(false);
     }
    return(true);
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Synoptique_set ( syn_id )
  {
/*--------------------------------------------- Controle de saisie -----------------------------------------------------------*/
    if (!Synoptique_set_controle_ppage()) return;

    var json_request =
       { parent_id: table.ajax.json().synoptiques.filter( function(item)
                                                           { return item.page.toUpper==$('#idModalSynEditPPage').val().toUpper } )[0].id,
         page: $('#idModalSynEditPage').val(),
         libelle: $('#idModalSynEditDescription').val(),
         access_level: $('#idModalSynEditAccessLevel').val()
       };
    if (syn_id>0) json_request.syn_id = syn_id;                                                         /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/syn/set", JSON.stringify(json_request), function(Response)
     { $('#idTableSyn').DataTable().ajax.reload(null, false);
       $('#idToastStatus').toast('show');
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Add_Dls ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.id==syn_id } )[0];
    $('#idModalAddDlsTechID').val("");
    $('#idModalAddDlsTechID').attr("oninput", "Synoptique_Add_Dls_controle_techid()" );
    $('#idModalAddDlsDescription').val("");
    $('#idModalAddDlsValider').attr( "onclick", "Synoptique_Add_Dls()" );
    $('#idModalAddDls').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Add ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.id==syn_id } )[0];
    $('#idModalSynEditTitre').text ( "Ajouter un synoptique" );
    $('#idModalSynEditPage').val("");
    $('#idModalSynEditPPage').val ( selection.ppage );
    $('#idModalSynEditPPage').attr("oninput", "Synoptique_set_controle_ppage()");
    $('#idModalSynEditDescription').val("");
    $('#idModalSynEditAccessLevel').attr("max", localStorage.getItem("access_level") );
    $('#idModalSynEditAccessLevel').val(0);
    $('#idModalSynEditValider').attr( "onclick", "Synoptique_set('0')" );
    $('#idModalSynEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Edit ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.id==syn_id } )[0];
    $('#idModalSynEditTitre').text ( "Modifier le synoptique" );
    $('#idModalSynEditPPage').val ( selection.ppage );
    $('#idModalSynEditPPage').attr("oninput", "Synoptique_set_controle_ppage()");
    $('#idModalSynEditPage').val( selection.page );
    $('#idModalSynEditDescription').val( selection.libelle );
    $('#idModalSynEditAccessLevel').attr("max", localStorage.getItem("access_level") );
    $('#idModalSynEditAccessLevel').val( selection.access_level );
    $('#idModalSynEditValider').attr( "onclick", "Synoptique_set('"+selection.id+"')" );
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
 function Show_Modal_Del ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.id==syn_id } )[0];
    Show_modal_del ( "Détruire le synoptique ?",
                     "Etes-vous sur de vouloir supprimer le synoptique suivant et toutes ses dépendances (DLS, mnémoniques, ...) ?<hr>"+
                     "<strong>"+selection.page+" - "+selection.libelle+"</strong>",
                      "Valide_del_synoptique("+syn_id+")" );
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
         rowId: "id",
         columns:
          [ { "data": null, "title":"<i class='fas fa-star'></i> Level", "className": "align-middle text-center",
              "render": function (item)
                { return( Badge_Access_level ( item.access_level ) ); }
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
            { "data": null, "title":"Actions", "orderable": false, "className":"text-center",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Ouvrir l'atelier", "Redirect", '/tech/atelier/'+item.id, "image", null );
                  boutons += Bouton_actions_add ( "outline-primary", "Configurer", "Show_Modal_Edit", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "outline-success", "Ajouter un synoptique fils", "Show_Modal_Add", item.id, "plus", null );
                  boutons += Bouton_actions_add ( "outline-info", "Ajouter un module D.L.S", "Show_Modal_Add_Dls", item.id, "code", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le synoptique", "Show_Modal_Del", item.id, "trash", null );
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
