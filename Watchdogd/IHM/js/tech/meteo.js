 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Demande l'envoi d'un METEO de test ******************************************************/
 function METEO_Refresh ( )
  { $('#idTableMETEO').DataTable().ajax.reload(null, false);
  }
/************************************ Demande l'envoi d'un SMS de test ********************************************************/
 function METEO_Test ( id )
  { table = $('#idTableMETEO').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { tech_id: selection.tech_id,
       zmq_tag : "test"
     };
    Send_to_API ( 'POST', "/api/process/send", JSON.stringify(json_request), null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function METEO_Del_Valider ( selection )
  { var json_request = JSON.stringify({ uuid : selection.uuid, tech_id: selection.tech_id });
    Send_to_API ( 'DELETE', "/api/process/config", json_request, function(Response)
     { Process_reload ( json_request.uuid );
       $('#idTableMETEO').DataTable().ajax.reload(null, false);
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function METEO_Del ( id )
  { table = $('#idTableMETEO').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Supprimer la connexion "+selection.tech_id,
                     "Etes-vous sûr de vouloir supprimer cette connexion ?",
                     selection.tech_id + " - code insee "+selection.code_insee,
                     function () { METEO_Del_Valider( selection ) } ) ;
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function METEO_Set ( selection )
  { var json_request =
       { uuid       : $('#idTargetInstance').val(),
         tech_id    : $('#idMETEOTechID').val().toUpperCase(),
         token      : $('#idMETEOToken').val(),
         description: $('#idMETEODescription').val(),
         code_insee : $('#idMETEOCodeInsee').val(),
       };
    if (selection) json_request.id = parseInt(selection.id);                                            /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { if (selection.uuid != json_request.uuid) Process_reload ( selection.uuid ); /* Restart de l'ancien subprocess si uuid différent */
       Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       $('#idTableMETEO').DataTable().ajax.reload(null, false);
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function METEO_Edit ( id )
  { table = $('#idTableMETEO').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    $('#idMETEOTitre').text("Editer la source Météo " + selection.tech_id);
    Select_from_api ( "idTargetInstance", "/api/process/list", "name=meteo", "Process", "uuid", function (Response)
                        { return ( Response.host ); }, null );
    $('#idMETEOTechID').val( selection.tech_id ).off("input").on("input", function () { Controle_tech_id( "idMETEO", null ); } );
    $('#idMETEODescription').val( selection.description );
    $('#idMETEOToken').val( selection.token );
    $('#idMETEOCodeInsee').val( selection.code_insee );
    $('#idMETEOValider').off("click").on( "click", function () { METEO_Set(selection); } );
    $('#idMETEOEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function METEO_Add ( )
  { $('#idMETEOTitre').text("Ajouter une source Météo");
    Select_from_api ( "idTargetInstance", "/api/process/list", "name=meteo", "Process", "uuid", function (Response)
                        { return ( Response.host ); }, null );
    $('#idMETEOTechID').val("").off("input").on("input", function () { Controle_tech_id( "idMETEO", null ); } );
    $('#idMETEODescription').val("");
    $('#idMETEOToken').val("");
    $('#idMETEOCodeInsee').val("");
    $('#idMETEOValider').off("click").on( "click", function () { METEO_Set(null); } );
    $('#idMETEOEdit').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableMETEO').DataTable(
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
           { "data": "description", "title":"Description", "className": "align-middle " },
           { "data": "token", "title":"token", "className": "align-middle " },
           { "data": "code_insee", "title":"Code Insee", "className": "align-middle " },
           { "data": null, "title":"comm", "className": "align-middle text-center",
             "render": function (item)
               { if (item.comm==true) { return( Bouton ( "success", "Le bit est a 1", null, null, "1" ) );        }
                                 else { return( Bouton ( "outline-secondary", "Le bit est a 0", null, null, "0" ) ); }
               },
           },
           { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
             "render": function (item)
               { boutons = Bouton_actions_start ();
                 boutons += Bouton_actions_add ( "primary", "Editer la connexion", "METEO_Edit", item.id, "pen", null );
                 boutons += Bouton_actions_add ( "outline-primary", "Tester la connexion", "METEO_Test", item.id, "question", null );
                 boutons += Bouton_actions_add ( "danger", "Supprimer la connexion", "METEO_Del", item.id, "trash", null );
                 boutons += Bouton_actions_end ();
                 return(boutons);
               },
           }
         ],
       /*order: [ [0, "desc"] ],*/
       responsive: true,
     });

  }
