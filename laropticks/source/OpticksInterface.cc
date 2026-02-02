

#include "laropticks/include/OpticksInterface.h"


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
#include "SEventConfig.hh"
#include "OPTICKS_LOG.hh"
#include "fhiclcpp/ParameterSet.h"


namespace phot{

    OpticksInterface::OpticksInterface( fhicl::ParameterSet const &config) : IOpticalPropagation()
 																		  ,OpticksSensorIdentifier(nullptr)
 																		  ,OpticksHits(nullptr)
    {}

	OpticksInterface::~OpticksInterface()=default;


// Initialize Opticks and Its Libraries
  void OpticksInterface::init(){
  	  // Initialize Opticks Logs for Information and Debugging
	  std::cout << "--- Initiation OpticksInterface ----" << std::endl;
	  mf::LogInfo("OpticksInterface::init") << "Initializing OpticksInterface";
      int argc = 0; char** argv = nullptr;
      OPTICKS_LOG(argc, argv);
      cudaDeviceSynchronize();
  	  SEventConfig::Initialize();

/*
      // Initialize
      DetectorIds = GetPhotonDetectors();
	  OpticksSensorIdentifier = new MySensorIdentifier(DetectorIds);
      OpticksHits = OpticksHitHandler::getInstance();

      // Set Geometry
      G4CXOpticks::SetSensorIdentifier(OpticksSensorIdentifier);
      G4CXOpticks::SetGeometryFromGDML();
*/
  }


  // Adjust this function
  void OpticksInterface::CollectPhotons(){

      mf::LogInfo("OpticksInterface::CollectPhotons") << "Collecting Photons";
/*
      U4::CollectGenstep_DsG4Scintillation_r4695(&aTrack, &aStep, Num, scnt, ScintillationTime);

      int CollectedPhotons=SEvt::GetNumPhotonCollected(0);
      int maxPhoton=SEventConfig::MaxPhoton();

      if(CollectedPhotons>=(maxPhoton)){


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
        std::cout <<"----- Hello Opticks Here Begin Job ------ " << std::endl;
        //mf::LogInfo("OpticksInterface") << "Not implemented";
  		init();
	}

	//-------------------------------------------------------------------------//
	/*!
	* Apply fast simulation to a single \c art::Event .
	*/
	OpticksInterface::UPVecBTR OpticksInterface::executeEvent(VecSED const& edeps)
	{
        mf::LogTrace("OpticksInterface::executeEvent") << "Using Opticks tool";
  		// SimPhotonsLite
  		auto opbtr = std::make_unique<std::vector<sim::OpDetBacktrackerRecord>>();


        int num_points = 0;
        int num_fastph = 0;
        int num_slowph = 0;
        int num_fastdp = 0;
        int num_slowdp = 0;

        mf::LogTrace("OpticksInterface")<< "Passing SimPhotons from " << edeps.size() << " to Opticks\n";

  	    int trackID,nphot;
  		double edeposit;


        for (auto const& edepi : edeps) {

            if (!(num_points % 1000)) {
              mf::LogTrace("OpticalPropPDFastSimPAR")
                << "SimEnergyDeposit: " << num_points << " " << edepi.TrackID() << " " << edepi.Energy()
                << "\nStart: " << edepi.Start() << "\nEnd: " << edepi.End()
                << "\nNF: " << edepi.NumFPhotons() << "\nNS: " << edepi.NumSPhotons()
                << "\nSYR: " << edepi.ScintYieldRatio() << "\n";
            }
            num_points++;

        	num_fastph +=edepi.NumFPhotons();
        	num_slowph +=edepi.NumSPhotons();

        	//trackID = edepi.TrackID();
        	///nphot = edepi.NumPhotons();
        	//edeposit = edepi.Energy() / nphot;
        	//double pos[3] = {edepi.MidPointX(), edepi.MidPointY(), edepi.MidPointZ()};
		    //geo::Point_t const ScintPoint = {pos[0], pos[1], pos[2]};

		    mf::LogTrace("OpticalPropPDFastSimPAR")
		    << "Total points: " << num_points << ", total fast photons: " << num_fastph
		    << ", total slow photons: " << num_slowph << "\ndetected fast photons: " << num_fastdp
		    << ", detected slow photons: " << num_slowdp;
		}
		return {};
	}

	//-------------------------------------------------------------------------//
	/*
	* Finalize fast simulation.
	*/
	void OpticksInterface::endJob()
	{
        std::cout <<"----- Hello Opticks Here End Job ------ " << std::endl;
		mf::LogInfo("OpticksInterface") << "Not implemented (Hello From Opticks)";
	}

	void phot::OpticksInterface::InitializeTools( CLHEP::HepRandomEngine& poisson, CLHEP::HepRandomEngine& scint_time) 	{
 		// If Opticks doesn't use RNGs, leave empty. 
       }
}
