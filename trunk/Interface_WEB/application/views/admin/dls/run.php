<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<div class="content-wrapper">
	<section class="content-header">
		<?php echo $pagetitle; ?>
	</section>

	<section class="content">
		<div class="row">
			<div class="col-md-12">
				<div class="box">

       <div class="box-header with-border">
         <div class="btn-group pull-right">
             <?php echo anchor('admin/dls', 'Retour Liste', array('class' => 'btn btn-default')); ?>
         </div>
       </div>

				<div class="box-body">

      <?php $flash_error=$this->session->flashdata('flash_error');
            if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
      <?php $flash_msg=$this->session->flashdata('flash_message');
            if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

      <table id="dlsrun" class="table table-striped table-hover">
        <thead>
          <tr>
            <th>Tech_ID</th>
            <th>ID</th>
            <th>Shortname</th>
            <th>Started</th>
            <th>Start_date</th>
            <th>Version</th>
            <th>Conso</th>
            <th>Debug</th>
            <th>Comm</th>
            <th>Defaut</th>
            <th>Alarme</th>
            <th>Alerte</th>
            <th>Veille</th>
            <th>Derangement</th>
            <th>Danger</th>
            <th>Acquit</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($dlss as $tech_id => $dls):?>
            <tr>
              <td><?php echo $tech_id; ?></td>
              <td><?php echo $dls->id; ?></td>
              <td><?php echo $dls->shortname; ?></td>
              <td><?php if ($dls->started)
                       { echo '<span class="label label-success">Yes</span>'; }
                  else { echo '<span class="label label-default">No</span>'; }
                  ?></td>
              <td><?php echo $dls->start_date; ?></td>
              <td><?php echo $dls->version; ?></td>
              <td><?php echo $dls->conso; ?></td>
              <td><?php if ($dls->debug)
                       { echo anchor('admin/dls/undebug/'. $tech_id, '<span class="label label-warning">Actif</span>'); }
                  else { echo anchor('admin/dls/debug/'.$tech_id, '<span class="label label-default">Inactif</span>'); }
                  ?></td>
              <td><?php if ($dls->bit_comm_out)
                       { echo '<span class="label label-danger">Hors Comm !</span>'; }
                  else { echo '<span class="label label-success">OK</span>'; }
                  ?></td>
              <td><?php if ($dls->bit_defaut)
                       { echo '<span class="label label-danger">Défaut</span>'; }
                  else if ($dls->bit_defaut_fixe)
                       { echo '<span class="label label-warning">Fixe</span>'; }
                  else { echo '<span class="label label-default">None</span>'; }
                  ?></td>
              <td><?php if ($dls->bit_alarme)
                       { echo '<span class="label label-danger">Alarme</span>'; }
                  else if ($dls->bit_alarme_fixe)
                       { echo '<span class="label label-warning">Fixe</span>'; }
                  else { echo '<span class="label label-default">None</span>'; }
                  ?></td>
              <td><?php if ($dls->bit_alerte)
                       { echo '<span class="label label-danger">Alerte</span>'; }
                  else if ($dls->bit_alerte_fixe)
                       { echo '<span class="label label-warning">Fixe</span>'; }
                  else { echo '<span class="label label-default">None</span>'; }
                  ?></td>
              <td><?php if ($dls->bit_veille)
                       { echo '<span class="label label-success">OK</span>'; }
                  else { echo '<span class="label label-danger">NOK</span>'; }
                  ?></td>
              <td><?php if ($dls->bit_derangement)
                       { echo '<span class="label label-danger">Dérangement</span>'; }
                  else if ($dls->bit_derangement_fixe)
                       { echo '<span class="label label-warning">Fixe</span>'; }
                  else { echo '<span class="label label-default">None</span>'; }
                  ?></td>
              <td><?php if ($dls->bit_danger)
                       { echo '<span class="label label-danger">Danger</span>'; }
                  else if ($dls->bit_danger_fixe)
                       { echo '<span class="label label-warning">Fixe</span>'; }
                  else { echo '<span class="label label-default">None</span>'; }
                  ?></td>
              <td><?php echo anchor('admin/dls/acquit/'.$tech_id, '<span class="label label-success">Acquitter</span>'); ?></td>
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
