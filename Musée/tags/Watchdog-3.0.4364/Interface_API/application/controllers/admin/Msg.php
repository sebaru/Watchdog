<?php
defined('BASEPATH') OR exit('No direct script access allowed');

putenv('GOOGLE_APPLICATION_CREDENTIALS='.realpath('application/third_party/texttospeech/json/testseboc-fdab13333e50.json'));
#require realpath('application/third_party/texttospeech/vendor/autoload.php');

# Imports the Google Cloud client library
#use Google\Cloud\Speech\SpeechClient;
 
class Msg extends Admin_Controller{

/******************************************************************************************************************************/
 function __construct()
  { parent::__construct();
    $this->load->model('Msg_model');
    $this->load->model('Dls_model');
		
		  /* Title Dls :: Common */
    $this->admin_page_title->push('Messages');
    $this->data['pagetitle'] = "<h1>Messages</h1>";
    /* Breadcrumbs :: Common */
 			$this->admin_breadcrumbs->unshift(1, 'Modules D.L.S', 'admin/dls');
    $this->msgs_types = array
     ( '<img style="width: 20px" data-toggle="tooltip" title="Information" src="https://icons.abls-habitat.fr/assets/gif/Info.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Alerte"      src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_rouge.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Défaut"      src="https://icons.abls-habitat.fr/assets/gif/Pignon_jaune.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Alarme"      src="https://icons.abls-habitat.fr/assets/gif/Pignon_orange.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Veille"      src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_vert.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Attente"     src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_noir.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Danger"      src="https://icons.abls-habitat.fr/assets/gif/Croix_rouge_rouge.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Dérangement" src="https://icons.abls-habitat.fr/assets/gif/Croix_rouge_jaune.svg" />',
     );
    $this->msgs_types_sms = array
     ( '<span class="label label-default">Désactivé</span>',
       '<span class="label label-success">Activé</span>',
       '<span class="label label-warning">GSM Only</span>',
       '<span class="label label-warning">SMSBOXOnly</span>'
     );
  }
/******************************************************************************************************************************/
	public function index($id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    $id = (int) $id;
 			$this->admin_breadcrumbs->unshift(2, 'Liste des messages', 'admin/msg/index/'.$id);
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();                                              /* Breadcrumbs */
    $this->data['msgs'] = $this->Msg_model->get_by_dls_id($id)->result();                                    /* Get all users */

    $this->template->admin_render('admin/msg/index', $this->data);                                           /* Load Template */
 	}
/******************************************************************************************************************************/
 function dico()
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
		
			 $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
		/*	$this->data['msgs'] = $this->Msg_model->get_all_msgs($dls_id);*/
			 /* Load Template */
			 $this->template->admin_render('admin/msg/dico', $this->data);
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

 			$msgs = $this->Msg_model->get_all($start, $length);
		  foreach($msgs->result() as $msg)
     {	$caret = '<div class="dropdown">'.
                '<button class="btn btn-xs btn-primary dropdown-toggle" type="button" data-toggle="dropdown">'.
                '<span class="caret"></span></button>'.
                '<ul class="dropdown-menu">'.
                '<li><a href='.site_url('admin/msg/edit/'.$msg->id).'>'.
                    '<i class="fa fa-pencil" style="color:green"></i>'.
                    '<i class="fa fa-enveloppe" style="color:blue"></i>Editer</a></li>'.
                 '<li class="divider"></li>'.
                 '<li><a href='.site_url('admin/msg/delete/'.$msg->id).'>'.
                     '<i class="fa fa-times" style="color:red"></i>Supprimer</a></li>'.
                 '</ul></div>';

       $data[] = array(	($msg->enable ? '<span class="label label-success">Activé</span>'
                                      : '<span class="label label-default">Inactif</span>'),
                        $msg->shortname, $msg->tech_id,
                        anchor('admin/msg/edit/'.$msg->id, $msg->acronyme),
                        $this->msgs_types[$msg->type],
                        anchor('admin/msg/edit/'.$msg->id, $msg->libelle ),
                        $this->msgs_types_sms[$msg->sms],
                        ($msg->audio ? '<span class="label label-success">Activé</span>'
                                     : '<span class="label label-default">Inactif</span>'),
                        $msg->time_repeat
			                   );
     }
		  $total_msg = $this->Msg_model->get_count();
		
		  $output = array( "draw" => $draw,	"recordsTotal" => $total_msg,	"recordsFiltered" => $total_msg,	"data" => $data	);
    echo json_encode($output);
		  exit();
  }
