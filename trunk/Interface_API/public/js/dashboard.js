 document.addEventListener('DOMContentLoaded', Load_dashboard, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_dashboard ()
  { console.log ("in load dashboard !");

    var xhr = new XMLHttpRequest;
    xhr.open('GET', "/api/status", true);
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status != 200)
        { document.getElementById("idModalDetail").innerHTML = "Une erreur inconnue est survenue.";
          $('#idModalError').modal("show");
        }
       var Response = JSON.parse(xhr.responseText);
       console.debug(Response);
     };
    xhr.send();

  }
