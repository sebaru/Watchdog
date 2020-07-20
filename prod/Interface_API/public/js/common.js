 document.addEventListener('DOMContentLoaded', Load_common, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_common ()
  { if (document.getElementById("idUsername") !== null)
     { document.getElementById("idUsername").innerHTML = localStorage.getItem("username");
       document.getElementById("idHrefUsername").href = "/tech/user/"+localStorage.getItem("username");
     }
  }

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Show_Error ( message )
  { $('#idModalDetail').html( "Une erreur s'est produite: "+message );
    $('#idModalError').modal("show");
  }

/********************************* Chargement d'une courbe dans u synoptique 1 au démrrage ************************************/
 function Charger_une_courbe ( idChart, tech_id, acronyme, period )
  { var xhr = new XMLHttpRequest;
    xhr.open('GET', "/api/archive/get/"+tech_id+"/"+acronyme+"/"+period );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { var json = JSON.parse(xhr.responseText);
          var dates;
          if (period=="HOUR") dates = json.enregs.map( function(item) { return item.date.split(' ')[1]; } );
                         else dates = json.enregs.map( function(item) { return item.date; } );
          var valeurs = json.enregs.map( function(item) { return item.moyenne; } );
          var data = { labels: dates,
                       datasets: [ { label: json.libelle,
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
                                               scaleLabel: { display: true, labelString: json.unite }
                                             }
                                           ]
                                  }
                        };
          var ctx = document.getElementById(idChart).getContext('2d');
          var myChart = new Chart(ctx, { type: 'line', data: data, options: options } );
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send();

    if (period=="HOUR") setInterval( function() { window.location.reload(); }, 60000);
    else if (period=="DAY")  setInterval( function() { window.location.reload(); }, 300000);
    else setInterval( function() { window.location.reload(); }, 600000);
	 }