/******************************************************************************************************************************/
 public function activate($id = NULL)
  { if ( ! $this->wtd_auth->logged_in() ) { redirect('auth', 'refresh'); }

    $id = (int) $id;
    $target = $this->Msg_model->get($id);
    if (!isset($target))
     { $this->session->set_flashdata('flash_error', 'Message '.$id.' inconnu' );
       redirect('admin/msg/index/'.$target->dls_id);
     }
    $this->Msg_model->activate($id);
    $this->wtd_log->add("Message ".$target->id." (".$target->libelle.") activé");
    redirect('admin/msg/index/'.$target->dls_id);
  }
/******************************************************************************************************************************/
 public function deactivate($id = NULL)
  { if ( ! $this->wtd_auth->logged_in() ) { redirect('auth', 'refresh'); }

    $id = (int) $id;
    $target = $this->Msg_model->get($id);
    if (!isset($target))
     { $this->session->set_flashdata('flash_error', 'Message '.$id.' inconnu' );
       redirect('admin/msg/index/'.$target->dls_id);
     }
    $this->Msg_model->deactivate($id);
    $this->wtd_log->add("Message ".$target->id." (".$target->libelle.") désactivé");
    redirect('admin/msg/index/'.$target->dls_id);
 	}
/******************************************************************************************************************************/	
	public function delete($id)
 	{ $id = (int) $id;
   	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $target = $this->Msg_model->get($id);
    if (!isset($target))
     { $this->session->set_flashdata('flash_error', 'Message '.$id.' inconnu' );
       redirect('admin/msg/index/'.$target->dls_id);
     }
    if ($this->session->user_access_level>=$target->access_level)
     { if($this->Msg_model->delete($target->id))
 			    { $flash = 'Message '.$target->id.' ('.$target->tech_id.', '.$target->libelle.') supprimé';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_log->add($flash);
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de message '.$id ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de suppression de message ". $target->id." (".$target->libelle.")");
     }
    redirect('admin/msg/index/'.$target->dls_id);
	 }

/******************************************************************************************************************************/	
 public function edit($id)
 	{ $id = (int) $id;
		  if (!isset($id)) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }

			 // check if the message exists before trying to edit it
			 $this->data['message'] = $message = $this->Msg_model->get($id);
 			if(!isset($message))
     { $this->session->set_flashdata('flash_error', "Ce message n'existe pas");
       redirect('admin/dls/index');
     }

 			if ($this->session->user_access_level<$message->access_level)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/msg/index/'.$message->dls_id);
     }

        /* Breadcrumbs */
    $this->admin_breadcrumbs->unshift(2, "Edition du message", 'admin/msg/edit/'.$id );
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
		
                		/* Validate form input */
    $this->form_validation->set_rules('libelle', 'Libellé', 'required');
 			if ($this->form_validation->run() == TRUE)
     {	$data = array( 'enable'         => $this->input->post('enable') !== NULL ? TRUE : FALSE,
                      'libelle'        => $this->input->post('libelle'),
		                    'libelle_audio'  => $this->input->post('libelle_audio'),
                      'sms'            => $this->input->post('sms_mode'),
		                    'libelle_sms'    => $this->input->post('libelle_sms'),
                 			);

       if($this->Msg_model->update($message->id, $data))
 			    { $flash='Le message <strong>'.$message->tech_id.':'.$message->acronyme.'</strong> ('.$message->id.') a été modifié'; 
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_log->add( $flash );
          redirect('admin/msg/index/'.$message->dls_id);
 			    }
       else
 			    { $flash='Tentative de modification du message '.$message->tech_id.':'.$message->acronyme.'('.$message->id.')'; 
          $this->session->set_flashdata('flash_error', $flash );
          $this->wtd_log->add( $flash );
          redirect('admin/msg/index/'.$message->dls_id);
 			    }
		   }
    else $this->session->set_flashdata('flash_error', validation_errors() );

		  $this->data['enable'] = array(	'name'  => 'enable',
                                   'id'    => 'enable',
                                  /*'type'  => 'text',*/
                                 /*'class' => 'form-control',*/
                                   'checked' => $this->form_validation->set_value('enable', $message->enable)
                                 );

		  $this->data['ppage'] = array(	'name'  => 'ppage',
	                                 'id'    => 'ppage',
	                                 'type'  => 'text',
                                  'class' => 'form-control',
                                  'disabled' => 'TRUE',
	                                 'value' => $this->form_validation->set_value('ppage', $message->ppage)
                                );
		  $this->data['page'] = array(	'name'  => 'page',
	                                'id'    => 'page',
	                                'type'  => 'text',
                                 'class' => 'form-control',
                                 'disabled' => 'TRUE',
	                                'value' => $this->form_validation->set_value('page', $message->page)
                               );
		  $this->data['dls_shortname'] = array(	'name'  => 'dls_shortname',
	                                         'id'    => 'dls_shortname',
	                                         'type'  => 'text',
                                          'class' => 'form-control',
                                          'disabled' => 'TRUE',
	                                         'value' => $this->form_validation->set_value('dls_shortname', $message->shortname)
                                        );
		  $this->data['dls_tech_id'] = array(	'name'  => 'dls_tech_id',
	                                       'id'    => 'dls_tech_id',
	                                       'type'  => 'text',
                                        'class' => 'form-control',
                                        'disabled' => 'TRUE',
	                                       'value' => $this->form_validation->set_value('dls_tech_id', $message->tech_id)
                                      );
		  $this->data['acronyme'] = array(	'name'  => 'acronyme',
                                     'id'    => 'acronyme',
                                     'type'  => 'text',
                                     'class' => 'form-control',
                                     'disabled' => 'TRUE',
                                     'value' => $this->form_validation->set_value('acronyme', $message->acronyme)
                                   );
		  $this->data['libelle'] = array(	'name'  => 'libelle',
	                                   'id'    => 'libelle',
	                                   'type'  => 'text',
                                    'class' => 'form-control',
                                    /*'disabled' => 'TRUE',*/
	                                   'value' => $this->form_validation->set_value('libelle', $message->libelle)
                                  );

		  $this->data['type'] = $message->type;

		  $this->data['libelle_audio'] = array(	'name'  => 'libelle_audio',
	                                   'id'    => 'libelle_audio',
	                                   'type'  => 'text',
                                    'class' => 'form-control',
                                    /*'disabled' => 'TRUE',*/
	                                   'value' => $this->form_validation->set_value('libelle_audio', $message->libelle_audio)
                                  );
		  $this->data['sms_mode'] = array(	'name'  => 'sms_mode',
                                     'id'    => 'sms_mode',
                                     'options'  => $this->msgs_types_sms,
                                     'class' => 'form-control',
                                     'selected' => array($message->sms),
                                   );
		  $this->data['libelle_sms'] = array(	'name'  => 'libelle_sms',
	                                       'id'    => 'libelle_sms',
	                                       'type'  => 'text',
                                        'class' => 'form-control',
                                        /*'disabled' => 'TRUE',*/
	                                       'value' => $this->form_validation->set_value('libelle_sms', $message->libelle_sms)
                                      );
		  $this->template->admin_render('admin/msg/edit', $this->data);
	 }
