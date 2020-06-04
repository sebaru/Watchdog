<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Syn extends Public_Controller {

   public function __construct()
    { parent::__construct();
       $this->load->model('Syn_model');
       $this->load->model('Msg_model');
       $this->data["load_js"] = "syn.js";
        /* Load :: Common */
      /*$this->load->helper('number');
      $this->load->model('dashboard_model');*/
    }
/******************************************************************************************************************************/
 public function show ($id=NULL)
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    if (!isset($id)) $id=1;
    $syn = $this->Syn_model->get($id);
    $this->data["syn"] = $syn->libelle;
    $this->data["body_ready"] = "Charger_syn(".$id.")";
    $this->template->public_render('public/syns', $this->data);
 	}
/******************************************************************************************************************************/
 public function clic ($tech_id=NULL,$acronyme=NULL)
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $this->wtd_webservice->post( '/memory', array ( "mode" => "SET", "type" => "DI",
                                                    "tech_id" => $tech_id, "acronyme" => $acronyme ) );
    $this->wtd_log->add('Suite clic synoptique, positionnement du bit DI ' . $tech_id . ':' . $acronyme );
  }
/******************************************************************************************************************************/
 public function index ()
  { redirect ( "syn/show/1" );
  }
/******************************************************************************************************************************/
 public function get ($id=NULL)
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    header("Content-Type: application/json; charset=UTF-8");
    if (!isset($id)) $id=1;
    $motifs = $this->Syn_model->motifs($id);
    foreach ($motifs as $motif):
     { $result = $this->wtd_webservice->post( '/memory',
                 array ( "mode" => "GET", "type" =>"I", "num" => "-1",
                         "tech_id" => $motif->tech_id, "acronyme" => $motif->acronyme ) );
       if($result!=FALSE)
        { $motif->color  = $result->color;
          $motif->cligno = $result->cligno;
          $motif->mode   = $result->mode;
        }
     }
    endforeach;
  		$comments    = $this->Syn_model->comments($id);
  		$passerelles = $this->Syn_model->passerelles($id);
  		$liens       = $this->Syn_model->liens($id);
    foreach ($liens as $lien):
     { $result = $this->wtd_webservice->post( '/memory',
                  array ( "mode" => "GET", "type" =>"I", "num" => "-1",
                          "tech_id" => $motif->tech_id, "acronyme" => $motif->acronyme ) );
        if($result!=FALSE)
         { $motif->fill   = $result->color;
           $motif->cligno = $result->cligno;
           $motif->mode   = $result->mode;
         }
      }
    endforeach;

  		$rectangles  = $this->Syn_model->rectangles($id);
    foreach ($rectangles as $rectangle):
      { $result = $this->wtd_webservice->post( '/memory',
                  array ( "mode" => "GET", "type" =>"I", "num" => "-1",
                          "tech_id" => $rectangle->tech_id, "acronyme" => $rectangle->acronyme ) );
        if($result!=FALSE)
         { $rectangle->color  = $result->color;
           $rectangle->cligno = $result->cligno;
           $rectangle->etat   = $result->etat;
         }
        else $rectangle->color = $rectangle->def_color;
      }
    endforeach;

		  $output = array( "motifs" => $motifs,	"comments" => $comments, "passerelles" => $passerelles,
                     "liens" => $liens, "rectangles" => $rectangles	);
		  echo json_encode($output);
 	}
}
