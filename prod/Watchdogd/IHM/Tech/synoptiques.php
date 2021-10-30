<div class="container-fluid">

 <div class="row m-2">
   <h3><i class="fas fa-mimage text-primary"></i> Liste des Synoptiques</strong></h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Show_Modal_Syn_Add(0)" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un Synoptique</button>
   </div>
 </div>

<hr>

<div class="table-responsive">
  <table id="idTableSyn" class="table table-striped table-bordered table-hover">
    <thead class="thead-dark">
				</thead>
			 <tbody>
    </tbody>
  </table>
</div>


<!-- Container -->
</div>


<div id="idModalSynEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalSynEditTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Page du Parent</label>
						     <select id="idModalSynEditPPage" class="col-7 col-sm-8 custom-select border-info"></select>
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Nom de la Page</label>
						     <input id="idModalSynEditPage" type="text" class="form-control" placeholder="Titre du synoptique">
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right"><i class="fas fa-star"></i> Level</label>
						     <input id="idModalSynEditAccessLevel" type="number" class="form-control" min=0 max=9 placeholder="Level">
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Mode d'Affichage</label>
						     <select id="idModalSynEditAffichage" class="col-7 col-sm-8 custom-select border-info">
             <option value="0">Mode Simple</option>
             <option value="1">Mode Full</option>
           </select>
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Libellé du synoptique</label>
						     <input id="idModalSynEditLibelle" type="text" class="form-control" placeholder="Libellé du synoptique">
     					</div>
       </div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalSynEditValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<!----------------------------------------------------------------------------------------------------------------------------->
<div id="idSynEditImage" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idSynEditImageTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

        <div id="idSynEditImageListe" class="col">
        </div>
    </div>
  </div>
</div>


<script src="/js/tech/synoptiques.js" type="text/javascript"></script>
