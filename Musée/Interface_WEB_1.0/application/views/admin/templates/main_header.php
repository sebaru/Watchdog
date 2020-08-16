  <header class="main-header">
    <!-- Logo -->

    <a href="<?php echo site_url('admin/dashboard'); ?>" class="logo">
    		<span class="logo-mini"><?php echo $instance_description ?></span>
		    <span class="logo-lg"><?php echo $instance_description ?></span>
	   </a>

    <!-- Header Navbar: style can be found in header.less -->
    <nav class="navbar navbar-static-top">
      <!-- Sidebar toggle button-->
      <a href="#" class="sidebar-toggle" data-toggle="push-menu" role="button">
        <span class="sr-only">Toggle navigation</span>
      </a>

      <div class="navbar-custom-menu">
        <ul class="nav navbar-nav">
          <!-- Messages: style can be found in dropdown.less-->

          <li>
            <a href="/"><i class="fa fa-image text-orange"></i><span>Mode Client</span></a>
          </li>

          <!-- User Account: style can be found in dropdown.less -->
          <li class="hidden-xs" class="dropdown">
            <a href="#" class="dropdown-toggle" data-toggle="dropdown"><i class="fa fa-user text-green"></i><?php echo " ".$this->session->username; ?></a>

            <ul class="dropdown-menu">
              <li>
                <a href="<?php echo site_url('admin/users/edit/'.$this->session->user_id); ?>" class="btn btn-secondary"><i class="fa fa-user"></i>Mon profil</a>
              </li>
              <li class="footer">
                <a href="<?php echo site_url('auth/logout/admin'); ?>" class="btn btn-secondary"><i class="fa fa-sign-out"></i>Se déconnecter</a>
              </li>
            </ul>
          </li>

        </ul>
      </div>
    </nav>
  </header>
