 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Load_page ( )
  { $('#idMainContainer').fadeIn("slow");
    $('#appareil').focus();
    $('#appareil').on("change", function () { $('#username').focus(); } );
    $('#username').on("change", function () { $('#password').focus(); } );
    $('#password').keypress( function(event)
     { var keycode = (event.keyCode ? event.keyCode : event.which);
       if(keycode == '13') { Send_credential(); }
     });
  }
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Send_credential ()
  { var appareil = localStorage.getItem ( "appareil" );
    if (appareil == null) appareil = "New Device";

    var json_request = JSON.stringify(
     { username : $('#username').val(),
       appareil : $('#appareil').val(),
       password : $('#password').val(),
       useragent : window.navigator.userAgent
     });

    Send_to_API ( 'POST', "/api/connect", json_request, function (Response)
     { localStorage.setItem("username",           Response.username );
       localStorage.setItem("appareil",           Response.appareil );
       localStorage.setItem("access_level",       Response.access_level );
       localStorage.setItem("instance",           Response.instance );
       localStorage.setItem("instance_is_master", Response.instance_is_master );
       localStorage.setItem("wtd_session",        Response.wtd_session );
       window.location.replace("/");
     });
  }
