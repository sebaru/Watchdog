 document.addEventListener('DOMContentLoaded', Load_common_tech, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_common_tech ()
  { $('#idTargetInstance').empty();
    if (localStorage.getItem("instance_is_master")=="true")
     { Send_to_API ( "GET", "/api/instance/list", null, function (Response)
        { $('#idTargetInstance').append("<option value='MASTER'>MASTER</option>");

          $.each ( Response.instances, function ( i, instance )
           { $('#idTargetInstance').append("<option value='"+instance.instance_id+"'"+
                                             (instance.instance_id == Get_target_instance() ? "selected" : "")+">"+
                                             instance.instance_id+"</option>"); } );
          $('#idTitleInstance').text(Get_target_instance());
        }, null);
     }
    else
     { localStorage.setItem ( "TargetInstance", localStorage.getItem("instance") );
       $('#idTargetInstance').append("<option value='"+localStorage.getItem("instance")+"'>"+localStorage.getItem("instance")+"</option>");
     }
  }
/********************************************* Redirige la page ***************************************************************/
 function Get_target_instance ( )
  { instance = localStorage.getItem ( "TargetInstance" );
    if (instance == null) { instance=localStorage.getItem("instance"); }
    return(instance);
  }

/********************************************* Redirige la page ***************************************************************/
 function Set_target_instance ( instance )
  { localStorage.setItem ( "TargetInstance", instance );
    window.location.reload();
  }
/********************************************* Redirige la page ***************************************************************/
 function Change_target_instance ( instance )
  { Set_target_instance ( $('#idTargetInstance').val() ); }

/********************************************* Reload Process *****************************************************************/
 function Process_reload ( instance, thread, hard )
  { var json_request = JSON.stringify(
       { instance: instance,
         thread  : thread,
         hard    : (hard === "true" ? true : false),
       }
     );
    Send_to_API ( "POST", "/api/process/reload", json_request, null, null);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
