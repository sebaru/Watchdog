 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Demande de refresh **********************************************************************/
 function TELEINFO_Refresh ( )
  { $('#idTableTELEINFO').DataTable().ajax.reload(null, false);
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function TELEINFO_Set ( selection )
  { var json_request =
     { uuid:        $('#idTargetProcess').val(),
       thread_tech_id:     $('#idTELEINFOTechID').val(),
       description: $('#idTELEINFODescription').val(),
       port:        $('#idTELEINFOPort').val(),
     };
    if (selection) json_request.id = parseInt(selection.id);                                            /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { if (selection && selection.uuid != json_request.uuid) Process_reload ( selection.uuid );/* Restart de l'ancien subprocess si uuid différent */
       Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       TELEINFO_Refresh();
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function TELEINFO_Edit ( id )
  { table = $('#idTableTELEINFO').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=teleinfoedf", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, selection.uuid );
    $('#idTELEINFOTitre').text("Editer la connexion GSM " + selection.thread_tech_id);
    $('#idTELEINFOTechID').val( selection.thread_tech_id ).off("input").on("input", function () { Controle_thread_tech_id( "idTELEINFO", selection.thread_tech_id ); } );
    $('#idTELEINFODescription').val( selection.description );
    $('#idTELEINFOPort').val( selection.port );
    $('#idTELEINFOValider').off("click").on( "click", function () { TELEINFO_Set(selection); } );
    $('#idTELEINFOEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function TELEINFO_Add ( )
  { $('#idTELEINFOTitre').text("Ajouter un équipement GSM");
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=teleinfoedf", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, null );
    $('#idTELEINFOTechID').val("").off("input").on("input", function () { Controle_thread_tech_id( "idTELEINFO", null ); } );
    $('#idTELEINFODescription').val("");
    $('#idTELEINFOPort').val("");
    $('#idTELEINFOValider').off("click").on( "click", function () { TELEINFO_Set(null); } );
    $('#idTELEINFOEdit').modal("show");
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function TELEINFO_Del_Valider ( selection )
  { var json_request = { uuid : selection.uuid, thread_tech_id: selection.thread_tech_id };
    Send_to_API ( 'DELETE', "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );
       TELEINFO_Refresh();
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function TELEINFO_Del ( id )
  { table = $('#idTableTELEINFO').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Supprimer la connexion "+selection.thread_tech_id,
                     "Etes-vous sûr de vouloir supprimer cette connexion ?",
                     selection.thread_tech_id + " - "+selection.description,
                     function () { TELEINFO_Del_Valider( selection ) } ) ;
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableTELEINFO').DataTable(
     { pageLength : 50,
       fixedHeader: true, paging: false, ordering: true, searching: true,
       ajax: { url : "/api/process/config", type : "GET", data: { name: "teleinfoedf" }, dataSrc: "config",
               error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
             },
       rowId: "id",
       columns:
         [ { "data": "instance",   "title":"Instance",   "className": "align-middle text-center" },
           { "data": null, "title":"Tech_id", "className": "align-middle text-center",
             "render": function (item)
               { return( Lien ( "/tech/dls_source/"+item.thread_tech_id, "Voir la source", item.thread_tech_id ) ); }
           },
           { "data": "description", "title":"Description", "className": "align-middle " },
           { "data": "port", "title":"Device Port", "className": "align-middle " },
           { "data": null, "title":"IO_COMM", "className": "align-middle text-center",
             "render": function (item)
               { if (item.comm==true) { return( Bouton ( "success", "Comm OK", null, null, "1" ) );        }
                                 else { return( Bouton ( "outline-secondary", "Comm Failed", null, null, "0" ) ); }
               },
           },
           { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
             "render": function (item)
               { boutons = Bouton_actions_start ();
                 boutons += Bouton_actions_add ( "primary", "Editer la connexion", "TELEINFO_Edit", item.id, "pen", null );
                 boutons += Bouton_actions_add ( "danger", "Supprimer la connexion", "TELEINFO_Del", item.id, "trash", null );
                 boutons += Bouton_actions_end ();
                 return(boutons);
               },
           }
         ],
       /*order: [ [0, "desc"] ],*/
       responsive: true,
     });

  }
/*----------------------------------------------------------------------------------------------------------------------------*/
