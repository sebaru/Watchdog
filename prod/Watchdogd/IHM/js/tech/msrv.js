 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function MSRV_Sauver_parametre ( )
  { var json_request = JSON.stringify(
     { instance  : $('#idTargetInstance').val(),
       thread    : "MSRV",
       parametres: { instance_is_master: $('#idMSRVIsMaster').val(),
                     master_host:        $('#idMSRVMasterHost').val(),
                     description:        $('#idMSRVDescription').val(),
                   }
     });
    $('#idModalInfoDetail').html("<strong>Reboot en cours</strong><br>Attendez 10 secondes avant de vous reconnecter." );
    $('#idModalInfo').modal("show");
    Send_to_API ( 'POST', "/api/config/set", json_request, function ()
     { Process_reload ( $('#idTargetInstance').val(), "MSRV", false );
     }, null );
  }
/************************ Affichage des données relatives a l'instance donnée *************************************************/
 function MSRV_Load_config ()
  { if ($('#idTargetInstance').val()==null)
         target = "MASTER";
    else target = $('#idTargetInstance').val();
    Send_to_API ( "GET", "/api/config/get?instance="+target+"&thread=MSRV", null, function(Response)
     { var parametre={};
       Response.configs.map ( function (item) { parametre[item.nom] = item.valeur; console.log("test"+item.nom+" "+item.valeur); } );
       $('#idMSRVIsMaster').val( parametre.instance_is_master );
       $('#idMSRVMasterHost').val( parametre.master_host );
       $('#idMSRVDescription').val( parametre.description );
       $('#idMSRVLogLevel').val( parametre.log_level );
       $('#idMSRVLogDB').val( parametre.log_db );
       $('#idMSRVLogZMQ').val( parametre.log_zmq );
     }, null);
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function MSRV_Set_Log_Level ( )
  { var json_request = JSON.stringify(
     { instance : $('#idTargetInstance').val(),
       log_level: $("#idMSRVLogLevel").val(),
       log_db   : $("#idMSRVLogDB").val(),
       log_zmq  : $("#idMSRVLogZMQ").val()
     } );
    Send_to_API ( 'POST', "/api/instance/loglevel", json_request, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTargetInstance').empty();
    if (localStorage.getItem("instance_is_master")=="true")
     { Send_to_API ( "GET", "/api/instance/list", null, function (Response)
        { $('#idTargetInstance').append("<option value='MASTER'>MASTER</option>");

          $.each ( Response.instances, function ( i, instance )
           { $('#idTargetInstance').append("<option value='"+instance.instance_id+"'>"+instance.instance_id+"</option>"); } );
          $('#idTargetInstance').val("MASTER");
        }, null);
     }
    else
     { $('#idTargetInstance').append("<option value='LOCAL'>LOCAL</option>"); }

    MSRV_Load_config();
  }
