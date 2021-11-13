<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Icons extends Public_Controller {

/******************************************************************************************************************************/
	function __construct()
	 {	parent::__construct();
    $this->load->model('Icons_model');
  }

/******************************************************************************************************************************/
	function class_list()
	 { header("Content-Type: application/json; charset=UTF-8");
		  $classes = $this->Icons_model->class_list();                                                             /* Get all class */
    $result=array();
    foreach ($classes as $class):
      $result[] = array ( $class->id, $class->libelle );
    endforeach;
		  echo json_encode($result);
 	}
/******************************************************************************************************************************/
	function icon_list($classe_id)
	 { header("Content-Type: application/json; charset=UTF-8");
		  $icons = $this->Icons_model->icons_list($classe_id);                                                     /* Get all class */
    $result=array();
    foreach ($icons as $icon):
      $result[] = array ( $icon->id, $icon->libelle );
    endforeach;
		  echo json_encode($result);
 	}
}
/*----------------------------------------------------------------------------------------------------------------------------*/
