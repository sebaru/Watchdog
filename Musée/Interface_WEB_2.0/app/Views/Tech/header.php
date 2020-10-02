<!doctype html>
<html lang="fr">
    <head>
        <meta charset="utf-8">
        <title>Watchdog Tech</title>

        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
        <meta name="google" content="notranslate">
        <meta name="robots" content="noindex, nofollow">
        <link rel="icon" href="<?php echo base_url('/svg/logo.svg')?>">
        <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css" integrity="sha384-9aIt2nRpC12Uk9gS9baDl411NQApFmC26EwAOH8WgZl5MYYxFfc+NcPb1dKGj7Sk" crossorigin="anonymous">
        <link rel="stylesheet" type="text/css" href="https://cdn.datatables.net/v/dt/dt-1.10.21/fh-3.1.7/r-2.2.5/datatables.min.css"/>
        <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.56.0/codemirror.min.css">
        <style>
        input:focus { outline: 0 0 0 0  !important;
                      box-shadow: 0 0 0 0 !important;
                    }


        .navbar { background-color: rgba(30,28,56,0.8);
                }

        .nav-link:hover { color: white;
                          background-color: #48BBC0;
                        }
      </style>

    </head>

    <body>



  <div id="idToastAlert" class="toast" role="status" style="position: absolute; bottom: 50px; right: 50px; z-index: 99">
   <div class="toast-header">
     <i class="fas fa-exclamation-circle"></i>
     <strong class="mr-auto">Bootstrap</strong>
     <!--<small>11 mins ago</small>-->
     <button type="button" class="ml-2 mb-1 close" data-dismiss="toast" aria-label="Close">
       <span aria-hidden="true">&times;</span>
     </button>
   </div>
   <div class="toast-body">
     Hello, world! This is a toast message.
   </div>
</div>


<div class="position-fixed" style="bottom: 3rem; right: 3rem; z-index:9999">
  <div id="idToastStatus" data-delay="3000" class="toast bg-success" role="status">
   <div class="toast-header">
     <strong class="mr-auto"> Résultat de la commande</strong>
     <!--<small>11 mins ago</small>-->
     <button type="button" class="ml-2 mb-1 close" data-dismiss="toast" aria-label="Close">
       <span aria-hidden="true">&times;</span>
     </button>
   </div>
   <div class="toast-header">
     <i class="fas fa-check-circle text-success"></i><span>Succès !</span>
   </div>
  </div>
</div>

<div id="idModalError" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered" role="document">
    <div class="modal-content">
      <div class="modal-header bg-warning">
        <h5 class="modal-title text-justify"><i class="fas fa-exclamation-circle"></i>Erreur</h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">
        <p id="idModalDetail">Une erreur est survenue !</p>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-primary" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>

<div id="idModalInfo" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered" role="document">
    <div class="modal-content">
      <div class="modal-header bg-info">
        <h5 class="modal-title text-justify"><i class="fas fa-info-circle"></i>Information</h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">
        <p id="idModalInfoDetail">Une information est disponible</p>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-primary" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>

