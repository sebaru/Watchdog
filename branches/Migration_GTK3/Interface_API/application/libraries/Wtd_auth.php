<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Wtd_auth
 {	public function __construct()
	  {	/*$this->load->library(array('email'));*/
		   $this->load->helper(array('cookie', 'language','url'));
		   $this->load->library('session');
		   $this->load->model('wtd_auth_model');
   } 

   public function __get($var)
    { return get_instance()->$var; }

/******************************************************************************************************************************/
	public function logged_in()
	 { log_message('debug', 'logged_in as ' . $this->session->userdata('username') );
    $check = (bool)$this->session->userdata('username');
    if (!$check && get_cookie('user_identity') && get_cookie('user_remember'))
		   { error_log( "logged_in : pas de session, mais cookie present. test des cookies..." );
       $check = $this->wtd_auth_model->login_remembered_user();
     }
    return $check;
	 }
}
