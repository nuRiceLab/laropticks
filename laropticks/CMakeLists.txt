add_compile_options(-gdwarf-4)
cet_build_plugin(PDFullSimOpticks art::EDProducer
        SOURCE
        PDFullSimOpticks_module.cc
        source/OpticksInterface.cc
        source/MySensorIdentifier.cc
        source/OpticksHitHandler.cc
        source/GPUPrimaryPhoton.cc
        source/AnalysisManagerHelper.cc
        LIBRARIES PUBLIC
        Opticks::G4CX
        Opticks::U4
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
        cetlib_except::cetlib_except
)

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(fcl)
add_subdirectory(GDML)
