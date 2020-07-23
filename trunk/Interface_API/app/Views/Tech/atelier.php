<div class="container-fluid">
  <div class="row">
  			<div class="col-md-10">
       <svg id="TopSVG" viewBox="0 0 1200 600" class="bg-dark border border-warning"></svg>
					</div>
<!--
  			<div class="col-md-12">
    			<div class="col-md-2">
         <select id="select-classes" onchange="Charger_icone()"></select>
       </div>
    			<div class="col-md-3">
         <select id="select-icones"></select>
       </div>
    			<div class="col-md-1">
         <button class="btn btn-success" onclick="Ajouter_icone()">Ajouter l'icone</button>
       </div>
    			<div class="col-md-1">
         <button class="btn btn-success" onclick="Ajouter_lien()">Ajouter un lien</button>
       </div>
    			<div class="col-md-1">
         <button class="btn btn-success" onclick="Ajouter_rectangle()">Ajouter un rectangle</button>
       </div>
     </div>

-->
  			<div class="col-md-2 border border-info">
       <div class="btn-group btn-block m-1" role="group">
         <button class="btn btn-success" onclick="Sauvegarder_synoptique()"><i class="fas fa-save"> </i></button>
         <button class="btn btn-outline-danger ml-1" onclick="Supprimer_svg()"><i class="fas fa-trash"> </i></button>
       </div>
    			<div class="" id="WTD-ctrl-panel"><p>Rien n'est sélectionné</p></div>
     </div>
  </div>

<script src="<?php echo base_url('js/atelier.js')?>" type="text/javascript"></script>
<!-- Container -->
</div>
