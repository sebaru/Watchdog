<div class="container-fluid">

 <div class="row m-2">
 <h3>Statut du GSM sur <strong id='idTitleInstance'></strong></h3>

   <div class ="ml-auto btn-group">
        <button type="button" onclick="GSM_Sauver_parametre()" class="btn btn-outline-success"><i class="fas fa-save"></i> Sauvegarder</button>
        <button type="button" onclick="Redirect('/tech/smsg_map')" class="btn btn-primary"><i class="fas fa-directions"></i> Map</button>
        <button type="button" onclick="Redirect('/tech/process')" class="btn btn-secondary"><i class="fas fa-microchip"></i> Processus</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Thread "GSM" is not running !
   </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">GSM Tech_ID</label>
						     <input id="idGSMTechID" type="text" class="form-control" placeholder="Tech_ID du GSM">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Description</label>
						     <input id="idGSMDescription" type="text" class="form-control" placeholder="Description du téléphone et/ou sa position">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Clef API SMSBOX</label>
						     <input id="idGSMAPIKey" type="text" class="form-control" placeholder="Clef API SMSBox">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Statut Communication</label>
						     <input id="idGSMComm" type="text" class="form-control" placeholder="Communication">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Nombre de SMS envoyés</label>
						     <input id="idGSMNbrSMS" disabled type="number" class="form-control" placeholder="Nombre de SMS envoyés">
     					</div>
  					</div>

<!-- Container -->
</div>


<script src="/js/tech/smsg.js" type="text/javascript"></script>
