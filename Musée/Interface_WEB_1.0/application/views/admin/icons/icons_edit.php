<div class="content-wrapper">
	<section class="content-header">
		<?php echo $pagetitle; ?>
		<?php echo $breadcrumb; ?>
	</section>

	<section class="content">

		<div class="row">
			<div class="col-md-12">
				<div class="box box-info">
					<div class="box-header with-border">
						<h3 class="box-title">Editer l'icone <strong><?php echo $icon->libelle; ?></strong></h3>
					</div>
					<div class="box-body">

        <?php $flash_error=$this->session->flashdata('flash_error');
        if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
        <?php $flash_msg=$this->session->flashdata('flash_message');
        if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

        <?php echo form_open(uri_string(), array('class' => 'form-horizontal', 'id' => 'form-icon_edit')); ?>

        <div class="col-md-12 form-group">
          <div class="btn-group pull-right">
              <?php echo form_button(array('type' => 'submit', 'class' => 'btn btn-success', 'content' => 'Sauvegarder')); ?>
              <?php echo form_button(array('type' => 'reset', 'class' => 'btn btn-warning', 'content' => lang('actions_reset'))); ?>
              <?php echo anchor('admin/icons/icon_list/'.$icon->id_classe, 'Retour Liste', array('class' => 'btn btn-default')); ?>
          </div>
        </div>

        <div class="form-group">
          <label class='col-sm-2 control-label'>Libellé de l'icone</label>
          <div class="col-sm-4"><?php echo form_input($libelle);?></div>

          <label class='col-sm-2 control-label'>Classe de l'icone</label>
          <div class="col-sm-4">
            <select name="id_classe" class="form-control">
              <?php foreach ($classes as $classe):
                    if ($icon->id_classe==$classe->id) $selected="selected";else $selected='';
                    echo "<option value=".$classe->id." ".$selected.">".$classe->libelle."</option>";
                    endforeach;
               ?>
           </select>
          </div>
        </div>

        <div class="form-group">
          <label class='col-sm-2 control-label'>Type du fichier</label>
          <div class="col-sm-2">
            <select name="type_icone" class="form-control">
              <?php
              if ($icon->type=="gif") $selected="selected"; else $selected='';
              echo "<option value='gif' ".$selected.">GIF</option>";
              if ($icon->type=="svg") $selected="selected"; else $selected='';
              echo "<option value='svg' ".$selected.">SVG</option>";
              ?>
            </select>
          </div>
          <label class='col-sm-2 control-label'>Nombre de matrice</label>
          <div class="col-sm-2"><?php echo form_input($nbr_matrice);?></div>
        </div>

        <div class="form-group">
          <label class='col-sm-2 control-label'>Matrice initiale</label>
          <div class="col-sm-10">
            <img <?php echo 'src="https://icons.abls-habitat.fr/assets/gif/'.$icon->id.'.'.$icon->type.'"'; ?> />
          </div>
        </div>

        <?php for ($mat=1; $mat<$icon->nbr_matrice; $mat++) { ?>
        <div class="form-group">
          <label class='col-sm-2 control-label'>Matrice numéro <?php echo $mat; ?></label>
          <div class="col-sm-10">
            <img <?php echo 'src="https://icons.abls-habitat.fr/assets/gif/'.$icon->id.'.'.$icon->type.sprintf('.%02d',$mat).'"'; ?> />
            </div>
        </div>
        <?php } ?>

        <?php echo form_close();?>
        <hr>
        <?php echo form_open_multipart(site_url('admin/icons/icon_upload/'.$icon->id), array('class' => 'form-horizontal', 'id' => 'form-icon_upload')); ?>
        <div class="form-group">
          <label class='col-sm-2 control-label'>Ajouter une matrice</label>
          <div class="col-sm-4"><?php echo form_upload($new_file);?></div>
          <div class="col-sm-2">
              <?php echo form_button(array('type' => 'submit', 'class' => 'btn btn-success', 'content' => lang('actions_submit'))); ?>
          </div>
        </div>
        <?php echo form_close();?>

     </div>

    </div>
   </div>
  </div>
 </section>
</div>
