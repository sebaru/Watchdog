<div class="container-fluid">

 <div class="row m-2">
   <h3><i class="fas fa-users text-primary"></i> Gestion des Utilisateurs</h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Users_Show_add_user()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un utilisateur</button>
        <button type="button" onclick="Redirect('/tech/user')" class="btn btn-primary"><i class="fas fa-user"></i> Mon profil</button>
         <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

    <table id="idTableUsers" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>

<div id="idModalUserAdd" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> Ajouter un utilisateur</h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Username</span>
						     </div>
						     <input id="idModalUserNewUsername" type="text" class="form-control" placeholder="Nom du nouvel utilisateur">
     					</div>
   					</div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Password</span>
						     </div>
						     <input id="idModalUserNewPassword" type="password" class="form-control" placeholder="Mot de passe du nouvel utilisateur">
     					</div>
   					</div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">E-Mail</span>
						     </div>
						     <input id="idModalUserNewEmail" type="email" class="form-control" placeholder="E-mail du nouvel utilisateur">
     					</div>
   					</div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button onclick="Users_Valider_add_user()" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-plus"></i> Ajouter</button>
      </div>
    </div>
  </div>
</div>


<script src="/js/tech/users.js" type="text/javascript"></script>
