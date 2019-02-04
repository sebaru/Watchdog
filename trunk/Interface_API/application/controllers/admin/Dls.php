<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Dls extends Admin_Controller{
 function __construct()
  { parent::__construct();
    $this->load->model('Dls_model');
    $this->load->model('Syn_model');
  } 

/******************************************************************************************************************************/
	function get_all()
  {	header("Content-Type: application/json; charset=UTF-8");
		  $draw   = intval($this->input->get("draw"));
    $start  = intval($this->input->get("start"));
		  $length = intval($this->input->get("length"));

		  $data = array();
/*    if ( ! $this->wtd_auth->logged_in() )
     {	echo json_encode(array(	"draw"=>$draw,	"recordsTotal" => 0, "recordsFiltered" => 0,	"data" => $data ));
			    exit();
		   }*/

 			$dlss = $this->Dls_model->get_all_dls($start, $length);
		  foreach($dlss->result() as $dls)
     { $dls_status = array( "Never compiled yet", "Database Export failed", "<span class='label label-danger'>Error loading source file</span>",
                            "Error loading log file", "Syntax error", "Error Fork GCC",
                            "OK with Warnings", "OK",
                            "Functions are missing<br>Need compiling again.",
                            "Error, plugin is setting bits he does not own.",
                            "Error"
                          );

        $data[] = array(	"id" => $dls->id,
                         "enable" => $dls->actif,
 				                    "tech_id" => $dls->tech_id,
                         "package" => $dls->package,
                         "parent_page" => $dls->ppage, "page" => $dls->page,
 				                    "shortname" => $dls->shortname, "name" => $dls->name,
                         "compil_date" => $dls->compil_date, "compil_status" => $dls_status[$dls->compil_status],
                         "nbr_ligne" => $dls->nbr_ligne, "nbr_compil" => $dls->nbr_compil,
			                   );
		   }
		  echo json_encode($data);
		  exit();
  }
	
/******************************************************************************************************************************/		
 function create($syn_id=NULL)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    if(!isset($syn_id)) {	redirect('auth', 'refresh');	}
    $this->data['syn'] = $syn = $this->Syn_model->get($syn_id);
    if (!isset($syn))  {	redirect('auth', 'refresh');	}
    if ($this->session->user_access_level < $syn->access_level) {	redirect('auth', 'refresh');	}

 			$this->admin_breadcrumbs->unshift(2, 'Ajouter un module DLS', 'admin/dls/create');
 			$this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
			
 			$this->form_validation->set_rules('tech_id','Tech_ID','required|is_unique[dls.tech_id]');
 			$this->form_validation->set_rules('shortname','Shortname','required');
 			$this->form_validation->set_rules('name','Libellé','required');
			
 			if($this->form_validation->run())     
			  { $params = array( 'tech_id' => $this->input->post('tech_id'),
					                   'name' => $this->input->post('name'),
					                   'shortname' => $this->input->post('shortname'),
					                   'package' => $this->input->post('package'),
					                   'syn_id' => $syn->id,
					                   'actif' => 'FALSE',
				                  );
				
       if ($this->input->post('package'))	{	$data['package'] = $this->input->post('package');	}
                                     else {	$data['package'] = 'custom';	}

				   $dls_id = $this->Dls_model->add($params);
       $flash = 'Le dls <strong>'.$this->input->post('tech_id').'</strong> ('.$dls_id.') a été ajouté.';
				   $this->session->set_flashdata('message', $flash);
       $this->wtd_log->add($flash);
       $this->wtd_webservice->send('/reload/dls');
       redirect('admin/syn/index');
     }
    else $this->session->set_flashdata('flash_error', validation_errors() );

    $this->data['tech_id'] = array(	'name'  => 'tech_id',
	                                   'id'    => 'tech_id',
	                                   'type'  => 'text',
                                    'class' => 'form-control',
	                                   'value' => $this->form_validation->set_value('tech_id', $this->input->post('tech_id'))
                                  );

    $this->data['name'] = array(	'name'  => 'name',
                                 'id'    => 'name',
                                 'type'  => 'text',
                                 'class' => 'form-control',
                                 'value' => $this->form_validation->set_value('name', $this->input->post('name'))
                               );

    $this->data['shortname'] = array(	'name'  => 'shortname',
	                                     'id'    => 'shortmane',
	                                     'type'  => 'text',
                                      'class' => 'form-control',
	                                     'value' => $this->form_validation->set_value('shortname', $this->input->post('shortname'))
                                    );

    $this->data['package'] = array(	'name'  => 'package',
                                    'id'    => 'name',
                                    'type'  => 'text',
                                    'class' => 'form-control',
                                    'value' => $this->form_validation->set_value('name', $this->input->post('package'))
                                  );

				$this->template->admin_render('admin/dls/add', $this->data);
  }  
