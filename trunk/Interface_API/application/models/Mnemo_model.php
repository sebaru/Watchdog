<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Mnemo_model extends CI_Model
 { public function __construct()
    { parent::__construct(); }

/******************************************************************************************************************************/
   public function get($id)
    { $this->db->select("d.id as dls_id, d.tech_id, syn.access_level, d.shortname, ".
                        "m.*, ".
                        "parent_syn.page as ppage, syn.page as page");
       $this->db->from('mnemos as m');
       $this->db->join('dls as d','m.dls_id=d.id');
       $this->db->join('syns as syn','d.syn_id=syn.id');
       $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
       $this->db->where('m.id=',$id);
       /*$this->db->where("syn.access_level<=", $this->session->user_access_level );*/
       return $this->db->get()->row();
    }
/******************************************************************************************************************************/
   public function get_voice()
    { $this->db->select("m.ev_text");
       $this->db->from('mnemos as m');
       $this->db->where('m.ev_thread=',"VOICE");
       return $this->db->get()->result();
    }
/******************************************************************************************************************************/
   public function get_count()
    { $response = $this->db->select("COUNT(*) as num")->get("mnemos");
      $result = $response->row();
      if(isset($result)) return $result->num;
      return 0;
    }
        
/******************************************************************************************************************************/
   public function get_all($dls_id)
    { $this->db->select("d.id as dls_id, d.tech_id, syn.access_level, d.shortname, ".
                        "m.*, ".
                        "parent_syn.page as ppage, syn.page as page");
       $this->db->from('mnemos as m');
       $this->db->join('dls as d','m.dls_id=d.id');
       $this->db->join('syns as syn','d.syn_id=syn.id');
       $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
       $this->db->where('m.dls_id=',$dls_id);
       /*$this->db->where("syn.access_level<=", $this->session->user_access_level );*/
       return $this->db->get()->result();
    }
 
        
/******************************************************************************************************************************/
   function add($params)
    { $this->db->insert('mnemos',$params);
      return $this->db->insert_id();
    }
    
/******************************************************************************************************************************/
   function update($id,$params)
    { $this->db->where('id',$id);
      return $this->db->update('mnemos',$params);
    }
    
/******************************************************************************************************************************/
   function delete($id)
    { return $this->db->delete('mnemos',array('id'=>$id));
    }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
