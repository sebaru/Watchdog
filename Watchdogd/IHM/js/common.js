 document.addEventListener('DOMContentLoaded', Load_common, false);

 var Charts = new Array();

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

    xhr.timeout = 60000; // durée en millisecondes

    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       $(".ClassLoadingSpinner").hide();
       if (xhr.status == 200)
        { try { var Response = JSON.parse(xhr.responseText); }
          catch (error) { Response=undefined; }
          if (method=="DELETE" || method=="POST") $('#idToastStatus').toast('show');
          if (fonction_ok != null) fonction_ok(Response);
        }
       else { if (xhr.status != 503) Show_Error( xhr.statusText );
              try { var Response = JSON.parse(xhr.responseText); }
              catch (error) { Response=undefined; }
              if (fonction_nok != null) fonction_nok(Response);
            }
     }
    xhr.ontimeout = function() { console.log("XHR timeout for "+URL); }
    xhr.send(parametre);
  }
/************************************ Controle de saisie avant envoi **********************************************************/
 function isNum ( id )
  { FormatTag = RegExp(/^[0-9-]+$/);
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
    if (localStorage.getItem("access_level")>=6) { $('#idMenuTechnicien').show(); }
                                            else { $('#idMenuTechnicien').hide(); }
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
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Show_Info ( message )
  { $('#idModalInfoDetail').html( htmlEncode(message) );
    $('#idModalInfo').modal("show");
  }
/********************************************* Redirige la page ***************************************************************/
 function Redirect ( url )
  { /*$('body').fadeOut("high", function () { */ window.location = url; /* } );*/
  }

 function Reload_when_ready ( )
  { Send_to_API ( "GET", "/api/ping", null,
                  function (Response) { if(Response.Thread_run == false) setTimeout ( function () { Reload_when_ready() }, 1000 );
                                        else window.location.reload(false);
                                      },
                  function () { setTimeout ( function () { Reload_when_ready() }, 1000 ); }
                );
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
                "onclick="+clic_func+"('"+key+"')>"+
                "<span id='idButtonSpinner_"+key+"' class='spinner-border spinner-border-sm' style='display:none' "+
                "role='status' aria-hidden='true'></span> "+
                texte+
                "</button>";
     }
   else
    { result =  "<button "+
                "class='btn btn-"+color+" btn-block btn-sm' "+
                "data-toggle='tooltip' title='"+tooltip+"' "+
                "disabled>"+texte+
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
    if (level >= 9) return( Badge ( "danger", "Accès technicien 9", "9" ) );
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
 function Changer_img_src ( id, target, cligno )
  { var image = $('#'+id);
    console.log("Changer_img_src "+id+" from '" + image.attr('src') + "' to "+ target );

    if (cligno==false) { image.removeClass("wtd-cligno"); }
    if (image.attr('src')==target) return;

    if (image.attr('src') == "")
     { console.log("Changer_img_src "+id+" 1");
       image.slideUp("fast", function()
        { image.on("load", function() { image.slideDown("normal", function ()
                                         { if (cligno==true) { image.addClass("wtd-cligno"); } } ); } );
          image.attr("src", target);
          console.log("Changer_img_src "+id+" 1 fin:" + image.attr("src") );
        });
     }
    else
     { console.log("Changer_img_src "+id+" 2");
       image.fadeTo("fast", 0, function()
        { image.on("load", function() { image.fadeTo("normal", 1, function ()
                                         { if (cligno==true) { image.addClass("wtd-cligno"); } } ); } );
          image.attr("src", target);
          console.log("Changer_img_src "+id+" 2 fin:" + image.attr("src") );
        });
     }
  }
/********************************************* Remonte la page au top *********************************************************/
 function Scroll_to_top ()
  { window.scrollTo({ top: 0, left: 0, behavior: 'smooth' }); }
/********************************************* Affichage des vignettes ********************************************************/
 function Slide_down_when_loaded ( id )
  { var images = $('#'+id+' img[src]');
    var loaded_images_count = 0;
    if (images.length==0) { $('#'+id).slideDown("slow"); return; }
    images.on("load", function()
     { loaded_images_count++;
       if (loaded_images_count == images.length) { $('#'+id).slideDown("slow"); }
     });
  }
/****************************************** Escape les " et ' *****************************************************************/
 function htmlEncode ( string )
  { if (string===undefined) return("null");
    if (string===null) return("null");
    return ( string.replace(/'/g,'&apos;').replace(/"/g,'&quote;') ).replace(/</g,'&lt;').replace(/>/g,'&gt;');
  }
/****************************************** Are you sure **********************************************************************/
 function Show_modal_del ( titre, message, details, fonction )
  { $('#idModalDelTitre').text ( htmlEncode(titre) );
    $('#idModalDelMessage').html( htmlEncode(message) );
    $('#idModalDelDetails').html( htmlEncode(details) );
    $('#idModalDelValider').off("click").on( "click", fonction );
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

    var chartElement = document.getElementById(idChart);
    if (!chartElement) { console.log("Charger_une_courbe: Erreur chargement chartElement " + json_request ); return; }

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
       var ctx = chartElement.getContext('2d');
       if (!ctx) { console.log("Charger_une_courbe: Erreur chargement context " + json_request ); return; }

       if (Charts != null && Charts[idChart] != null) Charts[idChart].destroy();
       Charts[idChart] = new Chart(ctx, { type: 'line', data: data, options: options } );
     });

    /* if (period=="HOUR") setInterval( function() { window.location.reload(); }, 60000);
    else if (period=="DAY")  setInterval( function() { window.location.reload(); }, 300000);
    else setInterval( function() { window.location.reload(); }, 600000);*/
	 }

/********************************* Chargement d'une courbe dans u synoptique 1 au démrrage ************************************/
 function Charger_plusieurs_courbes ( idChart, tableau_map, period )
  { if (localStorage.getItem("instance_is_master")!="true") return;

    var chartElement = document.getElementById(idChart);
    if (!chartElement) { console.log("Charger_plusieurs_courbes: Erreur chargement chartElement " + json_request ); return; }

    if (period===undefined) period="HOUR";
    var json_request = JSON.stringify(
     { courbes: tableau_map.map( function (item)
                                  { return( { tech_id: item.tech_id, acronyme: item.acronyme } ) } ),
       period : period
     });

    Send_to_API ( "PUT", "/api/archive/get", json_request, function(Response)
     { var dates;
       var ctx = chartElement.getContext('2d');
       if (!ctx) { console.log("Charger_plusieurs_courbes: Erreur chargement context " + json_request ); return; }

       if (period=="HOUR") dates = Response.valeurs.map( function(item) { return item.date.split(' ')[1]; } );
                      else dates = Response.valeurs.map( function(item) { return item.date; } );
       var data = { labels: dates,
                    datasets: [],
                  }
       for (i=0; i<tableau_map.length; i++)
        { data.datasets.push ( { label: Response["courbe"+(i+1)].libelle+ " ("+Response["courbe"+(i+1)].unite+")",
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
       if (Charts != null && Charts[idChart] != null)
        { Charts[idChart].ctx.destroy();
          if (Charts[idChart].timeout != null) clearTimeout ( Charts[idChart].timeout );
        } else Charts[idChart] = new Object ();

       Charts[idChart].ctx = new Chart(ctx, { type: 'line', data: data, options: options } );
       if (period == "HOUR")
        { Charts[idChart].timeout = setTimeout ( function()                                                  /* Update graphe */
           { Charger_plusieurs_courbes ( idChart, tableau_map, period ); }, 60000 );
        }
     });
	 }
/******************************************************************************************************************************/
/* Get_url_parameter : Recupere un parametre de recherche dans l'URL                                                          */
/******************************************************************************************************************************/
 function Get_url_parameter ( name )
  { const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    return (urlParams.get(name));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
