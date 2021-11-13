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
             <?php echo anchor('admin/modbus/create', '<i class="fa fa-plus"></i>Ajouter un Wago/Modbus', array('class' => 'btn btn-primary')); ?>
             <?php echo anchor('admin/modbus/run', '<i class="fa fa-eye"></i> Voir le RUN', array('class' => 'btn btn-primary')); ?>
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
            <th class="hidden-xs">ID</th>
            <th>Enable</th>
            <th class="hidden-xs">Hostname</th>
            <th>Tech_id</th>
            <th>Description</th>
            <th class="hidden-xs">map_E</th>
            <th class="hidden-xs">max_nbr_E</th>
            <th class="hidden-xs">map_EA</th>
            <th class="hidden-xs">map_A</th>
            <th class="hidden-xs">map_AA</th>
            <th class="hidden-xs">watchdog</th>
            <th class="hidden-xs">Date<br>Creation</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($modbus as $module):?>
            <tr>
              <td>
                <div class="dropdown">
                  <button class="btn btn-xs btn-primary dropdown-toggle" type="button" data-toggle="dropdown"><span class="caret"></span></button>
                   <ul class="dropdown-menu">
                   <li><a href=<?php echo site_url('admin/modbus/edit/').$module->id; ?>>
                     <i class="fa fa-pencil" style="color:green"></i>
                     <i class="fa fa-cogs" style="color:blue"></i>Editer ce module</a></li>
                   <li><a href=<?php echo site_url('admin/modbus/run/').$module->id; ?>>
                     <i class="fa fa-pencil" style="color:green"></i>
                     <i class="fa fa-eye" style="color:blue"></i>Running Conf</a></li>
                   <li class="divider"></li>
                   <li><a href=<?php echo site_url('admin/modbus/delete/').$module->id; ?>>
                     <i class="fa fa-times" style="color:red"></i>Supprimer</a></li>
                   </ul>
               </div>
              </td>
              <td class="hidden-xs"><?php echo htmlspecialchars($module->id, ENT_QUOTES, 'UTF-8'); ?></td>
              <td><?php echo $module->enable ? anchor('admin/modbus/deactivate/'.$module->id, '<span class="label label-success">Activé</span>')
                                             : anchor('admin/modbus/activate/'. $module->id, '<span class="label label-default">Inactif</span>'); ?></td>
              <td class="hidden-xs"><?php echo htmlspecialchars($module->hostname, ENT_QUOTES, 'UTF-8'); ?></td>
              <td><a href="<?php echo site_url('admin/modbus/edit/'.$module->id); ?>" data-toggle="tooltip" title="Editer">
                  <?php echo htmlspecialchars($module->tech_id, ENT_QUOTES, 'UTF-8'); ?></a></td>
              <td><?php echo htmlspecialchars($module->description, ENT_QUOTES, 'UTF-8'); ?></td>
              <td class="hidden-xs"><?php echo htmlspecialchars($module->map_E, ENT_QUOTES, 'UTF-8'); ?></td>
              <td class="hidden-xs"><?php echo htmlspecialchars($module->max_nbr_E, ENT_QUOTES, 'UTF-8'); ?></td>
              <td class="hidden-xs"><?php echo htmlspecialchars($module->map_EA, ENT_QUOTES, 'UTF-8'); ?></td>
              <td class="hidden-xs"><?php echo htmlspecialchars($module->map_A, ENT_QUOTES, 'UTF-8'); ?></td>
              <td class="hidden-xs"><?php echo htmlspecialchars($module->map_AA, ENT_QUOTES, 'UTF-8'); ?></td>
              <td class="hidden-xs"><?php echo htmlspecialchars($module->watchdog, ENT_QUOTES, 'UTF-8'); ?></td>
              <td class="hidden-xs"><?php echo htmlspecialchars($module->date_create, ENT_QUOTES, 'UTF-8'); ?></td>
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
