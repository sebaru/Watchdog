<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Log extends Admin_Controller{
 function __construct()
  { parent::__construct();
    /*$this->load->model('Dls_model');*/
    $this->load->model('Syn_model');
		
		  /* Title Dls :: Common */
    $this->admin_page_title->push('Audit Log');
    $this->data['pagetitle'] = "<h1>Audit Log</h1>";
    /* Breadcrumbs :: Common */
    $this->admin_breadcrumbs->unshift(1, 'Audit Log', 'admin/log');
  } 

/******************************************************************************************************************************/
 function index()
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
		
			 $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
			 /* Load Template */
			 $this->template->admin_render('admin/log/index', $this->data);
		}
/******************************************************************************************************************************/
	function get()
  {	header("Content-Type: application/json; charset=UTF-8");
		  $draw   = intval($this->input->get("draw"));
    $start  = intval($this->input->get("start"));
		  $length = intval($this->input->get("length"));

		  $data = array();
    if ( ! $this->wtd_auth->logged_in() )
     {	echo json_encode(array(	"draw"=>$draw,	"recordsTotal" => 0, "recordsFiltered" => 0,	"data" => $data ));
			    exit();
		   }

 			$logs = $this->wtd_log->get_all($start, $length);
		  foreach($logs->result() as $log)
     { $data[] = array(	$log->date, $log->access_level, $log->username, $log->message );
		   }
		  $total_log = $this->wtd_log->get_total();
		
		  $output = array( "draw" => $draw,	"recordsTotal" => $total_log,	"recordsFiltered" => $total_log,	"data" => $data	);
		  echo json_encode($output);
		  exit();
  }    
}
/*----------------------------------------------------------------------------------------------------------------------------*/
