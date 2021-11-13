<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Mnemo_model extends CI_Model
 { public function __construct()
    { parent::__construct(); }

/******************************************************************************************************************************/
	  public function get_total_all()
	   { $count = $this->db->count_all('mnemos_DI');
      $count+= $this->db->count_all('mnemos_DO');
      $count+= $this->db->count_all('mnemos_AI');
      $count+= $this->db->count_all('mnemos_AO');
      $count+= $this->db->count_all('mnemos_CI');
      $count+= $this->db->count_all('mnemos_CH');
      $count+= $this->db->count_all('mnemos_HORLOGE');
		    return $count;
	   }

/******************************************************************************************************************************/
   private function get_mnemos($table, $tech_id, $acronyme=NULL)
    { $this->db->select("d.id, syn.access_level, d.shortname, ".
                        "m.*, ".
                        "parent_syn.page as ppage, syn.page as page");
   		  $this->db->from($table.' as m');
   		  $this->db->join('dls as d','m.tech_id=d.tech_id');
		     $this->db->join('syns as syn','d.syn_id=syn.id');
		     $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
   		  if (isset($tech_id))  $this->db->where('m.tech_id=',$tech_id);
   		  if (isset($acronyme)) $this->db->where('m.acronyme=',$acronyme);
       $this->db->where("syn.access_level<=", $this->session->user_access_level );
       if (isset($acronyme)) return $this->db->get()->row();
                        else return $this->db->get()->result();
    }
/******************************************************************************************************************************/
   public function get_all_DI($tech_id)
    { return( $this->get_mnemos ( 'mnemos_DI', $tech_id, NULL ) ); }
/******************************************************************************************************************************/
   public function get_DI($tech_id, $acronyme)
    { return( $this->get_mnemos ( 'mnemos_DI', $tech_id, $acronyme ) ); }

/******************************************************************************************************************************/
   public function get_all_DI_by_thread($tech_id,$thread)
    { $this->db->select("d.id, syn.access_level, d.shortname, ".
                        "m.*, ".
                        "parent_syn.page as ppage, syn.page as page");
   		 $this->db->from('mnemos_DI as m');
   		 $this->db->join('dls as d','m.tech_id=d.tech_id');
		    $this->db->join('syns as syn','d.syn_id=syn.id');
		    $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
   		 $this->db->where('m.src_thread LIKE', $thread );
   		 $this->db->where('m.src_text LIKE', $tech_id.":DI%");
      $this->db->where("syn.access_level<=", $this->session->user_access_level );
      $this->db->order_by("m.src_text");
      return($this->db->get()->result());
    }
/******************************************************************************************************************************/
   public function set_DI($tech_id,$acronyme,$params)
    { $this->db->where('m.tech_id',$tech_id);
		    $this->db->where('m.acronyme',$acronyme);
		    return $this->db->update('mnemos_DI as m',$params);
    }
/******************************************************************************************************************************/
   public function delete_DI($tech_id,$acronyme)
    { $this->db->where('tech_id',$tech_id);
		    $this->db->where('acronyme',$acronyme);
		    return $this->db->delete('mnemos_DI');
    }
/******************************************************************************************************************************/
   public function get_all_DO($tech_id)
    { return( $this->get_mnemos ( 'mnemos_DO', $tech_id, NULL ) ); }
/******************************************************************************************************************************/
   public function get_DO($tech_id, $acronyme)
    { return( $this->get_mnemos ( 'mnemos_DO', $tech_id, $acronyme ) ); }
/******************************************************************************************************************************/
   function set_DO($tech_id,$acronyme,$params)
    { $this->db->where('m.tech_id',$tech_id);
		    $this->db->where('m.acronyme',$acronyme);
		    return $this->db->update('mnemos_DO as m',$params);
    }
/******************************************************************************************************************************/
   function delete_DO($tech_id,$acronyme)
    { $this->db->where('tech_id',$tech_id);
		    $this->db->where('acronyme',$acronyme);
		    return $this->db->delete('mnemos_DO');
    }
/******************************************************************************************************************************/
   public function get_all_CI($tech_id)
    { return( $this->get_mnemos ( 'mnemos_CI', $tech_id, NULL ) ); }
/******************************************************************************************************************************/
   public function get_CI($tech_id,$acronyme)
    { return( $this->get_mnemos ( 'mnemos_CI', $tech_id, $acronyme ) ); }
/******************************************************************************************************************************/
   public function set_CI($tech_id,$acronyme,$params)
    { $this->db->where('m.tech_id',$tech_id);
		    $this->db->where('m.acronyme',$acronyme);
		    return $this->db->update('mnemos_CI as m',$params);
    }
/******************************************************************************************************************************/
   public function delete_CI($tech_id,$acronyme)
    { $this->db->where('tech_id',$tech_id);
		    $this->db->where('acronyme',$acronyme);
		    return $this->db->delete('mnemos_CI');
    }
/******************************************************************************************************************************/
   public function get_all_AI($tech_id)
    { return( $this->get_mnemos ( 'mnemos_AI', $tech_id, NULL ) ); }
/******************************************************************************************************************************/
   public function get_AI($tech_id, $acronyme)
    { return( $this->get_mnemos ( 'mnemos_AI', $tech_id, $acronyme ) ); }
/******************************************************************************************************************************/
   public function get_all_AI_by_thread($tech_id,$thread)
    { $this->db->select("d.id, syn.access_level, d.shortname, ".
                        "m.*, ".
                        "parent_syn.page as ppage, syn.page as page");
   		  $this->db->from('mnemos_AI as m');
   		  $this->db->join('dls as d','m.tech_id=d.tech_id');
		     $this->db->join('syns as syn','d.syn_id=syn.id');
		     $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
   		  $this->db->where('m.map_thread LIKE', $thread );
   		  $this->db->where('m.map_text LIKE', $tech_id.":AI%");
       $this->db->where("syn.access_level<=", $this->session->user_access_level );
       $this->db->order_by("m.map_text");
       return($this->db->get()->result());
    }
/******************************************************************************************************************************/
   public function set_AI($tech_id,$acronyme,$params)
    { $this->db->where('m.tech_id',$tech_id);
		    $this->db->where('m.acronyme',$acronyme);
		    return $this->db->update('mnemos_AI as m',$params);
    }
/******************************************************************************************************************************/
   public function delete_AI($tech_id,$acronyme)
    { $this->db->where('tech_id',$tech_id);
		    $this->db->where('acronyme',$acronyme);
		    return $this->db->delete('mnemos_AI');
    }
/******************************************************************************************************************************/
   public function get_all_CH($tech_id)
    { return( $this->get_mnemos ( 'mnemos_CH', $tech_id, NULL ) ); }
/******************************************************************************************************************************/
   public function get_CH($tech_id, $acronyme)
    { return( $this->get_mnemos ( 'mnemos_CH', $tech_id, $acronyme ) ); }
/******************************************************************************************************************************/
   public function set_CH($tech_id,$acronyme,$params)
    { $this->db->where('m.tech_id',$tech_id);
		    $this->db->where('m.acronyme',$acronyme);
		    return $this->db->update('mnemos_CH as m',$params);
    }
/******************************************************************************************************************************/
   public function delete_CH($tech_id,$acronyme)
    { $this->db->where('tech_id',$tech_id);
		    $this->db->where('acronyme',$acronyme);
		    return $this->db->delete('mnemos_CH');
    }
/******************************************************************************************************************************/
   public function get_HORLOGE_page($page_id)
    { $this->db->select("d.id, syn.access_level, d.shortname, ".
                        "m.*, ".
                        "parent_syn.page as ppage, syn.page as page");
   		 $this->db->from(' mnemos_HORLOGE as m');
   		 $this->db->join('dls as d','m.tech_id=d.tech_id');
		    $this->db->join('syns as syn','d.syn_id=syn.id');
		    $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
   		 if (isset($page_id))  $this->db->where('syn.page=',$page_id);
      $this->db->where("syn.access_level<=", $this->session->user_access_level );
      return $this->db->get()->result();
    }
/******************************************************************************************************************************/
   public function get_all_HORLOGE($tech_id)
    { return( $this->get_mnemos ( 'mnemos_HORLOGE', $tech_id, NULL ) ); }
/******************************************************************************************************************************/
   public function get_HORLOGE($tech_id, $acronyme)
    { return( $this->get_mnemos ( 'mnemos_HORLOGE', $tech_id, $acronyme ) ); }
/******************************************************************************************************************************/
   public function set_HORLOGE($tech_id,$acronyme,$params)
    { $this->db->where('tech_id',$tech_id);
		    $this->db->where('acronyme',$acronyme);
		    return $this->db->update('mnemos_HORLOGE',$params);
    }
/******************************************************************************************************************************/
   public function delete_HORLOGE($tech_id,$acronyme)
    { $this->db->where('tech_id',$tech_id);
		    $this->db->where('acronyme',$acronyme);
		    return $this->db->delete('mnemos_HORLOGE');
    }
/******************************************************************************************************************************/
   public function get_all_TEMPO($tech_id)
    { return( $this->get_mnemos ( 'mnemos_Tempo', $tech_id, NULL ) ); }
/******************************************************************************************************************************/
   public function get_TEMPO($tech_id, $acronyme)
    { return( $this->get_mnemos ( 'mnemos_Tempo', $tech_id, $acronyme ) ); }
/******************************************************************************************************************************/
   public function delete_TEMPO($tech_id,$acronyme)
    { $this->db->where('tech_id',$tech_id);
		    $this->db->where('acronyme',$acronyme);
		    return $this->db->delete('mnemos_Tempo');
    }
/******************************************************************************************************************************/
   public function get_all_AO($tech_id)
    { return( $this->get_mnemos ( 'mnemos_AO', $tech_id, NULL ) ); }
/******************************************************************************************************************************/
   public function get_AO($tech_id, $acronyme)
    { return( $this->get_mnemos ( 'mnemos_AO', $tech_id, $acronyme ) ); }
/******************************************************************************************************************************/
   public function get_all_AO_by_thread($tech_id,$thread)
    { $this->db->select("d.id, syn.access_level, d.shortname, ".
                        "m.*, ".
                        "parent_syn.page as ppage, syn.page as page");
   		  $this->db->from('mnemos_AO as m');
   		  $this->db->join('dls as d','m.tech_id=d.tech_id');
		     $this->db->join('syns as syn','d.syn_id=syn.id');
		     $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
   		  $this->db->where('m.map_thread LIKE', $thread );
   		  $this->db->where('m.map_text LIKE', $tech_id.":AO%");
       $this->db->where("syn.access_level<=", $this->session->user_access_level );
       $this->db->order_by("m.map_text");
       return($this->db->get()->result());
    }
/******************************************************************************************************************************/
   public function set_AO($tech_id,$acronyme,$params)
    { $this->db->where('m.tech_id',$tech_id);
		    $this->db->where('m.acronyme',$acronyme);
		    return $this->db->update('mnemos_AO as m',$params);
    }
/******************************************************************************************************************************/
   public function delete_AO($tech_id,$acronyme)
    { $this->db->where('tech_id',$tech_id);
		    $this->db->where('acronyme',$acronyme);
		    return $this->db->delete('mnemos_AO');
    }
/******************************************************************************************************************************/
   public function get_all_R($tech_id)
    { return( $this->get_mnemos ( 'mnemos_R', $tech_id, NULL ) ); }
/******************************************************************************************************************************/
   public function get_R($tech_id, $acronyme)
    { return( $this->get_mnemos ( 'mnemos_R', $tech_id, $acronyme ) ); }
/******************************************************************************************************************************/
   public function set_R($tech_id,$acronyme,$params)
    { $this->db->where('m.tech_id',$tech_id);
		    $this->db->where('m.acronyme',$acronyme);
		    return $this->db->update('mnemos_R as m',$params);
    }
/******************************************************************************************************************************/
   public function delete_R($tech_id,$acronyme)
    { $this->db->where('tech_id',$tech_id);
		    $this->db->where('acronyme',$acronyme);
		    return $this->db->delete('mnemos_R');
    }
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
