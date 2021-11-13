 document.addEventListener('DOMContentLoaded', Load_common_tech, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_common_tech ()
  { if (localStorage.getItem("instance_is_master")=="true")
     { $('#idAlertNotMaster').hide(); }
    else
     { $('#idAlertNotMaster').show(); }
  }

/********************************************* Reload Process *****************************************************************/
 function Process_reload ( instance, thread, hard )
  { var json_request =
       { thread  : thread,
         hard    : hard,
       };
    if(instance!=null) json_request.instance=instance;
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
