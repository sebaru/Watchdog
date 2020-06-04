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
      <h3 class="box-title"><?php echo anchor('admin/users/create', '<i class="fa fa-plus"></i> '. lang('users_create_user'), array('class' => 'btn btn-block btn-primary')); ?></h3>
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
         <th>Status</th>
									<th>Access level</th>
         <th>Login</th>
         <th class="hidden-xs">Description</th>
         <th class="hidden-xs">E-mail</th>
         <th class="hidden-xs">Envoi SMS</th>
         <th class="hidden-xs">Téléphone</th>
         <th class="hidden-xs">Date Création</th>
         <th class="hidden-xs">Date Modif</th>
        </tr>
       </thead>
       <tbody>
        <?php foreach ($users as $user):?>
         <tr>
          <td>
           <div class="dropdown">
            <button class="btn btn-xs btn-primary dropdown-toggle" type="button" data-toggle="dropdown">
              <span class="caret"></span>
            </button>
            <ul class="dropdown-menu">
             <li><a href="<?php echo site_url('admin/users/edit/'.$user->id); ?>">
              <i class="fa fa-pencil" style="color:green"></i>
              <i class="fa fa-user" style="color:blue"></i>Editer</a></li>
             <li class="divider"></li>
             <li><a href="<?php echo site_url('admin/users/delete/'.$user->id); ?>">
              <i class="fa fa-times" style="color:red"></i>Supprimer</a></li>
            </ul>
           </div>
          </td>
          <td class="hidden-xs"><?php echo htmlspecialchars($user->id, ENT_QUOTES, 'UTF-8'); ?></td>
          <td><?php echo $user->enable ? anchor('admin/users/deactivate/'.$user->id, '<span class="label label-success">Activé</span>')
                                       : anchor('admin/users/activate/'. $user->id, '<span class="label label-default">Inactif</span>'); ?></td>
          <td><?php echo htmlspecialchars($user->access_level, ENT_QUOTES, 'UTF-8'); ?></td>
          <td><a href="<?php echo site_url('admin/users/edit/'.$user->id); ?>" data-toggle="tooltip" title="Editer">
              <?php echo htmlspecialchars($user->username, ENT_QUOTES, 'UTF-8'); ?></a></td>
          <td class="hidden-xs"><?php echo htmlspecialchars($user->comment, ENT_QUOTES, 'UTF-8'); ?></td>
          <td class="hidden-xs"><?php echo htmlspecialchars($user->email, ENT_QUOTES, 'UTF-8'); ?></td>
          <td class="hidden-xs"><?php echo $user->sms_enable ? anchor('admin/users/sms_deactivate/'.$user->id, '<span class="label label-success">Activé</span>')
                                           : anchor('admin/users/sms_activate/'. $user->id, '<span class="label label-default">Inactif</span>'); ?></td>
          <td class="hidden-xs"><?php echo htmlspecialchars($user->sms_phone, ENT_QUOTES, 'UTF-8'); ?></td>
          <td class="hidden-xs"><?php echo htmlspecialchars($user->date_create, ENT_QUOTES, 'UTF-8'); ?></td>
          <td class="hidden-xs"><?php echo htmlspecialchars($user->date_modif, ENT_QUOTES, 'UTF-8'); ?></td>
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
