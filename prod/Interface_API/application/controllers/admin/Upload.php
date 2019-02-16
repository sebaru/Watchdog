<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Upload extends CI_Controller 
{
    public function __construct()
    {
        parent::__construct();
        //$this->load->model('files_model');
        $this->load->helper('url');
    }
 
 
	function upload_files() {
		
        if (isset($_FILES['files']) && !empty($_FILES['files'])) {
            $no_files = count($_FILES["files"]['name']);
            for ($i = 0; $i < $no_files; $i++) {
                if ($_FILES["files"]["error"][$i] > 0) {
					echo json_encode(array('status' => 'error', 'file' => '', 'file_title' =>'', 'msg' => "Error: " . $_FILES["files"]["error"][$i]));
                    
                } else {
					
					$ext = pathinfo($_FILES["files"]["name"][$i], PATHINFO_EXTENSION);	
					
					if ($this->input->post('file_name') && $this->input->post('file_path')){
						
						if (!file_exists('assets/'.$this->input->post('file_path'))) {
							mkdir('assets/'.$this->input->post('file_path'), 0777, true);
						}
												
						$file_title = $this->input->post('file_name').'.'.$ext; 
						$file = 'assets/'.$this->input->post('file_path').'/' . $file_title;
						
						if ($this->input->post('file_resize')){
							if (!file_exists('assets/'.$this->input->post('file_path').'/30x30')) {
								mkdir('assets/'.$this->input->post('file_path').'/30x30', 0777, true);
							}
							$file_30x30 = 'assets/'.$this->input->post('file_path').'/30x30/' . $file_title;
							
							if (!file_exists('assets/'.$this->input->post('file_path').'/82x82')) {
								mkdir('assets/'.$this->input->post('file_path').'/82x82', 0777, true);
							}
							$file_82x82 = 'assets/'.$this->input->post('file_path').'/82x82/' . $file_title;						
						}
						
					}else{
						$file_title = uniqid().'.'.$ext; 
						if ($this->input->post('file_path')){
							
							if (!file_exists('assets/'.$this->input->post('file_path'))) {
								mkdir('assets/'.$this->input->post('file_path'), 0777, true);
							}
							
							
							$file = 'assets/'.$this->input->post('file_path').'/' . $file_title;
						}else{
							$file = 'assets/uploads/' . $file_title;
						}
					}
					
					move_uploaded_file($_FILES["files"]["tmp_name"][$i], $file);
					
					if ($this->input->post('file_resize')){
						$this->load->library('image_lib');
						$config['image_library'] = 'gd2';
						$config['source_image'] = $file;
						//$config['create_thumb'] = TRUE;
						$config['maintain_ratio'] = TRUE;						
						$config['width']         = 30;
						$config['height']       = 30;
						$config['new_image'] =$file_30x30;
						$this->image_lib->initialize($config);
						$this->image_lib->resize();
						$this->image_lib->clear();
						$config['width']         = 82;
						$config['height']       = 82;
						$config['new_image'] =$file_82x82;
						$this->image_lib->initialize($config);
						$this->image_lib->resize();
					}
					
					echo json_encode(array('status' => 'success', 'file' => $file, 'file_title' =>$file_title, 'msg' => "Success"));
                    
                }
            }
        } else {
            echo json_encode(array('status' => 'error', 'file' => '', 'file_title' =>'', 'msg' => "Veuillez sélectionner un fichier" ));
        }
    }
 /*
	public function upload_file()
{
    $status = "";
    $msg = "";
    //$file_element_name = $_POST['element_name'];
	
	$title = uniqid('asset_');
         
    if ($status != "error")
    {
        $config['upload_path'] = 'assets/uploads/';
        $config['allowed_types'] = 'gif|jpg|png|doc|txt';
        $config['max_size'] = 1024 * 8;
        $config['encrypt_name'] = TRUE;
 
        $this->load->library('upload', $config);
 
        if (!$this->upload->do_upload())
        {
            $status = 'error';
            $msg = $file_element_name.$comma_separated.$this->upload->display_errors('', '');
        }
        else
        {
            $data = $this->upload->data();
            $file_id = $this->files_model->insert_file($data['file_name'], $title);
            if($file_id)
            {
                $status = "success";
                $msg = "File successfully uploaded";
            }
            else
            {
                unlink($data['full_path']);
                $status = "error";
                $msg = "Something went wrong when saving the file, please try again.";
            }
        }
        @unlink($_FILES[$file_element_name]);
    }
    echo json_encode(array('status' => $status, 'msg' => $msg));
}
*/    
}