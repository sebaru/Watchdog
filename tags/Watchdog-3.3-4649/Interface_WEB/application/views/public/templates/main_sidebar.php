<?php
defined('BASEPATH') OR exit('No direct script access allowed');

?>

<aside class="main-sidebar">
  <section class="sidebar">

                    <!-- Sidebar menu -->
    <ul class="sidebar-menu">

      <li class="header text-uppercase">Navigation principale</li>
						<li>
        <a href="<?php echo site_url('/'); ?>"> <i class="fa fa-image text-blue"></i> <span>Accueil</span> </a>
      </li>

 				 <li>
        <a href="<?php echo site_url('/msgs'); ?>"> <i class="fa fa-envelope text-blue"></i> <span>Messages</span> </a>
      </li>

 				 <li>
        <a href="<?php echo site_url('/msgs/histos'); ?>"> <i class="fa fa-history text-blue"></i> <span>Historiques</span> </a>
      </li>

 				 <li>
        <a href="<?php echo site_url('/tableau'); ?>"> <i class="fa fa-line-chart text-blue"></i> <span>Tableaux</span> </a>
      </li>

 				 <li>
        <a href="<?php echo site_url('/horloges'); ?>"> <i class="fa fa-calendar text-blue"></i> <span>Horloges</span> </a>
      </li>

    </ul>
  </section>
</aside>
