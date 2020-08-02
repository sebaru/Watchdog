<?php

class Msg_model extends CI_Model
 { public function __construct()
    { parent::__construct(); }

/******************************************************************************************************************************/
   public function type_to_icone($type)
    { switch($type)
       { case 0: return('<img style="width: 20px" data-toggle="tooltip" title="Information" src="https://icons.abls-habitat.fr/assets/gif/Pignon_vert.svg" /> Info');
         case 2: return('<img style="width: 20px" data-toggle="tooltip" title="Défaut"      src="https://icons.abls-habitat.fr/assets/gif/Pignon_jaune.svg" /> Défaut');
         case 3: return('<img style="width: 20px" data-toggle="tooltip" title="Alarme"      src="https://icons.abls-habitat.fr/assets/gif/Pignon_orange.svg" /> Alarme');
         case 4: return('<img style="width: 20px" data-toggle="tooltip" title="Veille"      src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_vert.svg" /> Veille');
         case 1: return('<img style="width: 20px" data-toggle="tooltip" title="Alerte"      src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_rouge.svg" /> Alerte');
         case 5: return('<img style="width: 20px" data-toggle="tooltip" title="Attente"     src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_noir.svg" /> Attente');
         case 6: return('<img style="width: 20px" data-toggle="tooltip" title="Danger"      src="https://icons.abls-habitat.fr/assets/gif/Croix_rouge_rouge.svg" /> Danger');
         case 7: return('<img style="width: 20px" data-toggle="tooltip" title="Dérangement" src="https://icons.abls-habitat.fr/assets/gif/Croix_rouge_jaune.svg" /> Dérangement');
       }
    }
/******************************************************************************************************************************/
   public function type_to_color($type)
    { switch ($type)
       { case 0: return("info");
         case 4: return("success");
         case 5: return("info");
         case 3:
         case 6:
         case 1: return("danger");
         case 7:
         case 2: return("warning");
       }
    }
/******************************************************************************************************************************/
   public function get_all_MSG($tech_id)
    { $this->db->select("d.id, syn.access_level, d.shortname, ".
                        "m.*, ".
                        "parent_syn.page as ppage, syn.page as page");
   		  $this->db->from('msgs as m');
   		  $this->db->join('dls as d','m.tech_id=d.tech_id');
		     $this->db->join('syns as syn','d.syn_id=syn.id');
		     $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
   		  $this->db->where('m.tech_id=',$tech_id);
       $this->db->where("syn.access_level<=", $this->session->user_access_level );
       /*log_message('debug', 'test!'.$this->db->last_query());*/
       return $this->db->get()->result();
    }
/******************************************************************************************************************************/
   public function get_all_profils_audio()
    { $this->db->select("m.acronyme, m.libelle");
  		  $this->db->from('mnemos_DI as m');
  		  $this->db->where('m.tech_id=','AUDIO');
  		  $this->db->where('m.acronyme LIKE','P_%');
  		  $this->db->where('m.acronyme <>','P_NONE');
  		  $this->db->order_by('m.acronyme');
      return $this->db->get()->result();
    }
/******************************************************************************************************************************/
   public function get_MSG($tech_id,$acronyme)
    { $this->db->select("d.id, d.tech_id, syn.access_level, d.shortname, ".
                        "m.*, ".
                        "parent_syn.page as ppage, syn.page as page");
  		  $this->db->from('msgs as m');
  		  $this->db->join('dls as d','m.tech_id=d.tech_id');
	     $this->db->join('syns as syn','d.syn_id=syn.id');
      $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
   	  $this->db->where('d.tech_id=',$tech_id);
  		  $this->db->where('m.acronyme=',$acronyme);
      $this->db->where("syn.access_level<=", $this->session->user_access_level );
      return($this->db->get()->row());
    }
/******************************************************************************************************************************/
   public function set_MSG($id,$params)
    { $this->db->where('m.id',$id);
      return $this->db->update('msgs as m',$params);
    }
/******************************************************************************************************************************/
   public function delete_MSG($id)
    { return $this->db->delete('msgs',array('id' => $id));
    }
/******************************************************************************************************************************/
   public function clear_histo()
    { $this->db->set('alive', 'NULL', FALSE);
  		  $this->db->where('alive', '1');
      return $this->db->update('histo_msgs');
    }
/************************************* Recherche d'ancien message dans l'historique *******************************************/
   public function get_histo($search)
    { $this->db->select("d.id, d.tech_id, d.name, m.acronyme, m.libelle, m.type, syn.access_level, d.shortname, ".
                        "h.*, ".
                        "parent_syn.page as ppage, syn.page as page");
  		  $this->db->from('histo_msgs as h');
  		  $this->db->join('msgs as m','h.id_msg=m.id');
  		  $this->db->join('dls as d','m.tech_id=d.tech_id');
	     $this->db->join('syns as syn','d.syn_id=syn.id');
	     $this->db->join('syns as parent_syn','syn.parent_id=parent_syn.id');
  		  $this->db->where('m.libelle like', '%'.$search.'%');
  		  $this->db->or_where('d.shortname like','%'.$search.'%');
  		  $this->db->or_where('d.tech_id like','%'.$search.'%');
  		  $this->db->or_where('syn.libelle like','%'.$search.'%');
      $this->db->where("syn.access_level<=", $this->session->user_access_level );
  		  $this->db->order_by('h.date_create', 'DESC');
  		  $this->db->limit('200');
      /*log_message('debug', 'test!'.$this->db->last_query());*/
      return($this->db->get()->result());
    }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
