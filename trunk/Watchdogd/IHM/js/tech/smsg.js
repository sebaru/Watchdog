 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function GSM_Sauver_parametre ( )
  { var json_request = JSON.stringify(
     { /*instance:      Get_locale_instance(),*/
       tech_id:       $('#idGSMTechID').val(),
       smsbox_apikey: $('#idGSMAPIKey').val(),
       description:   $('#idGSMDescription').val(),
     });
    Send_to_API ( 'POST', "/api/process/smsg/set", json_request, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  {
    $('#idTitleInstance').text(Get_locale_instance());

    Send_to_API ( "GET", "/api/process/smsg/status?instance="+Get_locale_instance(), null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
       $('#idGSMTechID').val( Response.tech_id );
       $('#idGSMDescription').val( Response.description );
       $('#idGSMAPIKey').val( Response.smsbox_apikey );
       $('#idGSMComm').val( (Response.comm_status ? "TRUE" : "FALSE" ) );
       $('#idGSMNbrSMS').val( Response.nbr_sms );
     }, null);
  }
