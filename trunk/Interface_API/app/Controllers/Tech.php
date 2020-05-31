<?php namespace App\Controllers;

class Tech extends BaseController
{
  public	function __construct()
	  {
/*parent::__construct();*/

/*		$this->form_validation->set_error_delimiters($this->config->item('error_start_delimiter', 'wtd_auth'), $this->config->item('error_end_delimiter', 'wtd_auth'));
  log_message('debug', 'test seb 1');
		$this->lang->load('auth');*/
	  }

/******************************************************************************************************************************/
 public function index()
  { return redirect()->to('/tech/dashboard');
  }


/******************************************************************************************************************************/
 public function dashboard()
   { if ( !isset(session()->get('user') )
      { return redirect()->to('/auth/login'); }
     if ( session()->get('user')->access_level<6 )
      { return redirect()->to('/auth/login'); }

    $data['title'] = "Chez moi !";
    echo view('Tech/header', $data);
    echo view('Tech/dashboard', $data);
    echo view('Tech/footer', $data);

  }
}
