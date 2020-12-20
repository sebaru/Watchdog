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
     { $('#idAlertNotMaster').show();
       if (localStorage.getItem("instance") === null) { localStorage.setItem("instance", "LOCAL" ); }
       localStorage.setItem ( "TargetInstance", localStorage.getItem("instance") );
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
/**************************** Appeller par le navigateur lors du changement d'instance dans le select *************************/
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
/********************************************* Renvoi un Select d'archivage ***************************************************/
 function Bouton_Archivage ( id, fonction, selected )
  { return("<select id='"+id+"' class='custom-select'"+
           "onchange="+fonction+">"+
           "<option value='0' "+(selected==0 ? "selected" : "")+">Aucun</option>"+
           "<option value='1' "+(selected==1 ? "selected" : "")+">En temps reél</option>"+
           "<option value='2' "+(selected==2 ? "selected" : "")+">Un par minute</option>"+
           "<option value='3' "+(selected==3 ? "selected" : "")+">Un par heure</option>"+
           "<option value='4' "+(selected==4 ? "selected" : "")+">Un par jour</option>"+
           "</select>"
          );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
