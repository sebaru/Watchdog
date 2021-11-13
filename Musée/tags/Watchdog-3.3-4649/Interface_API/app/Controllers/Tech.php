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
   { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }
     if ( session()->get('user')->access_level<6 ) { return redirect()->to('/auth/login'); }

    $data['title'] = "Chez moi !";
    echo view('Tech/header', $data);
    echo view('Tech/dashboard', $data);
    echo view('Tech/footer', $data);

  }

/******************************************************************************************************************************/
 public function log()
   { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }
     if ( session()->get('user')->access_level<6 ) { return redirect()->to('/auth/login'); }

    echo view('Tech/header');
    echo view('Tech/log');
    echo view('Tech/footer');

  }
/******************************************************************************************************************************/
 public function synoptiques()
   { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }
     if ( session()->get('user')->access_level<6 ) { return redirect()->to('/auth/login'); }

    echo view('Tech/header');
    echo view('Tech/synoptiques');
    echo view('Tech/footer');

   }
/******************************************************************************************************************************/
 public function process()
   { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }
     if ( session()->get('user')->access_level<6 ) { return redirect()->to('/auth/login'); }

    echo view('Tech/header');
    echo view('Tech/process');
    echo view('Tech/footer');

   }
/******************************************************************************************************************************/
 public function atelier()
   { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }
     if ( session()->get('user')->access_level<6 ) { return redirect()->to('/auth/login'); }

    echo view('Tech/header');
    echo view('Tech/atelier');
    echo view('Tech/footer');

   }
/******************************************************************************************************************************/
 public function atelier_list()
   { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }
     if ( session()->get('user')->access_level<6 ) { return redirect()->to('/auth/login'); }

    echo view('Tech/header');
    echo view('Tech/atelier_list');
    echo view('Tech/footer');

   }
/******************************************************************************************************************************/
 public function users_sessions()
   { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }
     if ( session()->get('user')->access_level<6 ) { return redirect()->to('/auth/login'); }

    echo view('Tech/header');
    echo view('Tech/users_sessions');
    echo view('Tech/footer');
   }
/******************************************************************************************************************************/
 public function users_list()
   { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }
     if ( session()->get('user')->access_level<6 ) { return redirect()->to('/auth/login'); }

    echo view('Tech/header');
    echo view('Tech/users_list');
    echo view('Tech/footer');
   }
/******************************************************************************************************************************/
 public function mnemos()
   { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }
     if ( session()->get('user')->access_level<6 ) { return redirect()->to('/auth/login'); }

    echo view('Tech/header');
    echo view('Tech/mnemos');
    echo view('Tech/footer');
   }
/******************************************************************************************************************************/
 public function dls()
   { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }
     if ( session()->get('user')->access_level<6 ) { return redirect()->to('/auth/login'); }

    echo view('Tech/header');
    echo view('Tech/dls');
    echo view('Tech/footer');
   }
}
