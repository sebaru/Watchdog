 document.addEventListener('DOMContentLoaded', Load_page, false);
 var Instances;


 function Go_to_dls_source ()
  { vars = window.location.pathname.split('/');
    Redirect ( "/tech/dls_source/"+vars[3] );
  }

 function Go_to_dls_run ()
  { vars = window.location.pathname.split('/');
    Redirect ( "/tech/dls_run/"+vars[3] );
  }

/******************************************************************************************************************************/
 function Mnemos_AI_set_archivage ( acronyme )
  { table = $('#idTableEntreeAna').DataTable();
    selection = table.ajax.json().AI.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "AI",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         archivage: parseInt($('#idAIArchivage'+acronyme).val())
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, function ()
     { $('#idTableEntreeAna').DataTable().ajax.reload(null, false);
     });
  }
/******************************************************************************************************************************/
 function Mnemos_CI_set_archivage ( acronyme )
  { table = $('#idTableCptImp').DataTable();
    selection = table.ajax.json().CI.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "CI",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         archivage: parseInt($('#idCIArchivage'+acronyme).val())
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, function ()
     { $('#idTableCptImp').DataTable().ajax.reload(null, false);
     });
  }
/******************************************************************************************************************************/
 function Mnemos_R_set_archivage ( acronyme )
  { table = $('#idTableRegistre').DataTable();
    selection = table.ajax.json().R.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "R",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         archivage: parseInt($('#idRArchivage'+acronyme).val())
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, function ()
     { $('#idTableRegistre').DataTable().ajax.reload(null, false);
     });
  }
/******************************************************************************************************************************/
 function Mnemos_MSG_set ( acronyme )
  { table = $('#idTableMessage').DataTable();
    selection = table.ajax.json().MSG.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "MSG",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         rate_limit : parseInt($('#idMSGRateLimit'+acronyme).val()),
         sms        : parseInt($('#idMSGSms'+acronyme).val()),
         audio_profil : $('#idMSGProfilAudio'+acronyme).val(),
         audio_libelle: $('#idMSGLibelleAudio'+acronyme).val(),
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, function ()
     { $('#idTableMessage').DataTable().ajax.reload(null, false);
     });
  }
