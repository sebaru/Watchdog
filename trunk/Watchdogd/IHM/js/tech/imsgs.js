 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function IMSGS_Sauver_parametre ( )
  { var json_request = JSON.stringify(
     { instance: $('#idTargetInstance').val(),
       tech_id : $('#idIMSGSTechID').val(),
       jabberid: $('#idIMSGSJabberID').val(),
       password: $('#idIMSGSPassword').val(),
     });
    Send_to_API ( 'POST', "/api/process/imsgs/set", json_request, function() { Load_page(); }, null );
  }
/************************************ Demande l'envoi d'un SMS de test ********************************************************/
 function IMSGS_Test ( )
  { var json_request = JSON.stringify(
     { instance: $('#idTargetInstance').val(),
     });
    Send_to_API ( 'PUT', "/api/process/imsgs/send", json_request, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function IMSGS_Load_config ()
  { Send_to_API ( "GET", "/api/process/imsgs/status?instance="+$('#idTargetInstance').val(), null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
       $('#idIMSGSTechID').val( Response.tech_id );
       $('#idIMSGSJabberID').val( Response.jabberid );
       $('#idIMSGSPassword').val( Response.password );
     }, null);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTargetInstance').empty();
    Send_to_API ( "GET", "/api/instance/list", null, function(Response)
     { $.each ( Response.instances, function ( i, instance )
        { $('#idTargetInstance').append("<option value='"+instance.instance_id+"'>"+instance.instance_id+"</option>"); } );
       IMSGS_Load_config ();
     }, null );
  }
