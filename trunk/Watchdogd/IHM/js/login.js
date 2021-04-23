
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Send_credential ()
  { var appareil = localStorage.getItem ( "appareil" );
    if (appareil == null) appareil = "New Device";

    var json_request = JSON.stringify(
     { username : document.getElementById("username").value,
       appareil : appareil,
       password : document.getElementById("password").value,
       useragent : window.navigator.userAgent
     });

    Send_to_API ( 'POST', "/api/connect", json_request, function (Response)
     { localStorage.setItem("username",           Response.username );
       localStorage.setItem("appareil",           appareil );
       localStorage.setItem("access_level",       Response.access_level );
       localStorage.setItem("instance",           Response.instance );
       localStorage.setItem("instance_is_master", Response.instance_is_master );
       localStorage.setItem("wtd_session",        Response.wtd_session );
       window.location.replace("/");
     });
  }
