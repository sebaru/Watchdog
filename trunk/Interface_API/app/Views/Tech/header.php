<!doctype html>
<html lang="fr">
    <head>
        <meta charset="utf-8">
        <title>Watchdog Tech</title>

        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
        <meta name="google" content="notranslate">
        <meta name="robots" content="noindex, nofollow">
        <link rel="icon" href="<?php echo base_url('/logo.svg')?>">
        <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css" integrity="sha384-9aIt2nRpC12Uk9gS9baDl411NQApFmC26EwAOH8WgZl5MYYxFfc+NcPb1dKGj7Sk" crossorigin="anonymous">
        <script src="https://kit.fontawesome.com/1ca1f7ba56.js" crossorigin="anonymous"></script>
        <style>
        .input-group-prepend span { width: 50px;
                                    background-color: #F1E413;
                                    color: black;
                                    border:0 !important;
                                  }

        input:focus { outline: 0 0 0 0  !important;
                      box-shadow: 0 0 0 0 !important;
                    }

      </style>

    </head>

    <body>

<!--
 <div id="toast-error" class="toast" role="alert" aria-live="assertive" aria-atomic="true" style="position: absolute; top: 0; right: 0;">
   <div class="toast-header">
     <i class="fas exclamation-circle"></i>
     <strong class="mr-auto">Erreur !</strong>
     <small>so sorry !</small>
     <button type="button" class="ml-2 mb-1 close" data-dismiss="toast" aria-label="Close">
       <span aria-hidden="true">&times;</span>
     </button>
   </div>
   <div class="toast-body">
     This is a Error Toast.
   </div>
 </div>
-->

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

<header>
	<nav class="navbar navbar-dark bg-primary navbar-expand-lg sticky-top shadow mb-2"> <!-- fixed-top -->
  <a class="navbar-brand" href="<?php echo base_url('/');?>"><img src="<?php echo base_url('/logo.svg')?>" alt="Watchdog Logo" width=50></a>
  <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbar-toggled" aria-controls="navbar-toggled" aria-expanded="false" aria-label="Toggle navigation">
    <span class="navbar-toggler-icon"></span>
  </button>

  <div class="collapse navbar-collapse" id="navbar-toggled">
    <ul class="navbar-nav mr-auto">
      <a class="nav-link active" href="<?php echo base_url('tech/dashboard'); ?>"> <i class="fas fa-tachometer-alt"></i><span> Dashboard</span></a>

      <li class="nav-item dropdown">
        <a class="nav-link dropdown-toggle" href="#" id="navbarCONFIG" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          <i class="fa fa-cog"></i> Configuration
        </a>
        <div class="dropdown-menu" aria-labelledby="navbarCONFIG">
          <a class="dropdown-item" href="<?php echo base_url('tech/syn'); ?>"> <i class="fas fa-image text-danger"></i> <span> Synoptiques</span> </a>
          <a class="dropdown-item" href="<?php echo site_url('tech/dls'); ?>"> <i class="fas fa-code text-primary"></i> <span>Modules D.L.S</span> </a>
          <a class="dropdown-item" href="<?php echo site_url('tech/tableau'); ?>"> <i class="fas fa-chart-line text-secondary"></i> <span>Tableaux</span> </a>
        </div>
      </li>


      <li class="nav-item dropdown">
        <a class="nav-link dropdown-toggle" href="#" id="navbarINSTANCE" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          Instance
        </a>
        <div class="dropdown-menu" aria-labelledby="navbarINSTANCE">
          <a class="dropdown-item" href="<?php echo site_url('tech/process'); ?>"><i class="fas fa-microchip text-primary"></i> <span>Gestion des Processus</span></a>
          <a class="dropdown-item" href="<?php echo site_url('tech/maintenance'); ?>"><i class="fas fa-wrench text-warning"></i> <span>Maintenance</span></a>
        </div>
      </li>

      <a class="nav-link" href="https://icons.abls-habitat.fr/admin/icons"><i class="fas fa-file-photo-o"></i> <span>Gestion des Icones</span></a>

      <li class="nav-item dropdown">
        <a class="nav-link dropdown-toggle" href="#" id="navbarIO" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          <i class="fa fa-robot"></i> Inputs/Outputs
        </a>
        <div class="dropdown-menu" aria-labelledby="navbarIO">
          <a class="dropdown-item" href="<?php echo site_url('tech/modbus'); ?>"> <i class="fas fa-cogs text-info"></i> <span>Module Wago/Modbus</span> </a>
          <a class="dropdown-item" href="<?php echo site_url('tech/ups'); ?>"> <i class="fas fa-battery-half text-success"></i> <span>Onduleurs</span> </a>
          <div class="dropdown-divider"></div>
          <a class="dropdown-item" href="#">Something else here</a>
        </div>
      </li>
    </ul>
    <ul class="navbar-nav">
    <form class="form-inline ml-2 my-2 my-lg-0">
      <input class="form-control mr-sm-2" type="search" placeholder="Rechercher" aria-label="Rechercher">
      <button class="btn btn-success my-2 my-sm-0" type="submit">Search</button> <!--btn-outline-success-->
    </form>
      <li class="nav-item dropdown">
        <a class="nav-link dropdown-toggle" href="#" id="navbarUSER" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          <i class="fas fa-user"></i> Utilisateurs
        </a>
        <div class="dropdown-menu dropdown-menu-right" aria-labelledby="navbarUSER">
          <a class="dropdown-item" href="<?php echo site_url('tech/users'); ?>"><i class="fas fa-user text-info"></i> <span>Gestion des utilisateurs</span></a>
          <a class="dropdown-item" href="<?php echo site_url('tech/log'); ?>"><i class="fas fa-database text-warning"></i> <span>Audit Log</span></a>
          <div class="dropdown-divider"></div>
          <a class="dropdown-item" href="auth/logout"><i class="fas fa-sign-out-alt text-danger"></i> <span>Logout</span> </a>
        </div>
      </li>
    </ul>
  </div>
</nav>
</header>
