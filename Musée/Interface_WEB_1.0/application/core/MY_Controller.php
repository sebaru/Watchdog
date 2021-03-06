<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class MY_Controller extends CI_Controller
{
	public function __construct()
	 { parent::__construct();
    $this->load->model('Config_model');

        /* COMMON :: ADMIN & PUBLIC */
        /* Load */
        $this->load->database();
        $this->load->library(array('form_validation', 'Wtd_auth', 'Wtd_log', 'Wtd_webservice', 'template', 'mobile_detect'));
        $this->load->helper(array('array', 'language', 'url'));
        $this->load->library('Admin_page_title');
        $this->load->library('Admin_breadcrumbs');
        $this->load->helper('menu');
        $this->lang->load(array('admin/main_header', 'admin/main_sidebar', 'admin/footer', 'admin/actions'));
        ini_set('display_errors', ($this->config->item('environnement')=='developpement' ? 1 : 0 ));

        /* Data */
        $this->data['lang']           = element($this->config->item('language'), $this->config->item('language_abbr'));
        $this->data['charset']        = $this->config->item('charset');
        $this->data['frameworks_dir'] = $this->config->item('frameworks_dir');
        $this->data['plugins_dir']    = $this->config->item('plugins_dir');
        $this->data['avatar_dir']     = $this->config->item('avatar_dir');
        $this->data['instance_description'] = $this->Config_model->get_param( $this->config->item('web_service_host'),
                                                                              'msrv', 'description' );
	 }
}

class Admin_Controller extends MY_Controller
 {	public function __construct()
	   {	parent::__construct();

      if ( ! $this->wtd_auth->logged_in_as_admin()) { redirect('auth/login', 'refresh'); }
      $this->admin_breadcrumbs->unshift(0, $this->lang->line('menu_dashboard'), 'admin/dashboard');
    }
 }

class Public_Controller extends MY_Controller
 {	public function __construct()
   	{	parent::__construct(); }
 }
