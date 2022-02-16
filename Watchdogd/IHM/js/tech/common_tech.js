 document.addEventListener('DOMContentLoaded', Load_common_tech, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_common_tech ()
  { if (localStorage.getItem("instance_is_master")=="true")
     { $('#idAlertNotMaster').hide(); }
    else
     { $('#idAlertNotMaster').show(); }
  }
/********************************************* Reload Process *****************************************************************/
 function Process_reload ( uuid )
  { var json_request = { uuid : uuid };
    Send_to_API ( "POST", "/api/process/reload", JSON.stringify(json_request), null, null);
  }
/********************************************* Renvoi un Select d'archivage ***************************************************/
 function Bouton_Archivage ( id, fonction, selected )
  { return("<select id='"+id+"' class='custom-select'"+
           "onchange="+fonction+">"+
           "<option value='0' "+(selected==0 ? "selected" : "")+">Aucun</option>"+
           "<option value='1' "+(selected==1 ? "selected" : "")+">Un pour 5 secondes</option>"+
           "<option value='2' "+(selected==2 ? "selected" : "")+">Un par minute</option>"+
           "<option value='3' "+(selected==3 ? "selected" : "")+">Un par heure</option>"+
           "<option value='4' "+(selected==4 ? "selected" : "")+">Un par jour</option>"+
           "</select>"
          );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function COMMON_Map ( thread_tech_id, thread_acronyme, tech_id, acronyme )
  { var json_request =
     { thread_tech_id : thread_tech_id,
       thread_acronyme: thread_acronyme,
       tech_id        : tech_id.toUpperCase(),
       acronyme       : acronyme.toUpperCase(),
     };
    Send_to_API ( 'POST', "/api/map", JSON.stringify(json_request) );
  }
/************************************ Controle de saisie avant envoi **********************************************************/
 function Controle_tech_id ( id_modal, tech_id_initial )
  { FormatPage = RegExp(/^[a-zA-Z0-9_\.]+$/);
    input = $('#'+id_modal+'TechID');

    if ( FormatPage.test(input.val())==false )
     { input.addClass("bg-danger");
       $('#'+id_modal+'Valider').attr("disabled", true);
       Popover_show ( input, 'Caractères autorisés', 'lettres, chiffres, _ et .' );
     }
    else
     { Send_to_API ( "GET", "/api/mnemos/tech_id", null, function(Response)
        { tech_id = input.val().toUpperCase();
          if ( Response.tech_ids.map ( function (item) { return(item.tech_id); } ).includes(tech_id) &&
              (tech_id_initial == null || tech_id_initial != tech_id) )
           { input.addClass("bg-danger");
             $('#'+id_modal+'Valider').attr("disabled", true);
             Popover_show ( input, 'Erreur !', 'Ce nom est déjà pris' );
           }
          else
           { input.removeClass("bg-danger");
             $('#'+id_modal+'Valider').attr("disabled", false);
             Popover_hide(input);
           }
        });
     }
  }
/********************************************* Controle du saisie du modal ****************************************************/
 function Common_Updater_Choix_Acronyme ( ids, classe, def_acronyme )
  {
    if ($('#'+ids+'SelectTechID').val()==null)
     { $('#'+ids+'SelectAcronyme').empty();
       return;
     }

    var json_request =
     { tech_id    : $('#'+ids+'SelectTechID').val().toUpperCase(),
       acronyme   : '',
       classe     : classe,
     };

    Send_to_API ( "PUT", "/api/mnemos/validate", JSON.stringify(json_request), function (Response)
     { $('#'+ids+'SelectAcronyme').empty();
       $.each ( Response.acronymes_found, function ( i, item )
        { $('#'+ids+'SelectAcronyme').append("<option value='"+item.acronyme+"'>"+item.acronyme+" - "+htmlEncode(item.libelle)+"</option>"); } );
       if (def_acronyme != null) $('#'+ids+'SelectAcronyme').val( def_acronyme );
     }, null );
  }
/********************************************* Controle du saisie du modal ****************************************************/
 function Common_Updater_Choix_TechID ( ids, classe, def_tech_id, def_acronyme )
  { var json_request = { acronyme: '', classe: classe };
    if (def_tech_id != null) { $('#'+ids+'RechercherTechID').val( '' ); }
    json_request.tech_id = $('#'+ids+'RechercherTechID').val();

    $('#'+ids+'SelectTechID').off("change").on("change", function () { Common_Updater_Choix_Acronyme ( ids, classe, def_acronyme ); } );

    Send_to_API ( "PUT", "/api/mnemos/validate", JSON.stringify(json_request), function (Response)
     { $('#'+ids+'SelectTechID').empty();
       $.each ( Response.tech_ids_found, function ( i, item )
        { $('#'+ids+'SelectTechID').append("<option value='"+item.tech_id+"'>"+item.tech_id+" - "+htmlEncode(item.name)+"</option>"); } );

       if (def_tech_id != null) $('#'+ids+'SelectTechID').val( def_tech_id );
       Common_Updater_Choix_Acronyme(ids, classe, def_acronyme );

       if ($('#'+ids+'SelectTechID').val() !== null)
        { $('#'+ids+'RechercherTechID').removeClass("border-warning");
          $('#'+ids+'Valider').prop("disabled", false);
        }
       else
        { $('#'+ids+'RechercherTechID').addClass("border-warning");
          $('#'+ids+'Valider').prop("disabled", true);
        }
     }, null );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