<div id="idModalDel" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-danger text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-trash"></i> <span id="idModalDelTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">
        <p id="idModalDelMessage"></p>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalDelValider" type="button" class="btn btn-danger" data-dismiss="modal"><i class="fas fa-trash"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<header>
	<nav class="navbar navbar-dark  navbar-expand-lg sticky-top shadow mb-2"> <!-- fixed-top -->
  <a class="navbar-brand" href="<?php echo base_url('/');?>"><img src="<?php echo base_url('/svg/logo.svg')?>" alt="Watchdog Logo" width=50></a>
  <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbar-toggled" aria-controls="navbar-toggled" aria-expanded="false" aria-label="Toggle navigation">
    <span class="navbar-toggler-icon"></span>
  </button>

  <div class="collapse navbar-collapse" id="navbar-toggled">
    <ul class="navbar-nav mr-auto">
      <a class="nav-link rounded" href="<?php echo base_url('tech/dashboard'); ?>"> <i class="fas fa-tachometer-alt"></i> Dashboard</a>

      <li class="nav-item dropdown">
        <a class="nav-link rounded dropdown-toggle" href="#" id="navbarCONFIG" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          <i class="fas fa-wrench"></i> Configuration
        </a>
        <div class="dropdown-menu" aria-labelledby="navbarCONFIG">
          <a class="dropdown-item" href="<?php echo base_url('tech/synoptiques'); ?>"> <i class="fas fa-image text-danger"></i> <span>Synoptiques</span> </a>
          <a class="dropdown-item" href="<?php echo base_url('tech/dls'); ?>"> <i class="fas fa-code text-primary"></i> <span>Modules D.L.S</span> </a>
          <a class="dropdown-item" href="<?php echo base_url('tech/tableau'); ?>"> <i class="fas fa-chart-line text-secondary"></i> <span>Tableaux</span> </a>
        </div>
      </li>


      <li class="nav-item dropdown">
        <a class="nav-link rounded dropdown-toggle" href="#" id="navbarINSTANCE" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          <i class="fas fa-crown"></i> Instance
        </a>
        <div class="dropdown-menu" aria-labelledby="navbarINSTANCE">
          <a class="dropdown-item" href="<?php echo base_url('tech/process'); ?>"><i class="fas fa-microchip text-primary"></i> <span>Gestion des Processus</span></a>
          <a class="dropdown-item" href="<?php echo base_url('tech/maintenance'); ?>"><i class="fas fa-wrench text-warning"></i> <span>Maintenance</span></a>
        </div>
      </li>

      <a class="nav-link rounded" href="https://icons.abls-habitat.fr/admin/icons"><i class="fas fa-file-photo-o"></i> <span>Gestion des Icones</span></a>

      <li class="nav-item dropdown">
        <a class="nav-link rounded dropdown-toggle" href="#" id="navbarIO" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          <i class="fas fa-cogs"></i> IO Threads
        </a>
        <div class="dropdown-menu" aria-labelledby="navbarIO">
          <a class="dropdown-item" href="<?php echo base_url('tech/modbus'); ?>"> <i class="fas fa-plug text-info"></i> <span>Modules Wago/Modbus</span> </a>
          <a class="dropdown-item" href="<?php echo base_url('tech/modbus_map'); ?>"> <i class="fas fa-directions text-info"></i> <span>Mappings Wago/Modbus</span> </a>
          <a class="dropdown-item" href="<?php echo base_url('tech/ups'); ?>"> <i class="fas fa-battery-half text-success"></i> <span>Onduleurs</span> </a>
          <div class="dropdown-divider"></div>
          <a class="dropdown-item" href="#">Something else here</a>
        </div>
      </li>
    </ul>

    <ul class="navbar-nav">
      <form class="form-inline ml-2">
        <input class="form-control mr-sm-2" type="search" placeholder="Rechercher" aria-label="Rechercher">
        <button class="btn btn-success rounded my-1 my-sm-0" type="submit">Search</button> <!--btn-outline-success-->
      </form>

      <div class ="nav-item ml-2">
        <div class="input-group">
  		      <select onchange="Change_selected_instance()" id="idSelectedInstance" class="custom-select text-info"></select>
					   </div>
      </div>

      <li class="nav-item dropdown">
        <a class="nav-link rounded dropdown-toggle ml-2" href="#" id="navbarUSER" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          <i class="fas fa-user"></i>
        </a>


        <div class="dropdown-menu dropdown-menu-right" aria-labelledby="navbarUSER">
          <a class="dropdown-item" id="idHrefUsername" href="#"><i class="fas fa-user text-info"></i> <span id="idUsername">-</span></a>
          <div class="dropdown-divider"></div>
          <a class="dropdown-item" href="<?php echo base_url('tech/users_list'); ?>"><i class="fas fa-users-cog text-info"></i> <span>Gestion des utilisateurs</span></a>
          <a class="dropdown-item" href="<?php echo base_url('tech/users_sessions'); ?>"><i class="fas fa-list text-info"></i> <span>Gestion des sessions</span></a>
          <a class="dropdown-item" href="<?php echo base_url('tech/log'); ?>"><i class="fas fa-database text-warning"></i> <span>Audit Log</span></a>
          <div class="dropdown-divider"></div>
          <a class="dropdown-item" href="/home/synmobile/1"><i class="fas fa-home text-primary"></i> <span>Mode Client</span> </a>
          <div class="dropdown-divider"></div>
          <a class="dropdown-item" href="#" onclick="Logout()"><i class="fas fa-sign-out-alt text-danger"></i> <span>Sortir</span> </a>
        </div>
      </li>
    </ul>

  </div>
</nav>
</header>

