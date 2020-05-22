<?php namespace App\Controllers;

class Auth extends BaseController
{
  public	function __construct()
	  {	/*parent::__construct();*/

/*		$this->form_validation->set_error_delimiters($this->config->item('error_start_delimiter', 'wtd_auth'), $this->config->item('error_end_delimiter', 'wtd_auth'));
  log_message('debug', 'test seb 1');
		$this->lang->load('auth');*/
	  }

/******************************************************************************************************************************/
 public function index()
  { return redirect()->to('auth/login');
  }


/******************************************************************************************************************************/
 public function login()
  {
    $data['title'] = "Chez moi";
    echo view('Auth/header', $data);
    echo view('Auth/body', $data);
    echo view('Auth/footer', $data);

  }

}
