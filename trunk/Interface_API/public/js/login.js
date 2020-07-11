
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
          sessionStorage.setItem("username", Response.username );
          document.cookie = "wtd_session="+Response.wtd_session+"; path=/";
          if (Response.access_level < 6) window.location.replace("/");
                                    else window.location.replace("/tech");
        }
       else if (xhr.status == 401)
        { $('#id-error-detail').innerHtml = "Vos identifiants et mots de passe sont incorrects";
          $('#id-modal-error').modal("show");
        }
       else if (xhr.status == 500)
        { $('#id-error-detail').innerHtml = "test";
          $('#id-modal-error').modal("show");
        }
       else
        { $('#id-error-detail').innerHtml = "Une erreur s'est produite...";
          $('#id-modal-error').modal("show");
        }
     };
    xhr.send(data);
  }
