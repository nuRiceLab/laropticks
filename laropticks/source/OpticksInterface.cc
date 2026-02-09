
// LArSoft related
#include "laropticks/include/OpticksInterface.h"
#include "laropticks/include/AnalysisManagerHelper.h"
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

// Geant4 Headers
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Material.hh"
#include "g4root.hh"
#include "G4TransportationManager.hh"
#include "G4DynamicParticle.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4TrackStatus.hh"
#include "G4TransportationManager.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4MaterialPropertiesIndex.hh"

#include "G4TouchableHandle.hh"
#include "G4Exception.hh"
#include "G4LogicalVolumeStore.hh"
// Xercesc
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/dom/DOM.hpp"

using namespace xercesc;

namespace phot{

    OpticksInterface::OpticksInterface( fhicl::ParameterSet const &config) : IOpticalPropagation()
																		  ,GDMLPath(config.get<std::string>("GDMLPath"))
 																		  ,OpticksSensorIdentifier(nullptr)
 																		  ,OpticksHits(nullptr)
																		  ,DetectorIds{}
																		  ,World(nullptr)

    {  }
	OpticksInterface::~OpticksInterface()=default;

// Initialize Opticks and Its Libraries
  void OpticksInterface::init(){



  	  // Initialize Opticks Logs for Information and Debugging
	  std::cout << "--- Initiation OpticksInterface ----" << std::endl;
	  mf::LogTrace("OpticksInterface::init") << "Initializing OpticksInterface";
      int argc = 0; char** argv = nullptr;
      OPTICKS_LOG(argc, argv);
      cudaDeviceSynchronize();
  	  SEventConfig::Initialize();

      // Lets Parse the GDML to pass it to Opticks

	  World = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume();


      // Initialize
      initPhotonDetectors();
	  OpticksSensorIdentifier = new MySensorIdentifier(DetectorIds);
      OpticksHits = OpticksHitHandler::getInstance();


      // Set Geometry
      G4CXOpticks::SetSensorIdentifier(OpticksSensorIdentifier);
      G4CXOpticks::SetGeometry(World);

      // Initialize root file (For Testing Purposes)
	  initFileManager();


  }

  // Adjust this function
  void OpticksInterface::CollectPhotons(G4Track *track,sim::SimEnergyDeposit edep){
      // Example of getting material properties
	  G4ThreeVector startPoint=G4ThreeVector(edep.StartX(),edep.StartY(),edep.StartZ());
	  G4ThreeVector endPoint=G4ThreeVector(edep.EndX(),edep.EndY(),edep.EndZ());
  	  auto touch = new G4TouchableHistory();
	  fTouchableHistories.push_back(touch);

      auto nav = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
	  nav->LocateGlobalPointAndUpdateTouchable(G4ThreeVector(edep.MidPointX(),edep.MidPointY(),edep.MidPointZ()), touch);

	  G4Material * mat= G4Material::GetMaterial(touch->GetVolume()->GetLogicalVolume()->GetMaterial()->GetName());
	  auto pTable= mat->GetMaterialPropertiesTable();

	  if(!pTable){
			std::cout << "Material Name " <<  mat->GetName() << std::endl;
			std::cout << "Null Material Properties Table" << std::endl;
		}

	  G4Step * astep = new G4Step();
	  G4StepPoint * preStep = new G4StepPoint();
	  G4StepPoint * postStep = new G4StepPoint();

	  astep->SetPostStepPoint(postStep);
	  astep->SetPreStepPoint(preStep);
	  astep->SetStepLength(edep.StepLength());
	  preStep->SetPosition(startPoint);
	  preStep->SetGlobalTime(edep.StartT());
	  preStep->SetMaterial(mat);
	  preStep->SetVelocity(startPoint.mag()/edep.StartT());

  	  postStep->SetPosition(endPoint);
  	  postStep->SetGlobalTime(edep.EndT());
  	  postStep->SetMaterial(mat);
	  postStep->SetVelocity(endPoint.mag()/edep.EndT());

      mf::LogTrace("OpticksInterface::CollectPhotons") << "Collecting Photons";

      U4::CollectGenstep_DsG4Scintillation_r4695(track, astep, edep.NumFPhotons(), 0, pTable->GetConstProperty(kFASTTIMECONSTANT));
      U4::CollectGenstep_DsG4Scintillation_r4695(track, astep, edep.NumSPhotons(), 1, pTable->GetConstProperty(kSLOWTIMECONSTANT));

      int CollectedPhotons=SEvt::GetNumPhotonCollected(0);
      int maxPhoton=SEventConfig::MaxPhoton();
      // Simulate in batch
      if(CollectedPhotons>=maxPhoton) {
		std::cout << "Simulating in Batch Mode ...." << std::endl;
			Simulate();
	  };

	  //fsteps.push_back(astep);
	  fstepPoints.push_back(preStep);
	  fstepPoints.push_back(postStep);
}




