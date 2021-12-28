 document.addEventListener('DOMContentLoaded', Load_page, false);

 function MODBUS_refresh ( )
  { $('#idTableMODBUS').DataTable().ajax.reload(null, false);
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Disable ( id )
  { table = $('#idTableMODBUS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { enable : false,
       uuid   : selection.uuid,
       tech_id: selection.tech_id,
       id     : selection.id
     };

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       $('#idTableMODBUS').DataTable().ajax.reload(null, false);
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Enable ( id )
  { table = $('#idTableMODBUS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { enable : true,
       uuid   : selection.uuid,
       tech_id: selection.tech_id,
       id     : selection.id
     };

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       $('#idTableMODBUS').DataTable().ajax.reload(null, false);
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function MODBUS_Del_Valider ( selection )
  { var json_request = { uuid : selection.uuid, tech_id: selection.tech_id };
    Send_to_API ( 'DELETE', "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );
       $('#idTableMODBUS').DataTable().ajax.reload(null, false);
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function MODBUS_Del ( id )
  { table = $('#idTableMODBUS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Supprimer la connexion "+selection.tech_id,
                     "Etes-vous sûr de vouloir supprimer cette connexion ?",
                     selection.tech_id + " - "+selection.hostnamename +" - "+ selection.description,
                     function () { MODBUS_Del_Valider( selection ) } ) ;
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function MODBUS_Set ( selection )
  { var json_request =
     { uuid:           $('#idTargetProcess').val(),
       tech_id:        $('#idMODBUSTechID').val(),
       hostname: $('#idMODBUSHostname').val(),
       description: $('#idMODBUSDescription').val(),
       watchdog: parseInt($('#idMODBUSWatchdog').val()),
       max_request_par_sec: parseInt($('#idMODBUSMaxRequestParSec').val()),
     };
    if (selection) json_request.id = parseInt(selection.id);                                            /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { if (selection && selection.uuid != json_request.uuid) Process_reload ( selection.uuid );/* Restart de l'ancien subprocess si uuid différent */
       Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       $('#idTableMODBUS').DataTable().ajax.reload(null, false);
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Edit ( id )
  { table = $('#idTableMODBUS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=modbus", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, selection.uuid );
    $('#idMODBUSTitre').text("Editer la connexion MODBUS " + selection.tech_id);
    $('#idMODBUSTechID').val( selection.tech_id ).off("input").on("input", function () { Controle_tech_id( "idMODBUS", selection.tech_id ); } );
    $('#idMODBUSHostname').val ( selection.hostname );
    $('#idMODBUSDescription').val( selection.description );
    $('#idMODBUSWatchdog').val( selection.watchdog );
    $('#idMODBUSMaxRequestParSec').val( selection.max_request_par_sec );
    $('#idMODBUSValider').off("click").on( "click", function () { MODBUS_Set(selection); } );
    $('#idMODBUSEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Add ( )
  { $('#idMODBUSTitre').text("Ajouter un MODBUS");
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=modbus", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, null );
    $('#idMODBUSTechID').val("").off("input").on("input", function () { Controle_tech_id( "idMODBUS", null ); } );
    $('#idMODBUSHostname').val ( "" );
    $('#idMODBUSDescription').val( "" );
    $('#idMODBUSWatchdog').val( "" );
    $('#idMODBUSMaxRequestParSec').val( "" );
    $('#idMODBUSValider').off("click").on( "click", function () { MODBUS_Set(null); } );
    $('#idMODBUSEdit').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableMODBUS').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id",
         ajax: {	url : "/api/process/config", type : "GET", data: { name: "modbus" }, dataSrc: "config",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "instance",   "title":"Instance",   "className": "align-middle text-center" },
            { "data": null, "title":"Enable", "className": "align-middle text-center",
               "render": function (item)
                { if (item.enable==true)
                  { return( Bouton ( "success", "Désactiver le module", "MODBUS_Disable", item.id, "Actif" ) ); }
                 else
                  { return( Bouton ( "outline-secondary", "Activer le module", "MODBUS_Enable", item.id, "Désactivé" ) ); }
                },
            },
            { "data": null, "title":"Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
            },
            { "data": "description", "title":"Description", "className": "align-middle text-center " },
            { "data": "watchdog", "title":"Watchdog (s)", "className": "align-middle text-center " },
            { "data": "hostname", "title":"Hostname", "className": "align-middle text-center " },
            { "data": "max_request_par_sec", "title":"Max Requete/s", "className": "align-middle text-center " },
            { "data": null, "title":"IO_COMM", "className": "align-middle text-center",
              "render": function (item)
                { if (item.comm==true) { return( Bouton ( "success", "Comm OK", null, null, "1" ) );        }
                                  else { return( Bouton ( "outline-secondary", "Comm Failed", null, null, "0" ) ); }
                },
            },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer le module", "MODBUS_Edit", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le module", "MODBUS_Del", item.id, "trash", null );
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
/*----------------------------------------------------------------------------------------------------------------------------*/
