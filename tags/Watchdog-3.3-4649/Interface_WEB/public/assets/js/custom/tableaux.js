	$(document).ready(function() {

 });

 var myChart;

/********************************** Fonction de conversion des coleus HEX vers RGB*********************************************/
 function hexToRgb(hex)
  {
        // long version
        r = hex.match(/^#([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})$/i);
        if (r) { return r.slice(1,4).map(function(x) { return parseInt(x, 16); }); }
        // short version
        r = hex.match(/^#([0-9a-f])([0-9a-f])([0-9a-f])$/i);
        if (r) { return r.slice(1,4).map(function(x) { return 0x11 * parseInt(x, 16); }); }
        return null;
  }

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Charger_une_courbe ( tech_id, acronyme, color, period )
  { $.getJSON( base_url + "archive/get_ea/"+tech_id+"/"+acronyme+"/"+period, function (json)
     { var dates = json.data.map( function(item) { return item.date; } );
       var valeurs = json.data.map( function(item) { return item.moyenne; } );

       if (typeof myChart === 'undefined')
        { var data = { labels: dates,
                       datasets: [ { label: json.libelle,
                                     borderColor: "rgba("+hexToRgb(color)+", 1.0)",
                                     backgroundColor: "rgba("+hexToRgb(color)+", 0.1)",
                                     borderWidth: "1",
                                     tension: "0.1",
                                     radius: "1",
                                     data: valeurs,
                                     yAxisID: "B",
                                   },
                                 ],
                     }
          var options = { scales: { yAxes: [ { id: "B", type: "linear", position: "right",
                                               scaleLabel: { display: true, labelString: json.unite }
                                             }
                                           ]
                                  }
                        };
          var ctx = document.getElementById('myTableau').getContext('2d');
          myChart = new Chart(ctx, { type: 'line', data: data, options: options } );
          console.log("New Chart");
        }
       else
        { /*myChart.data.labels.push(dates);*/
          newdata = { label: json.libelle,
                      borderColor: "rgba("+hexToRgb(color)+", 1.0)",
                      backgroundColor: "rgba("+hexToRgb(color)+", 0.1)",
                      borderWidth: "1",
                      tension: "0.1",
                      radius: "1",
                      data: valeurs,
                      yAxisID: "B",
                    };
          myChart.data.datasets.push(newdata);
          myChart.update();
          console.log("Update Chart");
        }
     });

	 }

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Charger_tableau ( tableau_id, period )
  { $.getJSON( base_url + "archive/get_courbes/"+tableau_id, function (json)
     { $(json).each( function ()
        { Charger_une_courbe( this.tech_id, this.acronyme, this.color, period );
        })
     });
    if (period=="HOUR") setInterval( function() { window.location.reload(); }, 60000);
    else if (period=="DAY")  setInterval( function() { window.location.reload(); }, 300000);
    else setInterval( function() { window.location.reload(); }, 600000);
	 }
