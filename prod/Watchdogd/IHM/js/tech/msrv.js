 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function MSRV_Sauver_parametre ( )
  { var json_request = JSON.stringify(
     { instance:           Get_target_instance(),
       thread:             "MSRV",
       parametres: { instance_is_master: $('#idMSRVIsMaster').val(),
                     master_host:        $('#idMSRVMasterHost').val(),
                     description:        $('#idMSRVDescription').val(),
                   }
     });
    $('#idModalInfoDetail').html("<strong>Reboot en cours</strong><br>Attendez 10 secondes avant de vous reconnecter." );
    $('#idModalInfo').modal("show");
    Send_to_API ( 'POST', "/api/config/set", json_request, function ()
     { Process_reload ( Get_target_instance(), "MSRV", false );
     }, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTitleInstance').val(Get_target_instance());
    Send_to_API ( "GET", "/api/config/get?instance="+Get_target_instance()+"&thread=MSRV", null, function(Response)
     { var parametre={};
       Response.configs.map ( function (item) { parametre[item.nom] = item.valeur; console.log("test"+item.nom+" "+item.valeur); } );
       $('#idMSRVIsMaster').val( parametre.instance_is_master );
       $('#idMSRVMasterHost').val( parametre.master_host );
       $('#idMSRVDescription').val( parametre.description );
     }, null);
  }
