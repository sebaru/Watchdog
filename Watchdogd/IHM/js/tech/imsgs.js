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
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').slideUp(); }
                                  else { $('#idAlertThreadNotRunning').slideDown(); }
       $('#idIMSGSTechID').val( Response.tech_id );
       $('#idIMSGSJabberID').val( Response.jabberid );
       $('#idIMSGSPassword').val( Response.password );
     }, null);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTargetInstance').empty();
    Select_from_api ( "idTargetInstance", "/api/instance/list", null, "instances", "instance_id", function (Response)
                       { return ( Response.instance_id ); }, "MASTER" );
    IMSGS_Load_config ();
  }
