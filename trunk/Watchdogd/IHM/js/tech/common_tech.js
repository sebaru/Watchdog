 document.addEventListener('DOMContentLoaded', Load_common_tech, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_common_tech ()
  { Send_to_API ( "GET", "/api/instance/list", null, function (Response)
     { $('#idSelectedInstance').empty();
       $('#idSelectedInstance').append("<option value='MASTER'>MASTER</option>");

       $.each ( Response.instances, function ( i, instance )
        { $('#idSelectedInstance').append("<option value='"+instance.instance_id+"'"+
                                          (instance.instance_id == Get_locale_instance() ? "selected" : "")+">"+
                                          instance.instance_id+"</option>"); } );
       $('#idTitleInstance').text(Get_locale_instance());
     });
  }
/********************************************* Redirige la page ***************************************************************/
 function Get_locale_instance ( )
  { instance = localStorage.getItem ( "Instance" );
    if (instance == null)
     { instance="MASTER"; localStorage.setItem ( "Instance", instance ); }
    return(instance);
  }

/********************************************* Redirige la page ***************************************************************/
 function Set_locale_instance ( instance )
  { localStorage.setItem ( "Instance", instance );
    window.location.reload();
  }
/********************************************* Redirige la page ***************************************************************/
 function Change_selected_instance ( instance )
  { Set_locale_instance ( $('#idSelectedInstance').val() ); }

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
