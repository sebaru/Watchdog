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
       { parent_id: $('#idModalSynEditPPage').val(),
         page     : $('#idModalSynEditPage').val(),
         libelle  : $('#idModalSynEditDescription').val(),
         access_level: $('#idModalSynEditAccessLevel').val()
       };
    if (syn_id>0) json_request.syn_id = syn_id;                                                         /* Ajout ou édition ? */

    fichierSelectionne = $('#idModalSynEditImageCustom')[0].files[0];

    if (fichierSelectionne!=null)
     { json_request.image = "custom"; }
    else
     { json_request.image = $('#idModalSynEditImageSelect').val(); }

    Send_to_API ( "POST", "/api/syn/set", JSON.stringify(json_request), function(Response)
     { if (fichierSelectionne == null) $('#idTableSyn').DataTable().ajax.reload(null, false);
     }, null );

    if (syn_id>0 && fichierSelectionne != null)
     { var reader = new FileReader();
       reader.onloadend = function()
        { Send_to_API ( "POSTFILE", "/api/upload?filename=syn_"+syn_id+"&type="+fichierSelectionne.type+"&thumb=100", reader.result, function(Response)
           { $('#idTableSyn').DataTable().ajax.reload(null, false);
           }, null );
        };
       reader.readAsArrayBuffer(fichierSelectionne);
     }
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Syn_Add ( syn_id )
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
    $('#idModalSynEditPage').attr("oninput", "Synoptique_set_controle_page(null)");
    Synoptique_set_controle_page (null)
    $('#idModalSynEditDescription').val("");
    $('#idModalSynEditAccessLevel').attr("max", localStorage.getItem("access_level") );
    $('#idModalSynEditAccessLevel').val(0);
    $('#idModalSynEditValider').attr( "onclick", "Synoptique_set('0')" );
    $('#idModalSynEditImageSelect').val("Accueil");
    $('#idModalSynEditImage').attr("src", "/img/syn_maison.png");
    $('#idModalSynEditImage').css("max-width", "100px");
    $('#idModalSynEditImageSelect').change ( function () { Select_image_changed(syn_id) } );
    $('#idModalSynEditImageCustom').val('');
    $('#idModalSynEdit').modal("show");
  }
/******************************************************************************************************************************/
 function Select_image_changed( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.id==syn_id } )[0];
    if ($('#idModalSynEditImageSelect').val() != "custom")
     { $('#idModalSynEditImage').attr("src", "/img/"+$('#idModalSynEditImageSelect').val()); }
    else
     { $('#idModalSynEditImage').attr("src", "/upload/syn_"+selection.id+".jpg"); }

  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Syn_Edit ( syn_id )
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
    $('#idModalSynEditPage').attr("oninput", "Synoptique_set_controle_page('"+selection.page+"')");
    Synoptique_set_controle_page (selection.page)
    $('#idModalSynEditDescription').val( selection.libelle );
    $('#idModalSynEditAccessLevel').attr("max", localStorage.getItem("access_level") );
    $('#idModalSynEditAccessLevel').val( selection.access_level );
    $('#idModalSynEditValider').attr( "onclick", "Synoptique_set('"+selection.id+"')" );
    $('#idModalSynEditImageSelect').val(selection.image);
    $('#idModalSynEditImageSelect').change ( function () { Select_image_changed(syn_id) } );
    if (selection.image != "custom")
     { $('#idModalSynEditImage').attr("src", "/img/"+selection.image); }
    else
     { $('#idModalSynEditImage').attr("src", "/upload/syn_"+selection.id+".jpg"); }
    $('#idModalSynEditImage').css("max-width", "100px");
    $('#idModalSynEditImageCustom').val('');
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
                { if(item.image=="custom") target="/upload/syn_"+item.id+".jpg";
                  else target = "/img/"+item.image;
                  return( Lien( '/'+item.page, "Voir le synoptique", "<img src='"+target+"' height=80px loading=lazy alt='No Image !' >" ) ); }
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
                  boutons += Bouton_actions_add ( "outline-primary", "Configurer", "Show_Modal_Syn_Edit", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "outline-primary", "Ajouter un synoptique fils", "Show_Modal_Syn_Add", item.id, "plus", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le synoptique", "Show_Modal_Syn_Del", item.id, "trash", null );
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