  void OpticksInterface::Simulate(){

      mf::LogTrace("OpticksInterface::Simulate") << "Initiation GPU Simulation";
	  std::cout << "OpticksInterface::Simulate" << std::endl;
      G4CXOpticks * g4xc=G4CXOpticks::Get();
      //Event id needed in here
	  int eventID=0;
	  g4xc->simulate(eventID,0);
      cudaDeviceSynchronize();
      if(SEvt::GetNumHit(0)>0){

          OpticksHits->CollectHits();
      }else std::cout << "OpticksInterface::Simulate: No Hits" << std::endl;
	  //Event id needed here
      g4xc->reset(eventID);

  }

  void OpticksInterface::initPhotonDetectors(){
       mf::LogTrace("OpticksInterface::GetPhotonDetectors") << "Collecting PhotonDetectors From GDML";

	    //Geant4
  		// Surfaces
  		G4OpticalSurface * ArapucaSurface= new G4OpticalSurface("ArapucaSurface",unified,polished,dielectric_metal);
  		// Detector Material (This needs to go to GDML)
  		G4Material * ArapucaWindowMaterial= G4Material::GetMaterial("ArapucaWindowProperties");
  		G4MaterialPropertiesTable * mpt=nullptr;
  		if (ArapucaWindowMaterial)
  		{
  			mpt=ArapucaWindowMaterial->GetMaterialPropertiesTable();
  			ArapucaSurface->SetMaterialPropertiesTable(mpt);
			if(!mpt) std::cout << "ArapucaSurface, No Materials Table" << std::endl;
  		}
  		else
  		{
  			G4cout<<"Error, No Detector Material "<< G4endl;
  			assert(false);
  		}

	 G4int count=0;
	 G4int sid=0;

	// This is a minimal parser to get auxilary values from gdml and assign them to detector logical volumes
  	XercesDOMParser parser;
  	parser.parse((getBuildDir()+GDMLPath).c_str());

  	DOMDocument* doc = parser.getDocument();
  	DOMElement* root = doc->getDocumentElement();

  	// Find all <volume> elements
  	DOMNodeList* volumes = root->getElementsByTagName(XMLString::transcode("volume"));
  	G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  	G4LogicalVolume* lv=nullptr;
  	for (XMLSize_t i = 0; i < volumes->getLength(); ++i) {
  		DOMElement* vol = dynamic_cast<DOMElement*>(volumes->item(i));
  		std::string volName = XMLString::transcode(vol->getAttribute(XMLString::transcode("name")));

  		// Find all <aux> children
  		DOMNodeList* auxList = vol->getElementsByTagName(XMLString::transcode("auxiliary"));

  		for (XMLSize_t j = 0; j < auxList->getLength(); ++j) {
  			DOMElement* aux = dynamic_cast<DOMElement*>(auxList->item(j));

  			std::string type  = XMLString::transcode(aux->getAttribute(XMLString::transcode("auxtype")));
  			std::string value = XMLString::transcode(aux->getAttribute(XMLString::transcode("auxvalue")));

  			// Assign Skin Surfaces
  			if (type=="Surface"){
				lv=lvStore->GetVolume(volName);
				if(lv)
  					new G4LogicalSkinSurface(volName+"_Surface",lv,ArapucaSurface);
				else std::cout << "Cant find logical volume for " << volName << std::endl;
  				count++;
  			}

  			if (type=="PD" and value=="PhotonDetector")
  			{
  				G4cout << "Attaching sensitive detector " << value
						 << " to volume " << volName
						 <<  G4endl << G4endl;
  				DetectorIds.insert(std::pair<G4String,G4int>(volName+"_PV",sid++));

  			}
  		}
  	}


 }


	//-------------------------------------------------------------------------//
	/*!
	* Initalize fast simulation.
	*/
	void OpticksInterface::beginJob()
	{
        mf::LogTrace("OpticksInterface::beginJob") << " Opticks Initialization";
		std::cout << "Begin Job" << std::endl;
		// initialize the parser
  		XMLPlatformUtils::Initialize();
  	}


