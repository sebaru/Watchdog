<div class="container-fluid">

 <div class="row m-2">
   <div class="col-auto"><h3><img src="/img/wago_750342.webp" style="width:80px" alt="Wago 750-342">Liste des Modules WAGO sur Modbus</h3></div>
   <div class ="ml-auto btn-group align-items-center">
        <button type="button" onclick="MODBUS_Add()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter une connexion</button>
        <button type="button" onclick="MODBUS_Refresh()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
   </div>
 </div>

<hr>

    <table id="idTableMODBUS" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
      </thead>
      <tbody>
      </tbody>
    </table>

<hr>

<div class="col-auto"><h3><img src="/img/wago_750342.webp" style="width:80px" alt="Wago 750-342">Configuration des I/O WAGO</h3></div>

<ul class="nav nav-tabs" role="tablist">
  <li class="nav-item"><a class="nav-link active" data-toggle="tab" href="#idTabEntreeTor">
        <img style="width: 30px" data-toggle="tooltip" title="Entrées TOR" src="/img/Entree.png" />Entrées TOR</a></li>
  <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabEntreeAna">
        <img style="width: 30px" data-toggle="tooltip" title="Entrées ANA" src="/img/Entree_Analogique.png" />Entrées ANA</a></li>
  <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabSortieTor">
        <img style="width: 30px" data-toggle="tooltip" title="Sorties TOR" src="/img/Sortie.png" />Sorties TOR</a></li>
  <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabSortieAna">
        <img style="width: 30px" data-toggle="tooltip" title="Sorties ANA" src="/img/Sortie_Analogique.png" />Sorties ANA</a></li>
</ul>

<div class="tab-content">

<!----------------------------------------------------------------------------------------------------------------------------->
<div id="idTabEntreeTor" class="tab-pane fade in table-responsive mt-1" role="tabpanel">

    <table id="idTableMODBUS_DI" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
      </thead>
      <tbody>
      </tbody>
    </table>
</div>

<!----------------------------------------------------------------------------------------------------------------------------->
<div id="idTabSortieTor" class="tab-pane fade in table-responsive mt-1" role="tabpanel">

    <table id="idTableMODBUS_DO" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
      </thead>
      <tbody>
      </tbody>
    </table>
</div>
<!----------------------------------------------------------------------------------------------------------------------------->
<div id="idTabEntreeAna" class="tab-pane fade in table-responsive mt-1" role="tabpanel">

    <table id="idTableMODBUS_AI" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
      </thead>
      <tbody>
      </tbody>
    </table>
</div>
<!----------------------------------------------------------------------------------------------------------------------------->
<div id="idTabSortieAna" class="tab-pane fade in table-responsive mt-1" role="tabpanel">

    <table id="idTableMODBUS_AO" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
      </thead>
      <tbody>
      </tbody>
    </table>
</div>

</div> <!-- TabContent -->
<!-- Container -->
</div>


<!------------------------------------------------- Modal Edit Analog Input --------------------------------------------------->
<div id="idMODBUSEditAI" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idMODBUSEditAITitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Type de borne</label>
           <select id="idMODBUSEditAITypeBorne" class="custom-select border-info">
             <option value="3">750455 - 4/20 mA</option>
             <option value="4">750462 - Pt-100</option>
           </select>
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Minimum</label>
           <input id="idMODBUSEditAIMin" type="number" class="form-control" placeholder="Valeur Min">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Maximum</label>
           <input id="idMODBUSEditAIMax" type="number" class="form-control" placeholder="Valeur Max">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Unité</label>
           <input id="idMODBUSEditAIUnite" type="text" class="form-control" placeholder="°C, km/h, ...">
          </div>
       </div>

      </div>

      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idMODBUSEditAIValider" type="button" class="btn btn-primary"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>
<!------------------------------------------------- Modal Edit Analog Output -------------------------------------------------->
<div id="idMODBUSEditAO" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idMODBUSEditAOTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Wago Tech_ID</label>
           <select id="idMODBUSEditAOWagoTechID" class="custom-select border-info" placeholder="Module WAGO"></select>
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Wago AO</label>
           <input id="idMODBUSEditAOWagoTag" type="number" required min=0 max=128 class="form-control" placeholder="AOxx">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Type de Borne</label>
           <select id="idMODBUSEditAOType" class="custom-select border-info">
             <option value="0">a definir</option>
             <option value="1">a definir</option>
           </select>
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Unité</label>
           <input id="idMODBUSEditAOUnite" type="text" class="form-control" placeholder="°C, km/h, ...">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Min</label>
           <input id="idMODBUSEditAOMin" type="number" class="form-control" placeholder="Valeur Min">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Max</label>
           <input id="idMODBUSEditAOMax" type="number" class="form-control" placeholder="Valeur Max">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Rechercher une Target</label>
           <input id="idMODBUSEditAORechercherTechID" oninput="ModbusMap_Update_Choix_Tech_ID('idMODBUSEditAO', 'AO')" type="text" class="col-9 form-control" placeholder="Rechercher un Tech_id">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Target TechID</label>
           <select id="idMODBUSEditAOSelectTechID" onchange="ModbusMap_Update_Choix_Acronyme('idMODBUSEditAO', 'AO')" class="col-9 custom-select border-info"></select>
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Target Acronyme</label>
           <select id="idMODBUSEditAOSelectAcronyme" class="col-9 custom-select border-info"></select>
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Question Vocale</label>
           <input id="idMODBUSEditMapQuestionVoc" type="text" class="form-control" placeholder="Question vocale associée">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Réponse Vocale</label>
           <input id="idMODBUSEditMapReponseVoc" type="text" class="form-control" placeholder="Réponse vocale associée">
          </div>
          <small>$1 est la valeur dynamique du bit interne</small>
       </div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idMODBUSEditValider" type="button" class="btn btn-primary"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<!---------------------------------------------------- Modal Edit Modbus ------------------------------------------------------>

<div id="idMODBUSEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idMODBUSTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Choix de l'instance</label>
           <select id="idTargetProcess" class="custom-select border-info"></select>
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">TechID</label>
           <input id="idMODBUSTechID" required type="text" class="form-control" placeholder="Tech ID du module">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Description</label>
           <input id="idMODBUSDescription" type="text" class="form-control" placeholder="Ou est le module ?">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Hostname</label>
           <input id="idMODBUSHostname" required type="text" class="form-control" placeholder="@IP ou hostname">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Watchdog</label>
           <input id="idMODBUSWatchdog" type="number" class="form-control" min=10 max=1200 placeholder="Nombre de 1/10 de secondes avant de couper les sorties">
           <div class="input-group-append">
            <span class="input-group-text">secondes</span>
           </div>
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group align-items-center ">
           <label class="col-5 col-sm-4 col-form-label text-right">Max Requetes/sec</label>
           <input id="idMODBUSMaxRequestParSec" type="number" class="form-control" min=1 max=100 placeholder="Nombre de requetes max par seconde">
           <div class="input-group-append">
            <span class="input-group-text form-control">par seconde</span>
           </div>
          </div>
        </div>

      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idMODBUSValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="/js/tech/modbus.js" type="text/javascript"></script>
