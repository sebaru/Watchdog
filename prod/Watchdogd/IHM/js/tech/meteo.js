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
  { Send_to_API ( "GET", "/api/process/meteo/status", null, function(Response)
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
  }
