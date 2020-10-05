<div class="container-fluid">

 <div class="row m-2">
   <h3><img src="/img/sms.jpg" style="width:80px" alt="Commandes SMS">Mapping des Entrées SMS via GSM</h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Show_Modal_Map_Edit_DI('-1')" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un mapping GSM</button>
        <button type="button" onclick="Redirect('/tech/smsg')" class="btn btn-primary"><i class="fas fa-list"></i> Config GSM</button>
        <button type="button" onclick="Redirect('/tech/process')" class="btn btn-secondary"><i class="fas fa-microchip"></i> Processus</button>
   </div>
 </div>

<hr>

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Thread "GSM" is not running !
   </div>

<!----------------------------------------------------------------------------------------------------------------------------->

    <table id="idTableGSM" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>

<!------------------------------------------------- Modal Edit Digital Input -------------------------------------------------->
<div id="idModalEditDI" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalEditTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="row">
        <div class="col form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">GSM Tech_ID</span>
						     </div>
						     <select id="idModalEditGSMTechID" class="custom-select"></select>
     					</div>
   					</div>
  					</div>

       <div class="row">
        <div class="col form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Commande SMS</span>
						     </div>
						     <input id="idModalEditGSMTag" type="text" class="form-control" placeholder="Commande SMS">
     					</div>
   					</div>
  					</div>

       <div class="row">
        <div class="col form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Target Tech_ID</span>
						     </div>
						     <input id="idModalEditTechID" oninput="Modal_Edit_Input_Changed('idModalEditDI')" type="text" class="form-control" placeholder="Tech_id du bit cible">
     					</div>
          <small id="idModalEditTechIDPropose"></small>
   					</div>
  					</div>

       <div class="row">
        <div class="col form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Target Acronyme</span>
						     </div>
						     <input id="idModalEditAcronyme" oninput="Modal_Edit_Input_Changed('idModalEditDI')" type="text" class="form-control" placeholder="Acronyme cible">
     					</div>
          <small id="idModalEditAcronymePropose"></small>
        </div>
  					</div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalEditValider" type="button" class="btn btn-primary"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>


<script src="/js/tech/smsg_map.js" type="text/javascript"></script>
