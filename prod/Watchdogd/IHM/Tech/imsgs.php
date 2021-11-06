<div class="container">

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Thread <a href="/tech/process">IMSGS</a> is not running !
   </div>

 <div class="row m-2">
   <h3><img src="/img/imsgs.png" style="width:80px" alt="Configuration IMSGS">Configuration Messagerie Instantanée</h3>

   <div class ="ml-auto btn-group align-items-center">
        <button type="button" onclick="Redirect('/tech/command_text')" class="btn btn-primary"><i class="fas fa-directions"></i> Map</button>
        <button type="button" onclick="IMSGS_Load_config()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Choix de l'instance</label>
           <select id="idTargetInstance" class="custom-select border-info" onchange="IMSGS_Load_config()"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">IMSG Tech_ID</label>
						     <input id="idIMSGSTechID" type="text" class="form-control" maxlength="32" placeholder="Tech_ID du Thread">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">XMPP JabberID</label>
						     <div class="input-group-prepend">
             <div class="input-group-text">@</div>
           </div>
           <input id="idIMSGSJabberID" type="email" class="form-control" placeholder="username@server.tld">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">XMPP Password</label>
						     <input id="idIMSGSPassword" type="password" class="form-control" placeholder="">
     					</div>
  					</div>

   <div class="row m-2">
     <div class="ml-auto">
        <button type="button" onclick="IMSGS_Test()" class="btn btn-outline-info"><i class="fas fa-question"></i> Test envoi IMSGS</button>
        <button type="button" onclick="IMSGS_Sauver_parametre()" class="btn btn-outline-success"><i class="fas fa-save"></i> Sauvegarder</button>
			  </div>
			</div>

<!-- Container -->
</div>

<script src="/js/tech/imsgs.js" type="text/javascript"></script>
