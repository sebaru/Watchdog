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

	/**
	 * Forgotten password feature
	 *
	 * @param string $identity
	 *
	 * @return array|bool
	 * @author Mathew
	 */
	public function forgotten_password($identity)
	{
		if ($this->wtd_auth_model->forgotten_password($identity))
		{
			// Get user information
			$identifier = $this->wtd_auth_model->identity_column; // use model identity column, so it can be overridden in a controller
			$user = $this->where($identifier, $identity)->where('active', 1)->users()->row();

			if ($user)
			{
				$data = array(
					'identity' => $user->{$this->config->item('identity', 'wtd_auth')},
					'forgotten_password_code' => $user->forgotten_password_code
				);

				if (!$this->config->item('use_ci_email', 'wtd_auth'))
				{
					$this->set_message('forgot_password_successful');
					return $data;
				}
				else
				{
					$message = $this->load->view($this->config->item('email_templates', 'wtd_auth') . $this->config->item('email_forgot_password', 'wtd_auth'), $data, TRUE);
					$this->email->clear();
					$this->email->from($this->config->item('admin_email', 'wtd_auth'), $this->config->item('site_title', 'wtd_auth'));
					$this->email->to($user->email);
					$this->email->subject($this->config->item('site_title', 'wtd_auth') . ' - ' . $this->lang->line('email_forgotten_password_subject'));
					$this->email->message($message);

					if ($this->email->send())
					{
						$this->set_message('forgot_password_successful');
						return TRUE;
					}
					else
					{
						$this->set_error('forgot_password_unsuccessful');
						return FALSE;
					}
				}
			}
			else
			{
				$this->set_error('forgot_password_unsuccessful');
				return FALSE;
			}
		}
		else
		{
			$this->set_error('forgot_password_unsuccessful');
			return FALSE;
		}
	}

	/**
	 * forgotten_password_complete
	 *
	 * @param string $code
	 *
	 * @return array|bool
	 * @author Mathew
	 */
	public function forgotten_password_complete($code)
	{
		$this->wtd_auth_model->trigger_events('pre_password_change');

		$identity = $this->config->item('identity', 'wtd_auth');
		$profile  = $this->where('forgotten_password_code', $code)->users()->row(); // pass the code to profile

		if (!$profile)
		{
			$this->wtd_auth_model->trigger_events(array('post_password_change', 'password_change_unsuccessful'));
			$this->set_error('password_change_unsuccessful');
			return FALSE;
		}

		$new_password = $this->wtd_auth_model->forgotten_password_complete($code, $profile->salt);

		if ($new_password)
		{
			$data = array(
				'identity'     => $profile->{$identity},
				'new_password' => $new_password
			);
			if(!$this->config->item('use_ci_email', 'wtd_auth'))
			{
				$this->set_message('password_change_successful');
				$this->wtd_auth_model->trigger_events(array('post_password_change', 'password_change_successful'));
				return $data;
			}
			else
			{
				$message = $this->load->view($this->config->item('email_templates', 'wtd_auth').$this->config->item('email_forgot_password_complete', 'wtd_auth'), $data, true);

				$this->email->clear();
				$this->email->from($this->config->item('admin_email', 'wtd_auth'), $this->config->item('site_title', 'wtd_auth'));
				$this->email->to($profile->email);
				$this->email->subject($this->config->item('site_title', 'wtd_auth') . ' - ' . $this->lang->line('email_new_password_subject'));
				$this->email->message($message);

				if ($this->email->send())
				{
					$this->set_message('password_change_successful');
					$this->wtd_auth_model->trigger_events(array('post_password_change', 'password_change_successful'));
					return TRUE;
				}
				else
				{
					$this->set_error('password_change_unsuccessful');
					$this->wtd_auth_model->trigger_events(array('post_password_change', 'password_change_unsuccessful'));
					return FALSE;
				}

			}
		}

		$this->wtd_auth_model->trigger_events(array('post_password_change', 'password_change_unsuccessful'));
		return FALSE;
	}

	/**
	 * forgotten_password_check
	 *
	 * @param string $code
	 *
	 * @return object|bool
	 * @author Michael
	 */
	public function forgotten_password_check($code)
	{
		$profile = $this->where('forgotten_password_code', $code)->users()->row(); // pass the code to profile

		if (!is_object($profile))
		{
			$this->set_error('password_change_unsuccessful');
			return FALSE;
		}
		else
		{
			if ($this->config->item('forgot_password_expiration', 'wtd_auth') > 0)
			{
				//Make sure it isn't expired
				$expiration = $this->config->item('forgot_password_expiration', 'wtd_auth');
				if (time() - $profile->forgotten_password_time > $expiration)
				{
					//it has expired
					$this->wtd_auth_model->clear_forgotten_password_code($code);
					$this->set_error('password_change_unsuccessful');
					return FALSE;
				}
			}
			return $profile;
		}
	}

/******************************************************************************************************************************/
	public function logout()
 	{	$this->wtd_log->add("User logoff");
    $this->session->unset_userdata(array('username', 'user_id', 'user_access_level'));

		// delete the remember me cookies if they exist
  //if (get_cookie('user_identity')) {	delete_cookie('user_identity'); }
		//if (get_cookie('user_remember'))	{	delete_cookie('user_remember');	}

		// Destroy the session
		  $this->session->sess_destroy();

		//Recreate the session
				session_start();
	 		$this->session->sess_regenerate(TRUE);
  		return TRUE;
	 }

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
