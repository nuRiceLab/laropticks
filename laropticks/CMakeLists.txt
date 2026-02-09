cet_write_plugin_builder(lar::OpticalPropagationTools::Opticks art::tool Modules INSTALL_BUILDER)


include (lar::OpticalPropagationTools::Opticks)
add_compile_options(-gdwarf-4)
cet_build_plugin(Opticks
        lar::OpticalPropagationTools::Opticks
        IMPL_SOURCE
        source/OpticksInterface.cc
        source/MySensorIdentifier.cc
        source/OpticksHitHandler.cc
        source/AnalysisManagerHelper.cc
        LIBRARIES
        PUBLIC
        Opticks::G4CX
        Opticks::U4
        OpticalPropagationTools
        PRIVATE
        larsim::PhotonPropagation
        larsim::IonizationScintillation
        larcore::ServiceUtil
        art_plugin_support::toolMaker
        fhiclcpp::fhiclcpp
        messagefacility::MF_MessageLogger
        lardataobj::Simulation
        nusimdata::SimulationBase

)

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(fcl)
add_subdirectory(GDML)
add_subdirectory(examples)
