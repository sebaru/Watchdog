 document.addEventListener('DOMContentLoaded', Load_log, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_log ()
  { $('#idTableLog').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/log/get",	type : "GET", dataSrc: "logs",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns: [ { "data": "date", "title":"Date" },
                    { "data": "access_level", "title": "Level" },
                    { "data": "username", "title": "Username" },
                    { "data": "message", "title":"Message" }
                  ],
         order: [ [0, "desc"] ],
         responsive: true,
       }
     );
  }
