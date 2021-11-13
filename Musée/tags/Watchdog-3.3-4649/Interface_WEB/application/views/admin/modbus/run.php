<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<div class="content-wrapper">
	<section class="content-header">
		<?php echo $pagetitle; ?>
  <?php echo $breadcrumb; ?>
	</section>

	<section class="content">
		<div class="row">
			<div class="col-md-12">
				<div class="box">

       <div class="box-header with-border">
         <div class="box-title btn-group pull-right">
             <?php echo anchor('admin/modbus/index', '<i class="fa fa-cogs"></i>Retour Liste', array('class' => 'btn btn-primary')); ?>
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
            <th>Tech_id</th>
            <th>Mode</th>
            <th>Started</th>
            <th>Comm</th>
            <th>Nb DI</th>
            <th>Nb DO</th>
            <th>Nb AI</th>
            <th>Nb AO</th>
            <th>Nbr_deconnect</th>
            <th>Transaction ID</th>
            <th>Transaction/s</th>
            <th>Délai d'attente</th>
            <th>Last response (s)</th>
            <th>Date next EANA</th>
            <th>Date retente</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($modbus as $tech_id => $module):?>
            <tr>
              <td><?php echo $tech_id; ?></td>
              <td><?php echo $module->mode; ?></td>
              <td><?php echo $module->started; ?></td>
              <td><?php echo $module->comm; ?></td>
              <td><?php echo $module->nbr_entree_tor; ?></td>
              <td><?php echo $module->nbr_sortie_tor; ?></td>
              <td><?php echo $module->nbr_entree_ana; ?></td>
              <td><?php echo $module->nbr_sortie_ana; ?></td>
              <td><?php echo $module->nbr_deconnect; ?></td>
              <td><?php echo $module->transaction_id; ?></td>
              <td><?php echo $module->nbr_request_par_sec; ?></td>
              <td><?php echo $module->delai; ?></td>
              <td><?php echo $module->last_reponse; ?></td>
              <td><?php echo $module->date_next_eana; ?></td>
              <td><?php echo $module->date_retente; ?></td>
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
