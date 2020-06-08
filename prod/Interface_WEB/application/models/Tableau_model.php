<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Tableau_model extends CI_Model
 { public function __construct()
    { parent::__construct(); }

/******************************************************************************************************************************/
   public function get($id)
    { $this->db->select("*");
      $this->db->where('id=',$id);
      $this->db->where('access_level<=',$this->session->user_access_level);
      $this->db->from("tableau");
      return $this->db->get()->row();
    }
/******************************************************************************************************************************/
   public function get_courbes($id)
    { $this->db->select("t.titre, c.*");
      $this->db->where('tableau_id=',$id);
      $this->db->where('access_level<=',$this->session->user_access_level);
      $this->db->from("courbes as c");
      $this->db->join("tableau as t","t.id=c.tableau_id","INNER");
      return $this->db->get()->result();
    }
/******************************************************************************************************************************/
	  public function get_total()
	   { $response = $this->db->select("COUNT(*) as num")->get("tableau");
		    $result = $response->row();
		    if(isset($result)) return $result->num;
		    return 0;
	   }

/******************************************************************************************************************************/
   public function get_all()
    { $this->db->select("*");
      $this->db->from("tableau");
      $response = $this->db->get()->result();
      /*error_log ( 'get_all_modbus '. $this->db->last_query() );*/
      return $response;
    }
/******************************************************************************************************************************/
   function update($id,$params)
    { $this->db->where('id',$id);
      return $this->db->update('tableau',$params);
    }

/******************************************************************************************************************************/
   function delete($id)
    { return $this->db->delete('tableau',array('id'=>$id));
    }
/******************************************************************************************************************************/
   function add($params)
    { $this->db->set('date_create',   date("Y-m-d H:i:s"));
      $this->db->insert('tableau',$params);
      return $this->db->insert_id();
    }
/******************************************************************************************************************************/
   function add_courbe($params)
    { $this->db->insert('courbes',$params);
      return $this->db->insert_id();
    }
/******************************************************************************************************************************/
   function set_courbe($id,$params)
    { $this->db->where('id',$id);
      return $this->db->update('courbes',$params);
    }
/******************************************************************************************************************************/
   function del_courbe($id)
    { return $this->db->delete('courbes',array('id'=>$id));
    }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
