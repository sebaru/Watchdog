<div class="container-fluid">

 <div class="row m-2">
 <h3>Mapping des I/O Wago sur Modbus</h3>

   <div class ="ml-auto btn-group">
        <button type="button" onclick="Show_Modal_ModbusMap_Add()"class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter</button>
        <button type="button" onclick="Go_to_modbus_run()"class="btn btn-info"><i class="fas fa-eye"></i> RUN</button>
        <button type="button" onclick="window.location='/tech/modbus'" class="btn btn-secondary"><i class="fas fa-list"></i> Retour</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>
<!----------------------------------------------------------------------------------------------------------------------------->
 <hr>
      <h4><img style="width: 30px" data-toggle="tooltip" title="Entrées TOR"
                       src="https://icons.abls-habitat.fr/assets/gif/Entree.png" />Entrées TOR</h4>

    <table id="idTableModbusMapDI" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
<!----------------------------------------------------------------------------------------------------------------------------->
 <hr>
      <h4><img style="width: 30px" data-toggle="tooltip" title="Sorties TOR"
                       src="https://icons.abls-habitat.fr/assets/gif/Sortie.png" />Sorties TOR</h4>

    <table id="idTableModbusMapDO" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
<!----------------------------------------------------------------------------------------------------------------------------->
 <hr>
      <h4><img style="width: 30px" data-toggle="tooltip" title="Entrées ANA"
                       src="https://icons.abls-habitat.fr/assets/gif/Entree_Analogique.png" />Entrées ANA</h4>

    <table id="idTableModbusMapAI" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
<!----------------------------------------------------------------------------------------------------------------------------->
 <hr>
      <h4><img style="width: 30px" data-toggle="tooltip" title="Sorties ANA"
                       src="https://icons.abls-habitat.fr/assets/gif/Sortie_Analogique.png" />Sorties ANA</h4>

    <table id="idTableModbusMapAO" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>

<div id="idModalEditDI" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalEditTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Wago References</span>
						     </div>
						     <input id="idModalEditWagoRef" type="text" class="form-control" placeholder="Reference WAGO">
     					</div>
   					</div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Target Tech_ID</span>
						     </div>
						     <input id="idModalEditTechID" oninput="Modal_Edit_Input_Changed('idModalEditDI')" type="text" class="form-control" placeholder="Tech_id du bit cible">
     					</div>
          <small id="idModalEditTechIDPropose"></small>
   					</div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Target Acronyme</span>
						     </div>
						     <input id="idModalEditAcronyme" oninput="Modal_Edit_Input_Changed('idModalEditDI')" type="text" class="form-control" placeholder="Acronyme cible">
     					</div>
          <small id="idModalEditAcronymePropose"></small>
        </div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalEditValider" type="button" class="btn btn-primary"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<div id="idModalEditDO" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalEditTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Wago References</span>
						     </div>
						     <input id="idModalEditWagoRef" type="text" class="form-control" placeholder="Reference WAGO">
     					</div>
   					</div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Target Tech_ID</span>
						     </div>
						     <input id="idModalEditTechID" oninput="Modal_Edit_Input_Changed('idModalEditDO')" type="text" class="form-control" placeholder="Tech_id du bit cible">
     					</div>
          <small id="idModalEditTechIDPropose"></small>
   					</div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Target Acronyme</span>
						     </div>
						     <input id="idModalEditAcronyme" oninput="Modal_Edit_Input_Changed('idModalEditDO')" type="text" class="form-control" placeholder="Acronyme cible">
     					</div>
          <small id="idModalEditAcronymePropose"></small>
        </div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalEditValider" type="button" class="btn btn-primary"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<div id="idModalModbusDel" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-danger text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-trash"></i> <span id="idModalModbusDelTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">
        <p id="idModalModbusDelMessage"></p>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalModbusDelValider" type="button" class="btn btn-danger" data-dismiss="modal"><i class="fas fa-trash"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="<?php echo base_url('js/modbus_map.js')?>" type="text/javascript"></script>
