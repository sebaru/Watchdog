<div class="container-fluid">

 <div class="row m-2">
   <h3><img src="/img/wago_750342.webp" style="width:80px" alt="Wago 750-342">Mapping des I/O Wago sur Modbus</h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Redirect('/tech/modbus')" class="btn btn-primary"><i class="fas fa-list"></i> Liste WAGO</button>
        <button type="button" onclick="Redirect('/tech/process')" class="btn btn-secondary"><i class="fas fa-microchip"></i> Processus</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Thread <a href="/tech/process">Modbus</a> is not running !
   </div>

          <ul class="nav nav-tabs" role="tablist">
            <li class="nav-item"><a class="nav-link active" data-toggle="tab" href="#idTabEntreeTor">
                  <img style="width: 30px" data-toggle="tooltip" title="Entrées TOR"
                       src="https://icons.abls-habitat.fr/assets/gif/Entree.png" />Entrées TOR</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabEntreeAna">
                  <img style="width: 30px" data-toggle="tooltip" title="Entrées ANA"
                       src="https://icons.abls-habitat.fr/assets/gif/Entree_Analogique.png" />Entrées ANA</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabSortieTor">
                  <img style="width: 30px" data-toggle="tooltip" title="Sorties TOR"
                       src="https://icons.abls-habitat.fr/assets/gif/Sortie.png" />Sorties TOR</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabSortieAna">
                  <img style="width: 30px" data-toggle="tooltip" title="Sorties ANA"
                       src="https://icons.abls-habitat.fr/assets/gif/Sortie_Analogique.png" />Sorties ANA</a></li>
          </ul>

<div class="tab-content">

<!----------------------------------------------------------------------------------------------------------------------------->
<div id="idTabEntreeTor" class="tab-pane fade in table-responsive mt-1" role="tabpanel">

 <div class="row m-2">
   <div class ="ml-auto btn-group">
        <button type="button" onclick="Show_Modal_Map_Edit_DI('-1')" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un mapping DI</button>
   </div>
 </div>
    <table id="idTableModbusMapDI" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
</div>

<!----------------------------------------------------------------------------------------------------------------------------->
<div id="idTabSortieTor" class="tab-pane fade in table-responsive mt-1" role="tabpanel">

 <div class="row m-2">
   <div class ="ml-auto btn-group">
        <button type="button" onclick="Show_Modal_Map_Edit_DO('-1')" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un mapping DO</button>
   </div>
 </div>

    <table id="idTableModbusMapDO" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
</div>
<!----------------------------------------------------------------------------------------------------------------------------->
<div id="idTabEntreeAna" class="tab-pane fade in table-responsive mt-1" role="tabpanel">

 <div class="row m-2">
   <div class ="ml-auto btn-group">
        <button type="button" onclick="Show_Modal_Map_Edit_AI('-1')" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un mapping AI</button>
   </div>
 </div>

    <table id="idTableModbusMapAI" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
</div>
<!----------------------------------------------------------------------------------------------------------------------------->
<div id="idTabSortieAna" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
 <div class="row m-2">
   <div class ="ml-auto btn-group">
        <button type="button" onclick="Show_Modal_Map_Edit_AO('-1')" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un mapping AI</button>
   </div>
 </div>
    <table id="idTableModbusMapAO" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
</div>

</div> <!-- TabContent -->
<!-- Container -->
</div>

<!------------------------------------------------- Modal Edit Digital Input -------------------------------------------------->
<div id="idModalEditDI" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalEditDITitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Wago Tech_ID</label>
						     <input id="idModalEditDIWagoTechID" type="text" class="form-control" placeholder="Module WAGO">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Wago DI</label>
						     <input id="idModalEditDIWagoTag" type="text" class="form-control" placeholder="DIxx">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Rechercher une Target</label>
						     <input id="idModalEditDIRechercherTechID" oninput="ModbusMap_Update_Choix_Tech_ID('idModalEditDI', 'DI')" type="text" class="col-9 form-control" placeholder="Rechercher un Tech_id">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Target TechID</label>
						     <select id="idModalEditDISelectTechID" onchange="ModbusMap_Update_Choix_Acronyme('idModalEditDI', 'DI')" class="col-9  custom-select"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Target Acronyme</label>
						     <select id="idModalEditDISelectAcronyme" class="col-9 custom-select"></select>
     					</div>
  					</div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalEditDIValider" type="button" class="btn btn-primary"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<!------------------------------------------------- Modal Edit Digital Output ------------------------------------------------->
