 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Demande de refresh **********************************************************************/
 function PHIDGET_Refresh ( )
  { $('#idTablePHIDGET').DataTable().ajax.reload(null, false);
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function PHIDGET_Disable ( id )
  { table = $('#idTablePHIDGET').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { enable : false,
       uuid   : selection.uuid,
       thread_tech_id: selection.thread_tech_id,
       id     : selection.id
     };

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       PHIDGET_Refresh();
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function PHIDGET_Enable ( id )
  { table = $('#idTablePHIDGET').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { enable : true,
       uuid   : selection.uuid,
       thread_tech_id: selection.thread_tech_id,
       id     : selection.id
     };

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       PHIDGET_Refresh();
     }, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function PHIDGET_Set ( selection )
  { var json_request =
     { uuid:        $('#idTargetProcess').val(),
       thread_tech_id:     $('#idPHIDGETTechID').val(),
       description: $('#idPHIDGETDescription').val(),
       hostname   : $('#idPHIDGETHostname').val(),
       password   : $('#idPHIDGETPassword').val(),
       serial     : $('#idPHIDGETSerial').val(),
     };
    if (selection) json_request.id = parseInt(selection.id);                                            /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { if (selection && selection.uuid != json_request.uuid) Process_reload ( selection.uuid );/* Restart de l'ancien subprocess si uuid différent */
       Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       $('#idTablePHIDGET').DataTable().ajax.reload(null, false);
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function PHIDGET_Edit ( id )
  { table = $('#idTablePHIDGET').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=phidget", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, selection.uuid );
    $('#idPHIDGETTitre').text("Editer la connexion " + selection.thread_tech_id);
    $('#idPHIDGETTechID').val( selection.thread_tech_id ).off("input").on("input", function () { Controle_thread_tech_id( "idPHIDGET", selection.thread_tech_id ); } );
    $('#idPHIDGETDescription').val( selection.description );
    $('#idPHIDGETHostname').val( selection.hostname );
    $('#idPHIDGETPassword').val( selection.password );
    $('#idPHIDGETSerial').val( selection.serial );
    $('#idPHIDGETValider').off("click").on( "click", function () { PHIDGET_Set(selection); } );
    $('#idPHIDGETEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function PHIDGET_Add ( )
  { $('#idPHIDGETTitre').text("Ajouter un équipement GSM");
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=phidget", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, null );
    $('#idPHIDGETTechID').val("").off("input").on("input", function () { Controle_thread_tech_id( "idPHIDGET", null ); } );
    $('#idPHIDGETDescription').val("");
    $('#idPHIDGETHostname').val( "" );
    $('#idPHIDGETPassword').val( "" );
    $('#idPHIDGETSerial').val( "" );
    $('#idPHIDGETValider').off("click").on( "click", function () { PHIDGET_Set(null); } );
    $('#idPHIDGETEdit').modal("show");
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function PHIDGET_Del_Valider ( selection )
  { var json_request = { uuid : selection.uuid, thread_tech_id: selection.thread_tech_id };
    Send_to_API ( 'DELETE', "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );
       PHIDGET_Refresh();
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function PHIDGET_Del ( id )
  { table = $('#idTablePHIDGET').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Supprimer la connexion "+selection.thread_tech_id,
                     "Etes-vous sûr de vouloir supprimer cette connexion ?",
                     selection.thread_tech_id + " - "+selection.description,
                     function () { PHIDGET_Del_Valider( selection ) } ) ;
  }

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTablePHIDGET').DataTable(
     { pageLength : 50,
       fixedHeader: true, paging: false, ordering: true, searching: true,
       ajax: { url : "/api/process/config", type : "GET", data: { name: "phidget" }, dataSrc: "config",
               error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
             },
       rowId: "id",
       columns:
         [ { "data": "instance",   "title":"Instance",   "className": "align-middle text-center" },
           { "data": null, "title":"Enable", "className": "align-middle text-center",
              "render": function (item)
               { if (item.enable==true)
                  { return( Bouton ( "success", "Désactiver le module", "PHIDGET_Disable", item.id, "Actif" ) ); }
                 else
                  { return( Bouton ( "outline-secondary", "Activer le module", "PHIDGET_Enable", item.id, "Désactivé" ) ); }
               },
           },
           { "data": null, "title":"Tech_id", "className": "align-middle text-center",
             "render": function (item)
               { return( Lien ( "/tech/dls_source/"+item.thread_tech_id, "Voir la source", item.thread_tech_id ) ); }
           },
           { "data": "description", "title":"Description", "className": "align-middle text-center " },
           { "data": "hostname", "title":"Hostname", "className": "align-middle text-center " },
           { "data": "password", "title":"Password", "className": "align-middle text-center " },
           { "data": "serial", "title":"Serial Number", "className": "align-middle text-center " },
           { "data": null, "title":"IO_COMM", "className": "align-middle text-center",
            "render": function (item)
              { if (item.comm==true) { return( Bouton ( "success", "Comm OK", null, null, "1" ) );        }
                                else { return( Bouton ( "outline-secondary", "Comm Failed", null, null, "0" ) ); }
              },
           },
           { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
             "render": function (item)
               { boutons = Bouton_actions_start ();
                 boutons += Bouton_actions_add ( "primary", "Editer la connexion", "PHIDGET_Edit", item.id, "pen", null );
                 boutons += Bouton_actions_add ( "danger", "Supprimer la connexion", "PHIDGET_Del", item.id, "trash", null );
                 boutons += Bouton_actions_end ();
                 return(boutons);
               },
           }
         ],
       /*order: [ [0, "desc"] ],*/
       responsive: true,
     });
  }
