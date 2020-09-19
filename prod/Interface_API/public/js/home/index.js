 document.addEventListener('DOMContentLoaded', Load_portail, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_portail ()
  { level = localStorage.getItem("access_level");
         if (level === null) window.location.replace("/home/login");
    else if (level < 6) window.location.replace("/home/syn");
    else window.location.replace("/tech");
  }
