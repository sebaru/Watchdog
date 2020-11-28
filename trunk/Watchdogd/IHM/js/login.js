
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Send_credential ()
  { var json_request = JSON.stringify(
     { username : document.getElementById("username").value,
       password : document.getElementById("password").value
     });

    Send_to_API ( 'POST', "/api/connect", json_request, function (Response)
     { localStorage.setItem("username", Response.username );
       localStorage.setItem("access_level", Response.access_level );
       localStorage.setItem("instance", Response.instance );
       localStorage.setItem("instance_is_master", Response.instance_is_master );
       document.cookie = "wtd_session="+Response.wtd_session+"; path=/";
       console.log
       if (Response.access_level>=6) window.location.replace("/tech/dashboard");
       else window.location.replace("/home/index");
     });
  }
