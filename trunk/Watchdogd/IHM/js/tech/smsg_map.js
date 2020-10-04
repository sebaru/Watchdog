 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_GSM_Del ( map_tech_id, map_tag )
  { var json_request = JSON.stringify(
       { classe     : type,
         map_tech_id: map_tech_id,
         map_tag    : map_tag
       }
     );
    Send_to_API ( "DELETE", "/api/map/del", json_request, function(Response)
     { $('#idTableGSM').DataTable().ajax.reload(null, false);
     }, null );
  }

/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del ( type, selection )
  { $('#idModalDelTitre').text ( "Détruire le mapping ?" );
    $('#idModalDelMessage').html("Etes-vous sur de vouloir supprimer ce mapping ?"+
                                 "<hr>"+
                                 "<strong>"+selection.map_tech_id+":"+selection.map_tag +
                                 " <-> " + selection.tech_id + ":" + selection.acronyme + "</strong>" +
                                 "<br>" + selection.libelle
                                );
    $('#idModalDelValider').attr( "onclick",
                                  "Valider_GSM_Del('"+selection.map_tech_id+"','"+selection.map_tag+"')" );
    $('#idModalDel').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_DI ( id )
  { table = $('#idTableGSM').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "DI", selection )
  }

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Edit_DI ( )
  { if ($('#idModalEditDI #idModalEditValider').hasClass("disabled")) return;
    $('#idModalEditDI').modal("hide");
    var json_request = JSON.stringify(
       { classe     : 'DI',
         thread     : 'SMSG',
         tech_id    : $('#idModalEditDI #idModalEditTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditDI #idModalEditAcronyme').val().toUpperCase(),
         map_tech_id: $('#idModalEditDI #idModalEditGSMTechID').val().toUpperCase(),
         map_tag    : $('#idModalEditDI #idModalEditGSMTag').val(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTableGSM').DataTable().ajax.reload(null, false);
     }, null);
  }

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Modal_Edit_Input_Changed ( target )
  { const Ascii_charset = RegExp(/^[a-zA-Z0-9][a-zA-Z0-9_]*$/);

    if ($('#'+target+' #idModalEditTechID').val().length==0)
     { $('#'+target+' #idModalEditTechID').removeClass("bg-danger bg-warning").addClass("bg-warning");
       $('#'+target+' #idModalEditTechIDPropose').text("Aucun match");
       $('#'+target+' #idModalEditAcronyme').removeClass("bg-danger bg-warning").addClass("bg-warning");
       $('#'+target+' #idModalEditAcronyme').prop("disabled", true);
       $('#'+target+' #idModalEditAcronyme').val("");
       $('#'+target+' #idModalEditAcronymePropose').text("Aucun match");
       $('#'+target+' #idModalEditValider').addClass("disabled");
       return;
     }

    if (!Ascii_charset.test($('#'+target+' #idModalEditTechID').val()))
     { $('#'+target+' #idModalEditTechID').removeClass("bg-danger bg-warning").addClass("bg-danger");
       $('#'+target+' #idModalEditTechIDPropose').text("Caractères autorisés : A-Z, 0-9 et _ (sauf au début)");
       $('#'+target+' #idModalEditValider').addClass("disabled");
       return;
     }

    if ($('#'+target+' #idModalEditAcronyme').val().length && !Ascii_charset.test($('#'+target+' #idModalEditAcronyme').val()))
     { $('#'+target+' #idModalEditAcronyme').removeClass("bg-danger bg-warning").addClass("bg-danger");
       $('#'+target+' #idModalEditAcronymePropose').text("Caractères autorisés : A-Z, 0-9 et _ (sauf au début)");
       $('#'+target+' #idModalEditValider').addClass("disabled");
       return;
     }

    var json_request = JSON.stringify(
       { tech_id    : $('#'+target+' #idModalEditTechID').val().toUpperCase(),
         acronyme   : $('#'+target+' #idModalEditAcronyme').val().toUpperCase(),
         clase      : "DI",
       }
     );
    Send_to_API ( "PUT", "/api/mnemos/validate", json_request, function (Response)
     { var tech_id_found=false, tech_id_propose="";
       var acronyme_found=false, acronyme_propose="";

       for (var i = 0; i < Response.nbr_tech_ids_found; i++)
        { tech_id_propose=tech_id_propose + Response.tech_ids_found[i].tech_id+" ";
          if (Response.tech_ids_found[i].tech_id == $('#'+target+' #idModalEditTechID').val().toUpperCase()) tech_id_found=true;
        }
       if (tech_id_found==true)
        { $('#'+target+' #idModalEditTechID').removeClass("bg-danger bg-warning");
          $('#'+target+' #idModalEditTechIDPropose').text("Match !");
          $('#'+target+' #idModalEditAcronyme').prop("disabled", false);
        }
       else
        { $('#'+target+' #idModalEditTechID').removeClass("bg-danger bg-warning").addClass("bg-warning");
          $('#'+target+' #idModalEditTechIDPropose').text("Choix: " + tech_id_propose);
          $('#'+target+' #idModalEditAcronyme').prop("disabled", true);
          $('#'+target+' #idModalEditAcronyme').val("");
        }

       for (var i = 0; i < Response.acronymes_found.length; i++)
        { acronyme_propose=acronyme_propose + Response.acronymes_found[i].acronyme+" ";
          if (Response.acronymes_found[i].acronyme == $('#'+target+' #idModalEditAcronyme').val().toUpperCase()) acronyme_found=true;
        }
       if (acronyme_found==true)
        { $('#'+target+' #idModalEditAcronyme').removeClass("bg-danger bg-warning");
          $('#'+target+' #idModalEditAcronymePropose').text("Match !");
        }
       else
        { $('#'+target+' #idModalEditAcronyme').removeClass("bg-danger bg-warning").addClass("bg-warning");
          $('#'+target+' #idModalEditAcronymePropose').text("Choix: " + acronyme_propose);
        }
       if (tech_id_found==true && acronyme_found==true)
        { $('#'+target+' #idModalEditValider').removeClass("disabled"); }
       else
        { $('#'+target+' #idModalEditValider').addClass("disabled");    }
     });
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_DI ( id )
  { if (id>0)
     { table = $('#idTableGSM').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditDI #idModalEditTitre').text ( "Editer MAP GSM" );
       $('#idModalEditDI #idModalEditTechID').val ( selection.tech_id );
       $('#idModalEditDI #idModalEditAcronyme').val ( selection.acronyme );
       $('#idModalEditDI #idModalEditGSMTag').val     ( selection.map_tag );
       $('#idModalEditDI #idModalEditGSMTag').attr ( "readonly", false );
       $('#idModalEditDI #idModalEditValider').attr( "onclick", "Valider_Edit_DI()" );
       Send_to_API ( "GET", "/api/config/get?thread=SMSG&param=tech_id", null, function (Response)
        { $('#idModalEditGSMTechID').empty();
          $.each ( Response.configs, function ( i, config )
           { $('#idModalEditGSMTechID').append("<option value='"+config.valeur+"'"+
                                                (config.valeur == selection.tech_id ? "selected" : "")+">"+
                                                 config.valeur+"</option>"); } );
        });
     }
    else
     { $('#idModalEditDI #idModalEditTitre').text ( "Ajouter un mapping DI" );
       $('#idModalEditDI #idModalEditTechID').val ( '' );
       $('#idModalEditDI #idModalEditAcronyme').val ( '' );
       $('#idModalEditDI #idModalEditGSMTag').val     ( '' );
       $('#idModalEditDI #idModalEditGSMTag').attr ( "readonly", false );
       $('#idModalEditDI #idModalEditValider').attr( "onclick", "Valider_Edit_DI()" );
       Send_to_API ( "GET", "/api/config/get?thread=SMSG&param=tech_id", null, function (Response)
        { $('#idModalEditGSMTechID').empty();
          $.each ( Response.configs, function ( i, config )
           { $('#idModalEditGSMTechID').append("<option value='"+config.valeur+"'>"+config.valeur+"</option>"); } );
        });
     }
    Modal_Edit_Input_Changed('idModalEditDI');

    $('#idModalEditDI').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  {

    $('#idTableGSM').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id", paging: false,
         ajax: {	url : "/api/map/list",	type : "GET", dataSrc: "mappings", data: { "thread": "SMSG", "classe": "DI" },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "map_tech_id", "title":"GSM TechID", "className": "align-middle text-center" },
            { "data": "map_tag", "title":"Texte SMS", "className": "align-middle text-center" },
            { "data": null, "title":"Map", "className": "align-middle text-center",
              "render": function (item)
                { return( "->" ); }
            },
            { "data": null, "title":"BIT Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
            },
            { "data": "acronyme", "title":"BIT Acronyme", "className": "align-middle text-center" },
            { "data": "libelle", "title":"BIT Libelle", "className": "align-middle text-center" },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer cet objet", "Show_Modal_Map_Edit_DI", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer cet objet", "Show_Modal_Map_Del_DI", item.id, "trash", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            },
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

  }
