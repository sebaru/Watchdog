<div class="container-fluid">

 <div class="row m-2">
   <h3><img src="/img/commande_texte.png" style="width:80px" alt="Commandes textuelles">Mapping des Commandes Textuelles</h3>

   <div class ="ml-auto btn-group align-items-center">
        <button type="button" onclick="Show_Modal_Map_Edit_DI(null)" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un mapping</button>
   </div>
 </div>

<hr>

<!----------------------------------------------------------------------------------------------------------------------------->

    <table id="idTableTXT" class="table table-striped table-bordered table-hover">
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

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Commande textuelle</label>
						     <input id="idModalEditTXTTag" type="text" class="form-control" placeholder="Commande SMS">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Rechercher une Target</label>
						     <input id="idModalEditRechercherTechID" oninput="TXTMap_Update_Choix_Tech_ID()" type="text" class="col-9 form-control" placeholder="Rechercher un Tech_id">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Target TechID</label>
						     <select id="idModalEditSelectTechID" onchange="TXTMap_Update_Choix_Acronyme()" class="col-9 custom-select border-info"></select>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Target Acronyme</label>
						     <select id="idModalEditSelectAcronyme" class="col-9 custom-select border-info"></select>
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


<script src="/js/tech/command_text.js" type="text/javascript"></script>
