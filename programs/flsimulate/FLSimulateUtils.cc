// Ourselves
#include "FLSimulateUtils.h"

// Third party:
// - Bayeux:
#include "bayeux/datatools/kernel.h"
#include "bayeux/datatools/urn_query_service.h"

// This Project:
#include "falaise/detail/falaise_sys.h"

namespace FLSimulate {

std::string default_simulation_setup() { return "urn:snemo:demonstrator:simulation:2.3"; }

std::map<std::string, std::string> list_of_simulation_setups() {
  std::map<std::string, std::string> m;
  datatools::kernel& dtk = ::datatools::kernel::instance();
  if (dtk.has_urn_query()) {
    const datatools::urn_query_service& dtkUrnQuery = dtk.get_urn_query();
    std::vector<std::string> flsim_urn_infos;
    if (dtkUrnQuery.find_urn_info(flsim_urn_infos, falaise::detail::falaise_sys::fl_setup_db_name(),
                                  // Format : "urn:{namespace}:{experiment}:simulation:{version}"
                                  "(urn:)([^:]*)(:)([^:]*)(:simulation:)([^:]*)", "simsetup")) {
      for (const auto& flsim_urn_info : flsim_urn_infos) {
        const datatools::urn_info& ui = dtkUrnQuery.get_urn_info(flsim_urn_info);
        m[ui.get_urn()] = ui.get_description();
      }
    }
  }
  return m;
}

}  // end of namespace FLSimulate
