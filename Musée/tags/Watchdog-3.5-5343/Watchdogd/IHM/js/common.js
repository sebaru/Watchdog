 document.addEventListener('DOMContentLoaded', Load_common, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Send_to_API ( method, URL, parametre, fonction_ok, fonction_nok )
  { var xhr = new XMLHttpRequest;
    $(".ClassLoadingSpinner").show();

    if (method=="POST" || method=="PUT") { ContentType = 'application/json'; }
    else if (method=="POSTFILE") { ContentType = 'application/octet-stream'; method = "POST"; }
    else ContentType = null;

    if ( method == "GET" && parametre !== null )
     { xhr.open(method, URL+"?"+parametre, true); }
    else xhr.open(method, URL, true);

    if (ContentType != null) { xhr.setRequestHeader('Content-type', ContentType ); }

    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       $(".ClassLoadingSpinner").hide();
       if (xhr.status == 200)
        { try { var Response = JSON.parse(xhr.responseText); }
          catch (error) { Response=undefined; }
          if (method=="DELETE" || method=="POST") $('#idToastStatus').toast('show');
          if (fonction_ok != null) fonction_ok(Response);
        }
       else { Show_Error( xhr.statusText );
              try { var Response = JSON.parse(xhr.responseText); }
              catch (error) { Response=undefined; }
              if (fonction_nok != null) fonction_nok(Response);
            }
     }
    xhr.send(parametre);
  }
/************************************ Controle de saisie avant envoi **********************************************************/
 function isNum ( id )
  { FormatTag = RegExp(/^[0-9]+$/);
    input = $('#'+id);
    return ( FormatTag.test(input.val()) )
  }
/********************************************* Gestion des popovers ***********************************************************/
 function Popover_hide ( element )
  { element.popover('dispose');
  }
 function Popover_show ( element, titre, content, place )
  { Popover_hide ( element );
    element.popover({ container: 'body', title: titre, content: content});
    element.popover('show');
  }
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_common ()
  { if (document.getElementById("idUsername") !== null)
     { document.getElementById("idUsername").innerHTML = localStorage.getItem("username");
     }
   /* else
     { document.getElementById("idUsername").innerHTML = "Se connecter";
       document.getElementById("idHrefUsername").href = "/login";
     }
    /*$("body").tooltip({ selector: '[data-toggle=tooltip]' });*/
  }
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Logout ()
  { Send_to_API ( "PUT", "/api/disconnect", null, function() { localStorage.clear(); Redirect("/"); });
  }

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Show_Error ( message )
  { if (message == "Not Connected") { Logout(); }
    else
     { $('#idModalDetail').html( "Une erreur s'est produite: "+message );
       $('#idModalError').modal("show");
     }
  }

/********************************************* Redirige la page ***************************************************************/
 function Redirect ( url )
  { /*$('body').fadeOut("high", function () { */ window.location = url; /* } );*/
  }

