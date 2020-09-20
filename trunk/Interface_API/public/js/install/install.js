
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Installer ()
  { var json_request =
     { db_hostname: $('#idInstallDBHostname').val(),
       db_port:     $('#idInstallDBPort').val(),
       db_username: $('#idInstallDBUsername').val(),
       db_database: $('#idInstallDBDatabase').val(),
       db_password: $('#idInstallDBPassword').val(),
       is_master:   $('#idInstallIsMaster').val(),
       master_host: $('#idInstallMasterHost').val(),
       use_subdir:  $('#idInstallUseSubdir').val(),
       run_as:      $('#idInstallRunAs').val(),
     };
    Send_to_API ( 'POST', "/api/install", JSON.stringify(json_request), function()
     { $('#idSpinner').show();
       $('#idAction').text("Please Wait 10 seconds")
       setTimeout( function() { window.location = "/" }, 10000 );
     });
	 }
