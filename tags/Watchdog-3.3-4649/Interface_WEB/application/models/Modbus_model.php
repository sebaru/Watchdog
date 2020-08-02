<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Modbus_model extends CI_Model
 { public function __construct()
    { parent::__construct(); }

/******************************************************************************************************************************/
   public function get($id)
    { $this->db->where('id=',$id);
      $this->db->select("*");
      $this->db->from("modbus_modules");
      return $this->db->get()->row();
    }
    
/******************************************************************************************************************************/
	  public function get_total()
	   { $response = $this->db->select("COUNT(*) as num")->get("modbus_modules");
		    $result = $response->row();
		    if(isset($result)) return $result->num;
		    return 0;
	   }
        
/******************************************************************************************************************************/
   public function get_all()
    { $this->db->select("*");
      $this->db->from("modbus_modules");
      $response = $this->db->get();
      error_log ( 'get_all_modbus '. $this->db->last_query() );
      return $response;
    }
	
        
/******************************************************************************************************************************/
   function add($params)
    { $this->db->insert('modbus_modules',$params);
      return $this->db->insert_id();
    }
    
/******************************************************************************************************************************/
   function update($id,$params)
    { $this->db->where('id',$id);
      return $this->db->update('modbus_modules',$params);
    }
    
/******************************************************************************************************************************/
   function delete($id)
    { return $this->db->delete('modbus_modules',array('id'=>$id));
    }
/******************************************************************************************************************************/
	  public function activate($id)
   	{ $data = array( 'enable' => 1	);
   		 $this->db->update('modbus_modules', $data, array('id' => $id));
    		$return = $this->db->affected_rows() == 1;
	     return $return;
   	}
/******************************************************************************************************************************/
  	public function deactivate($id = NULL)
 	  {	$data = array( 'enable' => 0	);
  		  $this->db->update('modbus_modules', $data, array('id' => $id));
 	 	  $return = $this->db->affected_rows() == 1;
 		   return $return;
  	 }
    
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
