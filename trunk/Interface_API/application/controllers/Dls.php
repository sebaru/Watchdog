<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Dls extends Admin_Controller{
 function __construct()
  { parent::__construct();
    $this->load->model('Dls_model');
    $this->load->model('Syn_model');
    $this->dls_status = array( "Never compiled yet", "Database Export failed", "<span class='label label-danger'>Error loading source file</span>",
                               "Error loading log file", "Syntax error", "Error Fork GCC",
                               "OK with Warnings", "OK",
                               "Functions are missing<br>Need compiling again.",
                               "Error, plugin is setting bits he does not own.",
                               "Error"
                             );

        
  } 
/******************************************************************************************************************************/
 public function index($id=null)
  { header("Content-Type: application/json; charset=UTF-8");
    switch ( $this->input->method(TRUE) )
     { case "GET":    return ($this->get($id));
       case "DELETE": return ($this->delete($id));
       case "PUT":    return ($this->update());
       case "POST":   return ($this->create());
     }
    echo json_encode(array( "success" => "FALSE", "error" => "Method not implemented" ));
    exit();
  }
/******************************************************************************************************************************/
 public function list($id)
  { header("Content-Type: application/json; charset=UTF-8");

    if (!isset($id))
     { echo json_encode( array( "success" => "FALSE", "error" => "need SYN id" ) );
       exit();
     }

    $data = array();
/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode(array( "draw"=>$draw, "recordsTotal" => 0, "recordsFiltered" => 0, "data" => $data ));
       exit();
     }*/

    $dlss = $this->Dls_model->get_all($id);
    if (isset($dlss))
     { foreach($dlss as $dls)
        { $data[] = get_object_vars( $dls ); }
     }
    echo json_encode(array( "success" => "TRUE", "Dls" => $data));
    exit();
  }
/******************************************************************************************************************************/
 public function count()
  { header("Content-Type: application/json; charset=UTF-8");

/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode( array( "success" => "FALSE", "error" => "need to be authenticated" ) );
       exit();
     }*/

    $count = $this->Dls_model->get_count();
    echo json_encode(array( "success" => "TRUE", "count" => $count));
    exit();
  }
/******************************************************************************************************************************/
 private function get($id=NULL)
  {
/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode( array( "success" => "FALSE", "error" => "need to be authenticated" ) );
       exit();
     }*/

    $dls = $this->Dls_model->get($id);
    if (!isset($dls))
     { echo json_encode( array( "success" => "FALSE", "error" => "DLS unknown" ) ); }
    else
     { echo json_encode( array( "success" => "TRUE", "Dls" => get_object_vars($dls) )); }
    exit();
  }
/******************************************************************************************************************************/
 private function delete($id=null)
  {

/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode( array( "success" => "FALSE", "error" => "need to be authenticated" ) );
       exit();
     }*/

    $dls = $this->Dls_model->get($id);
    if (!isset($dls))
     { echo json_encode( array( "success" => "FALSE", "error" => "Dls unknown" ) );
       exit();
     }

/*    if ($mnemo->access_level < $this->session->user_access_level)
     { echo json_encode( array( "success" => "FALSE", "error" => "Not authorized" ) );
       exit();
     }*/
    $this->Dls_model->delete($dls->id);
    $this->wtd_webservice->send('/dls/delete?id='.$dls->id);
    echo json_encode(array( "success" => "TRUE", "DLS" => "deleted" ));
    $this->wtd_log->add('Le DLS '.$dls->id.' a été supprimé.');
    exit();
  }
/******************************************************************************************************************************/
 private function update()
  {

   /*if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }*/

    /*if ($this->session->user_access_level<$mnemo->access_level)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/mnemo/index/'.$mnemo->dls_id);
     }*/

    $input = json_decode(file_get_contents('php://input'));
    if (!isset($input)) { echo json_encode( array( "success" => "FALSE", "error" => "Parsing Error !" ) ); exit(); }

    $dls = $this->Dls_model->get($input->id);
    if (!isset($dls)) { echo json_encode( array( "success" => "FALSE", "error" => "DLS unknown !" ) ); exit(); }

    $data = array();
    if (isset($input->actif))     $data['actif']     = $input->actif;
    if (isset($input->syn_id))    $data['syn_id']    = $input->syn_id;
    if (isset($input->tech_id))   $data['tech_id']   = strtoupper($input->tech_id);
    if (isset($input->name))      $data['name']      = $input->name;
    if (isset($input->shortname)) $data['shortname'] = strtoupper($input->shortname);
    if (isset($input->package))   $data['package']   = $input->package;

    if($this->Dls_model->update($dls->id, $data))
     { echo json_encode( array( "success" => "TRUE", "Dls" => "Updated !" ) );
       $flash = 'Le dls '.$dls->tech_id.' ('.$dls->id.') a été updaté.';
       $this->wtd_log->add($flash);
       $this->wtd_webservice->send('/reload/dls');
     }
    else { echo json_encode( array( "success" => "FALSE", "error" => "Update error !" ) ); }
    exit();
  }
