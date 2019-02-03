<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Wtd_auth_model extends CI_Model
{	public function __construct()
	 {	$this->load->database();
   	$this->load->helper('cookie');
		  $this->load->helper('date');
		  $this->lang->load('auth');
  }

/******************************************************************************************************************************/
	public function activate($id)
 	{ $data = array( 'enable' => 1	);
 		 $this->db->update('users', $data, array('id' => $id));

  		$return = $this->db->affected_rows() == 1;
	   return $return;
 	}

/******************************************************************************************************************************/
	public function deactivate($id = NULL)
 	{	if ($this->wtd_auth->logged_in() && $this->session->user_id == $id)
 		  {	$this->set_error('deactivate_current_user_unsuccessful');
			    return FALSE;
		   }
 
 		 $data = array( 'enable' => 0	);
 
 		 $this->db->update('users', $data, array('id' => $id));
 
 		 $return = $this->db->affected_rows() == 1;
 		 return $return;
	}
/******************************************************************************************************************************/
	public function sms_activate($id)
 	{ $data = array( 'sms_enable' => 1	);
 		 $this->db->update('users', $data, array('id' => $id));

  		$return = $this->db->affected_rows() == 1;
 		 return $return;
 	}

/******************************************************************************************************************************/
	public function sms_deactivate($id = NULL)
 	{	$data = array( 'sms_enable' => 0	);
 
 		 $this->db->update('users', $data, array('id' => $id));
 
 		 $return = $this->db->affected_rows() == 1;
	 	 return $return;
	 }

/******************************************************************************************************************************/
	public function register($data = array())
	 { $this->db->set('username',      $data['username'] );
    $this->db->set('sms_phone',     $data['sms_phone'] );
    $this->db->set('access_level',  $data['access_level'] );
    $this->db->set('comment',       $data['comment'] );
    $this->db->set('imsg_enable',   $data['imsg_enable'] );
    $this->db->set('imsg_allow_cde',$data['imsg_allow_cde'] );
    $this->db->set('imsg_jabberid', $data['imsg_jabberid'] );
    $this->db->set('email',         $data['email'] );
    $this->db->set('enable',        $data['enable'] );
    $this->db->set('date_create',   date("Y-m-d H:i:s"));
    $this->db->set('date_modif',    date("Y-m-d H:i:s"));

    if (array_key_exists('password', $data))
     { $this->db->set('hash', password_hash($data['password'], PASSWORD_BCRYPT ) ); }
    
    $this->db->insert('users');

		  $id = $this->db->insert_id();
    return (isset($id));
	}

/******************************************************************************************************************************/
	public function login($username, $password, $remember=FALSE)
 	{	if (empty($username) || empty($password))
		   {	return FALSE; }
    
  		$query = $this->db->where('username', $username)
					   	             ->limit(1)
						                ->get('users');

		  //if ($this->is_max_login_attempts_exceeded($identity))
   		//{	$this->set_error('login_timeout');
//    			return FALSE;
		   //}

  		if ($query->num_rows() === 1)
   		{	$user = $query->row();
    			$check = password_verify ( $password, $user->hash );
       if ($check === TRUE)
			    {	if ($user->enable == 0)
          {	$this->wtd_log->add("Login failed (user ".$username." disabled)");
            return FALSE;
          }
         $this->set_session($user);
         $this->wtd_log->add("User login success");
     		/*		$this->update_last_login($user->id);
     				$this->clear_login_attempts($identity);*/

					  /*$this->remember_user($user->id);*/
      			return TRUE;
			    }
		   }
    $this->wtd_log->add("Login failed (user ".$username." wrong password)");
  		return FALSE;
	 }

/******************************************************************************************************************************/
	public function user_list()
 	{	$this->db->select( '*' );
  		$this->db->where('access_level<=', $this->session->user_access_level);
		
	  	$response = $this->db->get('users');
    error_log ( 'last_query = ' . $this->db->last_query() );
 	 	return $response;
	 }

/******************************************************************************************************************************/
	public function user($id = NULL)
	 {	$id = isset($id) ? $id : $this->session->userdata('user_id');
 		 $this->db->select( '*' );
  	 $this->db->where('access_level<=', $this->session->user_access_level);
		  $this->db->where('users.id=', $id);
 		 $this->db->limit(1);
  	 $response = $this->db->get('users');
    error_log ( 'last_query = ' . $this->db->last_query() );
		  return $response;
	 }

/******************************************************************************************************************************/
	public function update($id, array $data)
	 {
    $this->db->set('sms_phone',     $data['sms_phone'] );
    $this->db->set('access_level',  $data['access_level'] );
    $this->db->set('comment',       $data['comment'] );
    $this->db->set('imsg_enable',   $data['imsg_enable'] );
    $this->db->set('imsg_allow_cde',$data['imsg_allow_cde'] );
    $this->db->set('imsg_jabberid', $data['imsg_jabberid'] );
    $this->db->set('email',         $data['email'] );
    $this->db->set('date_modif',    'NOW()');

    if (array_key_exists('password', $data))
     { $this->db->set('hash', password_hash($data['password'], PASSWORD_BCRYPT ) ); }
    
    $this->db->where('id', $id);
    $this->db->where('access_level<=', $this->session->user_access_level );

    $retour = $this->db->update('users');
    error_log ( 'last_query = ' . $this->db->last_query() );
    return $retour;
 	}
/******************************************************************************************************************************/
	public function delete($id)
	{
    $this->db->where('id', $id);
    $this->db->where('access_level<=', $this->session->user_access_level );

    $retour = $this->db->delete('users');
 			return $retour;
	}
/******************************************************************************************************************************/
	public function set_session($user)
	 {	$session_data = array( 'username'             => $user->username,
		                         'user_email'           => $user->email,
		                         'user_access_level'    => $user->access_level,
		                         'user_id'              => $user->id,
		                       );
    log_message ( 'debug', 'New session for : '.$user->username . '-' . $user->email . '-' . $user->access_level . '-' . $user->id);
 		 $this->session->set_userdata($session_data);
	 	 return TRUE;
	 }

	/**
	 * login_remembed_user
	 *
	 * @return bool
	 * @author Ben Edmunds
	 */
	public function login_remembered_user()
 	{

		// check for valid data
	  	if (!get_cookie('user_identity')	|| !get_cookie('user_remember')	|| !$this->identity_check('user_identity'))
		   {	return FALSE;	}

		// get the user
		$query = $this->db->select($this->identity_column . ', id, email, last_login')
						  ->where($this->identity_column, urldecode(get_cookie($this->config->item('identity_cookie_name', 'ion_auth'))))
						  ->where('remember_code', get_cookie($this->config->item('remember_cookie_name', 'ion_auth')))
						  ->where('active', 1)
						  ->limit(1)
						  ->order_by('id', 'desc')
						  ->get($this->tables['users']);

		// if the user was found, sign them in
		if ($query->num_rows() == 1)
		{
			$user = $query->row();

			$this->update_last_login($user->id);

			$this->set_session($user);

			// extend the users cookies if the option is enabled
			if ($this->config->item('user_extend_on_login', 'ion_auth'))
			{
				$this->remember_user($user->id);
			}

			$this->trigger_events(array('post_login_remembered_user', 'post_login_remembered_user_successful'));
			return TRUE;
		}

		$this->trigger_events(array('post_login_remembered_user', 'post_login_remembered_user_unsuccessful'));
		return FALSE;
	}

}
