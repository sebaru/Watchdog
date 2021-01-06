 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appelé au chargement de la page ************************************************/
 function Change_page ( page )
  { $('#bodycard').fadeOut("high", function ()
     {
       Send_to_API ( "GET", "/api/syn/show", (page === "" ? null : "page="+page), function(Response)
        { console.log(Response);
          $('#idPageTitle').text(Response.libelle);
          $.each ( Response.child_syns, function (i, item) { $('#bodycard').append ( Creer_card ( item ) ); } );
          $('#bodycard').fadeIn("slow");
        }, null );
     });
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Creer_card ( Response )
  {
    var col = $('<div></div>');
    col.addClass('col p-1');

    var card = $('<div></div>');
    card.addClass("card text-center bg-transparent");
    col.append(card);

    var img = $('<img>');
    img.attr("src", "/upload/syn_"+Response.id+".jpg");
    img.attr("onclick", "Change_page('"+Response.page+"')");
    img.addClass("card-img-top wtd-synoptique");

    var card_header = $('<div></div>');
    card_header.addClass("card-header");
    card.append(img);

    var activite = $('<img>');
    activite.addClass("wtd-vignette");
    activite.attr("src", "/img/pignon.svg");
    card_header.append(activite);
    card.append(card_header);

    var secu_bien = $('<img>');
    secu_bien.addClass("wtd-vignette");
    secu_bien.attr("src", "/img/pignon.svg");
    card_header.append(secu_bien);
    card.append(card_header);

    var secu_pers = $('<img>');
    secu_pers.addClass("wtd-vignette");
    secu_pers.attr("src", "/img/pignon.svg");
    card_header.append(secu_pers);
    card_header.append(" "+Response.page);
    card.append(card_header);

    var card_body = $('<div></div>');
    card_body.addClass("card-body");
    card_body.text(Response.libelle);
    card.append(card_body);
    return(col);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    Change_page( vars[1] );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