/********************************************* Barre de boutons ***************************************************************/
 function Bouton_actions_start ( )
  { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"); }

 function Bouton_actions_add ( color, tooltip, clic_func, key, icone, texte )
  { return( "<button "+
            "class='btn btn-"+color+" btn-sm' "+
            "data-toggle='tooltip' title='"+htmlEncode(tooltip)+"' "+
            "onclick="+clic_func+"('"+key+"')>"+
            (icone!==null ? "<i class='fas fa-"+icone+"'></i> " : "") +
            (texte!==null ? texte : "") +
            "</button>");
  }
 function Bouton_actions_end ( )
  { return ("</div>"); }

/********************************************** Bouton unitaire ***************************************************************/
 function Bouton ( color, tooltip, clic_func, key, texte )
  { if (clic_func !== null)
     { result = "<button "+
                "class='btn btn-"+color+" btn-block btn-sm' "+
                "data-toggle='tooltip' title='"+tooltip+"' "+
                "onclick="+clic_func+"('"+key+"')>"+texte+
                "</button>";
     }
   else
    { result =  "<button "+
                "class='btn btn-"+color+" btn-block btn-sm' "+
                "data-toggle='tooltip' title='"+tooltip+"' "+
                ">"+texte+
                "</button>";
    }
   return( result );
  }

 function Lien ( target, tooltip, texte )
  { return( "<a href='"+target+"' data-toggle='tooltip' title='"+htmlEncode(tooltip)+"'>"+texte+"</a>" );
  }

 function Badge ( color, tooltip, texte )
  { return("<span "+
           "class='badge badge-"+color+"' "+
           "data-toggle='tooltip' title='"+tooltip+"'>"+texte+
           "</span>" );
  }

/*****************************************Peuple un selecten fonction d'un retour API *****************************************/
 function Select_from_api ( id, url, url_parameter, array_out, array_item, to_string, selected )
  { $('#'+id).empty();
    Send_to_API ( "GET", url, url_parameter, function(Response)
     { $.each ( Response[array_out], function ( i, item )
        { $('#'+id).append("<option value='"+item[array_item]+"'>"+to_string(item)+"</option>"); } );
       if (selected!=null) $('#'+id).val(selected);
     }, null );
  }
/********************************************* Renvoi un Badge d'access Level *************************************************/
 function Badge_Access_level ( level )
  { if (level == 1) return( Badge ( "info", "Accès de niveau 1", "1" ) );
    if (level == 2) return( Badge ( "info", "Accès de niveau 2", "2" ) );
    if (level == 3) return( Badge ( "info", "Accès de niveau 3", "3" ) );
    if (level == 4) return( Badge ( "secondary", "Accès de niveau 4", "4" ) );
    if (level == 5) return( Badge ( "primary", "Accès de niveau 5", "5" ) );
    if (level == 6) return( Badge ( "warning", "Accès technicien 6", "6" ) );
    if (level == 7) return( Badge ( "warning", "Accès technicien 7", "7" ) );
    if (level == 8) return( Badge ( "warning", "Accès technicien 8", "8" ) );
    if (level == 9) return( Badge ( "danger", "Accès technicien 9", "9" ) );
    return( Badge ( "success", "Accès public", "0" ) );
  }

/********************************************* Renvoi un Select d'access Level ************************************************/
 function Select ( id, fonction, array, selected )
  { retour = "<select id='"+id+"' class='custom-select'"+
             "onchange="+fonction+">";
    valeur = array.map ( function(item) { return(item.valeur); } );
    texte  = array.map ( function(item) { return(item.texte); } );
    for ( i=0; i<valeur.length; i++ )
     { retour += "<option value='"+valeur[i]+"' "+(selected==valeur[i] ? "selected" : "")+">"+texte[i]+"</option>"; }
    retour +="</select>";
    return(retour);
  }
/********************************************* Renvoi un Select d'access Level ************************************************/
 function Select_Access_level ( id, fonction, selected )
  { retour = "<select id='"+id+"' class='custom-select'"+
             "onchange="+fonction+">";
    for ( i=0; i<=localStorage.getItem("access_level"); i++ )
     { retour += "<option value='"+i+"' "+(selected==i ? "selected" : "")+">"+i+"</option>"; }
    retour +="</select>";
    return(retour);
  }
/********************************************* Affichage des vignettes ********************************************************/
 function Changer_img_src ( id, target )
  { $('#'+id).fadeOut("slow", function()
     { $('#'+id).attr("src", target).fadeIn("slow"); } );
  }
/****************************************** Escape les " et ' *****************************************************************/
 function htmlEncode ( string )
  { if (string===null) return("null");
    return ( string.replace(/'/g,'&#39').replace(/"/g,'&#34') ).replace(/</g,'&lt;').replace(/>/g,'&gt;');
  }
/****************************************** Are you sure **********************************************************************/
 function Show_modal_del ( titre, message, details, fonction )
  { $('#idModalDelTitre').text ( htmlEncode(titre) );
    $('#idModalDelMessage').html( htmlEncode(message) );
    $('#idModalDelDetails').html( htmlEncode(details) );
    $('#idModalDelValider').attr( "onclick", fonction );
    $('#idModalDel').modal("show");
  }
/********************************************* Renvoi un input ****************************************************************/
 function Input ( type, id, change_fonction, place_holder, value, controle_function )
  { retour = "<input id='"+id+"' class='form-control' type='"+type+"' "+
             "placeholder='"+htmlEncode(place_holder)+"' "+
             "onchange="+change_fonction+" ";
    if (controle_function !== undefined)
     { retour = retour + "oninput="+controle_function+" "; }
    retour = retour + "value='"+htmlEncode(value)+"'/>";
    return(retour);
  }
/********************************* Chargement d'une courbe dans u synoptique 1 au démrrage ************************************/
 function Charger_une_courbe ( idChart, tech_id, acronyme, period )
  { if (localStorage.getItem("instance_is_master")!="true") return;
    if (period===undefined) period="HOUR";
    var json_request = JSON.stringify(
     { courbes: [ { tech_id : tech_id, acronyme : acronyme, } ],
       period   : period
     });

    Send_to_API ( "PUT", "/api/archive/get", json_request, function(json)
     { var dates;
       if (period=="HOUR") dates = json.valeurs.map( function(item) { return item.date.split(' ')[1]; } );
                      else dates = json.valeurs.map( function(item) { return item.date; } );
       var valeurs = json.valeurs.map( function(item) { return item.moyenne1; } );
console.debug(json.courbe1.libelle);
       var data = { labels: dates,
                    datasets: [ { label: json.courbe1.libelle,
                                  borderColor: "rgba(0, 100, 255, 1.0)",
                                  backgroundColor: "rgba(0, 100, 100, 0.1)",
                                  borderWidth: "1",
                                  tension: "0.1",
                                  radius: "1",
                                  data: valeurs,
                                  yAxisID: "B",
                                },
                              ],
                  }
       var options = { maintainAspectRatio: false,
                       scales: { yAxes: [ { id: "B", type: "linear", position: "left",
                                            scaleLabel: { display: true, labelString: json.courbe1.unite }
                                          }
                                        ]
                               }
                     };
       var ctx = document.getElementById(idChart).getContext('2d');
       var myChart = new Chart(ctx, { type: 'line', data: data, options: options } );
     });

    if (period=="HOUR") setInterval( function() { window.location.reload(); }, 60000);
    else if (period=="DAY")  setInterval( function() { window.location.reload(); }, 300000);
    else setInterval( function() { window.location.reload(); }, 600000);
	 }

/********************************* Chargement d'une courbe dans u synoptique 1 au démrrage ************************************/
 function Charger_plusieurs_courbes ( idChart, tableau_map, period )
  { if (localStorage.getItem("instance_is_master")!="true") return;

    if (period===undefined) period="HOUR";
    var json_request = JSON.stringify(
     { courbes: tableau_map.map( function (item)
                                  { return( { tech_id: item.tech_id, acronyme: item.acronyme } ) } ),
       period : period
     });

    Send_to_API ( "PUT", "/api/archive/get", json_request, function(Response)
     { var dates;
       if (period=="HOUR") dates = Response.valeurs.map( function(item) { return item.date.split(' ')[1]; } );
                      else dates = Response.valeurs.map( function(item) { return item.date; } );
       var data = { labels: dates,
                    datasets: [],
                  }
       for (i=0; i<tableau_map.length; i++)
        { data.datasets.push ( { label: tableau_map[i].libelle+ " ("+tableau_map[i].unite+")",
                                 borderColor: tableau_map[i].color,
                                 backgroundColor: "rgba(100, 100, 100, 0.0)",
                                 /*backgroundColor: "rgba(100, 100, 100, 0.1)",*/
                                 borderWidth: "1",
                                 tension: "0.1",
                                 radius: "1",
                                 data: Response.valeurs.map( function(item) { return(item["moyenne"+(i+1)]); } ),
                                 yAxisID: "B",
                               });
        }
console.debug(data);
       var options = { maintainAspectRatio: false,
                       scales: { yAxes: [ { id: "B", type: "linear", position: "right",
                                            scaleLabel: { display: false, labelString: tableau_map[0].unite }
                                          },
                                        ]
                               }
                     };
       var ctx = document.getElementById(idChart).getContext('2d');
       var myChart = new Chart(ctx, { type: 'line', data: data, options: options } );
     });

    if (period=="HOUR") setInterval( function() { window.location.reload(); }, 60000);
    else if (period=="DAY")  setInterval( function() { window.location.reload(); }, 300000);
    else setInterval( function() { window.location.reload(); }, 600000);
	 }
/*----------------------------------------------------------------------------------------------------------------------------*/
