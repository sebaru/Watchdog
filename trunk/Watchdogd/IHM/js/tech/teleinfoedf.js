 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function TINFO_Sauver_parametre ( )
  { var json_request = JSON.stringify(
     { instance:    $('#idTargetInstance').val(),
       tech_id:     $('#idTINFOTechID').val(),
       port:        parseInt($('#idTINFOPort').val()),
       description: $('#idTINFODescription').val(),
     });
    Send_to_API ( 'POST', "/api/process/teleinfoedf/set", json_request, function() { Load_page(); }, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function TINFO_Load_config ()
  { Send_to_API ( "GET", "/api/process/teleinfoedf/status?instance="+$('#idTargetInstance').val(), null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').slideUp(); }
                                  else { $('#idAlertThreadNotRunning').slideDown(); }
       $('#idTINFOTechID').val( Response.tech_id );
       $('#idTINFOPort').val( Response.port );
       $('#idTINFODescription').val( Response.description );
       $('#idTINFOMode').val( Response.mode );
       $('#idTINFORetry').val( Response.retry_in );
       $('#idTINFOLastView').val( Response.last_view );
       $('#idTINFOComm').val( (Response.comm_status ? "TRUE" : "FALSE" ) );
     }, null);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTargetInstance').empty();
    Send_to_API ( "GET", "/api/process/teleinfoedf/list", null, function(Response)
     { $.each ( Response.tinfos, function ( i, tinfo )
        { $('#idTargetInstance').append("<option value='"+tinfo.instance+"'>"+
                                          tinfo.tech_id+ " sur " +tinfo.instance+"</option>"); } );
       TINFO_Load_config ();
     }, null );
  }