<div id="idModalEditDO" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalEditDOTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Wago Tech_ID</label>
						     <input id="idModalEditDOWagoTechID" type="text" class="form-control" placeholder="Module WAGO">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Wago DO</label>
						     <input id="idModalEditDOWagoTag" type="text" class="form-control" placeholder="DOxx">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Rechercher une Target</label>
						     <input id="idModalEditDORechercherTechID" oninput="ModbusMap_Update_Choix_Tech_ID('idModalEditDO', 'DO')" type="text" class="col-9 form-control" placeholder="Rechercher un Tech_id">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Target TechID</label>
						     <select id="idModalEditDOSelectTechID" onchange="ModbusMap_Update_Choix_Acronyme('idModalEditDO', 'DO')" class="col-9 custom-select"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Target Acronyme</label>
						     <select id="idModalEditDOSelectAcronyme" class="col-9 custom-select"></select>
     					</div>
  					</div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalEditDOValider" type="button" class="btn btn-primary"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<!------------------------------------------------- Modal Edit Analog Input --------------------------------------------------->
<div id="idModalEditAI" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalEditAITitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Wago Tech_ID</label>
						     <input id="idModalEditAIWagoTechID" type="text" class="form-control" placeholder="Module WAGO">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Wago AI</label>
						     <input id="idModalEditAIWagoTag" type="text" class="form-control" placeholder="AIxx">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Type de borne</label>
						     <select id="idModalEditAIType" class="custom-select">
             <option value="3">4/20 mA 750455</option>
             <option value="4">Pt-100 750461</option>
           </select>
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Unité</label>
						     <input id="idModalEditAIUnite" type="text" class="form-control" placeholder="°C, km/h, ...">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Minimum</label>
						     <input id="idModalEditAIMin" type="number" class="form-control" placeholder="Valeur Min">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Maximum</label>
						     <input id="idModalEditAIMax" type="number" class="form-control" placeholder="Valeur Max">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Rechercher une Target</label>
						     <input id="idModalEditAIRechercherTechID" oninput="ModbusMap_Update_Choix_Tech_ID('idModalEditAI', 'AI')" type="text" class="col-9 form-control" placeholder="Rechercher un Tech_id">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Target TechID</label>
						     <select id="idModalEditAISelectTechID" onchange="ModbusMap_Update_Choix_Acronyme('idModalEditAI', 'AI')" class="col-9 custom-select"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Target Acronyme</label>
						     <select id="idModalEditAISelectAcronyme" class="col-9 custom-select"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Question Vocale</label>
						     <input id="idModalEditAIMapQuestionVoc" type="text" class="form-control" placeholder="Question vocale associée">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Réponse Vocale</label>
						     <input id="idModalEditAIMapReponseVoc" type="text" class="form-control" placeholder="Réponse vocale associée">
     					</div>
          <small>$1 est la valeur dynamique du bit interne</small>
      </div>

      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalEditAIValider" type="button" class="btn btn-primary"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>
<!------------------------------------------------- Modal Edit Analog Output -------------------------------------------------->
<div id="idModalEditAO" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalEditAOTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Wago Tech_ID</label>
						     <input id="idModalEditAOWagoTechID" type="text" class="form-control" placeholder="Module WAGO">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Wago AO</label>
						     <input id="idModalEditAOWagoTag" type="text" class="form-control" placeholder="AOxx">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Type de Borne</label>
						     <select id="idModalEditAOType" class="custom-select">
             <option value="0">a definir</option>
             <option value="1">a definir</option>
           </select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Unité</label>
						     <input id="idModalEditAOUnite" type="text" class="form-control" placeholder="°C, km/h, ...">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Min</label>
						     <input id="idModalEditAOMin" type="number" class="form-control" placeholder="Valeur Min">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Max</label>
						     <input id="idModalEditAOMax" type="number" class="form-control" placeholder="Valeur Max">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Rechercher une Target</label>
						     <input id="idModalEditAORechercherTechID" oninput="ModbusMap_Update_Choix_Tech_ID('idModalEditAO', 'AO')" type="text" class="col-9 form-control" placeholder="Rechercher un Tech_id">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Target TechID</label>
						     <select id="idModalEditAOSelectTechID" onchange="ModbusMap_Update_Choix_Acronyme('idModalEditAO', 'AO')" class="col-9 custom-select"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Target Acronyme</label>
						     <select id="idModalEditAOSelectAcronyme" class="col-9 custom-select"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Question Vocale</label>
						     <input id="idModalEditMapQuestionVoc" type="text" class="form-control" placeholder="Question vocale associée">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Réponse Vocale</label>
						     <input id="idModalEditMapReponseVoc" type="text" class="form-control" placeholder="Réponse vocale associée">
     					</div>
          <small>$1 est la valeur dynamique du bit interne</small>
  					</div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalEditValider" type="button" class="btn btn-primary"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>
<!------------------------------------------------- Modal Delete Mapping ------------------------------------------------------>
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

<script src="/js/tech/modbus_map.js" type="text/javascript"></script>
