<div class="container">

 <div class="row m-2">
   <div class="col-auto"><h3><i class="fas fa-crown text-danger"></i> Statut du Processus MSRV sur</h3> </div>
  	<div class="col-auto">
     <select id='idTargetInstance' onchange="MSRV_Load_config()" class="custom-select border-info"></select>
   </div>

 </div>

<hr>
       <div class="alert alert-danger">Attention, la modification des données ci dessous peuvent déconnecter l'instance !</div>
       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-danger text-right">Instance is Master</label>
						       <select id="idMSRVIsMaster" class="custom-select border-info">
               <option value="true" selected>Oui</option>
               <option value="false">Non</option>
             </select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-danger text-right">Master_Host</label>
						     <input id="idMSRVMasterHost" type="text" class="form-control" placeholder="Hostname du master si cette instance est un slave">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Description de l'instance</label>
						     <input id="idMSRVDescription" type="text" class="form-control" placeholder="Nom de l'habitat par exemple">
     					</div>
  					</div>

   <div class ="row">
     <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="MSRV_Sauver_parametre()" class="btn btn-outline-danger"><i class="fas fa-save"></i> Sauvegarder</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
     </div>
   </div>
<!-- Container -->
</div>


<script src="/js/tech/msrv.js" type="text/javascript"></script>