/******************************************************************************************************************************/  
 private function create($syn_id=NULL)
  {

   /*if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }*/

    /*if ($this->session->user_access_level<$mnemo->access_level)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/mnemo/index/'.$mnemo->dls_id);
     }*/

    $input = json_decode(file_get_contents('php://input'));
    if (!isset($input)) { echo json_encode( array( "success" => "FALSE", "error" => "Parsing Error !" ) ); exit(); }

    $data = array();
    if (isset($input->syn_id))    $data['syn_id']    = $input->syn_id;
    else { echo json_encode( array( "success" => "FALSE", "error" => "Parsing Error !" ) ); exit(); }

    $syn = $this->Syn_model->get($input->syn_id);
    if (!isset($syn)) { echo json_encode( array( "success" => "FALSE", "error" => "Need a SYN !" ) ); exit(); }

    if (isset($input->tech_id))   $data['tech_id']   = strtoupper($input->tech_id);
    else { echo json_encode( array( "success" => "FALSE", "error" => "Parsing Error !" ) ); exit(); }
    if (isset($input->name))      $data['name']      = $input->name;
    else { echo json_encode( array( "success" => "FALSE", "error" => "Parsing Error !" ) ); exit(); }
    if (isset($input->shortname)) $data['shortname'] = strtoupper($input->shortname);
    else { echo json_encode( array( "success" => "FALSE", "error" => "Parsing Error !" ) ); exit(); }
    if (isset($input->package))   $data['package']   = $input->package;
    else $data['package'] = 'custom';
    $data['actif'] = "FALSE";

    $dls_id = $this->Dls_model->add($params);
    echo json_encode( array( "success" => "TRUE", "Dls" => "Added !" ) );
    $flash = 'Le dls '.$dls->tech_id.' ('.$dls_id.') a été ajouté.';
    $this->wtd_log->add($flash);
  }  
/******************************************************************************************************************************/
 public function activate($id=NULL)
  {
   /*if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }*/

    /*if ($this->session->user_access_level<$mnemo->access_level)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/mnemo/index/'.$mnemo->dls_id);
     }*/

    $target_dls = $this->Dls_model->get($id);
    if (!isset($target_dls))
     { echo json_encode( array( "success" => "FALSE", "error" => "Dls unknown !" ) ); exit(); }

/*    if ($target_dls->access_level < $this->session->user_access_level)*/
     { if($this->Dls_model->activate($target_dls->id))
        { $flash = 'Module D.L.S ('.$target_dls->tech_id.', '.$target_dls->name.') activé';
          $this->wtd_webservice->send('/dls/activate?id='.$target_dls->id);
          $this->wtd_log->add($flash);
          echo json_encode( array( "success" => "TRUE", "Dls" => "Dls activé !" ) );
        }
     }
/*    else
     { $this->wtd_log->add("Tentative d'activation du module D.L.S ".
                           $target_dls->id." (".$target_dls->tech_id.", ".$target_dls->name.")");
     }*/
  }
/******************************************************************************************************************************/
 public function deactivate($id=NULL)
  {
   /*if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }*/

    /*if ($this->session->user_access_level<$mnemo->access_level)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/mnemo/index/'.$mnemo->dls_id);
     }*/

    $target_dls = $this->Dls_model->get($id);
    if (!isset($target_dls))
     { echo json_encode( array( "success" => "FALSE", "error" => "Dls unknown !" ) ); exit(); }

/*    if ($target_dls->access_level < $this->session->user_access_level)*/
     { if($this->Dls_model->deactivate($target_dls->id))
        { $flash = 'Module D.L.S ('.$target_dls->tech_id.', '.$target_dls->name.') désactivé';
          $this->wtd_webservice->send('/dls/deactivate?id='.$target_dls->id);
          $this->wtd_log->add($flash);
          echo json_encode( array( "success" => "TRUE", "Dls" => "Dls désactivé !" ) );
        }
     }
/*    else
     { $this->wtd_log->add("Tentative désactivation du module D.L.S ".
                           $target_dls->id." (".$target_dls->tech_id.", ".$target_dls->name.")");
     }*/
  }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
