	$(document).ready(function() {
    Charger_courbe( "dls_tour_per_sec", "SYS", "DLS_TOUR_PER_SEC", "DAY" );
    Charger_courbe( "dls_bit_per_sec", "SYS", "DLS_BIT_PER_SEC", "DAY" );
    Charger_courbe( "dls_wait", "SYS", "DLS_WAIT", "DAY" );
    Charger_courbe( "arch_request_number", "SYS", "ARCH_REQUEST_NUMBER", "DAY" );
 });

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Charger_courbe ( chart, tech_id, acronyme, period )
  { $.getJSON( base_url + "archive/get_ea/"+tech_id+"/"+acronyme+"/"+period, function (json)
     { var dates = json.data.map( function(item) { return item.date; } );
       var valeurs = json.data.map( function(item) { return item.moyenne; } );
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
       var options = { scales: { yAxes: [ { id: "B", type: "linear", position: "left",
                                            scaleLabel: { display: true, labelString: json.unite }
                                          }
                                        ]
                               }
                     };
       var ctx = document.getElementById(chart).getContext('2d');
       var myChart = new Chart(ctx, { type: 'line', data: data, options: options } );
     });

	 }
