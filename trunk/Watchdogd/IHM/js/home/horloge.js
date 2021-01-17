 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Créé un nouveau tableau *****************************************************************/
 function Horloge_ticks_add (  )
  { vars = window.location.pathname.split('/');
    if ( vars[3] == undefined ) return; else target = vars[3];

    var json_request = JSON.stringify(
       { horloge_id: target,
         lundi   : false,
         mardi   : false,
         mercredi: false,
         jeudi   : false,
         vendredi: false,
         samedi  : false,
         dimanche: false,
         heure   : 12,
         minute  : 0
       }
     );
    Send_to_API ( "POST", "/api/horloge/ticks/set", json_request, function (Response)
     { $('#idTableHorloge').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Créé un nouveau tableau *****************************************************************/
 function Horloge_ticks_del (  )
  { vars = window.location.pathname.split('/');
    if ( vars[3] == undefined ) return; else target = vars[3];

    var json_request = JSON.stringify(
       { id: target }
     );
    Send_to_API ( "DELETE", "/api/horloge/ticks/del", json_request, function (Response)
     { $('#idTableHorloge').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Créé un nouveau tableau *****************************************************************/
 function Horloge_ticks_Set ( id )
  { table = $('#idTableHorloge').DataTable();
    selection = table.ajax.json().horloge_ticks.filter( function(item) { return item.id==id } )[0];
    var json_request = JSON.stringify(
       { id: id,
         lundi   : ($('#idHorlogeLundi_'+id).val() == "1" ? true : false),
         mardi   : ($('#idHorlogeMardi_'+id).val() == "1" ? true : false),
         mercredi: ($('#idHorlogeMercredi_'+id).val() == "1" ? true : false),
         jeudi   : ($('#idHorlogeJeudi_'+id).val() == "1" ? true : false),
         vendredi: ($('#idHorlogeVendredi_'+id).val() == "1" ? true : false),
         samedi  : ($('#idHorlogeSamedi_'+id).val() == "1" ? true : false),
         dimanche: ($('#idHorlogeDimanche_'+id).val() == "1" ? true : false),
         heure   : $('#idHorlogeHeure_'+id).val().split(':')[0],
         minute  : $('#idHorlogeHeure_'+id).val().split(':')[1],
       }
     );
    Send_to_API ( "POST", "/api/horloge/ticks/set", json_request, function (Response)
     { /*$('#idTableHorloge').DataTable().ajax.reload(null, false);*/
     }, null );
  }

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    if ( vars[3] == undefined ) return; else target = vars[3];

    Send_to_API ( "GET", "/api/horloge/get", "horloge_id="+target, function(Response)
     { $('#idHorlogeTitle').text( Response.libelle );
     }, null );

    array_jour = [ { valeur: 0, texte: "Non" }, { valeur: 1, texte: "Oui" } ];

    $('#idTableHorloge').DataTable(
       { pageLength : 25,
         fixedHeader: true, paging: false, searching: false, ordering: false,
         ajax: {	url : "/api/horloge/ticks/list",	type : "GET", dataSrc: "horloge_ticks", data: { "horloge_id": target },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
          [ { "data": null, "title":"Lundi", "className": "align-middle text-center",
              "render": function (item)
                { return( Select ( "idHorlogeLundi_"+item.id, "Horloge_ticks_Set('"+item.id+"')",
                                   array_jour, (item.lundi=="1" ? 1 : 0) ) );
                }
            },
            { "data": null, "title":"Mardi", "className": "align-middle text-center",
              "render": function (item)
                { return( Select ( "idHorlogeMardi_"+item.id, "Horloge_ticks_Set('"+item.id+"')",
                                   array_jour, (item.mardi=="1" ? 1 : 0) ) );
                }
            },
            { "data": null, "title":"Mercredi", "className": "align-middle text-center",
              "render": function (item)
                { return( Select ( "idHorlogeMercredi_"+item.id, "Horloge_ticks_Set('"+item.id+"')",
                                   array_jour, (item.mercredi=="1" ? 1 : 0) ) );
                }
            },
            { "data": null, "title":"Jeudi", "className": "align-middle text-center",
              "render": function (item)
                { return( Select ( "idHorlogeJeudi_"+item.id, "Horloge_ticks_Set('"+item.id+"')",
                                   array_jour, (item.jeudi=="1" ? 1 : 0) ) );
                }
            },
            { "data": null, "title":"Vendredi", "className": "align-middle text-center",
              "render": function (item)
                { return( Select ( "idHorlogeVendredi_"+item.id, "Horloge_ticks_Set('"+item.id+"')",
                                   array_jour, (item.vendredi=="1" ? 1 : 0) ) );
                }
            },
            { "data": null, "title":"Samedi", "className": "align-middle text-center",
              "render": function (item)
                { return( Select ( "idHorlogeSamedi_"+item.id, "Horloge_ticks_Set('"+item.id+"')",
                                   array_jour, (item.samedi=="1" ? 1 : 0) ) );
                }
            },
            { "data": null, "title":"Dimanche", "className": "align-middle text-center",
              "render": function (item)
                { return( Select ( "idHorlogeDimanche_"+item.id, "Horloge_ticks_Set('"+item.id+"')",
                                   array_jour, item.dimanche ) );
                }
            },
            { "data": null, "title":"Heure", "className": "align-middle text-center",
              "render": function (item)
                { return( Input ( "time", "idHorlogeHeure_"+item.id, "Horloge_ticks_Set('"+item.id+"')",
                                  "Selectionnez l'heure", item.heure.padStart(2, "0")+":"+item.minute.padStart(2, "0") ) );
                }
            },
            { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "danger", "Effacer", "Horloge_ticks_del", item.id, "trash", null );
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
