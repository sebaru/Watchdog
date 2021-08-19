 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function SMS_Sauver_parametre ( )
  { var json_request = JSON.stringify(
     { instance:      $('#idTargetInstance').val(),
       tech_id:       $('#idGSMTechID').val(),
       ovh_service_name: $('#idGSMOVHServiceName').val(),
       ovh_application_key: $('#idGSMOVHApplicationKey').val(),
       ovh_application_secret: $('#idGSMOVHApplicationSecret').val(),
       ovh_consumer_key: $('#idGSMOVHConsumerKey').val(),
       description:   $('#idGSMDescription').val(),
     });
    Send_to_API ( 'POST', "/api/process/smsg/set", json_request, function() { Load_page(); }, null );
  }
/************************************ Demande l'envoi d'un SMS de test ********************************************************/
 function SMS_test ( target )
  { var json_request = JSON.stringify(
     { instance: $('#idTargetInstance').val(),
       mode: target
     });
    Send_to_API ( 'PUT', "/api/process/smsg/send", json_request, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function SMS_Load_config ()
  { Send_to_API ( "GET", "/api/process/smsg/status?instance="+$('#idTargetInstance').val(), null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
       $('#idGSMTechID').val( Response.tech_id );
       $('#idGSMDescription').val( Response.description );
       $('#idGSMOVHServiceName').val( Response.ovh_service_name );
       $('#idGSMOVHApplicationKey').val( Response.ovh_application_key );
       $('#idGSMOVHApplicationSecret').val( Response.ovh_application_secret );
       $('#idGSMOVHConsumerKey').val( Response.ovh_consumer_key );
       $('#idGSMComm').val( (Response.comm_status ? "TRUE" : "FALSE" ) );
       $('#idGSMNbrSMS').val( Response.nbr_sms );
     }, null);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTargetInstance').empty();
    Send_to_API ( "GET", "/api/process/smsg/list", null, function(Response)
     { $.each ( Response.gsms, function ( i, gsm )
        { $('#idTargetInstance').append("<option value='"+gsm.instance+"'>"+
                                          gsm.tech_id+ " sur " +gsm.instance+"</option>"); } );
       SMS_Load_config ();
     }, null );
  }
