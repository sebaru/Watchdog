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
             <?php echo anchor('admin/tableau/create', '<i class="fa fa-plus"></i>Ajouter un tableau', array('class' => 'btn btn-primary')); ?>
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
            <th class="hidden-xs">#</th>
            <th>Access_Level</th>
            <th>Titre</th>
            <th class="hidden-xs">Date<br>Creation</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($tableaux as $tableau):?>
            <tr>
              <td>
                <div class="dropdown">
                  <button class="btn btn-xs btn-primary dropdown-toggle" type="button" data-toggle="dropdown"><span class="caret"></span></button>
                   <ul class="dropdown-menu">
                   <li><a href=<?php echo site_url('admin/tableau/edit/').$tableau->id; ?>>
                     <i class="fa fa-pencil" style="color:green"></i>
                     <i class="fa fa-line-chart" style="color:blue"></i>Editer ce tableau</a></li>
                   <li class="divider"></li>
                   <li><a href=<?php echo site_url('admin/tableau/delete/').$tableau->id; ?>>
                     <i class="fa fa-times" style="color:red"></i>Supprimer</a></li>
                   </ul>
               </div>
              </td>
              <td><a href="<?php echo site_url('archive/tableau/'.$tableau->id); ?>" data-toggle="tooltip" title="Voir les graphes">
                  <?php echo $tableau->id; ?></a></td>
              <td class="hidden-xs"><?php echo $tableau->access_level; ?></td>
              <td><a href="<?php echo site_url('admin/tableau/edit/'.$tableau->id); ?>" data-toggle="tooltip" title="Editer">
                  <?php echo $tableau->titre; ?></a></td>
              <td class="hidden-xs"><?php echo $tableau->date_create; ?></td>
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
