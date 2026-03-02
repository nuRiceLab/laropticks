cet_make_library(LIBRARY_NAME GPUPhotonPropagation INTERFACE
        SOURCE ILArOpticks.h
        LIBRARIES INTERFACE
        lardataobj::Simulation
        nusimdata::SimulationBase
)

cet_make_library(LIBRARY_NAME_VAR module_libs
        SOURCE
        source/OpticksInterface.cc
        source/MySensorIdentifier.cc
        source/OpticksHitHandler.cc
        source/AnalysisManagerHelper.cc
        PUBLIC
        laropticks::GPUPhotonPropagation
        Opticks::G4CX
        Opticks::U4
        OpticalPropagationTools
        messagefacility::MF_MessageLogger
        art::Framework_Principal
        larcore::ServiceUtil
        larcorealg::Geometry
        PRIVATE
        larsim::PhotonPropagation_PhotonVisibilityService_service
        larsim::PhotonPropagation
        larsim::Simulation_LArG4Parameters_service
        lardata::LArPropertiesService
        lardataobj::Simulation
        larcore::ServiceUtil
        nurandom::RandomUtils_NuRandomService_service
        art::Framework_Principal
        art::Framework_Services_Registry
        art_plugin_support::toolMaker
        canvas::canvas
        fhiclcpp::fhiclcpp
        CLHEP::Random
        nusimdata::SimulationBase
)


add_compile_options(-gdwarf-4)

cet_write_plugin_builder(laropticks::GPUPhotonPropagation
        art::EDProducer Modules
        INSTALL_BUILDER
        LIBRARIES CONDITIONAL ${module_libs}
)

include (laropticks::GPUPhotonPropagation)

cet_build_plugin(PDFullSimOpticks laropticks::GPUPhotonPropagation
        LIBRARIES PRIVATE
        ${module_libs}
        cetlib_except::cetlib_except
)

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(fcl)
add_subdirectory(GDML)
add_subdirectory(examples)
