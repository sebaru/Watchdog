
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Send_credential ()
  { var data = new FormData();
    data.append('username', document.getElementById("username").value);
    data.append('password', document.getElementById("password").value);

    var xhr = new XMLHttpRequest;
    xhr.open('POST', "/auth/login", true);
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { var Response = JSON.parse(xhr.responseText);
          console.debug(Response);
          localStorage.setItem("username", Response.username );
          localStorage.setItem("access_level", Response.access_level );
          document.cookie = "wtd_session="+Response.wtd_session+"; path=/";
          if (Response.access_level < 6) window.location.replace("/");
                                    else window.location.replace("/tech");
        }
       else if (xhr.status == 401)
        { Show_Error ( "Vos identifiants et mots de passe sont incorrects" ); }
       else
        { Show_Error ( xhr.statusText ); }
     };
    xhr.send(data);
  }
