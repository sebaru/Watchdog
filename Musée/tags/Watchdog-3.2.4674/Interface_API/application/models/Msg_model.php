<?php

class Msg_model extends CI_Model
 { public function __construct()
    { parent::__construct(); }
    
/******************************************************************************************************************************/
 function get($id)
  { $this->db->select("msgs.*, dls.id as dls_id, dls.tech_id, dls.shortname, syn.access_level,".
                      "parent_syn.page as ppage, syn.page as page");
		  $this->db->from('msgs');
    $this->db->join('dls','msgs.dls_id=dls.id');
    $this->db->join('syns as syn','dls.syn_id=syn.id');
    $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
  		$this->db->where('syn.access_level<=', $this->session->user_access_level);
		  $this->db->where('msgs.id=',$id);
    return $this->db->get()->row();
  }
/******************************************************************************************************************************/
 function get_count()
  { $this->db->from('msgs');
    $this->db->join('dls','msgs.dls_id=dls.id');
		  $this->db->join('syns as syn','dls.syn_id=syn.id');
		  $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
  		$this->db->where('syn.access_level<=', $this->session->user_access_level);
    return $this->db->count_all_results();
  }

/******************************************************************************************************************************/
 function get_all($start, $length)
  { $this->db->select("msgs.*, ".
                      "dls.id as dls_id, dls.tech_id, dls.shortname, syn.access_level,".
                      "parent_syn.page as ppage, syn.page as page");
		  $this->db->from('msgs');
    $this->db->join('dls','msgs.dls_id=dls.id');
    $this->db->join('syns as syn','dls.syn_id=syn.id');
    $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
  		$this->db->where('syn.access_level<=', $this->session->user_access_level);
    if ($length != 0) { $this->db->limit($length,$start); }
    $result = $this->db->get();
    /*error_log ( 'msg get_all 2 '. $this->db->last_query() );*/
    return $result;
  }
/******************************************************************************************************************************/
 function get_by_dls_id($dls_id)
  { $this->db->select("msgs.*, ".
                      "dls.id as dls_id, dls.tech_id, dls.shortname, syn.access_level,".
                      "parent_syn.page as ppage, syn.page as page");
		  $this->db->from('msgs');
    $this->db->join('dls','msgs.dls_id=dls.id');
    $this->db->join('syns as syn','dls.syn_id=syn.id');
    $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
  		$this->db->where('syn.access_level<=', $this->session->user_access_level);
  		$this->db->where('dls.id=', $dls_id );
    $result = $this->db->get();
    /*error_log ( 'msg get_all 2 '. $this->db->last_query() );*/
    return $result;
  }
/******************************************************************************************************************************/
 function delete($id)
  { return $this->db->delete('msgs',array('id'=>$id)); }
/******************************************************************************************************************************/
 public function activate($id)
 	{ $data = array( 'enable' => 1	);
 		 $this->db->update('msgs', $data, array('id' => $id));
  		$return = $this->db->affected_rows() == 1;
    return $return;
 	}
/******************************************************************************************************************************/
	public function deactivate($id = NULL)
  {	$data = array( 'enable' => 0	);
		  $this->db->update('msgs', $data, array('id' => $id));
 	  $return = $this->db->affected_rows() == 1;
	   return $return;
	 }
/******************************************************************************************************************************/
 public function update($id,$params)
  { $this->db->where('id',$id);
    $return = $this->db->update('msgs',$params);
    error_log ( 'msg update '. $this->db->last_query() );
    return $return;
  }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
