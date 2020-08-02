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
         <div class="col-md-12 form-group">
           <div class="btn-group pull-right">
               <?php echo anchor('admin/process', '<i class="fa fa-chip"></i>Voir les process', array('class' => 'btn btn-primary')); ?>
           </div>
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
            <th>Host</th>
            <th>Thread</th>
            <th>Parametre</th>
            <th>Valeur</th>
            <th>Delete</th>
          </tr>
        </thead>
        <tbody>
          <tr>
           <form method="post" action=<?php echo site_url("/admin/config/set/".$host."/".$thread) ?>>
              <td><?php echo $host; ?></td>
              <td><?php echo $thread; ?></td>
              <td><input type="text" name="parametre" placeholder="Nom du parametre"/></td>
              <td><input type="text" name="valeur" placeholder="Valeur du parametre"/>
                  <input type="submit" value="Envoyer" /></td>
           </form>
          </tr>
          <?php foreach ($configs as $config):?>
            <tr>
              <td><?php echo $config->instance_id; ?></td>
              <td><?php echo $config->nom_thread; ?></td>
              <td><?php echo $config->nom; ?></td>
              <td><?php echo $config->valeur; ?></td>
              <td><?php echo anchor('admin/config/delete/'.$config->instance_id."/".$config->nom_thread."/".$config->nom, 'Supprimer',
                             array('class' => 'btn btn-warning')); ?>
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
