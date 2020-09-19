 document.addEventListener('DOMContentLoaded', Load_log, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_log ()
  { console.log ("in load log !");
    $('#idTableLog').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/log/get",	type : "GET", dataSrc: "logs" },
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
