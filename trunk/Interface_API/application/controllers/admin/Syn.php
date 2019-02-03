<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Syn extends Admin_Controller{
    function __construct()
     { parent::__construct();
       $this->load->model('Syn_model');
		     } 

/******************************************************************************************************************************/	
	function get_all()
  {	header("Content-Type: application/json; charset=UTF-8");
		  $draw   = intval($this->input->get("draw"));
    $start  = intval($this->input->get("start"));
		  $length = intval($this->input->get("length"));

		  $data = array();
		
/*		  if ( ! $this->wtd_auth->logged_in() )
     {	echo json_encode(array(	"draw"=>$draw,	"recordsTotal" => 0, "recordsFiltered" => 0,	"data" => $data ));
			    exit();
		   }*/

  		$syns = $this->Syn_model->get_all($start, $length);
		  foreach($syns->result() as $syn)
     {	$data[] = array(	"id" => $syn->id,	"access_level" => $syn->access_level,
                        "parent_page" => $syn->ppage, "page" => $syn->page,
                      );
		   }

		  echo json_encode($data);
		  exit();
	 }

/******************************************************************************************************************************/
 function create($id=null)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       return($this->index());
     }

			// check if the syn exists before trying to edit it
 			$syn = $this->Syn_model->get($id);
 			if(!isset($syn))
     { $this->session->set_flashdata('flash_error', "Ce synoptique n'existe pas");
       return($this->index());
     }

	 		$params = array(	'access_level' => 0,
			                  'libelle' => "Nouveau synoptique",
			                  'page' => "NEW",
                     'parent_id' => $id,
		                 );

				$new_syn_id = $this->Syn_model->create($params);
    $message = 'Le synoptique '.$new_syn_id.' a été ajouté.';
				$this->session->set_flashdata('flash_message',$message);
    $this->wtd_log->add($message);
				redirect('admin/syn', 'refresh');
  }  

/******************************************************************************************************************************/
 function edit($id=null)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
		
 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       return($this->index());
     }

			// check if the syn exists before trying to edit it
 			$syn = $this->data['syn'] = $this->Syn_model->get($id);
 			if(!isset($syn))
     { $this->session->set_flashdata('flash_error', "Ce synoptique n'existe pas");
       return($this->index());
     }

				$this->admin_breadcrumbs->unshift(2, 'Editer un synoptique', 'admin/syn/edit');
				$this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
				$this->data['syns'] = $this->Syn_model->get_all(0,0);

				$this->form_validation->set_rules('access_level','Access Level','required');
				$this->form_validation->set_rules('libelle','Libellé','required');
				$this->form_validation->set_rules('page','Page','required');
			
				if($this->form_validation->run())     
		 		{	$params = array(	'access_level' => $this->input->post('access_level'),
						                  'libelle' => $this->input->post('libelle'),
						                  'page' => $this->input->post('page'),
					                 );

  					$this->Syn_model->update($id,$params);            
					//redirect('admin/syn');
       $this->session->set_flashdata('flash_message','Le synoptique '.$params['page'].' a été modifié.');
       $this->wtd_log->add('Le synoptique '.$params['page'].' a été modifié.');
 						redirect('admin/syn', 'refresh');
				 }
    else
     { $this->session->set_flashdata('flash_error', validation_errors() ); }

		  $this->data['ppage'] = array(	'name'  => 'ppage',
			                               'id'      => 'ppage',
			                               'type'    => 'text',
                                  'class'   => 'form-control',
                                  'disabled' => 'TRUE',
			                               'value'   => $this->form_validation->set_value('ppage', $syn->ppage)
		                              );
		  $this->data['access_level'] = array(	'name'  => 'access_level',
                                      'id'    => 'access_level',
                                      'type'  => 'number',
                                      'class' => 'form-control',
                                      'value' => $this->form_validation->set_value('access_level', $syn->access_level)
                                    );
		  $this->data['page'] = array(	'name'  => 'page',
                                 'id'    => 'page',
                                 'type'  => 'text',
                                 'class' => 'form-control',
                                 'value' => $this->form_validation->set_value('page', $syn->page)
                               );

		  $this->data['libelle'] = array(	'name'  => 'libelle',
                                    'id'    => 'libelle',
                                    'type'  => 'text',
                                    'class' => 'form-control',
                                    'value' => $this->form_validation->set_value('libelle', $syn->libelle)
                                  );
        /* Load Template */
		  $this->template->admin_render('admin/syn/edit', $this->data);
	} 

/******************************************************************************************************************************/	
	function delete($id)
  {	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
 			if ($id==1)
     { $this->session->set_flashdata('flash_error', 'Suppression du synoptique racine interdite !');
       redirect('admin/syn/index');
     }


    $target = $this->Syn_model->get($id);
    if (!isset($target))
     { $this->session->set_flashdata('flash_error', 'Synoptique '.$id.' inconnu' );
       redirect('admin/syn/index');
     }
    if ($target->access_level < $this->session->user_access_level)
     { if($this->Syn_model->delete($target->id))
 			    { $flash = 'Synoptique '.$target->libelle.' ('.$target->id.') supprimé';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_webservice->send('/reload/dls');
          $this->wtd_log->add($flash);
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de suppression du Synoptique '.$target->id ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de suppression du synoptique ".$target->libelle.' ('.$target->id.')' );
     }
				redirect('admin/syn/index');
  }
	
}
/*----------------------------------------------------------------------------------------------------------------------------*/