/******************************************************************************************************************************/
function save_audio(){
		if (!isset($_POST['audio-filename']) && !isset($_POST['video-filename'])) {
			echo 'PermissionDeniedError';
			return;
		}

		$fileName = '';
		$tempName = '';

		if (isset($_POST['audio-filename'])) {
			$fileName = $_POST['audio-filename'];
			$tempName = $_FILES['audio-blob']['tmp_name'];
		} else {
			$fileName = $_POST['video-filename'];
			$tempName = $_FILES['video-blob']['tmp_name'];
		}

		if (empty($fileName) || empty($tempName)) {
			echo 'PermissionDeniedError';
			return;
		}
		
		$filePath = 'assets/audio/' . $fileName;
				
		// make sure that one can upload only allowed audio/video files
		$allowed = array(
			'webm',
			'wav',
			'mp4',
			'mp3',
			'ogg',
			'raw'
		);
		$extension = pathinfo($filePath, PATHINFO_EXTENSION);
		if (!$extension || empty($extension) || !in_array($extension, $allowed)) {
			echo 'Invalid extension : '.$filePath;
			return;
		}

		if (!move_uploaded_file($tempName, $filePath)) {
			echo ('Problem saving file.');
			return;
		}

		echo $this->text_to_speech($filePath);
		//echo $filePath;
	}
	
	private function text_to_speech($file){
		
				
		# Your Google Cloud Platform project ID
		$projectId = 'testseboc';

		# Instantiates a client
		$speech = new SpeechClient([
			'projectId' => $projectId,
			'languageCode' => 'fr-FR',
		]);
		
		# The name of the audio file to transcribe
		$fileName = realpath($file);
		
		# The audio file's encoding and sample rate
		$extension = pathinfo($fileName, PATHINFO_EXTENSION);
		if ($extension=='wav'){
			$options = [];
		}else{
			$options = [
				'encoding' => 'LINEAR16',
				'sampleRateHertz' => 16000,
			];
		}

		# Detects speech in the audio file
		$results = $speech->recognize(fopen($fileName, 'r'), $options);
		
		if (count($results)){
			foreach ($results as $result) {
				return ucfirst($result->alternatives()[0]['transcript'] . PHP_EOL);
			}
		}else{
			 return "Nous n'avons pas compris votre message. Veuillez réessayer.";
		}
	
	}	
		
    
}
