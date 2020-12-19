 document.addEventListener('DOMContentLoaded', Load_common, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Send_to_API ( method, URL, parametre, fonction_ok, fonction_nok )
  { var xhr = new XMLHttpRequest;
    $(".ClassLoadingSpinner").show();
    xhr.open(method, URL, true);
    if (method=="POST") { xhr.setRequestHeader('Content-type', 'application/json'); }
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

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_common ()
  { if (document.getElementById("idUsername") !== null)
     { document.getElementById("idUsername").innerHTML = localStorage.getItem("username");
       document.getElementById("idHrefUsername").href = "/home/user/"+localStorage.getItem("username");
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
  { window.location = url;
  }

/********************************************* Barre de boutons ***************************************************************/
 function Bouton_actions_start ( )
  { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"); }

 function Bouton_actions_add ( color, tooltip, clic_func, key, icone, texte )
  { return( "<button "+
            "class='btn btn-"+color+" btn-sm' "+
            "data-toggle='tooltip' title='"+tooltip+"' "+
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
  { return( "<a href='"+target+"' data-toggle='tooltip' title='"+tooltip+"'>"+texte+"</a>" );
  }

 function Badge ( color, tooltip, texte )
  { return("<span "+
           "class='badge badge-pill badge-"+color+"' "+
           "data-toggle='tooltip' title='"+tooltip+"'>"+texte+
           "</span>" );
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
/****************************************** Escape les " et ' *****************************************************************/
 function htmlEncode ( string )
  { if (string===null) return("null");
    return ( string.replace(/'/g,'&#39').replace(/"/g,'&#34') );
  }
/****************************************** Are you sure **********************************************************************/
 function Show_modal_del ( titre, message, fonction )
  { $('#idModalDelTitre').text ( htmlEncode(titre) );
    $('#idModalDelMessage').html( htmlEncode(message) );
    $('#idModalDelValider').attr( "onclick", fonction );
    $('#idModalDel').modal("show");
  }
/********************************************* Renvoi un input ****************************************************************/
 function Input ( type, id, change_fonction, place_holder, value )
  { retour = "<input id='"+id+"' class='form-control' type='"+type+"' "+
             "placeholder='"+htmlEncode(place_holder)+"' "+
             "onchange="+change_fonction+" "+
             "value='"+htmlEncode(value)+"'/>";
    return(retour);
  }
/********************************* Chargement d'une courbe dans u synoptique 1 au démrrage ************************************/
 function Charger_une_courbe ( idChart, tech_id, acronyme, period )
  { if (localStorage.getItem("instance_is_master")!="true") return;
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

