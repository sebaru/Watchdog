<div class="container-fluid">

 <div class="row m-2">
   <h3><i class="fas fa-crown text-danger"></i> Statut du Processus MSRV sur <strong id='idTitleInstance'></strong></h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="MSRV_Sauver_parametre()" class="btn btn-outline-success"><i class="fas fa-save"></i> Sauvegarder</button>
        <button type="button" onclick="Redirect('/tech/process')" class="btn btn-secondary"><i class="fas fa-microchip"></i> Processus</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Instance is Master</label>
						       <select id="idMSRVIsMaster" class="custom-select">
               <option value="true" selected>Oui</option>
               <option value="false">Non</option>
             </select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Master_Host</label>
						     <input id="idMSRVMasterHost" type="text" class="form-control" placeholder="Hostname du master si cette instance est un slave">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Description de l'instance</label>
						     <input id="idMSRVDescription" type="text" class="form-control" placeholder="Nom de l'habitat par exemple">
     					</div>
  					</div>

<!-- Container -->
</div>


<script src="/js/tech/msrv.js" type="text/javascript"></script>
