<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<div class="content-wrapper">
	<section class="content-header">
		<h1>Choisissez le tableau à afficher</h1>
	</section>

	<section class="content">
		<div class="row">
			<div class="col-md-12">
				<div class="box">

       <div class="box-header with-border">
         <!--<div class="box-title btn-group pull-right">
             <?php echo anchor('admin/tableau/create', '<i class="fa fa-plus"></i>Ajouter un tableau', array('class' => 'btn btn-primary')); ?>
         </div>-->
       </div>

				<div class="box-body">

      <table class="table table-striped table-hover">
        <thead>
          <!--<tr>
            <th>Titre des tableaux</th>
          </tr>-->
        </thead>
        <tbody>
          <?php foreach ($tableaux as $tableau):?>
            <tr>
              <td><i class="fa fa-line-chart text-red"></i>
                  <a href="<?php echo site_url('archive/tableau/'.$tableau->id); ?>" data-toggle="tooltip" title="Voir les graphes">
                  <?php echo $tableau->titre; ?></a></td>
            </tr>
          <?php endforeach;?>
        </tbody>
      </table>

				</div>
				</div>
			</div>
		</div>
	</section>
</div>