	//-------------------------------------------------------------------------//
	/*!
	* Simulate photons per art Event \c art::Event .
	*/
	OpticksInterface::UPVecBTR OpticksInterface::executeEvent(VecSED const& edeps)
	{

  		if(!World) init();

		std::cout << "OpticksInterface::executeEvent" << std::endl;

        mf::LogTrace("OpticksInterface::executeEvent") << "Using Opticks tool";
  		// SimPhotonsLite
  		auto opbtr = std::make_unique<std::vector<sim::OpDetBacktrackerRecord>>();

        int num_points = 0;
        int num_fastph = 0;
        int num_slowph = 0;
        int num_fastdp = 0;
        int num_slowdp = 0;

        mf::LogTrace("OpticksInterface::executeEvent")<< "Edep size " << edeps.size() << "\n";

  	    int trackID,nphot;
  		double edeposit;
		// Get The Parent Information
		simb::MCParticle mp=fParticleList->at(0);
	    G4ParticleDefinition* pdef = G4ParticleTable::GetParticleTable()->FindParticle(mp.PdgCode());
		G4DynamicParticle * DParticle= new G4DynamicParticle(pdef,G4ThreeVector(mp.Px(0),mp.Py(0),mp.Pz(0)),mp.E(0));
		G4Track * aTrack =new G4Track(DParticle,mp.T(),G4ThreeVector(mp.Vx(0),mp.Vy(0),mp.Vz(0)));
		aTrack->SetTrackID(mp.TrackId());
		aTrack->SetParentID(mp.Mother());
  		G4TrackStatus status = static_cast<G4TrackStatus>(mp.StatusCode());
		aTrack->SetTrackStatus(status);
		ftracks.push_back(aTrack);
		fDynamicParticles.push_back(DParticle);


        for (auto const& edepi : edeps) {
            if (!(num_points % 1000))
			{
			  std::cout <<" Opticks "
              //mf::LogTrace("OpticksInterface")
                << "SimEnergyDeposit: " << num_points << " " << edepi.TrackID() << " " << edepi.Energy()
                << "\nStart: " << edepi.Start() << "\nEnd: " << edepi.End()
                << "\nNF: " << edepi.NumFPhotons() << "\nNS: " << edepi.NumSPhotons()
                << "\nSYR: " << edepi.ScintYieldRatio()
				<< "PDG: " << edepi.PdgCode()<<"\n"
            	<< std::endl;

            }
			CollectPhotons(aTrack,edepi);
            num_points++;

        	num_fastph +=edepi.NumFPhotons();
        	num_slowph +=edepi.NumSPhotons();



		    mf::LogTrace("OpticksInterface:executeEvent")
		    << "Total points: " << num_points << ", total fast photons: " << num_fastph
		    << ", total slow photons: " << num_slowph << "\ndetected fast photons: " << num_fastdp
		    << ", detected slow photons: " << num_slowdp;
		}
		Simulate();
		return {};
	}

	//-------------------------------------------------------------------------//
	/*
	* Finalize fast simulation.
	*/
	void OpticksInterface::endJob()
	{
  		std::cout << "OpticksInterface::endJob" << std::endl;

		mf::LogTrace("OpticksInterface::endJob") << "Finalizing the job process for Opticks";
  		// Write and Close File
  		auto analysisManager = G4AnalysisManager::Instance();
  		if (analysisManager){
  			std::cout << "Saving Events to " << analysisManager->GetFileName() <<" root file .." << std::endl;
  			analysisManager->Write();
  			analysisManager->CloseFile();
  		}
  	    XMLPlatformUtils::Terminate();

		// Release memory
		ReleaseMemory(fstepPoints,"StepPoints");
		//ReleaseMemory(fsteps,"Steps");  // Causing Seg Faults
		ReleaseMemory(fDynamicParticles,"DynamicParticles");
		ReleaseMemory(ftracks,"Tracks");
		ReleaseMemory(fTouchableHistories,"TouchableHistories");

    }

	void OpticksInterface::InitializeTools( CLHEP::HepRandomEngine& poisson, CLHEP::HepRandomEngine& scint_time) 	{
 		// Opticks doesn't need this class, leave empty.
       }

	std::string OpticksInterface::getBuildDir() {
		const char* env = std::getenv("MRB_BUILDDIR");
		if(!env)
			throw std::runtime_error("MRB_BUILDDIR not set");
  		return std::string(env)+"/";
	}

	void OpticksInterface::initFileManager() {
    	// Get the analysis manager
     	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  		if (analysisManager)
  		  analysisManager->OpenFile("test.root");

  		//Opticks Hits
  		analysisManager->CreateNtuple("OpticksHits","Opticks Hits");
  		analysisManager->CreateNtupleIColumn("evtID");
  		analysisManager->CreateNtupleIColumn("hit_Id");
  		analysisManager->CreateNtupleIColumn("SensorID");
  		analysisManager->CreateNtupleDColumn("x");
  		analysisManager->CreateNtupleDColumn("y");
  		analysisManager->CreateNtupleDColumn("z");
  		analysisManager->CreateNtupleDColumn("t");
  		analysisManager->CreateNtupleDColumn("wavelength");
  		analysisManager->FinishNtuple();
	}
	// Do nothing for PDFastSimPAr.cc
	// This is mainly for opticks
	void OpticksInterface::SetParticleList(std::vector<simb::MCParticle> const* plist)
	{
		fParticleList = plist;

    }


}
