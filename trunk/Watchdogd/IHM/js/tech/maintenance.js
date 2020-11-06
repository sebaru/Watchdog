 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Maintenance_Compiler_tous_dls ( )
  { var json_request = JSON.stringify( { compil_all: true } );
    Send_to_API ( 'POST', "/api/dls/compil", json_request, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Maintenance_Reset_Instance ( )
  { var json_request = JSON.stringify( { instance: Get_target_instance() } );
    Send_to_API ( 'POST', "/api/instance/reset", json_request, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Maintenance_Set_Log_Level ( )
  { var json_request = JSON.stringify( { instance: Get_target_instance(), log_level: $("#idMaintenanceLogLevel").val() } );
    Send_to_API ( 'POST', "/api/log/set", json_request, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { /*var json_request =
     { hostname:    $('#idArchiveDBHostname').val(),
       port:        $('#idArchiveDBPort').val(),
       username:    $('#idArchiveDBUsername').val(),
       database:    $('#idArchiveDBDatabase').val(),
       retention:   $('#idArchiveDBRetention').val(),
       buffer_size: $('#idArchiveDBBufferSize').val(),
     };
    if ($('#idArchiveDBPassword').val().length > 0) { json_request.password = $('#idArchiveDBPassword').val(); }
    Send_to_API ( "GET", "/api/config/get", null, function ( Response )
     { /*if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
       $('#idArchiveDBHostname').val(Response.hostname);
       $('#idArchiveDBPort').val(Response.port);
       $('#idArchiveDBUsername').val(Response.username);
       $('#idArchiveDBDatabase').val(Response.database);
       $('#idArchiveDBRetention').val(Response.retention);
       $('#idArchiveDBBufferSize').val(Response.buffer_size);
     });*/

  }
