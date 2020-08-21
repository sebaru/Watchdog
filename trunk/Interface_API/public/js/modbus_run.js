 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Send_to_API ( "GET", "/api/process/modbus/run/thread", null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
     });
    $('#idTableModbusRun').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id",
         ajax: {	url : "/api/process/modbus/run/modules",	type : "GET", dataSrc: "modules",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "tech_id", "title":"TechID", "className": "align-middle text-center" },
            { "data": "mode", "title":"Mode", "className": "align-middle text-center" },
            { "data": null, "title":"started", "className": "align-middle text-center",
              "render": function (item)
                { if (item.thread_is_running==false)
                   { return( Bouton ( "outline-warning", "Le thread ne tourne pas", null, null, "Thread OFF" ) ); }
                  else if (item.started==false)
                   { return( Bouton ( "outline-secondary", "Le module est désactivé", null, null, "Non" ) ); }
                  else { return( Bouton ( "success", "Le module est activé", null, null, "Oui" ) ); }
                }
            },
            { "data": "started", "title":"Mode", "className": "align-middle text-center" },
            { "data": "nbr_entree_tor", "title":"nbr_entree_tor", "className": "align-middle text-center hidden-xs" },
            { "data": "nbr_entree_ana", "title":"nbr_entree_ana", "className": "align-middle text-center hidden-xs" },
            { "data": "nbr_sortie_tor", "title":"nbr_sortie_tor", "className": "align-middle text-center hidden-xs" },
            { "data": "nbr_sortie_ana", "title":"nbr_sortie_ana", "className": "align-middle text-center hidden-xs" },

          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

  }
