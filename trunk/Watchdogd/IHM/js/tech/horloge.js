 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Créé un nouveau horloge *****************************************************************/
 function Horloge_Set ( id )
  { table = $('#idTableHorloge').DataTable();
    selection = table.ajax.json().horloges.filter( function(item) { return item.id==id } )[0];
    var json_request = JSON.stringify(
       { id: id,
         access_level: $('#idHorlogeLevel_'+id).val(),
       }
     );
    Send_to_API ( "POST", "/api/horloge/set", json_request, function (Response)
     { $('#idTableHorloge').DataTable().ajax.reload(null, false);
     }, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableHorloge').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/horloge/list",	type : "GET", dataSrc: "horloges",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
          [ { "data": null, "title":"Level", "className": "align-middle ",
              "render": function (item)
                { return( Select_Access_level ( "idHorlogeLevel_"+item.id, "Horloge_Set('"+item.id+"')", item.access_level )
                        );
                }
            },
            { "data": null, "title":"TechID", "className": "align-middle ",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
            },
            { "data": null, "title":"Acronyme", "className": "align-middle",
              "render": function (item)
                { return( Lien ( "/home/horloge/"+item.id, "Editer les ticks", item.acronyme ) ); }
            },
            { "data": null, "title":"Libellé", "className": "align-middle",
              "render": function (item)
                { return( Lien ( "/home/horloge/"+item.id, "Editer les ticks", item.libelle ) ); }
            },
            { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "primary", "Editer les ticks", "Redirect", "/home/horloge/"+item.id, "pen", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            }
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );
  }
