 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Ephemeride_Sauver_parametre ( )
  { var json_request = JSON.stringify(
     { tech_id:     $('#idEphemerideTechID').val(),
       description: $('#idEphemerideDescription').val(),
       token:       $('#idEphemerideToken').val(),
       code_insee:  $('#idEphemerideCodeInsee').val(),
     });
    Send_to_API ( 'POST', "/api/process/ephemeride/set", json_request, function() { Load_page(); }, null );
  }
/************************************ Demande l'envoi d'un Ephemeride de test ********************************************************/
 function Ephemeride_test ( target )
  { var json_request = JSON.stringify({});
    Send_to_API ( 'PUT', "/api/process/ephemeride/test", json_request, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Ephemeride_Load_config ()
  { Send_to_API ( "GET", "/api/process/ephemeride/status", null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
       $('#idEphemerideTechID').val( Response.tech_id );
       $('#idEphemerideDescription').val( Response.description );
       $('#idEphemerideToken').val( Response.token );
       $('#idEphemerideCodeInsee').val( Response.code_insee );
     }, null);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Ephemeride_Load_config ();
  }
