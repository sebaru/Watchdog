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
	public function logout()
 	{	$this->wtd_log->add("User logoff");
    $this->session->unset_userdata(array('username', 'user_id', 'user_access_level'));

		// Destroy the session
		  $this->session->sess_destroy();

		//Recreate the session
				session_start();
	 		$this->session->sess_regenerate(TRUE);
  		return TRUE;
	 }

/******************************************************************************************************************************/
	public function logged_in()
	 { /*log_message('debug', 'logged_in as ' . $this->session->userdata('username') );*/
    $check = (bool)$this->session->userdata('username');
    return $check;
	 }
/******************************************************************************************************************************/
	public function logged_in_as_admin()
	 { /*log_message('debug', 'logged_in as ' . $this->session->userdata('username') );*/
    $check = (bool)$this->session->userdata('username');
    return($check && $this->session->user_access_level>=6);
	 }
}
