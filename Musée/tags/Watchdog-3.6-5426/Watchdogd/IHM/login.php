<!doctype html>
<html lang="fr">
    <head>
        <meta charset="utf-8">
        <title>Chez Moi !</title>

        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
        <meta name="google" content="notranslate">
        <meta name="robots" content="noindex, nofollow">
        <link rel="icon" href="/img/logo.svg">
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

<div class="container">
	<div class="d-flex justify-content-center h-100">
		<div class="card">
			<div class="card-header">
     <div class="row">
       <img class="col-4" src="/img/logo.svg" alt="Watchdog Logo">
       <h4 class="col-8 align-self-center text-center wtd_title">Connexion</h4>
     </div>
			</div>
			<div class="card-body">

					<div class="input-group form-group">
						<div class="input-group-prepend">
							<span class="input-group-text"><i class="fas fa-user"></i></span>
						</div>
						<input id="username" name="username" type="text" class="form-control" placeholder="username">
					</div>

					<div class="input-group form-group">
						<div class="input-group-prepend">
							<span class="input-group-text"><i class="fas fa-key"></i></span>
						</div>
						<input id="password" name="password" type="password" class="form-control" placeholder="password">
					</div>

					<div class="form-group">
						<button onclick="Send_credential()" class="btn float-right btn-primary">Entrer</button>
					</div>
			</div>
			<div class="card-footer">
				<div class="d-flex justify-content-center">
					<a class="card-link" href="https://wiki.abls-habitat.fr"><i class="fas fa-info-circle"></i> Go to ABLS Wiki !</a>
				</div>
			</div>
		</div>
	</div>

</div>

    <script src="https://code.jquery.com/jquery-3.5.1.slim.min.js" integrity="sha384-DfXdz2htPH0lsSSs5nCTpuj/zy4C+OGpamoFVy38MVBnE+IbbVYUew+OrCXaRkfj" crossorigin="anonymous"></script>
    <script src="https://cdn.jsdelivr.net/npm/popper.js@1.16.0/dist/umd/popper.min.js" integrity="sha384-Q6E9RHvbIyZFJoft+2mJbHaEWldlvI9IOYy5n3zV9zzTtmI3UksdQRVvoxMfooAo" crossorigin="anonymous"></script>
    <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/js/bootstrap.min.js" integrity="sha384-OgVRvuATP1z7JjHLkuOU7Xw704+h835Lr+6QL9UvYjZE3Ipu6Tp75j7Bh/kR0JKI" crossorigin="anonymous"></script>
    <script src="/js/login.js" type="text/javascript"></script>
    <script src="/js/common.js" type="text/javascript"></script>
  </body>
</html>
