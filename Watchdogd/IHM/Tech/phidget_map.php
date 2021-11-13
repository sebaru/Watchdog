<div class="container-fluid">

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Le Thread <a href="/tech/process">Phidget</a> ne tourne pas !
   </div>

 <div class="row m-2">
   <h3><img src="/img/phidget_hub5000.jpg" style="width:80px" alt="Phidget HB5000">Mapping des I/O Phidget</h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Redirect('/tech/phidget')" class="btn btn-primary"><i class="fas fa-list"></i> Liste des Hub Phidget</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

          <ul class="nav nav-tabs" role="tablist">
            <li class="nav-item"><a class="nav-link active" data-toggle="tab" href="#idTabEntreeTor">
                  <img style="width: 30px" data-toggle="tooltip" title="Entrées TOR"
                       src="/img/Entree.png" />Entrées TOR</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabEntreeAna">
                  <img style="width: 30px" data-toggle="tooltip" title="Entrées ANA"
                       src="/img/Entree_Analogique.png" />Entrées ANA</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabSortieTor">
                  <img style="width: 30px" data-toggle="tooltip" title="Sorties TOR"
                       src="/img/Sortie.png" />Sorties TOR</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabSortieAna">
                  <img style="width: 30px" data-toggle="tooltip" title="Sorties ANA"
                       src="/img/Sortie_Analogique.png" />Sorties ANA</a></li>
          </ul>

<div class="tab-content">

<!----------------------------------------------------------------------------------------------------------------------------->
<div id="idTabEntreeTor" class="tab-pane fade in table-responsive mt-1" role="tabpanel">

 <div class="row m-2">
   <div class ="ml-auto btn-group">
        <button type="button" onclick="Show_Modal_Map_Edit_DI('-1')" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un mapping DI</button>
   </div>
 </div>
    <table id="idTablePhidgetMapDI" class="table table-striped table-bordered table-hover w-100">
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

    <table id="idTablePhidgetMapDO" class="table table-striped table-bordered table-hover w-100">
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

    <table id="idTablePhidgetMapAI" class="table table-striped table-bordered table-hover w-100">
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
        <button type="button" onclick="Show_Modal_Map_Edit_AO('-1')" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un mapping AO</button>
   </div>
 </div>
    <table id="idTablePhidgetMapAO" class="table table-striped table-bordered table-hover w-100">
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
						     <label class="col-5 col-sm-4 col-form-label text-right">Phidget Hub</label>
						     <select id="idModalEditDIHub" class="custom-select border-info" placeholder="Nom du Hub associé"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">N° de port du HUB</label>
						     <input id="idModalEditDIPort" type="number" required min=0 max=6 class="form-control" placeholder="numéro">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Classe du Capteur</label>
						     <select id="idModalEditDICapteur" class="custom-select border-info" placeholder="Classe du capteur">
             <option value="DIGITAL-INPUT">Digital Input</option>
           </select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Rechercher une Target</label>
						     <input id="idModalEditDIRechercherTechID" oninput="PhidgetMap_Update_Choix_Tech_ID('idModalEditDI', 'DI')" type="text" class="col-9 form-control" placeholder="Rechercher un Tech_id">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Target TechID</label>
						     <select id="idModalEditDISelectTechID" required onchange="PhidgetMap_Update_Choix_Acronyme('idModalEditDI', 'DI')" class="col-9 custom-select border-info"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Target Acronyme</label>
						     <select id="idModalEditDISelectAcronyme" required class="col-9 custom-select border-info"></select>
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

