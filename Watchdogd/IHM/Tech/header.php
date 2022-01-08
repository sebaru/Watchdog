<!doctype html>
<html lang="fr">
    <head>
        <meta charset="utf-8">
        <title>Watchdog Tech</title>

        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
        <meta name="google" content="notranslate">
        <meta name="robots" content="noindex, nofollow">
        <link rel="icon" href="/img/logo.svg">
        <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.6.0/dist/css/bootstrap.min.css" integrity="sha384-B0vP5xmATw1+K9KRQjQERJvTumQW0nPEzvF6L/Z6nronJ3oUOFUFpCjEUQouq2+l" crossorigin="anonymous">
        <link rel="stylesheet" type="text/css" href="https://cdn.datatables.net/v/dt/dt-1.10.21/fh-3.1.7/r-2.2.5/datatables.min.css"/>
        <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.59.2/codemirror.min.css">
        <style>
        input:focus { outline: 0 0 0 0  !important;
                      box-shadow: 0 0 0 0 !important;
                    }


        .navbar { background-color: rgba(30,28,56,0.8);
                }

        .nav-link {
                  }

        .nav-link:hover { /*color: white !important; attention, cible aussi les nav-tabs */
                          background-color: #48BBC0;
                        }
        .courbe-dashboard { height: 300px; }

        .wtd-synoptique-preview { height: 80px; }
        .wtd-img-bit-interne { width: 40px; }
        .wtd-img-connecteur  { object-fit: contain; height: 196px; padding: 10px; }
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
  <div id="idToastStatus" data-delay="3000" class="toast hide bg-success" role="status">
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
       <hr>
        <strong id="idModalDelDetails"></strong>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalDelValider" type="button" class="btn btn-danger" data-dismiss="modal"><i class="fas fa-trash"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<!------------------------------------------------- Modal Map ----------------------------------------------------------------->
<div id="idMODALMap" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idMODALMapTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Rechercher une Target</label>
           <input id="idMODALMapRechercherTechID" type="text" class="col-7 form-control" placeholder="Rechercher un Tech_id">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Target TechID</label>
           <select id="idMODALMapSelectTechID" required class="col-7 custom-select border-info"></select>
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Target Acronyme</label>
           <select id="idMODALMapSelectAcronyme" required class="col-7 custom-select border-info"></select>
          </div>
       </div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idMODALMapValider" type="button" class="btn btn-primary"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<!----------------------------------------------------------------------------------------------------------------------------->
<header>
 <nav class="navbar navbar-dark  navbar-expand-lg sticky-top shadow mb-2"> <!-- fixed-top -->
  <a class="navbar-brand" href="/"><img src="/img/logo.svg" alt="Watchdog Logo" width=50></a>
  <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbar-toggled" aria-controls="navbar-toggled" aria-expanded="false" aria-label="Toggle navigation">
    <span class="navbar-toggler-icon"></span>
  </button>

  <div class="collapse navbar-collapse" id="navbar-toggled">
    <ul class="navbar-nav mr-auto">
      <a class="nav-link rounded" href="/tech/dashboard"> <i class="fas fa-tachometer-alt"></i> Dashboard</a>

      <li class="nav-item dropdown">
        <a class="nav-link rounded dropdown-toggle" href="#" id="navbarCONFIG" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          <i class="fas fa-wrench"></i> Configuration
        </a>
        <div class="dropdown-menu" aria-labelledby="navbarCONFIG">
          <a class="dropdown-item" href="/tech/synoptiques"> <i class="fas fa-image text-danger"></i> <span>Synoptiques</span> </a>
          <a class="dropdown-item" href="/tech/dls"> <i class="fas fa-code text-primary"></i> <span>Modules D.L.S</span> </a>
          <a class="dropdown-item" href="/tech/tableau"> <i class="fas fa-chart-line text-secondary"></i> <span>Tableaux</span> </a>
        </div>
      </li>


      <li class="nav-item dropdown">
        <a class="nav-link rounded dropdown-toggle" href="#" id="navbarDOMAINE" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          <i class="fas fa-crown"></i> Domaine
        </a>
        <div class="dropdown-menu" aria-labelledby="navbarDOMAINE">
          <a class="dropdown-item" href="/tech/instance"><i class="fas fa-crown text-danger"></i> <span>Instances</span></a>
          <a class="dropdown-item" href="/tech/process"><i class="fas fa-microchip text-primary"></i> <span>Processus</span></a>
          <a class="dropdown-item" href="/tech/archive"><i class="fas fa-database text-secondary"></i> <span>Archivage</span></a>
          <a class="dropdown-item" href="/tech/dashboard_courbes"> <i class="fas fa-chart-line text-secondary"></i> <span>Courbes</span> </a>
        </div>
      </li>

      <a class="nav-link rounded" href="/tech/io_config"><i class="fas fa-link"></i> <span>Connecteurs</span></a>

    </ul>

    <ul class="navbar-nav">

      <div class="mt-1 spinner-border text-primary ClassLoadingSpinner" style="display:none" role="status">
        <span class="sr-only">Loading...</span>
      </div>

      <a class="nav-link rounded" href="/tech/search"><i class="fas fa-search"></i> <span> Dictionnaire</span></a>

      <a class="nav-link rounded" href="https://docs.abls-habitat.fr"><i class="fas fa-book"></i> <span> Documentation</span></a>

      <li class="nav-item dropdown">
        <a class="nav-link rounded dropdown-toggle ml-2" href="#" id="navbarUSER" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          <i class="fas fa-user  text-warning"></i> <span id="idUsername">-</span>
        </a>

        <div class="dropdown-menu dropdown-menu-right" aria-labelledby="navbarUSER">
          <a class="dropdown-item" href="/tech/user" id="idHrefUsername"><i class="fas fa-user text-info"></i> Mon Profil</a>
          <div class="dropdown-divider"></div>
          <a class="dropdown-item" href="/tech/users"><i class="fas fa-users-cog text-info"></i> <span>Utilisateurs</span></a>
          <a class="dropdown-item" href="/tech/users_sessions"><i class="fas fa-list text-info"></i> <span>Sessions</span></a>
          <a class="dropdown-item" href="/tech/log"><i class="fas fa-binoculars text-warning"></i> <span>Audit Log</span></a>
          <div class="dropdown-divider"></div>
          <a class="dropdown-item" href="#" onclick="Logout()"><i class="fas fa-sign-out-alt text-danger"></i> <span>Sortir</span> </a>
        </div>
      </li>
    </ul>

  </div>
</nav>
</header>

   <div id="idAlertNotMaster" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Attention !</h4>
         Vous etes sur une instance Slave !
   </div>


