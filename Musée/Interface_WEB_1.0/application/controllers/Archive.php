<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Archive extends Public_Controller {

/************************************** Chargement des classes et icones pour le client lourd *********************************/
	function __construct()
	 {	parent::__construct();
    $this->load->model('Archive_model');
    $this->load->model('Tableau_model');
    $this->load->model('Mnemo_model');
  }

/******************************************************************************************************************************/
	function get_ea($tech_id=NULL, $acronyme=NULL, $period=NULL )
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    header("Content-Type: application/json; charset=UTF-8");
    if (!isset($period)) { $period="DAY"; }
		  switch($period)
     { default:
       case "HOUR" : $points = $this->Archive_model->get_ea_hour ( $tech_id, $acronyme ); break;
       case "DAY"  : $points = $this->Archive_model->get_ea_day  ( $tech_id, $acronyme ); break;
       case "WEEK" : $points = $this->Archive_model->get_ea_week ( $tech_id, $acronyme ); break;
       case "MONTH": $points = $this->Archive_model->get_ea_month( $tech_id, $acronyme ); break;
       case "YEAR" : $points = $this->Archive_model->get_ea_year ( $tech_id, $acronyme ); break;
     }
/* Recherche du libelle dans les AI ou R */
                  $result = $this->Mnemo_model->get_AI($tech_id,$acronyme);
    if (!$result) $result = $this->Mnemo_model->get_R($tech_id,$acronyme);
    $result->data = $points;
		  echo json_encode( $result );
 	}

/******************************************************************************************************************************/
	public function show($tech_id=NULL, $acronyme=NULL, $period=NULL )
 	{ if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    if (!isset($tech_id) || !isset($acronyme)) { $tech_id = "SYS"; $acronyme="DLS_WAIT"; }
    $this->data["load_js"] = "archives.js";
    $this->data["body_ready"] = 'Charger_courbe("'.$tech_id.'","'.$acronyme.'","'.$period.'")';
    $this->data["url_hour"]  = site_url('archive/show/'.$tech_id.'/'.$acronyme.'/HOUR');
    $this->data["url_day"]   = site_url('archive/show/'.$tech_id.'/'.$acronyme.'/DAY');
    $this->data["url_week"]  = site_url('archive/show/'.$tech_id.'/'.$acronyme.'/WEEK');
    $this->data["url_month"] = site_url('archive/show/'.$tech_id.'/'.$acronyme.'/MONTH');
    $this->data["url_year"]  = site_url('archive/show/'.$tech_id.'/'.$acronyme.'/YEAR');
    $this->template->public_render('public/archives', $this->data);                                          /* Load Template */
  }
/******************************************************************************************************************************/
	function get_courbes($tableau_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    header("Content-Type: application/json; charset=UTF-8");
    $tab = $this->Tableau_model->get_courbes($tableau_id);
    echo json_encode( $tab );
 	}
/******************************************************************************************************************************/
	public function tableau($tableau_id, $period=NULL )
 	{ if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    $this->data["load_js"] = "tableaux.js";
    $this->data["body_ready"] = 'Charger_tableau("'.$tableau_id.'","'.$period.'")';
    $this->data["url_hour"]  = site_url('archive/tableau/'.$tableau_id.'/HOUR');
    $this->data["url_day"]   = site_url('archive/tableau/'.$tableau_id.'/DAY');
    $this->data["url_week"]  = site_url('archive/tableau/'.$tableau_id.'/WEEK');
    $this->data["url_month"] = site_url('archive/tableau/'.$tableau_id.'/MONTH');
    $this->data["url_year"]  = site_url('archive/tableau/'.$tableau_id.'/YEAR');
    $this->template->public_render('public/tableau', $this->data);                                           /* Load Template */
  }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
