<div class="container">

 <div class="row m-2">
   <h3><i class="fas fa-user text-primary"></i> Mon profil</h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="User_sauver_parametre()" class="btn btn-outline-success"><i class="fas fa-save"></i> Sauvegarder</button>
        <button type="button" onclick="Redirect('/tech/users')" class="btn btn-primary"><i class="fas fa-users"></i> Tous les utilisateurs</button>
         <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Username</label>
						     <input id="idUserUsername" type="text" disabled class="form-control" placeholder="Mon login">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Adresse mail</label>
						     <input id="idUserEmail" type="text" class="form-control" placeholder="Mon adresse mail">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Adresse messagerie instantanée</label>
						     <input id="idUserXMPP" type="text" class="form-control" placeholder="Mon adresse de messagerie instantanée">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Téléphone</label>
						     <input id="idUserPhone" type="text" class="form-control" placeholder="Mon numéro de téléphone">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Commentaire</label>
						     <input id="idUserComment" type="text" class="form-control" placeholder="Qui suis-je ?">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Notification ?</label>
						     <input id="idUserNotification" type="checkbox" class="form-check" placeholder="Suis-je notifié ?">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Nouveau Mot de passe</label>
						     <input id="idUserPassword1" type="password" class="form-control" placeholder="Mon nouveau mot de passe">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Controle du nouveau<br>Mot de passe</label>
						     <input id="idUserPassword2" type="password" class="form-control" placeholder="Controle du nouveau mot de passe">
     					</div>
  					</div>

<!-- Container -->
</div>

<script src="/js/tech/user.js" type="text/javascript"></script>
