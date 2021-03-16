<div class="container">

 <div class="row m-2">
   <h3><img src="/img/meteo.svg" style="width:80px" alt="Récupération Météo">Configuration de la météo</h3>

   <div class ="ml-auto btn-group align-items-center">
        <button type="button" onclick="Meteo_Load_config()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

<div class="row m-2">
 </div>

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Thread <a href="/tech/process">Météo</a> is not running !
   </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Tech_ID</label>
						     <input id="idMeteoTechID" type="text" class="form-control" placeholder="Tech_ID du Thread Meteo">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Description</label>
						     <input id="idMeteoDescription" type="text" class="form-control" placeholder="Description de la position">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Token API</label>
						     <input id="idMeteoToken" type="text" class="form-control" placeholder="Token API Meteo Concept">
           <a target="_blank" href="https://api.meteo-concept.com/login" class="col-3 col-sm-2 col-form-label">Creer un token</a>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Code Insee Commune</label>
						     <input id="idMeteoCodeInsee" type="text" class="form-control" placeholder="Code Insee de la commune">
     					</div>
  					</div>

   <div class="row m-2">
     <div class="ml-auto">
        <button type="button" onclick="Meteo_test()" class="btn btn-outline-info"><i class="fas fa-question"></i> Test Reception</button>
        <button type="button" onclick="Meteo_Sauver_parametre()" class="btn btn-outline-success"><i class="fas fa-save"></i> Sauvegarder</button>
			  </div>
			</div>

<!-- Container -->
</div>


<script src="/js/tech/meteo.js" type="text/javascript"></script>
