<div class="container">

 <div class="row m-2">
   <h3><img src="/img/sms.jpg" style="width:80px" alt="Commandes SMS">Configuration des GSM</h3>

   <div class ="ml-auto btn-group align-items-center">
        <button type="button" onclick="Redirect('/tech/smsg_map')" class="btn btn-primary"><i class="fas fa-directions"></i> Map</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

<div class="row m-2">
 </div>

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Thread <a href="/tech/process">SMSG</a> is not running !
   </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Choix du GSM</label>
           <select id="idTargetInstance" class="custom-select border-info" onchanged="SMS_Load_config()"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">GSM Tech_ID</label>
						     <input id="idGSMTechID" type="text" class="form-control" placeholder="Tech_ID du GSM">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">GSM Description</label>
						     <input id="idGSMDescription" type="text" class="form-control" placeholder="Description du téléphone et/ou sa position">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">GSM Communication</label>
						     <input id="idGSMComm" type="text" class="form-control" placeholder="Communication" disabled>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Nombre de SMS envoyés</label>
						     <input id="idGSMNbrSMS" disabled type="number" class="form-control" placeholder="Nombre de SMS envoyés">
     					</div>
  					</div>

   <div class ="row">
     <div class ="ml-auto">
        <button type="button" onclick="SMS_test('gsm')" class="btn btn-outline-info"><i class="fas fa-question"></i> Test envoi GSM</button>
        <button type="button" onclick="SMS_Sauver_parametre()" class="btn btn-outline-success"><i class="fas fa-save"></i> Sauvegarder</button>
			  </div>
			</div>

<hr>
   <h3><img src="/img/sms.jpg" style="width:80px" alt="Commandes SMS">Backup OVH</h3>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">OVH Service Name</label>
						     <input id="idGSMOVHServiceName" type="text" class="form-control" placeholder="OVH Service Name (sms-xxxxxx-1)">
           <a target="_blank" href="https://eu.api.ovh.com/createToken/index.cgi?GET=/sms&GET=/sms/*/jobs&POST=/sms/*/jobs" class="col-2 col-form-label">Creer un token</a>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">OVH Application Key</label>
						     <input id="idGSMOVHApplicationKey" type="text" class="form-control" placeholder="OVH Application Key">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">OVH Application Secret</label>
						     <input id="idGSMOVHApplicationSecret" type="text" class="form-control" placeholder="OVH Application Secret">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">OVH Consumer Key</label>
						     <input id="idGSMOVHConsumerKey" type="text" class="form-control" placeholder="OVH Consumer Key">
     					</div>
  					</div>

   <div class ="row">
     <div class ="ml-auto">
        <button type="button" onclick="SMS_test('ovh')" class="btn btn-outline-info"><i class="fas fa-question"></i> Test envoi OVH</button>
        <button type="button" onclick="SMS_Sauver_parametre()" class="btn btn-outline-success"><i class="fas fa-save"></i> Sauvegarder</button>
			  </div>
			</div>

<!-- Container -->
</div>


<script src="/js/tech/smsg.js" type="text/javascript"></script>
