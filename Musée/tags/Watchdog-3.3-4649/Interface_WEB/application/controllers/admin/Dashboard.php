<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Dashboard extends Admin_Controller {

   public function __construct()
    { parent::__construct();
        /* Load :: Common */
      $this->load->helper('number');
      $this->load->model('dashboard_model');
      $this->load->model('Mnemo_model');
      $this->load->model('Dls_model');
      $this->load->model('Archive_model');
    }


	  public function index()
	   { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }

      $this->admin_page_title->push(lang('menu_dashboard'));
      $this->data['pagetitle'] = $this->admin_page_title->show();
      $this->data["load_js"] = "dashboard.js";
      $this->data["db_database"] = $this->db->database;
      $this->data["db_hostname"] = $this->db->hostname;
      $this->data["db_username"] = $this->db->username;
      $this->data["archdb_database"] = $this->Archive_model->archdb->database;
      $this->data["archdb_hostname"] = $this->Archive_model->archdb->hostname;
      $this->data["archdb_username"] = $this->Archive_model->archdb->username;
      /* Breadcrumbs */
      $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

      /* Data */
      $this->data['count_users']       = $this->dashboard_model->get_count_record('users');
     	$this->data['count_syns']        = $this->dashboard_model->get_count_record('syns');
     	$this->data['count_syns_motifs'] = $this->dashboard_model->get_count_record('syns_motifs');
     	$this->data['count_msgs']        = $this->dashboard_model->get_count_record('msgs');
      $this->data['count_dls']         = $this->dashboard_model->get_count_record('dls');
      $this->data['count_mnemos']      = $this->Mnemo_model->get_total_all();
      $this->data['count_logs']        = $this->dashboard_model->get_count_record('audit_log');
      $this->data['nbr_lignes_dls']    = $this->Dls_model->get_nbr_lignes();
      $this->data['disk_totalspace']   = $this->dashboard_model->disk_totalspace(DIRECTORY_SEPARATOR);
      $this->data['disk_freespace']    = $this->dashboard_model->disk_freespace(DIRECTORY_SEPARATOR);
      $this->data['disk_usespace']     = $this->dashboard_model->disk_usespace(DIRECTORY_SEPARATOR);
      $this->data['disk_usepercent']   = $this->dashboard_model->disk_usepercent(DIRECTORY_SEPARATOR, FALSE);
      $this->data['memory_usage']      = $this->dashboard_model->memory_usage();
      $this->data['memory_peak_usage'] = $this->dashboard_model->memory_peak_usage(TRUE);
      $this->data['memory_usepercent'] = $this->dashboard_model->memory_usepercent(TRUE, FALSE);

  	 		$this->data['status'] = $this->wtd_webservice->master_status();
	         /* Load Template */
      $this->template->admin_render('admin/dashboard/index', $this->data);
   	}
  }
