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
             <h3 class="box-title"><?php echo anchor('admin/icons/class_create', '<i class="fa fa-plus"></i>Ajouter une classe', array('class' => 'btn btn-block btn-primary')); ?></h3>
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
                 <th>Description</th>
                 <th>Date Création</th>
               </tr>
             </thead>
             <tbody>
               <?php foreach ($classes as $class):?>
               <tr>
                 <td>
                   <div class="dropdown">
                     <button class="btn btn-xs btn-primary dropdown-toggle" type="button" data-toggle="dropdown">
                       <span class="caret"></span>
                     </button>
                     <ul class="dropdown-menu">
                       <li><a href="<?php echo site_url('admin/icons/class_edit/'.$class->id); ?>">
                         <i class="fa fa-pencil" style="color:green"></i>
                         <i class="fa fa-file-photo-o" style="color:blue"></i>Editer la classe</a></li>
                       <li><a href="<?php echo site_url('admin/icons/icon_list/'.$class->id); ?>">
                         <i class="fa fa-list" style="color:green"></i>
                         <i class="fa fa-crop" style="color:blue"></i>Liste des icones</a></li>
                       <li><a href="<?php echo site_url('admin/icons/icon_create/'.$class->id); ?>">
                         <i class="fa fa-plus" style="color:green"></i>
                         <i class="fa fa-crop" style="color:blue"></i>Ajouter un icone</a></li>
                       <li class="divider"></li>
                       <li><a href="<?php echo site_url('admin/icons/class_delete/'.$class->id); ?>">
                         <i class="fa fa-times" style="color:red"></i>Supprimer</a></li>
                      </ul>
                    </div>
                 </td>             
                 <td><?php echo $class->id; ?></td>
                 <td><a href="<?php echo site_url('admin/icons/icon_list/'.$class->id); ?>" data-toggle="tooltip" title="Voir">
                     <?php echo htmlspecialchars($class->libelle, ENT_QUOTES, 'UTF-8'); ?></a></td>
                 <td><?php echo $class->date_create; ?></td>
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
