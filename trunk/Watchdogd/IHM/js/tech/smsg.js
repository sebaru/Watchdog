 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function SMS_Sauver_parametre ( )
  { var json_request = JSON.stringify(
     { instance:      Get_target_instance(),
       tech_id:       $('#idGSMTechID').val(),
       ovh_service_name: $('#idGSMOVHServiceName').val(),
       ovh_application_key: $('#idGSMOVHApplicationKey').val(),
       ovh_application_secret: $('#idGSMOVHApplicationSecret').val(),
       ovh_consumer_key: $('#idGSMOVHConsumerKey').val(),
       description:   $('#idGSMDescription').val(),
     });
    Send_to_API ( 'POST', "/api/process/smsg/set", json_request, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function SMS_Reload ( )
  { Process_reload ( Get_target_instance(), "SMSG", false );
  }
/************************************ Demande l'envoi d'un SMS de test ********************************************************/
 function SMS_test ( target )
  { var json_request = JSON.stringify(
     { instance: $('#idTargetInstance2').val(),
       mode: target
     });
    Send_to_API ( 'PUT', "/api/process/smsg/send", json_request, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_config_sms ()
  { Send_to_API ( "GET", "/api/process/smsg/status?instance="+$('#idTargetInstance2').val(), null, function(Response)
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
  { $('#idTargetInstance2').empty();
    Send_to_API ( "GET", "/api/process/smsg/list", null, function(Response)
     { $.each ( Response.gsms, function ( i, gsm )
        { $('#idTargetInstance2').append("<option value='"+gsm.instance+"'>"+
                                          gsm.tech_id+ " sur " +gsm.instance+"</option>"); } );
       Load_config_sms ();
     }, null );
  }
