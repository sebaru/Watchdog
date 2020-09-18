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
   {

    $data['title'] = "Chez moi !";
    echo view('Tech/header', $data);
    echo view('Tech/dashboard', $data);
    echo view('Tech/footer', $data);

  }

/******************************************************************************************************************************/
 public function log()
   {

    echo view('Tech/header');
    echo view('Tech/log');
    echo view('Tech/footer');

  }
/******************************************************************************************************************************/
 public function synoptiques()
   {

    echo view('Tech/header');
    echo view('Tech/synoptiques');
    echo view('Tech/footer');

   }
/******************************************************************************************************************************/
 public function process( $thread=NULL )
   {

    echo view('Tech/header');
    if (!$thread) echo view('Tech/process');
    else echo view('Tech/'.$thread);
    echo view('Tech/footer');

   }
/******************************************************************************************************************************/
 public function atelier()
   {
    echo view('Tech/header');
    echo view('Tech/atelier');
    echo view('Tech/footer');

   }
/******************************************************************************************************************************/
 public function atelier_list()
   {

    echo view('Tech/header');
    echo view('Tech/atelier_list');
    echo view('Tech/footer');

   }
/******************************************************************************************************************************/
 public function users_sessions()
   {

    echo view('Tech/header');
    echo view('Tech/users_sessions');
    echo view('Tech/footer');
   }
/******************************************************************************************************************************/
 public function users_list()
   {

    echo view('Tech/header');
    echo view('Tech/users_list');
    echo view('Tech/footer');
   }
/******************************************************************************************************************************/
 public function mnemos()
   {

    echo view('Tech/header');
    echo view('Tech/mnemos');
    echo view('Tech/footer');
   }
/******************************************************************************************************************************/
 public function dls()
   {

    echo view('Tech/header');
    echo view('Tech/dls');
    echo view('Tech/footer');
   }
/******************************************************************************************************************************/
 public function dls_source()
   {

    echo view('Tech/header');
    echo view('Tech/dls_source');
    echo view('Tech/footer');
   }
/******************************************************************************************************************************/
 public function run()
  { echo view('Tech/header');
    echo view('Tech/run');
    echo view('Tech/footer');
  }
/******************************************************************************************************************************/
 public function dls_run()
  { echo view('Tech/header');
    echo view('Tech/dls_run');
    echo view('Tech/footer');
  }
/******************************************************************************************************************************/
 public function modbus()
  { echo view('Tech/header');
    echo view('Tech/modbus');
    echo view('Tech/footer');
  }
/******************************************************************************************************************************/
 public function modbus_map()
  { echo view('Tech/header');
    echo view('Tech/modbus_map');
    echo view('Tech/footer');
  }
}
