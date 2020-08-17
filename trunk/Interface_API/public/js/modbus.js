 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Go_to_modbus_run ()
  { Redirect ( "/tech/modbus_run" );
  }
 function Go_to_modbus_map ()
  { Redirect ( "/tech/modbus_map" );
  }/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Del ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'DELETE', "/api/process/modbus/del", json_request, function ()
     { $('#idTableModbus').DataTable().ajax.reload(null, false);
     });
  }

/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Modbus_Del ( tech_id )
  { table = $('#idTableModbus').DataTable();
    selection = table.ajax.json().modules.filter( function(item) { return item.tech_id==tech_id } )[0];
    $('#idModalModbusDelTitre').text ( "Détruire le module ?" );
    $('#idModalModbusDelMessage').html("Etes-vous sur de vouloir supprimer ce module ?"+
                                    "<hr>"+
                                    "<strong>"+selection.tech_id + " - " + selection.hostname + "</strong>" +
                                    "<br>" + selection.description
                                   );
    $('#idModalModbusDelValider').attr( "onclick", "Valider_Modbus_Del('"+tech_id+"')" );
    $('#idModalModbusDel').modal("show");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Edit ( tech_id )
  { var json_request = JSON.stringify(
       { tech_id : $('#idModalModbusEditTechID').val(),
         hostname: $('#idModalModbusEditHostname').val(),
         description: $('#idModalModbusEditDescription').val(),
         watchdog: $('#idModalModbusEditWatchdog').val(),
       }
     );
    Send_to_API ( 'POST', "/api/process/modbus/set", json_request, function ()
     { $('#idTableModbus').DataTable().ajax.reload(null, false);
     });
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Modbus_Edit ( tech_id )
  { table = $('#idTableModbus').DataTable();
    selection = table.ajax.json().modules.filter( function(item) { return item.tech_id==tech_id } )[0];
    $('#idModalModbusEditTitre').text ( "Editer WAGO - " + selection.tech_id );
    $('#idModalModbusEditTechID').val ( selection.tech_id );
    $('#idModalModbusEditTechID').attr ( "readonly", true );
    $('#idModalModbusEditHostname').val ( selection.hostname );
    $('#idModalModbusEditDescription').val( selection.description );
    $('#idModalModbusEditWatchdog').val( selection.watchdog );
    $('#idModalModbusEditValider').attr( "onclick", "Valider_Modbus_Edit('"+tech_id+"')" );
    $('#idModalModbusEdit').modal("show");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Add ( )
  { var json_request = JSON.stringify(
       { tech_id :    $('#idModalModbusEditTechID').val().toUpperCase(),
         hostname:    $('#idModalModbusEditHostname').val(),
         description: $('#idModalModbusEditDescription').val(),
         watchdog:    $('#idModalModbusEditWatchdog').val(),
       }
     );

    Send_to_API ( 'POST', "/api/process/modbus/add", json_request, function ()
     { $('#idTableModbus').DataTable().ajax.reload(null, false);
     });
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Modbus_Add ()
  { $('#idModalModbusEditTitre').text ( "Ajouter un composant WAGO" );
    $('#idModalModbusEditTechID').val ( "" );
    $('#idModalModbusEditTechID').attr ( "readonly", false );
    $('#idModalModbusEditHostname').val ( "" );
    $('#idModalModbusEditDescription').val( "" );
    $('#idModalModbusEditWatchdog').val( "" );
    $('#idModalModbusEditValider').attr( "onclick", "Valider_Modbus_Add()" );
    $('#idModalModbusEdit').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableModbus').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id",
         ajax: {	url : "/api/process/modbus/list",	type : "GET", dataSrc: "modules",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "tech_id", "title":"TechID", "className": "align-middle text-center" },
            { "data": "hostname", "title":"Hostname", "className": "align-middle text-center hidden-xs" },
            { "data": null, "title":"Started", "className": "align-middle text-center",
              "render": function (item)
                { if (item.enable==true)
                   { return( Bouton ( "success", "Désactiver le WAGO",
                                      "Modbus_disable_module", item.tech_id, "Actif" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le WAGO",
                                      "Modbus_enable_module", item.tech_id, "Désactivé" ) );
                   }
                },
            },
            { "data": "description", "title":"Description", "className": "align-middle text-center hidden-xs" },
            { "data": "watchdog", "title":"Watchdog", "className": "align-middle text-center hidden-xs" },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer le module", "Show_Modal_Modbus_Edit", item.tech_id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le module", "Show_Modal_Modbus_Del", item.tech_id, "trash", null );
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
