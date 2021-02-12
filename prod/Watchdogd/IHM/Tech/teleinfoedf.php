<div class="container">

 <div class="row m-2">
   <h3><img src="/img/linky.jpg" style="width:80px" alt="Teleinfo E.D.F"> Configuration des modules Téléinfo EDF</h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="TINFO_Load_config()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Thread <a href="/tech/process">TELEINFOEDF</a> is not running !
   </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Choix du compteur</label>
           <select id="idTargetInstance" class="custom-select border-info" onchanged="TINFO_Load_config()"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Tech_ID</label>
						     <input id="idTINFOTechID" type="text" class="form-control" placeholder="Tech_ID du compteur EDF">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Port</label>
						     <input id="idTINFOPort" type="text" class="form-control" placeholder="Port de communication">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Description</label>
						     <input id="idTINFODescription" type="text" class="form-control" placeholder="Ou est le compteur ?">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Statut Communication</label>
						     <input disabled id="idTINFOComm" type="text" class="form-control" placeholder="Communication">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Mode</label>
						     <input disabled id="idTINFOMode" type="text" class="form-control" placeholder="Mode du module Teleinfo USB">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Retry in</label>
						     <input disabled id="idTINFORetry" type="text" class="form-control" placeholder="Délai avant nouvelle tentative de connexion">
						     <div class="input-group-append">
							     <span class="input-group-text">secondes</span>
						     </div>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Last View</label>
						     <input disabled id="idTINFOLastView" type="text" class="form-control" placeholder="Date de last view">
						     <div class="input-group-append">
							     <span class="input-group-text">secondes</span>
						     </div>
     					</div>
  					</div>

   <div class="row m-2">
     <div class="ml-auto">
        <button type="button" onclick="TINFO_Sauver_parametre()" class="btn btn-outline-success"><i class="fas fa-save"></i> Sauvegarder</button>
			  </div>
			</div>

<!-- Container -->
</div>


<script src="/js/tech/teleinfoedf.js" type="text/javascript"></script>
