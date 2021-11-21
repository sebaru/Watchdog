 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Meteo_Sauver_parametre ( )
  { var json_request = JSON.stringify(
     { tech_id:     $('#idMeteoTechID').val(),
       description: $('#idMeteoDescription').val(),
       token:       $('#idMeteoToken').val(),
       code_insee:  $('#idMeteoCodeInsee').val(),
     });
    Send_to_API ( 'POST', "/api/process/meteo/set", json_request, function() { Load_page(); }, null );
  }
/************************************ Demande l'envoi d'un Meteo de test ********************************************************/
 function Meteo_test ( target )
  { var json_request = JSON.stringify({});
    Send_to_API ( 'PUT', "/api/process/meteo/test", json_request, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Meteo_Load_config ()
  { Send_to_API ( "GET", "/api/process", "name=meteo", function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').slideUp(); }
                                  else { $('#idAlertThreadNotRunning').slideDown(); }
       $('#idMeteoTechID').val( Response.tech_id );
       $('#idMeteoDescription').val( Response.description );
       $('#idMeteoToken').val( Response.token );
       $('#idMeteoCodeInsee').val( Response.code_insee );
     }, null);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Meteo_Load_config ();
    $('#idTableMeteo').DataTable(
     { pageLength : 50,
       fixedHeader: true, paging: false, ordering: true, searching: true,
       ajax: {	url : "/api/process",	type : "GET", data: { name: "meteo" }, dataSrc: "infos",
               error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
             },
       rowId: "id",
       columns:
         [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
           { "data": null, "title":"Etat", "className": "align-middle ",
             "render": function (item)
               { if (item.etat==true) { return( Bouton ( "success", "Le bit est a 1", null, null, "1" ) );        }
                                 else { return( Bouton ( "outline-secondary", "Le bit est a 0", null, null, "0" ) ); }
               },
           },
           { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
             "render": function (item)
               { boutons = Bouton_actions_start ();
                 boutons += Bouton_actions_add ( "success", "Activer le bit", "Dls_run_MONO_set", item.acronyme, "power-off", null );
                 boutons += Bouton_actions_end ();
                 return(boutons);
               },
           }
         ],
       /*order: [ [0, "desc"] ],*/
       responsive: true,
     });

  }
