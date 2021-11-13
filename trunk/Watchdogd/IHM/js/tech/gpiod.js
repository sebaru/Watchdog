 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function GPIOD_Sauver_parametre ( id )
  { var json_request =
     { instance      : $('#idTargetInstance').val(),
       id            : id,
       mode_inout    : parseInt ($('#idGPIODInOut').val()),
       mode_activelow: parseInt($('#idGPIODActiveLow').val()),
     };
    Send_to_API ( 'POST', "/api/process/gpiod/set", JSON.stringify(json_request),
                  function() { Process_reload ( json_request.instance, "GPIOD", false ); }, null );
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
 function GPIOD_Show_modal_map ( id )
  { table = $('#idGPIODTable').DataTable();
    selection = table.rows().data().filter( function(item) { return (item.id==id) } )[0];
    $("#idModalGPIODMapTitre").text("Mapper la GPIO "+selection.gpio+ " sur " + $('#idTargetInstance').val() );
    if (selection.mode_inout==0) { classe="DI"; $("#idModalGPIODInOut").val("0"); }
                            else { classe="DO"; $("#idModalGPIODInOut").val("1"); }
    if (selection.mode_activelow==0) { $("#idModalGPIODActiveLow").val("0"); }
                                else { $("#idModalGPIODActiveLow").val("1"); }
    Common_Updater_Choix_TechID ( "idModalGPIODMap", classe, selection.tech_id, selection.acronyme );
    $("#idModalGPIODMap").modal("show");
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
               { if (item.mode_inout) return( "INPUT" );
                                 else return( "OUTPUT" );
               }
           },
           { "data": null, "title":"Active Low",    "className": "align-middle ",
             "render": function (item)
               { if (item.mode_activelow) return( "ActiveLow" );
                                     else return( "ActiveHigh" );
               }
           },
           { "data": "tech_id",  "title":"Map Tech_ID",  "className": "align-middle text-center" },
           { "data": "acronyme", "title":"Map Acronyme", "className": "align-middle text-center" },
           { "data": null, "title":"Actions", "orderable": false, "className": "align-middle",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "primary", "Mapper", "GPIOD_Show_modal_map", item.id, "directions", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                }
           }
         ],
       /*order: [ [0, "desc"] ],*/
       responsive: true,
     });
    GPIOD_Load_config ();
  }
