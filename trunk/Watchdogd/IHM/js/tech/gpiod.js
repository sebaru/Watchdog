 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function GPIOD_Sauver_parametre ( )
  { var json_request = JSON.stringify(
     { instance: $('#idTargetInstance').val(),
     });
    Send_to_API ( 'POST', "/api/process/gpiod/set", json_request, function() { Load_page(); }, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function GPIOD_Load_config ()
  { Send_to_API ( "GET", "/api/process/gpiod/status?instance="+$('#idTargetInstance').val(), null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').slideUp(); }
                                  else { $('#idAlertThreadNotRunning').slideDown(); }
       $('#idGPIODTable').DataTable().clear();
       $('#idGPIODTable').DataTable().rows.add( Response.gpios ).draw();

     }, null);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTargetInstance').empty();
    Select_from_api ( "idTargetInstance", "/api/instance/list", null, "instances", "instance_id", function (Response)
                       { return ( Response.instance_id ); }, "MASTER" );
    $('#idGPIODTable').DataTable(
     { pageLength : 50,
       fixedHeader: true,
       data: null,
       rowId: "id",
       columns:
         [ { "data": "gpio",   "title":"#Gpio",   "className": "align-middle text-center" },
           { "data": null, "title":"Mode I/O",    "className": "align-middle ",
             "render": function (item)
               { return("test"); }
           },
           { "data": null, "title":"Active Low",    "className": "align-middle ",
             "render": function (item)
               { return("test2"); }
           },
         ],
       /*order: [ [0, "desc"] ],*/
       responsive: true,
     });
    GPIOD_Load_config ();
  }
