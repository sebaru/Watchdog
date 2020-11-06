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
						     <label class="col-4 col-form-label text-right">Log Level</label>
						     <select id="idMaintenanceLogLevel" class="custom-select col-3" onchange="Maintenance_Set_Log_Level()">
             <option value="LOG_DEBUG">Debug</option>
             <option value="LOG_INFO">Info</option>
             <option value="LOG_NOTICE">Notice</option>
             <option value="LOG_WARNING">Warning</option>
             <option value="LOG_ERR">Error</option>
           </select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-4 col-form-label text-right">Compil All D.L.S Plugins</label>
           <button type="button" onclick="Maintenance_Compiler_tous_dls()" class="col-3 btn btn-outline-warning"><i class="fas fa-code"></i> Tout Compiler</button>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-4 col-form-label text-right">Resetter le système</label>
           <button type="button" onclick="Maintenance_Reset_Instance()" class="col-3 btn btn-outline-danger"><i class="fas fa-power-off"></i> Reset Système</button>
     					</div>
  					</div>

<!-- Container -->
</div>

<script src="/js/tech/maintenance.js" type="text/javascript"></script>
