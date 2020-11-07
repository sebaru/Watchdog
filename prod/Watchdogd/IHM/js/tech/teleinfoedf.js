 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function TINFO_Sauver_parametre ( )
  { var json_request = JSON.stringify(
     { /*instance:      Get_target_instance(),*/
       tech_id:     $('#idTINFOTechID').val(),
       port:        $('#idTINFOPort').val(),
       description: $('#idTINFODescription').val(),
     });
    Send_to_API ( 'POST', "/api/process/teleinfoedf/set", json_request, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function TINFO_Reload ( )
  { Process_reload ( Get_target_instance(), "TELEINFOEDF", false );
  }
/************************************ Demande l'envoi d'un SMS de test ********************************************************
 function TINFO_test ( )
  { var json_request = JSON.stringify(
     { instance:      Get_target_instance(),
     });
    Send_to_API ( 'PUT', "/api/process/smsg/send", json_request, null );
  }*/
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTitleInstance').val(Get_target_instance());
    Send_to_API ( "GET", "/api/process/teleinfoedf/status?instance="+Get_target_instance(), null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
       $('#idTINFOTechID').val( Response.tech_id );
       $('#idTINFOPort').val( Response.port );
       $('#idTINFODescription').val( Response.description );
       $('#idTINFOMode').val( Response.mode );
       $('#idTINFORetry').val( Response.retry_in );
       $('#idTINFOLastView').val( Response.last_view );
       $('#idTINFOComm').val( (Response.comm_status ? "TRUE" : "FALSE" ) );
     }, null);
  }
