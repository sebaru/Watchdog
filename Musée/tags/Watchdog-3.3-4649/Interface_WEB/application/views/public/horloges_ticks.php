<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<div class="content-wrapper">
	<section class="content-header">
		Liste des évènements pour <strong><?php echo $horloge->libelle; ?></strong>
	</section>

	<section class="content">
		<div class="row">
			<div class="col-md-12">
				<div class="box">

       <div class="box-header with-border">
         <label>Liste des évènements</label>
         <div class="box-title btn-group pull-right">
             <?php echo anchor('horloges/create_tick/'.$horloge->tech_id."/".$horloge->acronyme."/".$horloge->id,
                               '<i class="fa fa-plus"></i>Ajouter', array('class' => 'btn btn-primary')); ?>
         </div>
       </div>

				<div class="box-body">

      <?php $flash_error=$this->session->flashdata('flash_error');
            if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
      <?php $flash_msg=$this->session->flashdata('flash_message');
            if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>



      <table class="table table-striped table-hover">
        <thead>
          <tr>
            <th>#</th>
            <th>Heure</th>
            <th>Minute</th>
            <th>Modifier</th>
            <th>Supprimer</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($ticks as $tick):?>
            <tr>
                <td><?php echo $tick->id; ?></td>
                <form method="post" action=<?php echo site_url("/horloges/set/".$horloge->tech_id."/".$horloge->acronyme."/".$tick->id) ?>>
                  <td><?php echo form_input( "heure", $tick->heure); ?></td>
                  <td><?php echo form_input( "minute", $tick->minute ); ?></td>
                  <td><?php echo form_submit( "submit", "Modifier", array('class' => 'btn btn-primary') )?></td>
                </form>
                <td><?php echo anchor("horloges/delete/".$horloge->tech_id."/".$horloge->acronyme."/".$tick->id, 'Supprimer',
                               array('class' => 'btn btn-warning')); ?></td>
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
