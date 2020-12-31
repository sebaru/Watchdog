 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    Send_to_API ( "GET", "/api/syn/show", (vars[1] === "" ? null : "page="+vars[1]), function(Response)
     { console.log(Response);
     }, null );

  }
