 document.addEventListener('DOMContentLoaded', Load_portail, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_portail ()
  { level = localStorage.getItem("access_level");
    if (level === null)                                                                                 /* Si pas authentifié */
     { Send_to_API ( 'GET', "/api/ping", null, function (Response)
        { if (!Response)              { window.location.replace("/home/error"); }
          else if(Response.installed) { window.location.replace("/home/login"); }
          else                        { window.location.replace("/install");    }
        });
     }
    else if (level < 6) window.location.replace("/home/syn");                                /* Si authentifié en User normal */
    else                window.location.replace("/tech");                                     /* Si authentifié en Technicien */
  }
