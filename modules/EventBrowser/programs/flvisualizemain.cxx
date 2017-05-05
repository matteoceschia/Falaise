// flvisualize.cxx
// Author(s)     : Xavier Garrido <garrido@lal.in2p3.fr>
// Creation date : 2010-07-03
// Last modified : 2017-03-02
//
// Copyright (C) 2011-2017 Xavier Garrido <garrido@lal.in2p3.fr>
//
//
// \b Description:
//
//  A visualization program that displays SuperNEMO events
//  (simulated/calibrated/...).
//
// \b Example:
//
// @code
// flvisualize -i ${SNCORE_DATA_DIR}/resources/setup/snemo/samples/snemo_SD_CD_0.txt.gz
// @endcode
//
//

// Standard library:
#include <cstdlib>
#include <iostream>
#include <stdexcept>

// Third party:
// - Boost
#include <boost/program_options.hpp>
namespace bpo = boost::program_options;
// - ROOT:
#include <TApplication.h>
#include <TROOT.h>
#include <TSystem.h>
// - Bayeux/datatools:
#include <bayeux/version.h>
#include <bayeux/bayeux.h>
#include <bayeux/datatools/kernel.h>
#include <bayeux/datatools/ioutils.h>
// #include <bayeux/datatools/library_loader.h>
#include <bayeux/datatools/configuration/variant_service.h>
namespace dtc = datatools::configuration;

// This project:
#include <falaise/falaise.h>
#include <falaise/exitcodes.h>
// This plugin:
#include <falaise/snemo/view/event_browser.h>
#include <falaise/snemo/view/options_manager.h>
#include <falaise/snemo/detector/detector_manager.h>
#include "FLVisualizeArgs.h"
#include "FLVisualizeCommandLine.h"

//----------------------------------------------------------------------
// DECLARATIONS
//----------------------------------------------------------------------
namespace FLVisualize {

  //----------------------------------------------------------------------
  //! Perform simulation using command line args as given
  falaise::exit_code do_flvisualize(int argc_, char *argv_[]);

} // namespace FLVisualize

//----------------------------------------------------------------------
// MAIN PROGRAM
//----------------------------------------------------------------------
int main(int argc_, char *argv_[]) {
  falaise::initialize(argc_, argv_, FLVisualize::app_kernel_init_flags());

  // - Do the simulation.
  // Ideally, exceptions SHOULD NOT propagate out of this  - the error
  // code should be enough.
  falaise::exit_code ret = FLVisualize::do_flvisualize(argc_, argv_);

  falaise::terminate();
  return ret;
}

//----------------------------------------------------------------------
// DEFINITIONS
//----------------------------------------------------------------------
namespace FLVisualize {

  //----------------------------------------------------------------------
  //! Perform simulation using command line args as given
  falaise::exit_code do_flvisualize(int argc_, char *argv_[]) {
    namespace sv = snemo::visualization;
    // - Configure
    FLVisualizeArgs flVisParameters = FLVisualizeArgs::makeDefault();
    try {
      do_configure(argc_, argv_, flVisParameters);
    } catch (FLConfigDefaultError & e) {
      std::cerr << "Unable to configure core of flvisualize" << std::endl;
      return falaise::EXIT_UNAVAILABLE;
    } catch (FLConfigHelpHandled & e) {
      return falaise::EXIT_OK;
    } catch (FLConfigUserError & e) {
      return falaise::EXIT_USAGE;
    }

    // Variants support:
    dtc::variant_service vserv;
    try {
      if (flVisParameters.variants.is_active()) {
        vserv.configure(flVisParameters.variants);
        // Start and lock the variant service:
        DT_LOG_DEBUG(flVisParameters.logLevel,"Starting the variants service...");
        vserv.start();
        DT_LOG_DEBUG(flVisParameters.logLevel,"Variants service is started.");
        // From this point, all other services and/or processing modules can benefit
        // of the variant service during their configuration steps.
        // flVisParameters.variants.print(std::cerr, "flvisualize: variant service configuration:", "DEVEL: ");
      }
    } catch (std::exception & e) {
      std::cerr << "[datatools::configuration::variant_service::variant_exception] "
                << e.what()
                << std::endl;
      throw FLDialogOptionsError();
    }

    // - Run the browser:
    try {

      // Browser runs here...
      DT_LOG_NOTICE(flVisParameters.logLevel, "Configuring browser...");

      // Build detector manager
      sv::detector::detector_manager & detector_mgr = sv::detector::detector_manager::get_instance();
      detector_mgr.initialize();
      detector_mgr.construct();

      // Open a root application
      DT_THROW_IF(gROOT->IsBatch(), std::logic_error, "Can not be run in 'batch' mode");

      int narg = 1;
      TApplication * my_application = new TApplication("ROOT Application", &narg, argv_);

      // Get the screen dimensions
      int position_x, position_y;
      unsigned int screen_width, screen_height;
      gVirtualX->GetWindowSize(gClient->GetRoot()->GetId(),
                               position_x, position_y,
                               screen_width, screen_height);

      // The window is slightly smaller than the screen
      const double scale_factor = sv::view::options_manager::get_instance().get_scaling_factor();
      const int height = int(scale_factor*screen_height);
      const int width  = int(scale_factor*screen_width);

      // The event_browser* is autodestructive!!! NEVER delete manually!!
      sv::view::event_browser * my_event_browser
        = new sv::view::event_browser(gClient->GetRoot(), width, height);
      my_event_browser->initialize();

      DT_LOG_NOTICE(flVisParameters.logLevel, "Browser runs here...");
      my_application->Run(true);
      DT_LOG_NOTICE(flVisParameters.logLevel, "Browser stopped.");

    } catch (std::exception & e) {
      std::cerr << "flsimulate : setup/run of simulation threw exception" << std::endl;
      std::cerr << e.what() << std::endl;
      return falaise::EXIT_UNAVAILABLE;
    }

    if (vserv.is_started()) {
      // Terminate the variant service:
      DT_LOG_DEBUG(flVisParameters.logLevel,"Stopping the variants service...");
      vserv.stop();
      DT_LOG_DEBUG(flVisParameters.logLevel,"Variants service is stopped.");
    }

    return falaise::EXIT_OK;
  }

} // namespace FLVisualize