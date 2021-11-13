<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Auth extends MY_Controller {

	function __construct()
 	{	parent::__construct();	}

/******************************************************************************************************************************/
	function index()
	 { if ( ! $this->wtd_auth->logged_in() )
         { redirect('auth/login', 'refresh'); }
    else { redirect('/', 'refresh'); }
	 }


/******************************************************************************************************************************/
 function login()
 	{ if ( ! $this->wtd_auth->logged_in())
     { /* Valid form */
       $this->form_validation->set_rules('username', 'Username', 'required');
       $this->form_validation->set_rules('password', 'Password', 'required');

       if ($this->form_validation->run() == TRUE)
        { $remember = (bool) $this->input->post('remember');

          if ($this->wtd_auth_model->login($this->input->post('username'), $this->input->post('password'), $remember))
           { redirect('admin/dashboard'); }
          else
           { $this->session->set_flashdata('message', 'Username or Password Error');
				         redirect('auth/login', 'refresh');
           }
        }
       else
        { $this->data['message'] = (validation_errors()) ? validation_errors() : $this->session->flashdata('message');
          $this->data['username'] = array( 'name'        => 'username',
                                           'id'          => 'username',
                                           /*'type'        => 'email',*/
                                           'value'       => $this->form_validation->set_value('identity'),
                                           'class'       => 'form-control',
                                           'placeholder' => $this->lang->line('login_identity_label')
                                         );
          $this->data['password'] = array( 'name'        => 'password',
                                           'id'          => 'password',
                                           'type'        => 'password',
                                           'class'       => 'form-control',
                                           'placeholder' => $this->lang->line('login_your_password')
                                         );
          /* Load Template */
          $this->data['header']  = $this->load->view('auth/header', $this->data, TRUE);
          $this->data['content'] = $this->load->view('auth/login', $this->data, TRUE);
          $this->data['footer']  = $this->load->view('auth/footer', $this->data, TRUE);
          return $this->load->view('auth/template', $this->data);
        }
     }
    else { redirect('/', 'refresh'); }
  }
/******************************************************************************************************************************/
 function login_test()
 	{ if ($this->wtd_auth->logged_in()) exit();

    $input = json_decode(file_get_contents('php://input'));
    if ($input==NULL) { echo json_encode( array( "Parsing" => "Error !" ) ); exit(); }

    if ($this->wtd_auth_model->login($input->username, $input->password, "false"))
     { echo json_encode( array( "success" => "true", "login_status" => "OK" ) ); }
    else
     { echo json_encode( array( "success" => "false", "login_status" => "FAILED" ) );
     }
    exit();
  }

/******************************************************************************************************************************/
 function logout()
 	{ $logout = $this->wtd_auth->logout();
    $this->session->set_flashdata('message', 'Session terminée');
    redirect('auth/login', 'refresh');
	 }

	function _get_csrf_nonce()
	{
		$this->load->helper('string');
		$key   = random_string('alnum', 8);
		$value = random_string('alnum', 20);
		$this->session->set_flashdata('csrfkey', $key);
		$this->session->set_flashdata('csrfvalue', $value);

		return array($key => $value);
	}

	function _valid_csrf_nonce()
	{
		if ($this->input->post($this->session->flashdata('csrfkey')) !== FALSE &&
			$this->input->post($this->session->flashdata('csrfkey')) == $this->session->flashdata('csrfvalue'))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}


	
}
