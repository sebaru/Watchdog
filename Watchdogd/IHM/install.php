<!doctype html>
<html lang="fr">
    <head>
        <meta charset="utf-8">
        <title>Watchdog Tech</title>

        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
        <meta name="google" content="notranslate">
        <meta name="robots" content="noindex, nofollow">
        <link rel="icon" href="/img/logo.svg">
        <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css" integrity="sha384-9aIt2nRpC12Uk9gS9baDl411NQApFmC26EwAOH8WgZl5MYYxFfc+NcPb1dKGj7Sk" crossorigin="anonymous">
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

<div class="container-sm">
<header>
<nav class="navbar navbar-dark  navbar-expand-lg sticky-top shadow mb-2"> <!-- fixed-top -->
  <a class="navbar-brand" href="wiki.abls-habitat.fr"><img src="/img/logo.svg" alt="Watchdog Logo" width=50></a>
  <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbar-toggled" aria-controls="navbar-toggled" aria-expanded="false" aria-label="Toggle navigation">
    <span class="navbar-toggler-icon"></span>
  </button>
</nav>
</header>


<h3 class="text-center">Bienvenue sur Watchdog Abls-Habitat !</h3>
<hr>

 <div class="row m-2">
   <h4>Entrez les paramètres de configuration de votre instance</h4>

   <div class ="ml-auto btn-group">
        <button type="button" onclick="Installer()" class="btn btn-primary">
          <span id="idSpinner" class="spinner-border spinner-border-sm" style="display:none" role="status" aria-hidden="true"></span>
          <i class="fas fa-cog"></i><span id="idAction">Installer !</span>
        </button>
   </div>
 </div>

       <div class="col form-group">
         <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Nommez votre habitat</label>
           <input id="idInstallDescription" type="text" class="form-control" placeholder="Ma Maison">
         </div>
       </div>

       <div class="col form-group">
         <div class="input-group">
         <label class="col-5 col-sm-4 col-form-label text-right">Database Hostname</label>
         <input id="idInstallDBHostname" type="text" class="form-control" value="localhost">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>

       <div class="col form-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    <div class="input-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <label class="col-5 col-sm-4 col-form-label text-right">Database Port</label>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <input id="idInstallDBPort" type="number" min="0" max="65535" class="form-control" value="3306">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>

       <div class="col form-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    <div class="input-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <label class="col-5 col-sm-4 col-form-label text-right">Database Name</label>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <input id="idInstallDBDatabase" type="text" class="form-control" value="WatchdogDB">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>

       <div class="col form-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    <div class="input-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <label class="col-5 col-sm-4 col-form-label text-right">Database Username</label>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <input id="idInstallDBUsername" type="text" class="form-control" value="watchdog">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>

       <div class="col form-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    <div class="input-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <label class="col-5 col-sm-4 col-form-label text-right">Database Password</label>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <input id="idInstallDBPassword" type="password" class="form-control" placeholder="Database Password">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>

       <div class="col form-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    <div class="input-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <label class="col-5 col-sm-4 col-form-label text-right">Run_As</label>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <input id="idInstallRunAs" type="text" class="form-control" value="watchdog">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>

       <div class="col form-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    <div class="input-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <label class="col-5 col-sm-4 col-form-label text-right">Use SubDirectory</label>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         <select id="idInstallUseSubdir" class="custom-select">
               <option value="0" selected>Non</option>
               <option value="1">Oui</option>
             </select>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>

       <div class="col form-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    <div class="input-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <label class="col-5 col-sm-4 col-form-label text-right">Instance Maitre</label>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         <select id="idInstallIsMaster" class="custom-select">
               <option value="1" selected>Oui</option>
               <option value="0">Non</option>
             </select>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>

       <div class="col form-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    <div class="input-group">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <label class="col-5 col-sm-4 col-form-label text-right">Master Host</label>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       <input id="idInstallMasterHost" type="text" class="form-control" placeholder="Hostname de l'instance Master si celle-ci est un slave">
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               </div>


<!-- Footer -->
<footer class="page-footer bg-info mt-2 pt-0">

  <!-- Copyright -->
  <div class="footer-copyright text-center py-2"><span>© </span>
    <a href="https://ABLS-habitat.fr/">ABLS-Habitat.fr</a>
  </div>
  <!-- Copyright -->

</footer>

</div>

    <script src="/js/common.js" type="text/javascript"></script>
    <script src="/js/install/install.js" type="text/javascript"></script>
    <script src="https://code.jquery.com/jquery-3.5.1.min.js" crossorigin="anonymous"></script>
    <script src="https://cdn.jsdelivr.net/npm/popper.js@1.16.0/dist/umd/popper.min.js" integrity="sha384-Q6E9RHvbIyZFJoft+2mJbHaEWldlvI9IOYy5n3zV9zzTtmI3UksdQRVvoxMfooAo" crossorigin="anonymous"></script>
    <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/js/bootstrap.min.js" integrity="sha384-OgVRvuATP1z7JjHLkuOU7Xw704+h835Lr+6QL9UvYjZE3Ipu6Tp75j7Bh/kR0JKI" crossorigin="anonymous"></script>
  </body>
</html>

