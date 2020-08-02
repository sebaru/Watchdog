<!doctype html>
<html lang="fr">
    <head>
        <meta charset="utf-8">
        <title><?php echo $title?></title>

        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
        <meta name="google" content="notranslate">
        <meta name="robots" content="noindex, nofollow">
        <link rel="icon" href="<?php echo base_url('/logo.svg')?>">
        <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css" integrity="sha384-9aIt2nRpC12Uk9gS9baDl411NQApFmC26EwAOH8WgZl5MYYxFfc+NcPb1dKGj7Sk" crossorigin="anonymous">
        <script src="https://kit.fontawesome.com/1ca1f7ba56.js" crossorigin="anonymous"></script>
        <style>
        html,body { background-image: url('http://getwallpapers.com/wallpaper/full/9/d/c/679692.jpg');
                    background-size: cover;
                    background-repeat: no-repeat;
                    height: 100%;
                    background-color: rgba(30,28,56,1.0)
                  }

        .container { height: 100%;
                   }

        .card { margin-top: auto;
                margin-bottom: auto;
                background-color: rgba(30,28,56,0.5) !important;
              }

        .wtd_title { color: white; }

        .card-link { color: white; }
        .input-group-prepend span { width: 50px;
                                    background-color: #F1E413;
                                    color: black;
                                    border:0 !important;
                                  }

        input:focus { outline: 0 0 0 0  !important;
                      box-shadow: 0 0 0 0 !important;
                    }

        .wtd_login_btn { color: white;
                         background-color: #1E1C38;
                         width: 100px;
                       }

        .wtd_login_btn:hover { color: white;
                               background-color: #48BBC0;
                             }

      </style>

    </head>
    <body class="">

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
