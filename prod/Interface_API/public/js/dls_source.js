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
 function Go_to_run ()
  { vars = window.location.pathname.split('/');
    Redirect ( "/tech/run/"+vars[3] );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Compiler ()
  { vars = window.location.pathname.split('/');
    var xhr = new XMLHttpRequest;
    xhr.open('POST', "/api/dls/compil", true);
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify(
       { tech_id : vars[3],
         sourcecode: SourceCode.getDoc().getValue(),
       }
     );
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status != 200)
        { Show_Error ( xhr.statusText );
          return;
        }
       var Response = JSON.parse(xhr.responseText);
       $("#idErrorLog").html(Response.errorlog.replace(/(?:\r\n|\r|\n)/g, '<br>'));
       $("#idErrorLog").removeClass("alert-info alert-warning alert-danger alert-success");
       switch(Response.result)
        { case "success" : $("#idErrorLog").addClass("alert-success"); break;
          case "error"   : $("#idErrorLog").addClass("alert-danger"); break;
          case "warning" : $("#idErrorLog").addClass("alert-warning"); break;
        }
     };
    xhr.send(json_request);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    SourceCode = CodeMirror.fromTextArea( document.getElementById("idSourceCode"), { lineNumbers: true } );
    SourceCode.setSize( null, "100%");

    $("#idSourceTitle").text(vars[3]);

    var xhr = new XMLHttpRequest;
    xhr.open('GET', "/api/dls/source/"+vars[3], true);
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status != 200)
        { Show_Error ( xhr.statusText );
          return;
        }
       var Response = JSON.parse(xhr.responseText);
       SourceCode.getDoc().setValue(Response.sourcecode);
       $("#idErrorLog").html(Response.errorlog.replace(/(?:\r\n|\r|\n)/g, '<br>'));
     };
    xhr.send();

  }
