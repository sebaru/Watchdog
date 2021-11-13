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
             <?php echo anchor('admin/ups/create', '<i class="fa fa-plus"></i>Ajouter un Onduleur', array('class' => 'btn btn-primary')); ?>
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
            <th>Actions</th>
            <th>Enable</th>
            <th>Tech_id</th>
            <th>Name</th>
            <th class="hidden-xs">Host</th>
            <th class="hidden-xs">Admin User</th>
            <th class="hidden-xs">Admin Password</th>
            <th class="hidden-xs">Description</th>
            <th class="hidden-xs">Date<br>Creation</th>
            <th class="hidden-xs">Run</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($onduleurs as $onduleur):?>
            <tr>
              <td>
                <div class="dropdown">
                  <button class="btn btn-xs btn-primary dropdown-toggle" type="button" data-toggle="dropdown"><span class="caret"></span></button>
                   <ul class="dropdown-menu">
                   <li><a href=<?php echo site_url('admin/ups/edit/').$onduleur->id; ?>>
                     <i class="fa fa-pencil" style="color:green"></i>
                     <i class="fa fa-cogs" style="color:blue"></i>Editer cet onduleur</a></li>
                   <li class="divider"></li>
                   <li><a href=<?php echo site_url('admin/ups/delete/').$onduleur->id; ?>>
                     <i class="fa fa-times" style="color:red"></i>Supprimer</a></li>
                   </ul>
               </div>
              </td>
              <td><?php echo $onduleur->enable ? anchor('admin/ups/deactivate/'.$onduleur->id, '<span class="label label-success">Activé</span>')
                                             : anchor('admin/ups/activate/'. $onduleur->id, '<span class="label label-default">Inactif</span>'); ?></td>
              <td><a href="<?php echo site_url('admin/ups/edit/'.$onduleur->id); ?>" data-toggle="tooltip" title="Editer">
                  <?php echo $onduleur->tech_id; ?></a></td>
              <td><a href="<?php echo site_url('admin/ups/edit/'.$onduleur->id); ?>" data-toggle="tooltip" title="Editer">
                  <?php echo $onduleur->name; ?></a></td>
              <td class="hidden-xs"><?php echo $onduleur->host; ?></td>
              <td class="hidden-xs"><?php echo $onduleur->username; ?></td>
              <td class="hidden-xs"><?php echo $onduleur->password; ?></td>
              <td class="hidden-xs"><a href="<?php echo site_url('admin/ups/edit/'.$onduleur->id); ?>" data-toggle="tooltip" title="Editer">
                  <?php echo $onduleur->libelle; ?></a></td>
              <td class="hidden-xs"><?php echo $onduleur->date_create; ?></td>
              <td><?php echo anchor('admin/dls/run/'.$onduleur->tech_id, '<i class="fa fa-eye"></i> Voir le RUN', array('class' => 'btn btn-primary')); ?></td>
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