/******************************************************************************************************************************/
	public function delete($id)
 	{ $id = (int) $id;
   	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $target_dls = $this->Dls_model->get($id);
    if (!isset($target_dls)) { exit(); }
    if ($target_dls->access_level < $this->session->user_access_level)
     { if($this->Dls_model->delete($target_dls->id))
 			    { $flash = 'Module D.L.S ('.$target_dls->tech_id.', '.$target_dls->name.') supprimé';
          $this->wtd_webservice->send('/dls/delete?id='.$target_dls->id);
          $this->wtd_log->add($flash);
        }
		   }
    else
     { $this->wtd_log->add("Tentative de suppression du module D.L.S ".
                           $target_dls->id." (".$target_dls->tech_id.", ".$target_dls->name.")");
     }
	 }
/******************************************************************************************************************************/
	public function activate($id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    $target_dls = $this->Dls_model->get($id);
    if (!isset($target_dls)) { exit(); }
    if ($target_dls->access_level < $this->session->user_access_level)
     { if($this->Dls_model->activate($target_dls->id))
 			    { $flash = 'Module D.L.S ('.$target_dls->tech_id.', '.$target_dls->name.') activé';
          $this->wtd_webservice->send('/dls/activate?id='.$target_dls->id);
          $this->wtd_log->add($flash);
        }
		   }
    else
     { $this->wtd_log->add("Tentative d'activation du module D.L.S ".
                           $target_dls->id." (".$target_dls->tech_id.", ".$target_dls->name.")");
     }
	 }
/******************************************************************************************************************************/
	public function deactivate($id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    $target_dls = $this->Dls_model->get($id);
    if (!isset($target_dls)) { exit(); }
    if ($target_dls->access_level < $this->session->user_access_level)
     { if($this->Dls_model->deactivate($target_dls->id))
 			    { $flash = 'Module D.L.S ('.$target_dls->tech_id.', '.$target_dls->name.') désactivé';
          $this->wtd_webservice->send('/dls/deactivate?id='.$target_dls->id);
          $this->wtd_log->add($flash);
        }
		   }
    else
     { $this->wtd_log->add("Tentative désactivation du module D.L.S ".
                           $target_dls->id." (".$target_dls->tech_id.", ".$target_dls->name.")");
     }
	 }
