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
        <h3 class="box-title">Editer le tableau <strong><?php echo $tableau->id;?></strong></h3>
      </div>

      <div class="box-body">

        <?php $flash_error=$this->session->flashdata('flash_error');
        if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
        <?php $flash_msg=$this->session->flashdata('flash_message');
        if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

        <?php echo form_open(uri_string(), array('class' => 'form-horizontal', 'id' => 'form-edit_tableau')); ?>

        <div class="col-md-12 form-group">
          <div class="btn-group pull-right">
              <?php echo form_button(array('type' => 'submit', 'class' => 'btn btn-success', 'content' => lang('actions_submit'))); ?>
              <?php echo form_button(array('type' => 'reset', 'class' => 'btn btn-warning', 'content' => lang('actions_reset'))); ?>
              <?php echo anchor('admin/tableau', 'Retour Liste', array('class' => 'btn btn-default')); ?>
              <?php echo anchor('admin/tableau/add_courbe/'.$tableau->id, '<i class="fa fa-plus"></i>Ajouter une courbe', array('class' => 'btn btn-primary')); ?>
          </div>
        </div>

        <div class="form-group">
          <label class='col-sm-2 control-label'>Titre</label>
          <div class="col-sm-5"> <?php echo form_input($titre);?> </div>

          <label for="access_level" class="col-sm-2 control-label">Niveau de privilèges</label>
          <div class="col-sm-3">
	             <select name="access_level" class="form-control">
	               <?php for ($i=0;$i<$this->session->user_access_level;$i++)
                       { if ($access_level==$i) $selected="selected";else $selected='';?>
	                       <option value="<?php echo $i?>" <?php echo $selected?>><?php echo $i?></option>
                <?php  }?>
              </select>
          </div>
        </div>

        <?php echo form_close();?>

        <hr>

      <table class="table table-striped table-hover">
        <thead>
          <tr>
            <th>#</th>
            <th>Tech_id</th>
            <th>Acronyme</th>
            <th>Couleur</th>
            <th>Actions</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($courbes as $courbe):?>
            <tr>
              <form method="post" action=<?php echo site_url("/admin/tableau/set_courbe/".$tableau->id."/".$courbe->id ) ?>>
              <td><?php echo $courbe->id; ?></td>
              <td><?php echo form_input( "tech_id", $courbe->tech_id ); ?></td>
              <td><?php echo form_input( "acronyme", $courbe->acronyme ); ?></td>
              <td><?php echo form_input( "color", $courbe->color ); ?></td>
               <td><?php echo form_submit( "submit", "Modifier", array('class' => 'btn btn-primary') )?></td>
              </form>
              <td><?php echo anchor("admin/tableau/del_courbe/".$tableau->id."/".$courbe->id, 'Supprimer',
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
