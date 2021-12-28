 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function GPIOD_Sauver_parametre ( id )
  { var json_request =
     { instance      : $('#idTargetProcess').val(),
       id            : id,
       mode_inout    : parseInt ($('#idModalGPIODInOut').val()),
       mode_activelow: parseInt($('#idModalGPIODActiveLow').val()),
       tech_id       : $('#idModalGPIODSelectTechID').val(),
       acronyme      : $('#idModalGPIODSelectAcronyme').val(),
     };
    Send_to_API ( 'POST', "/api/process/gpiod/set", JSON.stringify(json_request),
                  function() { Process_reload ( json_request.instance, "GPIOD", false ); GPIOD_Load_config (); }, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function GPIOD_Load_config ()
  { Send_to_API ( "GET", "/api/process/gpiod/status?instance="+$('#idTargetProcess').val(), null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').slideUp(); }
                                  else { $('#idAlertThreadNotRunning').slideDown(); }
       $('#idGPIODTable').DataTable().clear();
       $('#idGPIODTable').DataTable().rows.add( Response.gpios ).draw();
     }, null);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function GPIOD_Updater_Choix_TechID ( tech_id, acronyme )
  { if ($("#idModalGPIODInOut").val()=="0") classe="DI"; else classe="DO";
    Common_Updater_Choix_TechID ( "idModalGPIOD", classe, selection.tech_id, selection.acronyme );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function GPIOD_Show_modal_map ( id )
  { table = $('#idGPIODTable').DataTable();
    selection = table.rows().data().filter( function(item) { return (item.id==id) } )[0];
    $("#idModalGPIODTitre").text("Mapper la GPIO "+selection.gpio+ " sur " + $('#idTargetProcess').val() );
    if (selection.mode_inout==0) { $("#idModalGPIODInOut").val("0"); }
                            else { $("#idModalGPIODInOut").val("1"); }
    $("#idModalGPIODInOut").off("change").on("change", function ()
     { GPIOD_Updater_Choix_TechID ( selection.tech_id, selection.acronyme ); } );
    if (selection.mode_activelow==0) { $("#idModalGPIODActiveLow").val("0"); }
                                else { $("#idModalGPIODActiveLow").val("1"); }
    GPIOD_Updater_Choix_TechID ( selection.tech_id, selection.acronyme );
    $("#idModalGPIODValider").attr( "onclick", "GPIOD_Sauver_parametre("+id+")" );
    $("#idModalGPIOD").modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTargetProcess').empty();
    Select_from_api ( "idTargetProcess", "/api/instance/list", null, "instances", "instance_id", function (Response)
                       { return ( Response.instance_id ); }, "MASTER" );
    $('#idGPIODTable').DataTable(
     { pageLength : 50,
       fixedHeader: true,
       data: null,
       rowId: "id",
       columns:
         [ { "data": "num", "title":"#Gpio", "className": "align-middle text-center" },
           { "data": null,  "title":"Mode I/O", "className": "align-middle text-center",
             "render": function (item)
               { if (item.mode_inout) return( "OUTPUT" );
                                 else return( "INPUT" );
               }
           },
           { "data": null, "title":"Active Low",    "className": "align-middle text-center",
             "render": function (item)
               { if (item.mode_activelow) return( "Active Low" );
                                     else return( "Active High" );
               }
           },
           { "data": "tech_id",  "title":"Map Tech_ID",  "className": "align-middle text-center" },
           { "data": "acronyme", "title":"Map Acronyme", "className": "align-middle text-center" },
           { "data": null, "title":"Actions", "orderable": false, "className": "align-middle",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "primary", "Editer", "GPIOD_Show_modal_map", item.id, "pen", null );
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
