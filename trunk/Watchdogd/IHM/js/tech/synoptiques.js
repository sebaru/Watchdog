 document.addEventListener('DOMContentLoaded', Load_syn, false);

/************************************ Controle de saisie avant envoi **********************************************************/
 function Synoptique_set_controle_add_page ( )
  { input = $('#idModalSynEditPage');
    if (table.ajax.json().synoptiques.filter( function(item)
                                               { return item.page.toUpperCase()==input.val().toUpperCase() } )[0] === undefined)
     { input.removeClass("bg-danger"); $('#idModalSynEditValider').removeAttr("disabled"); }
    else
     { input.addClass("bg-danger");    $('#idModalSynEditValider').attr("disabled", "on");
       return(false);
     }
    return(true);
  }
/************************************ Controle de saisie avant envoi **********************************************************/
 function Synoptique_set_controle_edit_page ( page_initiale )
  { input = $('#idModalSynEditPage');
    if (table.ajax.json().synoptiques.filter( function(item)
                                               { return item.page.toUpperCase()==input.val().toUpperCase() } )[0] === undefined ||
        input.val() == page_initiale
       )
     { input.removeClass("bg-danger"); $('#idModalSynEditValider').attr("disabled", false); }
    else
     { input.addClass("bg-danger");    $('#idModalSynEditValider').attr("disabled", true); }
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Synoptique_set ( syn_id )
  {
    var json_request =
       { parent_id: $('#idModalSynEditPPage').val(),
         page     : $('#idModalSynEditPage').val(),
         libelle  : $('#idModalSynEditDescription').val(),
         access_level: $('#idModalSynEditAccessLevel').val()
       };
    if (syn_id>0) json_request.syn_id = syn_id;                                                         /* Ajout ou édition ? */
    fichierSelectionne = $('#idModalSynEditImage')[0].files[0];

    Send_to_API ( "POST", "/api/syn/set", JSON.stringify(json_request), function(Response)
     { if (fichierSelectionne == null) $('#idTableSyn').DataTable().ajax.reload(null, false);
     }, null );

    if (syn_id>0 && fichierSelectionne != null)
     { var reader = new FileReader();
       reader.onloadend = function()
        { Send_to_API ( "POSTFILE", "/api/upload?filename=syn_"+syn_id+".jpg&thumb=100", reader.result, function(Response)
           { $('#idTableSyn').DataTable().ajax.reload(null, false);
           }, null );
        };
       reader.readAsArrayBuffer(fichierSelectionne);
     }
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Add ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.id==syn_id } )[0];
    $('#idModalSynEditTitre').text ( "Ajouter un synoptique" );
    $('#idModalSynEditPPage').empty();
    $.each ( table.ajax.json().synoptiques.sort( function(a, b)
                                                  { if (a.page<b.page) return(-1);
                                                    if (a.page>b.page) return(1);
                                                    return(0);
                                                  } ),
             function ( i, syn )
              { $('#idModalSynEditPPage').append("<option value='"+syn.id+"'>"+syn.page+"</option>"); } );
    if (syn_id>0) $('#idModalSynEditPPage').val ( syn_id );
    $('#idModalSynEditPage').val("");
    $('#idModalSynEditPage').attr("oninput", "Synoptique_set_controle_add_page()");
    Synoptique_set_controle_add_page ( )
    $('#idModalSynEditDescription').val("");
    $('#idModalSynEditAccessLevel').attr("max", localStorage.getItem("access_level") );
    $('#idModalSynEditAccessLevel').val(0);
    $('#idModalSynEditValider').attr( "onclick", "Synoptique_set('0')" );
    $('#idModalSynEditImage').val('');
    $('#idModalSynEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Edit ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.id==syn_id } )[0];
    $('#idModalSynEditTitre').text ( "Modifier le synoptique " + selection.page );
    $('#idModalSynEditPPage').empty();
    $.each ( table.ajax.json().synoptiques.sort( function(a, b)
                                                  { if (a.page<b.page) return(-1);
                                                    if (a.page>b.page) return(1);
                                                    return(0);
                                                  } ),
             function ( i, syn )
              { $('#idModalSynEditPPage').append("<option value='"+syn.id+"'>"+syn.page+"</option>"); } );
    $('#idModalSynEditPPage').val ( selection.pid );
    if (syn_id==1) $('#idModalSynEditPPage').attr("disabled", true );
              else $('#idModalSynEditPPage').attr("disabled", false );
    $('#idModalSynEditPage').val( selection.page );
    $('#idModalSynEditPage').attr("oninput", "Synoptique_set_controle_edit_page('"+selection.page+"')");
    $('#idModalSynEditDescription').val( selection.libelle );
    $('#idModalSynEditAccessLevel').attr("max", localStorage.getItem("access_level") );
    $('#idModalSynEditAccessLevel').val( selection.access_level );
    $('#idModalSynEditValider').attr( "onclick", "Synoptique_set('"+selection.id+"')" );
    $('#idModalSynEditImage').val('');
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
                     "Etes-vous sur de vouloir supprimer le synoptique suivant et toutes ses dépendances (DLS, mnémoniques, ...) ?",
                     selection.page+" - "+selection.libelle,
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
          [ { "data": null, "title":"Aperçu", "className": "align-middle text-center",
              "render": function (item)
                { return( "<a href='/"+item.page+"'><img src=/upload/syn_"+item.id+".jpg height=100px loading=lazy alt='No Image !' ></a>" ); }
            },
            { "data": null, "title":"<i class='fas fa-star'></i> Level", "className": "align-middle text-center",
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
            { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Ouvrir l'atelier", "Redirect", '/tech/atelier/'+item.id, "image", null );
                  boutons += Bouton_actions_add ( "outline-primary", "Configurer", "Show_Modal_Edit", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "outline-primary", "Ajouter un synoptique fils", "Show_Modal_Add", item.id, "plus", null );
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
