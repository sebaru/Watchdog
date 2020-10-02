 document.addEventListener('DOMContentLoaded', Load_syn, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valide_edit_synoptique ( syn_id )
  { var xhr = new XMLHttpRequest;
    xhr.open('POST', "/api/syn/set");
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify(
       { id: syn_id,
         page: $('#idModalSynEditPage').val(),
         ppage: $('#idModalSynEditPPage').val(),
         libelle: $('#idModalSynEditDescription').val(),
         access_level: $('#idModalSynEditAccessLevel').val()
       }
     );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idTableSyn').DataTable().ajax.reload(null, false);
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send(json_request);
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Edit ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.id==syn_id } )[0];
    $('#idModalSynEditTitre').text ( "Editer - " + selection.page );
    $('#idModalSynEditPage').val ( selection.page );
    $('#idModalSynEditPPage').val ( selection.ppage );
    $('#idModalSynEditPPage').attr ( "readonly", true );
    $('#idModalSynEditDescription').val(selection.libelle);
    $('#idModalSynEditAccessLevel').attr("max", localStorage.getItem("access_level")-1 );
    $('#idModalSynEditAccessLevel').val(selection.access_level );
    $('#idModalSynEditValider').attr( "onclick", "Valide_edit_synoptique("+syn_id+")" );
    $('#idModalSynEdit').modal("show");
  }

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valide_new_synoptique ( )
  { var xhr = new XMLHttpRequest;
    xhr.open('POST', "/api/syn/set");
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify(
       { page: $('#idModalSynEditPage').val(),
         ppage: $('#idModalSynEditPPage').val(),
         libelle: $('#idModalSynEditDescription').val(),
         access_level: $('#idModalSynEditAccessLevel').val()
       }
     );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idTableSyn').DataTable().ajax.reload(null, false);
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send(json_request);
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Add ( ppage )
  { table = $('#idTableSyn').DataTable();
    $('#idModalSynEditTitre').text ( "Ajouter un synoptique" );
    $('#idModalSynEditPage').val("");
    $('#idModalSynEditPPage').val ( ppage );
    $('#idModalSynEditPPage').attr ( "readonly", true );
    $('#idModalSynEditDescription').val("");
    $('#idModalSynEditAccessLevel').attr("max", localStorage.getItem("access_level")-1 );
    $('#idModalSynEditAccessLevel').val(0);
    $('#idModalSynEditValider').attr( "onclick", "Valide_new_synoptique()" );
    $('#idModalSynEdit').modal("show");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valide_del_synoptique ( syn_id )
  { var xhr = new XMLHttpRequest;
    xhr.open('DELETE', "/api/syn/del/"+syn_id );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idTableSyn').DataTable().ajax.reload(null, false);
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send();
  }

/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Del ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.id==syn_id } )[0];
    $('#idModalSynDelTitre').text ( "Détruire le synoptique ?" );
    $('#idModalSynDelMessage').html("Etes-vous sur de vouloir supprimer le synoptique suivant "+
                                    "et toutes ses dépendances (DLS, mnémoniques, ...) ?<hr>"+
                                    "<strong>"+selection.page+" - "+selection.libelle+
                                    "</strong>" );
    $('#idModalSynDelValider').attr( "onclick", "Valide_del_synoptique("+syn_id+")" );
    $('#idModalSynDel').modal("show");
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
          [ { "data": "id", "title":"#", "className": "text-center hidden-xs" },
            { "data": null, "className": "text-center align-middle hidden-xs",
              "render": function (item)
                { return( Badge ( (item.access_level>=6 ? "warning" : "info"), "Niveau de privilèges", item.access_level ) );
                },
              "title":"Level", "orderable": true
            },
            { "data": "ppage", "title": "Parent", "name": "ppage", "className": "hidden-xs" },
            { "data": null, "className": "align-middle hidden-xs",
              "render": function (item)
                { return( Lien ( "/atelier/"+item.id, "Ouvrir Atelier", item.page ) );
                },
              "title":"Page", "orderable": true
            },
            { "data": null, "className": "align-middle hidden-xs",
              "render": function (item)
                { return( Lien ( "/atelier/"+item.id, "Ouvrir Atelier", item.libelle ) );
                },
              "title":"Description", "orderable": true
            },
            { "data": null,
              "render": function (item)
                { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"+
                         "    <button class='btn btn-outline-primary btn-sm' "+
                                     "onclick=window.location.href='/atelier/"+item.id+"' "+
                                     "data-toggle='tooltip' title='Ouvrir Atelier'>"+
                                     "<i class='fas fa-image'></i></button>"+
                         "    <button class='btn btn-outline-secondary btn-sm' "+
                                     "onclick=Show_Modal_Edit("+item.id+") "+
                                     "data-toggle='tooltip' title='Configurer le synoptique'>"+
                                     "<i class='fas fa-pen'></i></button>"+
                         "    <button class='btn btn-outline-success btn-sm' "+
                                     "onclick=Show_Modal_Add('"+item.page+"') "+
                                     "data-toggle='tooltip' title='Ajoute un synoptique fils'>"+
                                     "<i class='fas fa-plus'></i></button>"+
                         "    <button class='btn btn-outline-info btn-sm' "+
                                     "data-toggle='tooltip' title='Ajout un module D.L.S'>"+
                                     "<i class='fas fa-plus'></i> <i class='fas fa-code'></i></button>"+
                         "    <button class='btn btn-danger btn-sm' "+
                                     "onclick=Show_Modal_Del("+item.id+") "+
                                     "data-toggle='tooltip' title='Supprimer le Synoptique\net ses dépendances'>"+
                                     "<i class='fas fa-trash'></i></button>"+
                         "</div>"
                        )
                },
              "title":"Actions", "orderable": false, "className":"text-center"
            }
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );
  }
