<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Ticks_model extends CI_Model
 { public function __construct()
    { parent::__construct(); }

/******************************************************************************************************************************/
   public function get($id)
    { $this->db->select("*");
   		  $this->db->from( "mnemos_HORLOGE_ticks as m" );
   		  $this->db->where('m.horloge_id=',$id);
       return $this->db->get()->result();
    }

/******************************************************************************************************************************/
   function add($params)
    { $this->db->insert('mnemos_HORLOGE_ticks',$params);
      return $this->db->insert_id();
    }
/******************************************************************************************************************************/
   public function set($id,$params)
    { $this->db->where('id',$id);
		    return $this->db->update('mnemos_HORLOGE_ticks',$params);
    }
/******************************************************************************************************************************/
   public function delete($id)
    { $this->db->where('id',$id);
		    return $this->db->delete('mnemos_HORLOGE_ticks');
    }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
