 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Phidget_refresh ( )
  { $('#idTablePhidgetHub').DataTable().ajax.reload(null, false);
    $('#idTablePhidgetIO').DataTable().ajax.reload(null, false);
    $('#idTablePhidgetRun').DataTable().ajax.reload(null, false);
  }
/************************************ Envoi les infos de modifications phidget ************************************************/
 function Phidget_enable_hub ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/process/phidget/start", json_request, function ()
     { $('#idTablePhidgetHub').DataTable().ajax.reload(null, false);
       $('#idTablePhidgetRun').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications phidget ************************************************/
 function Phidget_disable_hub ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/process/phidget/stop", json_request, function ()
     { $('#idTablePhidgetHub').DataTable().ajax.reload(null, false);
       $('#idTablePhidgetRun').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications phidget ************************************************/
 function Valider_Phidget_Del ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'DELETE', "/api/process/phidget/del", json_request, function ()
     { $('#idTablePhidgetHub').DataTable().ajax.reload(null, false);
       $('#idTablePhidgetRun').DataTable().ajax.reload(null, false);
     });
  }
/********************************************* Afichage du modal d'edition phidget ********************************************/
 function Show_Phidget_Del ( tech_id )
  { table = $('#idTablePhidgetHub').DataTable();
    selection = table.ajax.json().modules.filter( function(item) { return item.tech_id==tech_id } )[0];
    Show_modal_del ( "Détruire le module "+tech_id+" ?",
                     "Etes-vous sur de vouloir supprimer ce module ?",
                      selection.tech_id + " - " + selection.hostname + " - " + selection.description,
                     "Valider_Phidget_Del('"+tech_id+"')" );
  }
/************************************ Envoi les infos de modifications phidget ************************************************/
 function Valider_Phidget_Set ( tech_id )
  { var json_request = JSON.stringify(
       { tech_id : $('#idModalPhidgetTechID').val(),
         hostname: $('#idModalPhidgetHostname').val(),
         description: $('#idModalPhidgetDescription').val(),
       }
     );
    Send_to_API ( 'POST', "/api/process/phidget/set", json_request, function ()
     { $('#idTablePhidgetHub').DataTable().ajax.reload(null, false);
       $('#idTablePhidgetRun').DataTable().ajax.reload(null, false);
     });
  }
/********************************************* Afichage du modal d'edition phidget ********************************************/
 function Show_Phidget_Hub_Edit ( tech_id )
  { table = $('#idTablePhidgetHub').DataTable();
    selection = table.ajax.json().modules.filter( function(item) { return item.tech_id==tech_id } )[0];
    $('#idModalPhidgetTitre').text ( "Editer Phidget - " + selection.tech_id );
    $('#idModalPhidgetTechID').val ( selection.tech_id );
    $('#idModalPhidgetTechID').attr ( "readonly", true );
    $('#idModalPhidgetHostname').val ( selection.hostname );
    $('#idModalPhidgetDescription').val( selection.description );
    $('#idModalPhidgetValider').attr( "onclick", "Valider_Phidget_Edit('"+tech_id+"')" );
    $('#idModalPhidget').modal("show");
  }
/********************************************* Afichage du modal d'edition phidget ********************************************/
 function Show_Phidget_Hub_Add ()
  { $('#idModalPhidgetTitre').text ( "Ajouter un Hub Phidget" );
    $('#idModalPhidgetTechID').val ( "" );
    $('#idModalPhidgetTechID').attr ( "readonly", false );
    $('#idModalPhidgetHostname').val ( "" );
    $('#idModalPhidgetDescription').val( "" );
    $('#idModalPhidgetValider').attr( "onclick", "Valider_Phidget_Set()" );
    $('#idModalPhidget').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Send_to_API ( "GET", "/api/process/phidget/status", null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
     });
    $('#idTablePhidgetHub').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id",
         ajax: {	url : "/api/process/phidget/list_hub",	type : "GET", dataSrc: "hubs",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "tech_id", "title":"TechID", "className": "align-middle text-center" },
            { "data": "hostname", "title":"Hostname", "className": "align-middle text-center " },
            { "data": null, "title":"Enabled", "className": "align-middle text-center",
              "render": function (item)
                { if (item.enable==true)
                   { return( Bouton ( "success", "Désactiver le HUB",
                                      "Phidget_disable_hub", item.tech_id, "Actif" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le HUB",
                                      "Phidget_enable_hub", item.tech_id, "Désactivé" ) );
                   }
                },
            },
            { "data": "description", "title":"Description", "className": "align-middle text-center " },
            { "data": "date_create", "title":"Date Création", "className": "align-middle text-center " },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer le hub", "Show_Phidget_Hub_Edit", item.tech_id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le hub", "Show_Phidget_Hub_Del", item.tech_id, "trash", null );
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
