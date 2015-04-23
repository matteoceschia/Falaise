// snemo/digitization/sd_to_calo_signal_algo.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// Third party:
// - Bayeux/datatools :
#include <datatools/clhep_units.h>
// - Bayeux/geomtools:
#include <bayeux/geomtools/geom_id.h>
// Ourselves:
#include <snemo/digitization/sd_to_calo_signal_algo.h>

namespace snemo {
  
  namespace digitization {

    sd_to_calo_signal_algo::sd_to_calo_signal_algo()
    {
      _initialized_ = false;
      _geo_manager_ = 0;
      return;
    }

    sd_to_calo_signal_algo::sd_to_calo_signal_algo(const geomtools::manager & mgr_)
    {
      _initialized_ = false;
      set_geo_manager(mgr_);
      return;
    }

    sd_to_calo_signal_algo::~sd_to_calo_signal_algo()
    {   
      if (is_initialized())
	{
	  reset();
	}
      return;
    }

    void sd_to_calo_signal_algo::initialize()
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "SD to calo signal algorithm is already initialized ! ");
      _initialized_ = true;
      return;
    }
    
    bool sd_to_calo_signal_algo::is_initialized() const
    {
      return _initialized_;
    }

    void sd_to_calo_signal_algo::reset()
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "SD to calo signal algorithm is not initialized, it can't be reset ! ");
      _initialized_ = false;
      _geo_manager_ = 0;
      return;
    }

    void sd_to_calo_signal_algo::set_geo_manager(const geomtools::manager & mgr_ )
    {
      _geo_manager_ = & mgr_;
      return;
    }

    void sd_to_calo_signal_algo::process(const mctools::simulated_data & sd_,
					signal_data & signal_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "SD to calo signal algorithm is not initialized ! ");	
      _process(sd_, signal_data_);
      return ;
    }

    void sd_to_calo_signal_algo::_process(const mctools::simulated_data & sd_,
					  signal_data & signal_data)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "SD to calo signal algorithm is not initialized ! ");
      std::clog << "DEBUG : BEGINING OF CALO PROCESS " << std::endl;
      std::clog << "**************************************************************" << std::endl;
      // pickup the ID mapping from the geometry manager:
      const geomtools::mapping & the_mapping = _geo_manager_->get_mapping();

      // Loop on Calo step hits:
      const size_t number_of_hits = sd_.get_number_of_step_hits("calo");

      int32_t calo_signal_hit_id = 0;
      for (size_t ihit = 0; ihit < number_of_hits; ihit++)
	{
	  const mctools::base_step_hit & calo_hit = sd_.get_step_hit("calo", ihit);
	    
	  const double signal_time = calo_hit.get_time_stop();
	  const double energy_deposit = calo_hit.get_energy_deposit();
	  const double amplitude = _convert_energy_to_amplitude(energy_deposit);
	    
	  // extract the corresponding geom ID:
	  const geomtools::geom_id & calo_gid = calo_hit.get_geom_id();
	    
	  calo_signal & calo_signal = signal_data.add_calo_signal();
	  calo_signal.set_header(calo_signal_hit_id, calo_gid);
	  calo_signal.set_data(signal_time, amplitude); 
	  calo_signal.grab_auxiliaries().store("hit.id", calo_hit.get_hit_id());
	  calo_signal.tree_dump(std::clog, "***** Calo Signal : *****", "INFO : ");
	  calo_signal_hit_id++;
	}	  
      return ;
    }

    double sd_to_calo_signal_algo::_convert_energy_to_amplitude(const double energy_)
    {
      // 1 MeV is equivalent to 300 mV
      return energy_ * 0.3; // maybe units problem for the moment
    }

  } // end of namespace digitization

} // end of namespace snemo