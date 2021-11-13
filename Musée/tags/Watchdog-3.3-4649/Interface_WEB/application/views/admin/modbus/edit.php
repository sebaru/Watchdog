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
        <h3 class="box-title">Editer un module Wago/Modbus</h3>
      </div>

      <div class="box-body">

        <?php $flash_error=$this->session->flashdata('flash_error');
        if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
        <?php $flash_msg=$this->session->flashdata('flash_message');
        if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

        <?php echo form_open(uri_string(), array('class' => 'form-horizontal', 'id' => 'form-edit_modbus')); ?>

        <div class="col-md-12 form-group">
          <div class="btn-group pull-right">
              <?php echo form_button(array('type' => 'submit', 'class' => 'btn btn-success', 'content' => lang('actions_submit'))); ?>
              <?php echo form_button(array('type' => 'reset', 'class' => 'btn btn-warning', 'content' => lang('actions_reset'))); ?>
              <?php echo anchor('admin/modbus', 'Retour Liste', array('class' => 'btn btn-default')); ?>
          </div>
        </div>

        <div class="form-group">
          <label class='col-sm-2 control-label'>Hostname</label>
          <div class="col-sm-4"> <?php echo form_input($hostname);?> </div>
          <label class='col-sm-1 control-label'>Tech_id</label>
          <div class="col-sm-2">
            <?php echo form_input($tech_id);?>
          </div>
          <label class='col-sm-1 control-label'>Watchdog</label>
          <div class="col-sm-2">
            <?php echo form_input($watchdog);?>
          </div>
        </div>

        <div class="form-group">
          <label class='col-sm-2 control-label'>Description</label>
          <div class="col-sm-10"> <?php echo form_input($description);?> </div>
        </div>

        <div class="form-group">
          <label for='map_e' class='col-sm-2 control-label'>Map_E</label>
          <div class="col-sm-4"> <?php echo form_input($map_E);?> </div>
          <label for='max_nbr_e' class='col-sm-2 control-label'>Max_nbr_E</label>
          <div class="col-sm-4"> <?php echo form_input($max_nbr_E);?> </div>
        </div>

        <div class="form-group">
          <label for='map_ea' class='col-sm-2 control-label'>Map_EA</label>
          <div class="col-sm-10"> <?php echo form_input($map_EA);?> </div>
        </div>

        <div class="form-group">
          <label for='map_a' class='col-sm-2 control-label'>Map_A</label>
          <div class="col-sm-10"> <?php echo form_input($map_A);?> </div>
        </div>

        <div class="form-group">
          <label for='map_aa' class='col-sm-2 control-label'>Map_AA</label>
          <div class="col-sm-10"> <?php echo form_input($map_AA);?> </div>
        </div>

        <?php echo form_close();?>

      <table class="table table-striped table-hover">
        <thead>
          <tr>
            <th>Type</th>
            <th>Acronyme DLS</th>
            <th>Acronyme Wago</th>
            <th>Modifier</th>
            <th>Supprimer</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($maps_di as $map):?>
          <tr>
           <form method="post" action=<?php echo site_url("/admin/modbus/map/DI/".$map->tech_id."/".$map->acronyme) ?>>
              <td><img style="width: 30px" data-toggle="tooltip" title="Entrées TOR"
                       src="https://icons.abls-habitat.fr/assets/gif/Entree.png" /></td>
              <td><?php echo $map->tech_id.":".$map->acronyme; ?></td>
              <td><input type="text" name="src_text" placeholder="Acronyme WAGO" value="<?php echo $map->src_text; ?>"/></td>
              <td><input type="submit" value="Envoyer" /></td>
              <td><?php echo anchor('admin/modbus/map_delete/DI/'.$map->tech_id."/".$map->acronyme, 'Supprimer',
                             array('class' => 'btn btn-warning')); ?></td>
           </form>
          </tr>
          <?php endforeach;?>
          <?php foreach ($maps_ai as $map):?>
          <tr>
           <form method="post" action=<?php echo site_url("/admin/modbus/map/AI/".$map->tech_id."/".$map->acronyme) ?>>
              <td><img style="width: 30px" data-toggle="tooltip" title="Sortie ANA"
                       src="https://icons.abls-habitat.fr/assets/gif/Entree_Analogique.png" /></td>
              <td><?php echo $map->tech_id.":".$map->acronyme; ?></td>
              <td><input type="text" name="map_text" placeholder="Acronyme WAGO" value="<?php echo $map->map_text; ?>"/></td>
              <td><input type="submit" value="Envoyer" /></td>
              <td><?php echo anchor('admin/modbus/map_delete/AI/'.$map->tech_id."/".$map->acronyme, 'Supprimer',
                             array('class' => 'btn btn-warning')); ?></td>
           </form>
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
