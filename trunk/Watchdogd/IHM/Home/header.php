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
        <style>
        body { background-image: url('/img/fond_home.jpg');
               background-position: center;
               background-size: cover;
               background-repeat: no-repeat;
               background-attachment: fixed;
               height: 100%;
               background-color: rgba(30,28,56,1.0);
               padding-top: 90px;
               overflow-y: scroll;
             }

        .nav-link:hover { color: white !important;
                          background-color: #48BBC0;
                        }

        .wtd-cligno { animation-duration: 1.0s;
                      animation-name: wtdClignoFrames;
                      animation-iteration-count: infinite;
                      animation-fill-mode: backwards;
                      transition: none;
                    }
        @keyframes wtdClignoFrames
         {   0% { opacity: 1; }
            30% { opacity: 0; }
           100% { opacity: 1; }
         }

        .card { color: white; }


        .wtd-menu
          { border-radius: 20%;
            max-width: 48px;
            max-height: 48px;
            cursor: pointer;
          }

        .wtd-synoptique { border-radius: 20%;
                          width: auto;
                          height: auto;
                          max-height: 128px;
                          max-width: 128px;
                          cursor: pointer;
                        }

        .wtd-visuel { border-radius: 20%;
                      width: auto;
                      height: auto;
                      max-height: 96px;
                      max-width: 96px;
                      cursor: pointer;
                    }

	.wtd-courbe { background-color: white;
                      width: auto;
                      height: 300px;
                      cursor: pointer;
                    }

        .wtd-vignette
          { width: 24px;
            height: 24px;
          }

        @media (max-width: 768px)
         { .wtd-synoptique
            { max-height: 128px;
              max-width: 128px;
            }
           .wtd-vignette
            { max-width: 16px;
              max-height: 16px;
            }
           .wtd-menu
             { max-width: 32px;
               max-height: 32px;
             }
           .wtd-visuel
             { max-width: 32px;
               max-height: 32px;
             }
         }

        input:focus { outline: 0 0 0 0  !important;
                      box-shadow: 0 0 0 0 !important;
                    }


        .navbar { background-color: rgba(30,28,56,0.8);
                }

        .nav-link {
                  }

        .nav-link:hover { color: white !important;
                          background-color: #48BBC0;
                        }
    }
      </style>

    </head>

    <body class="bg-light">

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
	<nav class="navbar navbar-dark navbar-expand-md fixed-top shadow"> <!-- fixed-top -->
  <a class="navbar-brand" href="#" onclick="Change_page('1')">
    <img src="/img/syn_maison.png" alt="Accueil" class="wtd-menu">
  </a>

  <a class="nav-item"><img id="idMenuImgAccueil" src="" alt="Accueil" class="wtd-menu mr-1"></a>
  <a class="nav-item"><img id="idMasterVignetteActivite" class="wtd-menu mr-1" src=""></a>
  <a class="nav-item"><img id="idMasterVignetteSecuBien" class="wtd-menu mr-1" src=""></a>
  <a class="nav-item"><img id="idMasterVignetteSecuPers" class="wtd-menu mr-1" src=""></a>
  <ul class="navbar-nav">
    <a class="nav-link rounded d-none d-sm-inline" href="#"> <span id="idPageTitle">Loading...</span></a>
  </ul>

  <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbar-toggled" aria-controls="navbar-toggled" aria-expanded="false" aria-label="Toggle navigation">
    <span class="navbar-toggler-icon"></span>
  </button>

  <div class="collapse navbar-collapse" id="navbar-toggled">
    <ul class="navbar-nav  ml-auto">
      <a class="nav-link rounded" href="/tech/dashboard"><i class="fas fa-tachometer-alt"></i> <span>Mode Technicien</span></a>
      <li class="nav-item dropdown">
        <a class="nav-link rounded dropdown-toggle" href="#" id="navbarUSER" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          <i class="fas fa-user text-white"></i> <span id="idUsername">-</span>
        </a>

        <div class="dropdown-menu dropdown-menu-right" aria-labelledby="navbarUSER">
          <a class="dropdown-item" href="/home/user" id="idHrefUsername" href="#"><i class="fas fa-user text-info"></i> Mon Profil</a>
          <div class="dropdown-divider"></div>
          <a class="dropdown-item" href="/home/users"><i class="fas fa-users-cog text-info"></i> <span>Utilisateurs</span></a>
          <a class="dropdown-item" href="/tech/users_sessions"><i class="fas fa-list text-info"></i> <span>Sessions</span></a>
          <div class="dropdown-divider"></div>
          <a class="dropdown-item" href="#" onclick="Logout()"><i class="fas fa-sign-out-alt text-danger"></i> <span>Sortir</span> </a>
        </div>
      </li>
    </ul>
  </div>

</nav>
</header>

