<?php namespace App\Controllers;

class Auth extends BaseController
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
  { return redirect()->to('/auth/login');
  }


/******************************************************************************************************************************/
 public function login()
  {
    if ( $this->request->getPost('username') !== null && $this->request->getPost('password') !== null )
     { $ch = curl_init( "http://".$this->request->getPost('username').":".$this->request->getPost('password')."@localhost:5560/connect" );
       curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       $source = curl_exec($ch);
       if ($source===FALSE)
        { curl_close($ch);
          return redirect()->to('');
        }
       $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
       if ($code == 401)
        { curl_close($ch);
          $this->response->setStatusCode(401, "UnAuthorized");
          return;
        }
       /*log_message ( "error", $source );*/
       $result = json_decode($source);
       curl_close($ch);
       if (!isset($result->wtd_session))
        { $this->response->setStatusCode(401, "NoCookie");
          return;
        }
       session()->set( 'user', $result );
       if (session()->get('user')->access_level >= 6)
        { return redirect()->to('/tech'); }
       else
        { return redirect()->to('/'); }
     }

    $data['title'] = "Chez moi !";
    echo view('Auth/header', $data);
    echo view('Auth/body', $data);
    echo view('Auth/footer', $data);

  }
}
