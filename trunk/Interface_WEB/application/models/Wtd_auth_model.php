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
    $this->db->set('session_id',    "none");
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

  		if ($query->num_rows() === 1)
   		{	$user = $query->row();
    			$check = password_verify ( $password, $user->hash );
       if ($check === TRUE)
			    {	if ($user->enable == 0)
          {	$this->wtd_log->add("Login failed (user ".$username." disabled)");
            return FALSE;
          }
         $user->password = $password;
         $this->set_session($user);
         $this->db->where('username', $username)->set('session_id', $this->session->session_id)->update('users');

         $this->wtd_log->add("User login success");
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
    $this->db->set('comment',       $data['comment'] );
    $this->db->set('imsg_enable',   $data['imsg_enable'] );
    $this->db->set('imsg_allow_cde',$data['imsg_allow_cde'] );
    $this->db->set('imsg_jabberid', $data['imsg_jabberid'] );
    $this->db->set('email',         $data['email'] );
    if (array_key_exists ( 'access_level', $data ))
     { $this->db->set('access_level',  $data['access_level'] ); }
    $this->db->set('date_modif',    'NOW()');

    if (array_key_exists( 'password', $data ))
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
		                         'user_password'        => $user->password,
		                         'user_email'           => $user->email,
		                         'user_access_level'    => $user->access_level,
		                         'user_id'              => $user->id,
		                       );
    error_log ( 'New session for : '.$user->username . '-' . $user->email . '-' . $user->access_level . '-' . $user->id);
 		 $this->session->set_userdata($session_data);
	 	 return TRUE;
	 }

}
