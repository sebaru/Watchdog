 document.addEventListener('DOMContentLoaded', Load_page, false);

/******************************************************************************************************************************/
 function User_sauver_parametre (  )
  { var json_request = JSON.stringify(
       { username    : localStorage.getItem("username"),
         email       : $('#idUserEmail').val(),
         phone       : $('#idUserPhone').val(),
         xmpp        : $('#idUserXmpp').val(),
         comment     : $('#idUserComment').val(),
         notification: $('#idUserNotification').prop('checked'),
       }
     );

    Send_to_API ( 'POST', "/api/users/set", json_request, function ()
     {
     }, null);
  }
/******************************************************************************************************************************/
 function Users_Show_add_user ()
  { $('#idModalUserAdd').modal("show"); }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { var json_request = JSON.stringify( { username : localStorage.getItem("username") } );
    $('#idUserUsername').val(localStorage.getItem("username"));
    Send_to_API ( "PUT", "/api/users/get",	json_request, function(Response)
     { /*$('#idUserLevel').val(Response.access_level);*/
       $('#idUserEmail').val(Response.email);
       $('#idUserPhone').val(Response.phone);
       $('#idUserXMPP').val(Response.xmpp);
       $('#idUserComment').val(Response.comment);
       $('#idUserNotification').prop('checked', Response.notification);
     }, null );
  }
