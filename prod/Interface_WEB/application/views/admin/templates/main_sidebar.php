<?php
defined('BASEPATH') OR exit('No direct script access allowed');

?>

<aside class="main-sidebar">
  <section class="sidebar">
          <!-- Sidebar menu -->
    <ul class="sidebar-menu">
      <li class="header text-uppercase">Navigation principale</li>

      <li class="<?=active_link_controller('dashboard')?>">
       <a href="<?php echo site_url('admin/dashboard'); ?>"> <i class="fa fa-dashboard"></i> <span>Tableau de bord</span>
       </a>
      </li>

						<li class="header text-uppercase">Réglages</li>

						<li class="<?=active_link_controller('syn')?>">
        <a href="<?php echo site_url('admin/syn'); ?>"> <i class="fa fa-image"></i> <span>Synoptiques</span> </a>
      </li>

 				 <li class="<?=active_link_controller('dls')?>">
        <a href="<?php echo site_url('admin/dls'); ?>"> <i class="fa fa-code"></i> <span>Modules D.L.S</span> </a>
      </li>

 				 <li class="<?=active_link_controller('tableau')?>">
        <a href="<?php echo site_url('admin/tableau'); ?>"> <i class="fa fa-line-chart"></i> <span>Tableaux</span> </a>
      </li>

						<li class="header text-uppercase">Input/Ouput Config</li>
      <li>
        <a href="<?php echo site_url('admin/modbus'); ?>"> <i class="fa fa-cogs"></i> <span>Module Wago/Modbus</span> </a>
      </li>
      <li>
        <a href="<?php echo site_url('admin/ups'); ?>"> <i class="fa fa-battery-half"></i> <span>Onduleurs</span> </a>
      </li>

						<li class="header text-uppercase"><?php echo lang('menu_administration'); ?></li>

      <?php if ($this->session->user_access_level>=6) { ?>
      <li>
        <a href="<?php echo site_url('admin/process'); ?>"><i class="fa fa-microchip"></i> <span>Gestion des Processus</span></a>
      </li>

      <li>
        <a href="<?php echo site_url('admin/maintenance'); ?>"><i class="fa fa-wrench"></i> <span>Maintenance</span></a>
      </li>

      <li>
        <a href="<?php echo site_url('admin/users'); ?>"><i class="fa fa-user"></i> <span>Gestion des utilisateurs</span></a>
      </li>

      <li>
        <a href="https://icons.abls-habitat.fr/admin/icons"><i class="fa fa-file-photo-o"></i> <span>Gestion des Icones</span></a>
      </li>

      <li>
        <a href="<?php echo site_url('admin/log'); ?>"><i class="fa fa-database"></i> <span>Audit Log</span></a>
      </li>

      <?php } ?>

                    </ul>
                </section>
            </aside>
