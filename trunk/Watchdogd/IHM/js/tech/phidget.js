 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Phidget_refresh ( )
  { $('#idTablePhidgetHub').DataTable().ajax.reload(null, false);
    $('#idTablePhidgetIO').DataTable().ajax.reload(null, false);
    $('#idTablePhidgetRun').DataTable().ajax.reload(null, false);
  }
/************************************ Envoi les infos de modifications phidget ************************************************/
 function Phidget_enable_hub ( id )
  { var json_request = JSON.stringify( { id : id } );
    Send_to_API ( 'POST', "/api/process/phidget/hub_start", json_request, function ()
     { $('#idTablePhidgetHub').DataTable().ajax.reload(null, false);
       $('#idTablePhidgetRun').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications phidget ************************************************/
 function Phidget_disable_hub ( id )
  { var json_request = JSON.stringify( { id : id } );
    Send_to_API ( 'POST', "/api/process/phidget/hub_stop", json_request, function ()
     { $('#idTablePhidgetHub').DataTable().ajax.reload(null, false);
       $('#idTablePhidgetRun').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications phidget ************************************************/
 function Valider_Phidget_Hub_Del ( id )
  { var json_request = JSON.stringify( { id : id } );
    Send_to_API ( 'DELETE', "/api/process/phidget/hub_del", json_request, function ()
     { $('#idTablePhidgetHub').DataTable().ajax.reload(null, false);
       $('#idTablePhidgetRun').DataTable().ajax.reload(null, false);
     });
  }
/********************************************* Afichage du modal d'edition phidget ********************************************/
 function Show_Phidget_Hub_Del ( id )
  { table = $('#idTablePhidgetHub').DataTable();
    selection = table.ajax.json().hubs.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Détruire le module "+selection.hostname+" ?",
                     "Etes-vous sur de vouloir supprimer ce module ?",
                      selection.hostname + " - " + selection.description,
                     "Valider_Phidget_Hub_Del("+id+")" );
  }
/************************************ Envoi les infos de modifications phidget ************************************************/
 function Valider_Phidget_Hub_Set ( id )
  { var json_request =
       { enable     : false,
         hostname   : $('#idModalPhidgetHubHostname').val(),
         description: $('#idModalPhidgetHubDescription').val(),
         password   : $('#idModalPhidgetHubPassword').val(),
         serial     : $('#idModalPhidgetHubSerial').val(),
       };
    if (id!=-1) json_request.id = id;
    Send_to_API ( 'POST', "/api/process/phidget/hub_set", JSON.stringify(json_request), function ()
     { $('#idTablePhidgetHub').DataTable().ajax.reload(null, false);
       $('#idTablePhidgetRun').DataTable().ajax.reload(null, false);
     });
  }
/********************************************* Afichage du modal d'edition phidget ********************************************/
 function Show_Phidget_Hub_Edit ( id )
  { table = $('#idTablePhidgetHub').DataTable();
    selection = table.ajax.json().hubs.filter( function(item) { return item.id==id } )[0];
    $('#idModalPhidgetHubTitre').text ( "Editer Phidget - " + selection.hostname );
    $('#idModalPhidgetHubHostname').val ( selection.hostname );
    $('#idModalPhidgetHubDescription').val( selection.description );
    $('#idModalPhidgetHubPassword').val( selection.password );
    $('#idModalPhidgetHubSerial').val( selection.serial );
    $('#idModalPhidgetHubValider').attr( "onclick", "Valider_Phidget_Hub_Set("+id+")" );
    $('#idModalPhidgetHub').modal("show");
  }
/********************************************* Afichage du modal d'edition phidget ********************************************/
 function Show_Phidget_Hub_Add ()
  { $('#idModalPhidgetHubTitre').text ( "Ajouter un Hub Phidget" );
    $('#idModalPhidgetHubHostname').val ( "" );
    $('#idModalPhidgetHubDescription').val( "" );
    $('#idModalPhidgetHubPassword').val( "" );
    $('#idModalPhidgetHubSerial').val( "" );
    $('#idModalPhidgetHubValider').attr( "onclick", "Valider_Phidget_Hub_Set(-1)" );
    $('#idModalPhidgetHub').modal("show");
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
         ajax: {	url : "/api/process/phidget/hub_list",	type : "GET", dataSrc: "hubs",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": null, "title":"Enabled", "className": "align-middle text-center",
              "render": function (item)
                { if (item.enable==true)
                   { return( Bouton ( "success", "Désactiver le HUB",
                                      "Phidget_disable_hub", item.id, "Actif" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le HUB",
                                      "Phidget_enable_hub", item.id, "Désactivé" ) );
                   }
                },
            },
            { "data": "hostname", "title":"Hostname", "className": "align-middle text-center " },
            { "data": "description", "title":"Description", "className": "align-middle text-center " },
            { "data": "password", "title":"Password", "className": "align-middle text-center " },
            { "data": "serial", "title":"Serial Number", "className": "align-middle text-center " },
            { "data": "date_create", "title":"Date Création", "className": "align-middle text-center " },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer le hub", "Show_Phidget_Hub_Edit", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le hub", "Show_Phidget_Hub_Del", item.id, "trash", null );
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
