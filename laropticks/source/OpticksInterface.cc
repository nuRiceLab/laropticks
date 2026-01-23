
#include "../include/OpticksInterface.h"


#include "lardataobj/Simulation/OpDetBacktrackerRecord.h"
#include "lardataobj/Simulation/SimEnergyDeposit.h"
// Geometry Related
//#include "larcore/Geometry/AuxDetGeometry.h"
#include "messagefacility/MessageLogger/MessageLogger.h"


// Opticks Headers
#include "G4CXOpticks.hh"
#include "U4SensorIdentifier.h"
#include "SEvt.hh"
#include "U4.hh"

#include "fhiclcpp/ParameterSet.h"


namespace phot{

    OpticksInterface::OpticksInterface(fhicl::ParameterSet const& pset) : IOpticalPropagation()
 																		  ,GDMLPath(pset.get<std::string>("GDMLPath"))
 																		  ,OpticksSensorIdentifier(nullptr)
 																		  ,OpticksHits(nullptr)
{
      mf::LogInfo("OpticksInterface::init") << "Initializing OpticksInterface";
	}


	OpticksInterface::~OpticksInterface()=default;

  void OpticksInterface::init(){

      mf::LogInfo("OpticksInterface::init") << "Initializing OpticksInterface";
      // Initialize
      DetectorIds = GetPhotonDetectors();
	  OpticksSensorIdentifier = new MySensorIdentifier(DetectorIds);
      OpticksHits = OpticksHitHandler::getInstance();


      // Set Geometry
      G4CXOpticks::SetSensorIdentifier(OpticksSensorIdentifier);
      G4CXOpticks::SetGeometryFromGDML();

  }


  // Adjust this function
  void OpticksInterface::CollectPhotons(){

      mf::LogInfo("OpticksInterface::CollectPhotons") << "Collecting Photons";
/*
      U4::CollectGenstep_DsG4Scintillation_r4695(&aTrack, &aStep, Num, scnt, ScintillationTime);


      int CollectedPhotons=SEvt::GetNumPhotonCollected(0);
      int maxPhoton=SEventConfig::MaxPhoton();

      if(CollectedPhotons>=(maxPhoton*0.97)){


      }


*/
}



  //

  void OpticksInterface::GetHitsFromGPU(){
       mf::LogInfo("OpticksInterface::HitCollection") << "Getting HitsFromGPU";

  }


  void OpticksInterface::Simulate(){

      mf::LogInfo("OpticksInterface::Simulate") << "Initiation GPU Simulation";

      G4CXOpticks * g4xc=G4CXOpticks::Get();
      //Event id needed in here
	  int eventID=0;
	  g4xc->simulate(eventID,0);

      cudaDeviceSynchronize();

      if(SEvt::GetNumHit(0)>0){
          OpticksHits->CollectHits();
      }
	  //Event id needed here
      g4xc->reset(eventID);


  }

  std::map<G4String, G4int> OpticksInterface::GetPhotonDetectors(){
    mf::LogInfo("OpticksInterface::GetPhotonDetectors") << "Getting PhotonDetectors From GDML";
/*
    // Get the handle to the Auxiliary Geometry Service
    art::ServiceHandle<geo::AuxDetGeometry> auxDetGeomHandle;
    auto const& auxDetGeom = auxDetGeomHandle->GetProvider();


    // Get Detector Names
    // Loop over all Auxiliary Detectors (e.g., CRT modules)
    for (unsigned int i = 0; i < auxDetGeom.NAuxDets(); ++i) {

      // Get the specific detector object
      auto const& myAuxDet = auxDetGeom.AuxDet(i);

      // Get its center position
      double center[3];
      myAuxDet.GetCenter(center);

      std::cout << "AuxDet " << i << " is at ("
                << center[0] << ", " << center[1] << ", " << center[2]
                << ")" << std::endl;

      // access 'Sensitive' sub-volumes inside the detector
      for (unsigned int j = 0; j < myAuxDet.NSensitiveVolume(); ++j) {
        auto const& sensVol = myAuxDet.SensitiveVolume(j);
        // ... access sensitive volume info
      }
    }*/
	// place holder. will get these from the gdml files
	std::map<G4String, G4int> Ids = {
    	{"det1", 0},
    	{"det2", 1},
    	{"det3", 2},
    	{"det4", 3}
	};
	return Ids;
 }



	//-------------------------------------------------------------------------//
	/*!
	* Initalize fast simulation.
	*/
	void OpticksInterface::beginJob()
	{
		mf::LogError("OpticksInterface") << "Not implemented";
	}

	//-------------------------------------------------------------------------//
	/*!
	* Apply fast simulation to a single \c art::Event .
	*/
	OpticksInterface::UPVecBTR OpticksInterface::executeEvent(VecSED const& edeps)
	{
		mf::LogError("OpticksInterface") << "Not implemented";
		return {};
	}

	//-------------------------------------------------------------------------//
	/*
	* Finalize fast simulation.
	*/
	void OpticksInterface::endJob()
	{
		mf::LogError("OpticksInterface") << "Not implemented";
	}

	void phot::OpticksInterface::InitializeTools( CLHEP::HepRandomEngine& poisson, CLHEP::HepRandomEngine& scint_time) 	{
 		// If Opticks doesn't use RNGs, leave empty. 
       }
}