/******************************************************************************************************************************/
 function edit($id=null)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
		
 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }
		
			 // check if the dls exists before trying to edit it
			 $this->data['dls'] = $dls = $this->Dls_model->get($id);
 			if(!isset($dls))
     { $this->session->set_flashdata('flash_error', "Ce D.L.S n'existe pas");
       redirect('admin/dls/index');
     }

 			if ($this->session->user_access_level<$dls->access_level)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }

    $this->admin_breadcrumbs->unshift(2, 'Editer un module', 'admin/dls/edit/'.$id);
	   $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

				$this->form_validation->set_rules('tech_id','Tech ID','required');
				$this->form_validation->set_rules('shortname','Shortname','required');
			
   	if($this->form_validation->run() == TRUE)
	    { $params = array(	'tech_id'    => $this->input->post('tech_id'),
			                     'name'       => $this->input->post('name'),
			                     'shortname'  => $this->input->post('shortname'),
			                     'package'    => $this->input->post('package'),
			                     'actif'      => $this->input->post('actif'),
                 					);

	  				$this->Dls_model->update($id,$params);            
  					$this->session->set_flashdata('flash_message','Le dls <strong>'.$this->input->post('tech_id').'</strong>/'.
                                     $this->input->post('shortname').' a été modifié.');
       $this->wtd_log->add("Mise à jour du DLS ".$dls->id." (".$this->input->post('tech_id').") ");
       redirect('admin/dls/index');
		 		}
    else
     { $this->session->set_flashdata('flash_error', validation_errors() ); }

		  $this->data['tech_id'] = array(	'name'  => 'tech_id',
			                                 'id'    => 'tech_id',
			                                 'type'  => 'text',
                                    'class' => 'form-control',
			                                 'value' => $this->form_validation->set_value('tech_id', $dls->tech_id)
		                                );
		  $this->data['shortname'] = array(	'name'  => 'shortname',
                                      'id'    => 'shortname',
                                      'type'  => 'text',
                                      'class' => 'form-control',
                                      'value' => $this->form_validation->set_value('shortname', $dls->shortname)
                                    );
		  $this->data['name'] = array(	'name'  => 'name',
                                 'id'    => 'name',
                                 'type'  => 'text',
                                 'class' => 'form-control',
                                 'value' => $this->form_validation->set_value('name', $dls->name)
                               );

		  $this->data['package'] = array(	'name'  => 'package',
                                    'id'    => 'package',
                                    'type'  => 'text',
                                    'class' => 'form-control',
                                    'value' => $this->form_validation->set_value('package', $dls->package)
                                  );

		  $this->data['actif'] = array(	'name'  => 'actif',
                                  'id'    => 'actif',
                                  /*'type'  => 'text',*/
                                 /*'class' => 'form-control',*/
                                 'checked' => $this->form_validation->set_value('actif', $dls->actif)
                                );

        /* Load Template */
		  $this->template->admin_render('admin/dls/edit', $this->data);
 	} 
/******************************************************************************************************************************/
 function sourceedit($id=null)
  { /*if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
		
 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }*/
		
			 // check if the dls exists before trying to edit it
			 $dls = $this->Dls_model->get($id);
    if (!isset($dls)) { exit(); }

    $data = array(	"id" => $dls->id,
                   "sourcecode" => $dls->sourcecode,
                   "error_log" => $this->Dls_model->get_error_log($dls->id),
                   "tech_id" => $dls->tech_id,
                   "package" => $dls->package,
                   "parent_page" => $dls->ppage, "page" => $dls->page,
                   "shortname" => $dls->shortname, "name" => $dls->name,
                 );
		  echo json_encode($data);
 	} 
/******************************************************************************************************************************/
 function download_package($id=null)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
		
 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }
		
    $this->admin_breadcrumbs->unshift(2, 'Editer une source D.L.S', 'admin/dls/sourceedit');
	   $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
								
			 // check if the dls exists before trying to edit it
			 $this->data['dls'] = $dls = $this->Dls_model->get($id);
 			if(!isset($dls))
     { $this->session->set_flashdata('flash_error', "Ce D.L.S n'existe pas");
       redirect('admin/dls/index');
     }

 			$this->form_validation->set_rules('sourcecode','Code Source','required');

   	if($this->form_validation->run() == TRUE)
	    { $params = array(	'sourcecode' => $this->input->post('sourcecode'),
                 					);

	  				$this->Dls_model->update($dls->id,$params);            
  					$this->session->set_flashdata('flash_message','Le dls <strong>'.$dls->tech_id.'</strong>/'.
                                     $dls->shortname.' a été modifié.');
       $this->wtd_log->add("Mise à jour et compilation de la source DLS ".$dls->id." (".$dls->tech_id.") ");
       $this->wtd_webservice->compil($id);
		 		}
    else
     { $this->session->set_flashdata('flash_error', validation_errors() ); }

    $ch = curl_init( "https://packages.abls-habitat.fr/".$dls->package.".dls" );
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $new_source = curl_exec($ch);
    curl_close($ch);

    $this->data['dls']->sourcecode = $new_source;

    $this->data['error_log'] = $this->Dls_model->get_error_log($dls->id);
    $this->data['sourcecode'] = array(	'name'  => 'sourcecode',
                                       'id'    => 'sourcecode',
                                       'type'  => 'textarea',
                                       'class' => 'form-control',
                                       'value' => $this->form_validation->set_value('sourcecode', $dls->sourcecode)
                                     );
		  $this->template->admin_render('admin/dls/sourceedit', $this->data);
 	} 
 
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
