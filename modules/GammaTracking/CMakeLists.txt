# CMAKE_COMPILER
# - Basic CMake setup
# Check version meets ou requirements
# Declare project, which will configure compiler for us
cmake_minimum_required(VERSION 3.3)
project(FalaiseGammaTrackingPlugin VERSION "2.1.0")

# Ensure our code can see the Falaise headers
#include_directories(${Falaise_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

############################################################################################
# - GammaTracking library:

list(APPEND GammaTracking_HEADERS
  GammaTracking/event.h
  GammaTracking/tof_computing.h
  GammaTracking/gamma_tracking.h
)

list(APPEND GammaTracking_SOURCES
  GammaTracking/event.cc
  GammaTracking/tof_computing.cc
  GammaTracking/gamma_tracking.cc
)

# Build a dynamic library from our sources
add_library(GammaTracking SHARED ${GammaTracking_HEADERS} ${GammaTracking_SOURCES})

target_link_libraries(GammaTracking Bayeux::Bayeux)

# Apple linker requires dynamic lookup of symbols, so we
# add link flags on this platform
if(APPLE)
  set_target_properties(GammaTracking
    PROPERTIES LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

############################################################################################
# - GammaTracking modules:
# - Headers:
list(APPEND FalaiseGammaTrackingPlugin_HEADERS GammaTracking/gamma_tracking_driver.h)

# - Sources:
list(APPEND FalaiseGammaTrackingPlugin_SOURCES GammaTracking/gamma_tracking_driver.cc)
list(APPEND FalaiseGammaTrackingPlugin_SOURCES GammaTracking/gamma_tracking_module.cc)

############################################################################################

# Build a dynamic library from our sources
add_library(Falaise_GammaTracking SHARED
  ${FalaiseGammaTrackingPlugin_HEADERS}
  ${FalaiseGammaTrackingPlugin_SOURCES})

target_link_libraries(Falaise_GammaTracking GammaTracking FalaiseModule)

# Apple linker requires dynamic lookup of symbols, so we
# add link flags on this platform
if(APPLE)
  set_target_properties(Falaise_GammaTracking
    PROPERTIES
      LINK_FLAGS "-undefined dynamic_lookup"
      INSTALL_RPATH "@loader_path"
    )
endif()

# Install Targets
install(TARGETS GammaTracking Falaise_GammaTracking DESTINATION ${CMAKE_INSTALL_PLUGINDIR})

# Test support:
option(FalaiseGammaTrackingPlugin_ENABLE_TESTING "Build unit testing system for FalaiseGammaTracking" ON)
if(FalaiseGammaTrackingPlugin_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(GammaTracking/test)
endif()
