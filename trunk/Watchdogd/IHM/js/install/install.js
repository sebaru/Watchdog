
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Installer ()
  { var json_request =
     { description: $('#idInstallDescription').val(),
       db_hostname: $('#idInstallDBHostname').val(),
       db_port:     parseInt($('#idInstallDBPort').val()),
       db_username: $('#idInstallDBUsername').val(),
       db_database: $('#idInstallDBDatabase').val(),
       db_password: $('#idInstallDBPassword').val(),
       is_master:   parseInt($('#idInstallIsMaster').val()),
       master_host: $('#idInstallMasterHost').val(),
       use_subdir:  parseInt($('#idInstallUseSubdir').val()),
       run_as:      $('#idInstallRunAs').val(),
     };
    $('#idSpinner').show();
    $('#idAction').text("Please Wait !")

    Send_to_API ( 'POST', "/api/install", JSON.stringify(json_request), function()
     { setTimeout ( function () { window.location = "/"; }, 5000 );
     }, function()
     { setTimeout ( function ()
        { $('#idSpinner').hide();
          $('#idAction').text("Installer !")
        }, 2000 );
     });
	 }
