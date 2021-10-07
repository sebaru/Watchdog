 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Modbus_refresh ( )
  { $('#idTableModbus').DataTable().ajax.reload(null, false);
    $('#idTableModbusRun').DataTable().ajax.reload(null, false);
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Modbus_enable_module ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/process/modbus/start", json_request, function ()
     { $('#idTableModbus').DataTable().ajax.reload(null, false);
       $('#idTableModbusRun').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Modbus_disable_module ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/process/modbus/stop", json_request, function ()
     { $('#idTableModbus').DataTable().ajax.reload(null, false);
       $('#idTableModbusRun').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Del ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'DELETE', "/api/process/modbus/del", json_request, function ()
     { $('#idTableModbus').DataTable().ajax.reload(null, false);
       $('#idTableModbusRun').DataTable().ajax.reload(null, false);
     });
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Modbus_Del ( tech_id )
  { table = $('#idTableModbus').DataTable();
    selection = table.ajax.json().modules.filter( function(item) { return item.tech_id==tech_id } )[0];
    Show_modal_del ( "Détruire le module "+tech_id+" ?",
                     "Etes-vous sur de vouloir supprimer ce module ?",
                      selection.tech_id + " - " + selection.hostname + " - " + selection.description,
                     "Valider_Modbus_Del('"+tech_id+"')" );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Edit ( tech_id )
  { var json_request = JSON.stringify(
       { tech_id : $('#idModalModbusEditTechID').val(),
         hostname: $('#idModalModbusEditHostname').val(),
         description: $('#idModalModbusEditDescription').val(),
         watchdog: parseInt($('#idModalModbusEditWatchdog').val()),
         max_request_par_sec: parseInt($('#idModalModbusEditMaxRequestParSec').val()),
       }
     );
    Send_to_API ( 'POST', "/api/process/modbus/set", json_request, function ()
     { $('#idTableModbus').DataTable().ajax.reload(null, false);
       $('#idTableModbusRun').DataTable().ajax.reload(null, false);
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
    $('#idModalModbusEditMaxRequestParSec').val( selection.max_request_par_sec );
    $('#idModalModbusEditValider').attr( "onclick", "Valider_Modbus_Edit('"+tech_id+"')" );
    $('#idModalModbusEdit').modal("show");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Add ( )
  { var json_request = JSON.stringify(
       { tech_id :    $('#idModalModbusEditTechID').val().toUpperCase(),
         hostname:    $('#idModalModbusEditHostname').val(),
         description: $('#idModalModbusEditDescription').val(),
         watchdog:    parseInt($('#idModalModbusEditWatchdog').val()) ,
         max_request_par_sec: parseInt($('#idModalModbusEditMaxRequestParSec').val()),
       }
     );
    Send_to_API ( 'POST', "/api/process/modbus/add", json_request, function ()
     { $('#idTableModbus').DataTable().ajax.reload(null, false);
       $('#idTableModbusRun').DataTable().ajax.reload(null, false);
     });
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Modbus_Add ()
  { $('#idModalModbusEditTitre').text ( "Ajouter un composant WAGO" );
    $('#idModalModbusEditTechID').val ( "" );
    $('#idModalModbusEditTechID').attr ( "readonly", false );
    $('#idModalModbusEditHostname').val ( "" );
    $('#idModalModbusEditDescription').val( "" );
    $('#idModalModbusEditWatchdog').val( "5" );
    $('#idModalModbusEditMaxRequestParSec').val( "100" );
    $('#idModalModbusEditValider').attr( "onclick", "Valider_Modbus_Add()" );
    $('#idModalModbusEdit').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Send_to_API ( "GET", "/api/process/modbus/status", null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
     });
    $('#idTableModbus').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id",
         ajax: {	url : "/api/process/modbus/list",	type : "GET", dataSrc: "modules",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "tech_id", "title":"TechID", "className": "align-middle text-center" },
            { "data": "hostname", "title":"Hostname", "className": "align-middle text-center " },
            { "data": null, "title":"Enabled", "className": "align-middle text-center",
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
            { "data": "description", "title":"Description", "className": "align-middle text-center " },
            { "data": "watchdog", "title":"Watchdog (s)", "className": "align-middle text-center " },
            { "data": "max_request_par_sec", "title":"Max Requete/s", "className": "align-middle text-center " },
            { "data": "date_create", "title":"Date Création", "className": "align-middle text-center " },
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

    $('#idTableModbusRun').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id",
         ajax: {	url : "/api/process/modbus/modules_status",	type : "GET", dataSrc: "modules",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "tech_id", "title":"TechID", "className": "align-middle text-center" },
            { "data": "mode", "title":"Mode", "className": "align-middle text-center" },
            { "data": null, "title":"Started", "className": "align-middle text-center",
              "render": function (item)
                { if (item.started==false)
                       { return( Bouton ( "outline-warning", "Le module ne tourne pas", null, null, "Module OFF" ) ); }
                  else { return( Bouton ( "success", "Le module est activé", null, null, "Oui" ) ); }
                }
            },
            { "data": "last_reponse", "title":"Last Response", "className": "align-middle text-center " },
            { "data": "nbr_deconnect", "title":"Nbr Deconnect", "className": "align-middle text-center " },
            { "data": "date_retente", "title":"Date Retente", "className": "align-middle text-center " },
            { "data": "date_next_eana", "title":"Next EANA", "className": "align-middle text-center " },
            { "data": "nbr_request_par_sec", "title":"Requetes/s", "className": "align-middle text-center " },
            { "data": "nbr_entree_tor", "title":"Nbr Entrees TOR", "className": "align-middle text-center " },
            { "data": "nbr_entree_ana", "title":"Nbr Entrees ANA", "className": "align-middle text-center " },
            { "data": "nbr_sortie_tor", "title":"Nbr Sorties TOR", "className": "align-middle text-center " },
            { "data": "nbr_sortie_ana", "title":"Nbr Sorties ANA", "className": "align-middle text-center " },

          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

  }
