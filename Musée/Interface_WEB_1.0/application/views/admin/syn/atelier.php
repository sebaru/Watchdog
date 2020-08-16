<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<div class="content-wrapper">

	<section class="content">

		<div class="row">
			<div class="col-md-12">
				<div class="box">

       <div class="box-header with-border">
         <div class="col-md-12 form-group">
           <?php echo "Edition du synoptique <strong>".$syn."</strong>"; ?>
       </div>

     <div class="box-body">

           <div class="btn-group pull-right">
            <button class="btn btn-warning" onclick="Supprimer()">Supprimer</button>
            <button class="btn btn-default" onclick="Deselectionner()">UnSelect</button>
            <button class="btn btn-info" onclick="Dupliquer()">Dupliquer</button>
            <button class="btn btn-success" onclick="Sauvegarder_synoptique()">Sauvegarder</button>
           </div>
           <hr></hr>
           <div id="liste_passerelles" class="btn-group"></div>

         </div>

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

  			<div class="col-md-10">
       <svg id="TopSVG" width="100%" viewBox="0 0 1200 600" style="background-color: #666"></svg>
					</div>

  			<div id="WTD-ctrl-panel" class="col-md-2" style="border: solid black;">Rien n'est sélectionné</div>

					</div>
				</div>
			</div>
		</div>
	</section>

</div>