<!------------------------------------------------- Modal Edit Digital Output-------------------------------------------------->
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
						     <label class="col-5 col-sm-4 col-form-label text-right">Phidget Hub</label>
						     <select id="idModalEditDOHub" class="custom-select border-info" placeholder="Nom du Hub associé"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">N° de port du HUB</label>
						     <input id="idModalEditDOPort" type="number" required min=0 max=6 class="form-control" placeholder="numéro">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Classe</label>
						     <select id="idModalEditDOCapteur" class="custom-select border-info" placeholder="Classe du capteur">
             <option value="REL2001_0">REL2001 - Digital Output</option>
           </select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Rechercher une Target</label>
						     <input id="idModalEditDORechercherTechID" oninput="PhidgetMap_Update_Choix_Tech_ID('idModalEditDO', 'DO')" type="text" class="col-9 form-control" placeholder="Rechercher un Tech_id">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Target TechID</label>
						     <select id="idModalEditDOSelectTechID" required onchange="PhidgetMap_Update_Choix_Acronyme('idModalEditDO', 'DO')" class="col-9 custom-select border-info"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Target Acronyme</label>
						     <select id="idModalEditDOSelectAcronyme" required class="col-9 custom-select border-info"></select>
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
						     <label class="col-5 col-sm-4 col-form-label text-right">Phidget Hub</label>
						     <select id="idModalEditAIHub" class="custom-select border-info" placeholder="Nom du Hub associé"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">N° de port du HUB</label>
						     <input id="idModalEditAIPort" type="number" required min=0 max=6 class="form-control" placeholder="Numéro du port">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Classe du Capteur</label>
						     <select id="idModalEditAICapteur" class="custom-select border-info" placeholder="Classe du capteur">
             <option value="AC-CURRENT-10A">AC Current 0-10 A</option>
             <option value="AC-CURRENT-25A">AC Current 0-25 A</option>
             <option value="AC-CURRENT-50A">AC Current 0-50 A</option>
             <option value="AC-CURRENT-100A">AC Current 0-100 A</option>
             <option value="ADP1000-ORP" selected>ADP1000 - ORP</option>
             <option value="ADP1000-PH">ADP1000 - PHSensor</option>
             <option value="TMP1200_0-PT100-3850">TMP1200 - PT100 (3850ppm)</option>
             <option value="TMP1200_0-PT100-3920">TMP1200 - PT100 (3920ppm)</option>
             <option value="TEMP_1124_0">TEMP 1124_0</option>
           </select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Unité</label>
						     <input id="idModalEditAIUnite" type="text" required class="form-control" placeholder="Unité de mesure">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Valeur minimale</label>
						     <input id="idModalEditAIMin" type="number" required class="form-control" placeholder="Valeur minimale">
						     <div class="input-group-append">
							     <span id="idModalEditAIMinUnite" class="input-group-text"></span>
						     </div>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Valeur maximale</label>
						     <input id="idModalEditAIMax" type="number" required class="form-control" placeholder="Valeur maximale">
						     <div class="input-group-append">
							     <span id="idModalEditAIMaxUnite" class="input-group-text"></span>
						     </div>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Intervalle d'acquisition</label>
						     <input id="idModalEditAIIntervalle" type="number" required min=0 max=60000 class="form-control" placeholder="Fréquence d'acquisition">
						     <div class="input-group-append">
							     <span class="input-group-text">ms</span>
						     </div>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Rechercher une Target</label>
						     <input id="idModalEditAIRechercherTechID" oninput="PhidgetMap_Update_Choix_Tech_ID('idModalEditAI', 'AI')" type="text" class="col-9 form-control" placeholder="Rechercher un Tech_id">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Target TechID</label>
						     <select id="idModalEditAISelectTechID" onchange="PhidgetMap_Update_Choix_Acronyme('idModalEditAI', 'AI')" class="col-9 custom-select border-info"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Target Acronyme</label>
						     <select id="idModalEditAISelectAcronyme" class="col-9 custom-select border-info"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Question Vocale</label>
						     <input id="idModalEditAIMapQuestionVoc" type="text" class="form-control" placeholder="Question vocale associée">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Réponse Vocale</label>
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
						     <label class="col-5 col-sm-4 col-form-label text-right">Phidget Tech_ID</label>
						     <input id="idModalEditAOPhidgetTechID" type="text" class="form-control" placeholder="Module WAGO">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Phidget AO</label>
						     <input id="idModalEditAOPhidgetTag" type="number" required min=0 max=128 class="form-control" placeholder="AOxx">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Type de Borne</label>
						     <select id="idModalEditAOType" class="custom-select border-info">
             <option value="0">a definir</option>
             <option value="1">a definir</option>
           </select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Unité</label>
						     <input id="idModalEditAOUnite" type="text" class="form-control" placeholder="°C, km/h, ...">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Min</label>
						     <input id="idModalEditAOMin" type="number" class="form-control" placeholder="Valeur Min">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Max</label>
						     <input id="idModalEditAOMax" type="number" class="form-control" placeholder="Valeur Max">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Rechercher une Target</label>
						     <input id="idModalEditAORechercherTechID" oninput="PhidgetMap_Update_Choix_Tech_ID('idModalEditAO', 'AO')" type="text" class="col-9 form-control" placeholder="Rechercher un Tech_id">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Target TechID</label>
						     <select id="idModalEditAOSelectTechID" onchange="PhidgetMap_Update_Choix_Acronyme('idModalEditAO', 'AO')" class="col-9 custom-select border-info"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Target Acronyme</label>
						     <select id="idModalEditAOSelectAcronyme" class="col-9 custom-select border-info"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Question Vocale</label>
						     <input id="idModalEditMapQuestionVoc" type="text" class="form-control" placeholder="Question vocale associée">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Réponse Vocale</label>
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

<script src="/js/tech/phidget_map.js" type="text/javascript"></script>
