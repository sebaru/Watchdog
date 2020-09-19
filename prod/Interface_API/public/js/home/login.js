
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Send_credential ()
  { var json_request = JSON.stringify(
     { username : document.getElementById("username").value,
       password : document.getElementById("password").value
     });

    Send_to_API ( 'POST', "/api/connect", json_request, function (Response)
     { localStorage.setItem("username", Response.username );
       localStorage.setItem("access_level", Response.access_level );
       document.cookie = "wtd_session="+Response.wtd_session+"; path=/";
       window.location.replace("/home/index");
     });
  }
