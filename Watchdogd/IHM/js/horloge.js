/************************************ Créé un nouveau tableau *****************************************************************/
 function Horloge_ticks_add ( horloge_id )
  { var json_request = JSON.stringify(
       { horloge_id: horloge_id,
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
     { $('#idTableHorloge_'+horloge_id).DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Créé un nouveau tableau *****************************************************************/
 function Horloge_ticks_del ( ids )
  { split = ids.split(':');
    horloge_id = split[0];
    tick_id = split[1];
    var json_request = JSON.stringify( { id: tick_id } );
    Send_to_API ( "DELETE", "/api/horloge/ticks/del", json_request, function (Response)
     { $('#idTableHorloge_'+horloge_id).DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Créé un nouveau tableau *****************************************************************/
 function Horloge_ticks_Set ( ids )
  { split = ids.split(':');
    horloge_id = split[0];
    tick_id = split[1];
    console.log("horloge_id="+horloge_id);
    console.log("tick_id="+tick_id);
    table = $('#idTableHorloge_'+horloge_id).DataTable();
    selection = table.ajax.json().horloge_ticks.filter( function(item) { return item.id==tick_id } )[0];
    var json_request = JSON.stringify(
       { id: tick_id,
         lundi   : ($('#idHorlogeLundi_'+tick_id).val() == "1" ? true : false),
         mardi   : ($('#idHorlogeMardi_'+tick_id).val() == "1" ? true : false),
         mercredi: ($('#idHorlogeMercredi_'+tick_id).val() == "1" ? true : false),
         jeudi   : ($('#idHorlogeJeudi_'+tick_id).val() == "1" ? true : false),
         vendredi: ($('#idHorlogeVendredi_'+tick_id).val() == "1" ? true : false),
         samedi  : ($('#idHorlogeSamedi_'+tick_id).val() == "1" ? true : false),
         dimanche: ($('#idHorlogeDimanche_'+tick_id).val() == "1" ? true : false),
         heure   : $('#idHorlogeHeure_'+tick_id).val().split(':')[0],
         minute  : $('#idHorlogeHeure_'+tick_id).val().split(':')[1],
       }
     );
    Send_to_API ( "POST", "/api/horloge/ticks/set", json_request, function (Response)
     { $('#idTableHorloge_'+horloge_id).DataTable().ajax.reload(null, false);
     }, null );
  }

/********************************************* Appelé au chargement de la page ************************************************/
 function Creer_horloge ( Response )
  { var card = $('<div></div>').addClass("row bg-transparent")
               .append( $('<div></div>').addClass("col text-center")
                        .append( $('<img>').attr("src", "https://static.abls-habitat.fr/img/syn_horloge.svg" )
                                           .attr("onclick", "Charger_page_horloge('"+Response.tech_id+"')")
                                           .addClass("wtd-synoptique")
                               )
                      )
               .append( $('<div></div>').addClass('w-100') )
               .append( $('<div></div>').addClass("col text-center text-white")
                        .append( "<p>"+Response.dls_name+"</p>" )
                      );
    return(card);
  }
/********************************************* Appelé au chargement de l'horloge **********************************************/
 function Charger_une_horloge ( tech_id )
  { Send_to_API ( "GET", "/api/horloge/get", "tech_id="+tech_id, function(Response)
     { var bodymain = $("#bodymain");
       array_jour = [ { valeur: 0, texte: "Non" }, { valeur: 1, texte: "Oui" } ];
       $.each ( Response.horloges, function (i, horloge)
        { bodymain.append ( $("<h4></h4>" ).text( horloge.libelle ).addClass("text-white align-items-center") )
                  .append ( $("<table></table>").
                              attr("id","idTableHorloge_"+horloge.id).
                              addClass("table table-dark table-bordered"))
                  .append("<hr>");

          $("#idTableHorloge_"+horloge.id).DataTable(
             { pageLength : 25,
               fixedHeader: true, paging: false, searching: false, ordering: false,
               ajax: {	url : "/api/horloge/ticks/list",	type : "GET", dataSrc: "horloge_ticks", data: { "horloge_id": horloge.id },
                       error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
                     },
               rowId: "id",
               columns:
                [ { "data": null, "title":"Lundi", "className": "align-middle text-center",
                    "render": function (item)
                      { return( Select ( "idHorlogeLundi_"+item.id, "Horloge_ticks_Set('"+item.horloge_id+":"+item.id+"')",
                                         array_jour, (item.lundi=="1" ? 1 : 0) ) );
                      }
                  },
                  { "data": null, "title":"Mardi", "className": "align-middle text-center",
                    "render": function (item)
                      { return( Select ( "idHorlogeMardi_"+item.id, "Horloge_ticks_Set('"+item.horloge_id+":"+item.id+"')",
                                         array_jour, (item.mardi=="1" ? 1 : 0) ) );
                      }
                  },
                  { "data": null, "title":"Mercredi", "className": "align-middle text-center",
                    "render": function (item)
                      { return( Select ( "idHorlogeMercredi_"+item.id, "Horloge_ticks_Set('"+item.horloge_id+":"+item.id+"')",
                                         array_jour, (item.mercredi=="1" ? 1 : 0) ) );
                      }
                  },
                  { "data": null, "title":"Jeudi", "className": "align-middle text-center",
                    "render": function (item)
                      { return( Select ( "idHorlogeJeudi_"+item.id, "Horloge_ticks_Set('"+item.horloge_id+":"+item.id+"')",
                                         array_jour, (item.jeudi=="1" ? 1 : 0) ) );
                      }
                  },
                  { "data": null, "title":"Vendredi", "className": "align-middle text-center",
                    "render": function (item)
                      { return( Select ( "idHorlogeVendredi_"+item.id, "Horloge_ticks_Set('"+item.horloge_id+":"+item.id+"')",
                                         array_jour, (item.vendredi=="1" ? 1 : 0) ) );
                      }
                  },
                  { "data": null, "title":"Samedi", "className": "align-middle text-center",
                    "render": function (item)
                      { return( Select ( "idHorlogeSamedi_"+item.id, "Horloge_ticks_Set('"+item.horloge_id+":"+item.id+"')",
                                         array_jour, (item.samedi=="1" ? 1 : 0) ) );
                      }
                  },
                  { "data": null, "title":"Dimanche", "className": "align-middle text-center",
                    "render": function (item)
                      { return( Select ( "idHorlogeDimanche_"+item.id, "Horloge_ticks_Set('"+item.horloge_id+":"+item.id+"')",
                                         array_jour, item.dimanche ) );
                      }
                  },
                  { "data": null, "title":"Heure", "className": "align-middle text-center",
                    "render": function (item)
                      { return( Input ( "time", "idHorlogeHeure_"+item.id, "Horloge_ticks_Set('"+item.horloge_id+":"+item.id+"')",
                                        "Selectionnez l'heure", item.heure.padStart(2, "0")+":"+item.minute.padStart(2, "0") ) );
                     }
                  },
                  { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
                    "render": function (item)
                      { boutons = Bouton_actions_start ();
                        boutons += Bouton_actions_add ( "danger", "Effacer", "Horloge_ticks_del", item.horloge_id+":"+item.id, "trash", null );
                        boutons += Bouton_actions_end ();
                        return(boutons);
                      },
                  }
                ],
               /*order: [ [0, "desc"] ],*/
               responsive: true,
             }
           );
        });
       Slide_down_when_loaded ( "toplevel" );
     }, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Charger_page_horloge ( dls_tech_id )
  { console.log("Charger_page_horloge " + dls_tech_id);
    Scroll_to_top();
    $('#toplevel').slideUp("normal", function ()
     { $('#toplevel').empty()
                     .append("<div id='bodymain'></div>").addClass('mx-1');
       Charger_une_horloge ( dls_tech_id );
     });
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
