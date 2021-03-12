 document.addEventListener('DOMContentLoaded', Load_page, false);
 var SourceCode;

 function Go_to_mnemos ()
  { vars = window.location.pathname.split('/');
    Redirect ( "/tech/mnemos/"+vars[3] );
  }
 function Go_to_source ()
  { vars = window.location.pathname.split('/');
    Redirect ( "/tech/dls_source/"+vars[3] );
  }
 function Go_to_dls_run ()
  { vars = window.location.pathname.split('/');
    Redirect ( "/tech/dls_run/"+vars[3] );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Compiler ()
  { vars = window.location.pathname.split('/');
    var json_request = JSON.stringify(
     { tech_id : vars[3],
       sourcecode: SourceCode.getDoc().getValue(),
     });
    Send_to_API ( "POST", "/api/dls/compil", json_request, function(Response)
     { $("#idErrorLog").html(Response.errorlog.replace(/(?:\r\n|\r|\n)/g, '<br>'));
       $("#idErrorLog").removeClass("alert-info alert-warning alert-danger alert-success");
       switch(Response.result)
        { case "success" : $("#idErrorLog").addClass("alert-success"); break;
          case "error"   : $("#idErrorLog").addClass("alert-danger"); break;
          case "warning" : $("#idErrorLog").addClass("alert-warning"); break;
        }
     }, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    SourceCode = CodeMirror.fromTextArea( document.getElementById("idSourceCode"), { lineNumbers: true } );
    SourceCode.setSize( null, "100%");

    $("#idSourceTitle").text(vars[3]);

    var json_request = JSON.stringify(
     { tech_id : vars[3],
     });
    Send_to_API ( "PUT", "/api/dls/source", json_request, function(Response)
     { SourceCode.getDoc().setValue(Response.sourcecode);
       $("#idErrorLog").html(Response.errorlog.replace(/(?:\r\n|\r|\n)/g, '<br>'));
     }, null);
  }
