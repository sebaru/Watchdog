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
             <h3 class="box-title"><?php echo anchor('admin/icons/icon_create/'.$class_id, '<i class="fa fa-plus"></i>Ajouter un icone', array('class' => 'btn btn-block btn-primary')); ?></h3>
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
                 <th>ID</th>
                 <th>Type</th>
                 <th>Image</th>
                 <th>Description</th>
                 <th>Date d'ajout</th>
               </tr>
             </thead>
             <tbody>
               <?php foreach ($icons as $icon):?>
               <tr>
                 <td>
                   <div class="dropdown">
                     <button class="btn btn-xs btn-primary dropdown-toggle" type="button" data-toggle="dropdown">
                       <span class="caret"></span>
                     </button>
                     <ul class="dropdown-menu">
                       <li><a href="<?php echo site_url('admin/icons/icon_edit/'.$icon->id); ?>">
                         <i class="fa fa-pencil" style="color:green"></i>
                         <i class="fa fa-crop" style="color:blue"></i>Editer l'icone</a></li>
                       <li class="divider"></li>
                       <li><a href="<?php echo site_url('admin/icons/icon_delete/'.$icon->id_classe.'/'.$icon->id); ?>">
                         <i class="fa fa-times" style="color:red"></i>Supprimer</a></li>
                      </ul>
                    </div>
                 </td>
                 <td><?php echo $icon->id; ?></td>
                 <td><?php echo $icon->type; ?></td>
                 <td><a href="<?php echo site_url('admin/icons/icon_edit/'.$icon->id); ?>" data-toggle="tooltip" title="Voir">
                     <img <?php echo 'src="https://icons.abls-habitat.fr/assets/gif/'.$icon->id.'.'.$icon->type.'"'; ?> /></a></td>
                 <td><a href="<?php echo site_url('admin/icons/icon_edit/'.$icon->id); ?>" data-toggle="tooltip" title="Voir">
                     <?php echo htmlspecialchars($icon->libelle, ENT_QUOTES, 'UTF-8'); ?></a></td>
                 <td><?php echo $icon->date_create; ?></td>
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