/******************************************************************************************************************************/
 function Mnemos_HORLOGE_set ( acronyme )
  { table = $('#idTableHorloge').DataTable();
    selection = table.ajax.json().HORLOGE.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "HORLOGE",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         access_level : parseInt($('#idHORLOGELevel_'+acronyme).val()),
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, null, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    console.log ("in load page !");
    $('#idTitle').html(vars[3]);
    var tech_id = vars[3];

    $('#idTableEntreeTor').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "DI", "tech_id": tech_id }, dataSrc: "DI",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "mnemo_DI_id",
         columns:
           [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
             { "data": null, "title":"Libellé",    "className": "align-middle ",
               "render": function (item)
                 { return(htmlEncode(item.libelle)); }
             },
           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableEntreeAna').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "AI", "tech_id": tech_id }, dataSrc: "AI",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "mnemo_AI_id",
         columns:
           [ { "data": null, "title":"Acronyme", "className": "align-middle text-center",
               "render": function (item)
                 { return( Lien ( "/tech/courbe/"+item.tech_id+"/"+item.acronyme+"/HOUR", "Voir la courbe", item.acronyme ) ); }
             },
             { "data": null, "title":"Libellé",    "className": "align-middle ",
               "render": function (item)
                 { return(htmlEncode(item.libelle)); }
             },
             { "data": "unite",   "title":"Unité",   "className": "align-middle " },
             { "data": "type",   "title":"Type",   "className": "align-middle " },
             { "data": "min", "title":"min", "className": "align-middle " },
             { "data": "max", "title":"max", "className": "align-middle " },
             { "data": "map_thread", "title":"map_thread", "className": "align-middle " },
             { "data": "map_tech_id","title":"map_tech_id","className": "align-middle " },
             { "data": "map_tag",    "title":"map_tag",   "className": "align-middle " },
             { "data": null, "title":"Archivage", "className": "",
               "render": function (item)
                 { return(Bouton_Archivage ( "idAIArchivage"+item.acronyme, "Mnemos_AI_set_archivage('"+item.acronyme+"')", item.archivage )); }
             },
           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableSortieTor').DataTable(
       { pageLength : 50,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "DO", "tech_id": tech_id }, dataSrc: "DO",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },

         rowId: "mnemo_DO_id",
         columns:
           [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
             { "data": null, "title":"Libellé",    "className": "align-middle ",
               "render": function (item)
                 { return(htmlEncode(item.libelle)); }
             },
           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableSortieAna').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "AO", "tech_id": tech_id }, dataSrc: "AO",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns: [ { "data": "tech_id",  "title":"TechId",     "className": "align-middle text-center " },
                    { "data": "acronyme", "title":"Acronyme",   "className": "align-middle text-center" },
                    { "data": "libelle",  "title":"Libellé",    "className": "align-middle " },
                    { "data": "type",     "title":"Type",   "className": "align-middle " },
                    { "data": "min",      "title":"min", "className": "align-middle " },
                    { "data": "max",      "title":"max", "className": "align-middle " },
                    { "data": "map_thread", "title":"map_thread", "className": "align-middle " },
                    { "data": "map_tech_id","title":"map_tech_id","className": "align-middle " },
                    { "data": "map_tag",    "title":"map_tag",    "className": "align-middle " },
                  ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableTempo').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "TEMPO", "tech_id": tech_id }, dataSrc: "TEMPO",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
           [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
             { "data": null, "title":"Libellé",    "className": "align-middle ",
               "render": function (item)
                 { return(htmlEncode(item.libelle)); }
             },
           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableRegistre').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "R", "tech_id": tech_id }, dataSrc: "R",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
           [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
             { "data": null, "title":"Libellé",    "className": "align-middle ",
               "render": function (item)
                 { return(htmlEncode(item.libelle)); }
             },
             { "data": "unite",      "title":"Unité",    "className": "align-middle idden-xs" },
             { "data": null, "title":"Archivage", "className": "",
               "render": function (item)
                 { return(Bouton_Archivage ( "idRArchivage"+item.acronyme, "Mnemos_R_set_archivage('"+item.acronyme+"')", item.archivage )); }
             },
             { "data": "map_question_vocale",   "title":"Question Vocale",   "className": "align-middle " },
             { "data": "map_reponse_vocale",    "title":"Reponse Vocale",    "className": "align-middle " },
           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableCptImp').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "CI", "tech_id": tech_id }, dataSrc: "CI",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
           [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
             { "data": null, "title":"Libellé",    "className": "align-middle ",
               "render": function (item)
                 { return(htmlEncode(item.libelle)); }
             },
             { "data": null, "title":"Etat", "className": "align-middle text-center",
               "render": function (item)
                 { if (item.etat==true) { return( Bouton ( "success", "Activé", null, null, "Actif" ) );        }
                                   else { return( Bouton ( "outline-secondary", "Désactivé", null, null, "Inactif" ) ); }
                 },
             },
             { "data": "multi",      "title":"Multi.",   "className": "align-middle text-center " },
             { "data": "unite",      "title":"Unité",    "className": "align-middle text-center " },
             { "data": null, "title":"Archivage", "className": "",
               "render": function (item)
                 { return(Bouton_Archivage ( "idCIArchivage"+item.acronyme, "Mnemos_CI_set_archivage('"+item.acronyme+"')", item.archivage )); }
             },
           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableCptH').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "CH", "tech_id": tech_id }, dataSrc: "CH",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
           [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
             { "data": null, "title":"Libellé",    "className": "align-middle ",
               "render": function (item)
                 { return(htmlEncode(item.libelle)); }
             },
             { "data": null, "title":"Etat", "className": "",
               "render": function (item)
                 { if (item.etat==true) { return( Bouton ( "success", "Activé", null, null, "Actif" ) );        }
                                   else { return( Bouton ( "outline-secondary", "Désactivé", null, null, "Inactif" ) ); }
                 },
             },
             { "data": "valeur", "title":"Valeur", "className": "align-middle text-center" },
           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableMessage').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "MSG", "tech_id": tech_id }, dataSrc: "MSG",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "msg_id",
         columns:
           [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
             { "data": null, "title":"Libellé",    "className": "align-middle ",
               "render": function (item)
                 { return(htmlEncode(item.libelle)); }
             },
             { "data": null, "title":"Rate Limit", "className": "align-middle ",
               "render": function (item)
                 { array = [ { valeur: 0, texte: "Pas de limite", },
                             { valeur: 1, texte: "1 par minute", },
                             { valeur: 5, texte: "1 pour 5 minutes", },
                             { valeur: 30, texte: "1 pour 30 minutes", },
                             { valeur: 60, texte: "1 pour 60 minutes", }
                           ];
                   return( Select ( "idMSGRateLimit"+item.acronyme, "Mnemos_MSG_set('"+item.acronyme+"')", array, item.rate_limit ) );
                 }
             },
             { "data": null, "title":"Profil AUDIO", "className": "align-middle ",
               "render": function (item)
                 { return("<input id='idMSGProfilAudio"+item.acronyme+"' class='form-control' "+
                          "placeholder='Profil audio' "+
                          "onchange=Mnemos_MSG_set('"+item.acronyme+"') "+
                          "value='"+item.audio_profil+"'/>");
                 }
             },
             { "data": null, "title":"Libellé Audio", "className": "align-middle ",
               "render": function (item)
                 { return( Input ( "text", "idMSGLibelleAudio"+item.acronyme,
                                   "Mnemos_MSG_set('"+item.acronyme+"')",
                                   "Libellé audio du message ?",
                                   item.audio_libelle )
                         );
                 }
             },
             { "data": null, "title":"SMS", "className": "align-middle ",
               "render": function (item)
                 { array = [ { valeur: 0, texte: "Non", }, { valeur: 1, texte: "Oui", },
                             { valeur: 2, texte: "GSM Only", }, { valeur: 3, texte: "OVH Only" } ];
                   return( Select ( "idMSGSms"+item.acronyme, "Mnemos_MSG_set('"+item.acronyme+"')", array, item.sms_notification ) );
                 }
             },
           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableWatchdog').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "WATCHDOG", "tech_id": tech_id }, dataSrc: "WATCHDOG",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
           [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
             { "data": null, "title":"Libellé",    "className": "align-middle ",
               "render": function (item)
                 { return(htmlEncode(item.libelle)); }
             },           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );


    $('#idTableHorloge').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "HORLOGE", "tech_id": tech_id }, dataSrc: "HORLOGE",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
          [ { "data": null, "title":"Level", "className": "align-middle ",
              "render": function (item)
                { return( Select_Access_level ( "idHORLOGELevel_"+item.acronyme, "Mnemos_HORLOGE_set('"+item.acronyme+"')", item.access_level )
                        );
                }
            },
            { "data": null, "title":"Acronyme", "className": "align-middle",
              "render": function (item)
                { return( Lien ( "/horloge/"+item.id, "Editer les ticks", item.acronyme ) ); }
            },
            { "data": null, "title":"Libellé", "className": "align-middle",
              "render": function (item)
                { return( Lien ( "/horloge/"+item.id, "Editer les ticks", item.libelle ) ); }
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

    $('#idTabEntreeTor').tab('show');
  }
