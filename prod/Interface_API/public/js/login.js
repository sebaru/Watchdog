
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Send_credential ()
  { var data = new FormData();
    data.append('username', document.getElementById("username").value);
    data.append('password', document.getElementById("password").value);

    var xhr = new XMLHttpRequest;
    xhr.open('POST', "/auth/login", true);
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status != 200)
        { /*$('.toast').toast( { delay: 2000 } );
          $('.toast').toast('show');*/
          $('#id-error-detail').innerHtml = "Vos identifiants et mots de passe sont incorrects";
          $('#id-modal-error').modal("show");
        }
       var Response = JSON.parse(xhr.responseText);
       console.debug(Response);
       sessionStorage.setItem("username", Response.username );
       if (Response.access_level < 6) window.location.replace("/");
                                 else window.location.replace("/tech");
     };
    xhr.send(data);
  }
