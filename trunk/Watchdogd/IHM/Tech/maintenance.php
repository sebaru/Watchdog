<div class="container">

 <div class="row m-2">
   <h3><i class="fas fa-wrech text-secondary"></i> Maintenance de l'instance <strong id="idTitleInstance"></strong></h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Redirect('/tech/process')" class="btn btn-secondary"><i class="fas fa-microchip"></i> Processus</button>
   </div>
 </div>

<hr>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-6 col-form-label text-right">Compil All D.L.S Plugins</label>
           <button type="button" onclick="Maintenance_Compiler_tous_dls()" class="btn btn-warning"><i class="fas fa-code"></i> Tout Compiler</button>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-6 col-form-label text-right">Resetter le système</label>
           <button type="button" onclick="Maintenance_Reset_Instance()" class="btn btn-danger"><i class="fas fa-restart"></i> Reset Système</button>
     					</div>
  					</div>

<!-- Container -->
</div>

<script src="/js/tech/maintenance.js" type="text/javascript"></script>
