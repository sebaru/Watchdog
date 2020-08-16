<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Instance extends Admin_Controller{
 function __construct()
  { parent::__construct(); } 

/******************************************************************************************************************************/
 function index($instance=NULL)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
		
    if (isset($instance))
     { $this->wtd_webservice->set_instance($instance);
     }
			 redirect( 'admin/dashboard' );
		}
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
