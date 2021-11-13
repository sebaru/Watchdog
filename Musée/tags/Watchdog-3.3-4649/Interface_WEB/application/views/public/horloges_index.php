<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<div class="content-wrapper">
	<section class="content-header">
		<?php /*echo "Liste de toutes les horloges";*/ ?>
	</section>

	<section class="content">
		<div class="row">
			<div class="col-md-12">
				<div class="box">

       <div class="box-header with-border">
         <label>Liste des horloges</label>
       </div>

				<div class="box-body">

      <?php $flash_error=$this->session->flashdata('flash_error');
            if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
      <?php $flash_msg=$this->session->flashdata('flash_message');
            if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

      <table class="table table-striped table-hover">
        <thead>
          <tr>
            <th>Synoptique</th>
            <th>Libellé</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($horloges as $horloge):?>
            <tr>
                <td><?php echo $horloge->page; ?></td>
                <td><?php echo anchor("horloges/show/".$horloge->tech_id."/".$horloge->acronyme, $horloge->libelle ); ?></td>
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
