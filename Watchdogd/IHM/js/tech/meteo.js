 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Demande l'envoi d'un Meteo de test ********************************************************/
 function Meteo_test ( id )
  { table = $('#idTableMeteo').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request = { uuid : selection.uuid, tech_id : selection.tech_id, zmq_tag: "TEST" };
    Send_to_API ( 'POST', "/api/process", JSON.stringify( json_request ), null );
  }
/************************************ Demande l'envoi d'un Meteo de test ******************************************************/
 function Meteo_Refresh ( )
  { $('#idTableMeteo').DataTable().ajax.reload(null, false);
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function Meteo_Del_Valider ( selection )
  { var json_request = JSON.stringify({ uuid : selection.uuid, tech_id: selection.tech_id });
    Send_to_API ( 'DELETE', "/api/process/config", json_request, function(Response)
     { Process_reload ( json_request.uuid );
       $('#idTableMeteo').DataTable().ajax.reload(null, false);
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function Meteo_Del ( id )
  { table = $('#idTableMeteo').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Supprimer la connexion "+selection.tech_id,
                     "Etes-vous sûr de vouloir supprimer cette connexion ?",
                     selection.tech_id + " - code insee "+selection.code_insee,
                     function () { Meteo_Del_Valider( selection ) } ) ;
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Meteo_Set ( id )
  { var json_request =
       { uuid       : $('#idTargetInstance').val(),
         tech_id    : $('#idMeteoTechID').val().toUpperCase(),
         token      : $('#idMeteoToken').val(),
         description: $('#idMeteoDescription').val(),
         code_insee : $('#idMeteoCodeInsee').val(),
       };
    if (id) json_request.id = parseInt(id);                                                             /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );
       $('#idTableMeteo').DataTable().ajax.reload(null, false);
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Meteo_Edit ( id )
  { table = $('#idTableMeteo').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    $('#idMeteoTitre').text("Editer la source Météo " + selection.tech_id);
    Select_from_api ( "idTargetInstance", "/api/process/list", "name=meteo", "Process", "uuid", function (Response)
                        { return ( Response.host ); }, null );
    $('#idMeteoTechID').val( selection.tech_id ).off("input").on("input", function () { Controle_tech_id( "idMeteo", null ); } );
    $('#idMeteoDescription').val( selection.description );
    $('#idMeteoToken').val( selection.token );
    $('#idMeteoCodeInsee').val( selection.code_insee );
    $('#idMeteoValider').off("click").on( "click", function () { Meteo_Set(selection.id); } );
    $('#idMeteoEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Meteo_Add ( )
  { $('#idMeteoTitre').text("Ajouter une source Météo");
    Select_from_api ( "idTargetInstance", "/api/process/list", "name=meteo", "Process", "uuid", function (Response)
                        { return ( Response.host ); }, null );
    $('#idMeteoTechID').val("").off("input").on("input", function () { Controle_tech_id( "idMeteo", null ); } );
    $('#idMeteoDescription').val("");
    $('#idMeteoToken').val("");
    $('#idMeteoCodeInsee').val("");
    $('#idMeteoValider').off("click").on( "click", function () { Meteo_Set(null); } );
    $('#idMeteoEdit').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableMeteo').DataTable(
     { pageLength : 50,
       fixedHeader: true, paging: false, ordering: true, searching: true,
       ajax: { url : "/api/process/config", type : "GET", data: { name: "meteo" }, dataSrc: "config",
               error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
             },
       rowId: "id",
       columns:
         [ { "data": "host",   "title":"Host",   "className": "align-middle text-center" },
            { "data": null, "title":"Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
            },
           { "data": "token", "title":"token", "className": "align-middle " },
           { "data": "code_insee", "title":"Code Insee", "className": "align-middle " },
           { "data": "description", "title":"Description", "className": "align-middle " },
           { "data": null, "title":"comm", "className": "align-middle ",
             "render": function (item)
               { if (item.comm==true) { return( Bouton ( "success", "Le bit est a 1", null, null, "1" ) );        }
                                 else { return( Bouton ( "outline-secondary", "Le bit est a 0", null, null, "0" ) ); }
               },
           },
           { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
             "render": function (item)
               { boutons = Bouton_actions_start ();
                 boutons += Bouton_actions_add ( "primary", "Editer la connexion", "Meteo_Edit", item.id, "pen", null );
                 boutons += Bouton_actions_add ( "outline-info", "Tester la connexion", "Meteo_Test", item.id, "question", null );
                 boutons += Bouton_actions_add ( "danger", "Supprimer la connexion", "Meteo_Del", item.id, "trash", null );
                 boutons += Bouton_actions_end ();
                 return(boutons);
               },
           }
         ],
       /*order: [ [0, "desc"] ],*/
       responsive: true,
     });

  }
