
// LArSoft related
#include "laropticks/include/OpticksInterface.h"


using namespace xercesc;

namespace laropticks{
	OpticksInterface * OpticksInterface::instance = nullptr;

// Initialize Opticks and Its Libraries
  void OpticksInterface::init(){

  	  // Initialize Opticks Logs for Information and Debugging
	  std::cout << "--- Initiation OpticksInterface ----" << std::endl;
	  mf::LogTrace("OpticksInterface::init") << "Initializing OpticksInterface";

	  //std::cout << "Number of Detectors " << fGeom.NAuxDets() << std::endl;
      int argc = 0; char** argv = nullptr;
      OPTICKS_LOG(argc, argv);
      cudaDeviceSynchronize();
  	  SEventConfig::Initialize();

      // Lets Parse the GDML to pass it to Opticks
	  World = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume();

	  // Assign Logical and Physical Store Instances
	  lvStore = G4LogicalVolumeStore::GetInstance();
	  phyStore = G4PhysicalVolumeStore::GetInstance();

      // Initialize
      initPhotonDetectors();
	  if(DetectorIds.size()>0) OpticksSensorIdentifier = new MySensorIdentifier(DetectorIds);
	  else throw cet::exception("DetectorIds") << "Missing DetectorIds";

      OpticksHits = OpticksHitHandler::getInstance();
	  PhotonGen= GPUPrimaryPhoton::getInstance();

      // Set Geometry
      G4CXOpticks::SetSensorIdentifier(OpticksSensorIdentifier);
      G4CXOpticks::SetGeometry(World);

      // Initialize root file (For Testing Purposes)
	  initFileManager();

  }

  // Adjust this function
  void OpticksInterface::CollectPhotons(G4Track *track,sim::SimEnergyDeposit edep){
	  //std::cout << "Collecting Photons .." << std::endl;
      // Example of getting material properties
	  if(!track){
		std::cout << "Null Track !! TrkID " << edep.TrackID() << std::endl;
		 return;
	 }

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
			return;
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

      U4::CollectGenstep_DsG4Scintillation_r4695_LArSoft(track, astep, edep.NumFPhotons(), 0, pTable->GetConstProperty(kFASTTIMECONSTANT),edep.TrackID());
      U4::CollectGenstep_DsG4Scintillation_r4695_LArSoft(track, astep, edep.NumSPhotons(), 1, pTable->GetConstProperty(kSLOWTIMECONSTANT),edep.TrackID());

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
      G4CXOpticks * g4xc=G4CXOpticks::Get();
      //Event id needed in here

	  g4xc->simulate(eventID,0);
      cudaDeviceSynchronize();
      if(SEvt::GetNumHit(0)>0){
          OpticksHits->CollectHits(eventID,obtrHelpers);
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




	// This is a minimal parser to get auxilary values from gdml and assign them to detector logical volumes

  	unsigned int nChannels = fGeom->NOpDets();
  	std::string volName,detName,CryoName;
	int sid=0;
  	int count=0;

  	G4VPhysicalVolume * physv1=nullptr;
	if(nChannels>0){
		std::cout << "Collecting Detector Info From LArSoft .....";
		std::cout << "Number of PhotonDetectors " <<  fGeom->NOpDets() <<std::endl;
		// Getting the volume name that photons are comming from before reaching optical detector
		auto cryoID = fGeom->PositionToCryostatID(fGeom->OpDetGeoFromOpDet(0).GetCenter()); // Assuming this name common for all Cryos
		auto const& cryo = fGeom->Cryostat(cryoID);  // Get Cryo object
		auto cryoCenter = cryo.GetCenter();   // Get the Cryo Center
		CryoName=GetVolumeName(fGeom->VolumeName(cryoCenter))+"_PV";        // Obtain the volume name from the center
		physv1=phyStore->GetVolume(CryoName);  // Find the volume associated with the CryonName
		if(!physv1) {
			std::cout << "could nt find the physical volume " << CryoName << std::endl;
			assert(false);
		}
		// Loop through each detector and generate surfaces for sensitive detectors
		// Map Detector IDs to Detector Names so Opticks knows about the sensitive detectors

		for (size_t i =0 ; i < nChannels; ++i) {
			geo::OpDetGeo const& opDet = fGeom->OpDetGeoFromOpDet(i);

			//std::cout << opDet.OpDetInfo() << std::endl;
			volName=GetVolumeName(fGeom->VolumeName(opDet.GetCenter()));// Get Detector Name by its position and remove the cryoID

			detName=volName+"_PV"; // Assign PV to end of the string since G4 likes this
			sid=opDet.ID().OpDet; // Assign the sensor ID
			obtrHelpers.emplace(sid,sid); // Define the channels for backtrackerhelper
			DetectorIds.insert(std::pair<G4String,G4int>(detName,sid)); // map detector ids to detector names

			// Generate Skin or Border Surface
			//createG4SkinSurface(volName,ArapucaSurface);
			createG4BorderSurface(physv1,detName,ArapucaSurface);
		}

	}
    else {
		std::cout << "Detectors are nt initialized by LArSoft! We manually parse them here... " << std::endl;
  		XercesDOMParser parser;
  		parser.parse((GDMLPath).c_str());

  		DOMDocument* doc = parser.getDocument();
  		DOMElement* root = doc->getDocumentElement();

  		// Find all <volume> elements
  		DOMNodeList* volumes = root->getElementsByTagName(XMLString::transcode("volume"));

		physv1=nullptr;

  		for (XMLSize_t i = 0; i < volumes->getLength(); ++i) {
  			DOMElement* vol = dynamic_cast<DOMElement*>(volumes->item(i));
  			volName = XMLString::transcode(vol->getAttribute(XMLString::transcode("name")));

  			// Find all <aux> children
  			DOMNodeList* auxList = vol->getElementsByTagName(XMLString::transcode("auxiliary"));

  			for (XMLSize_t j = 0; j < auxList->getLength(); ++j) {
  				DOMElement* aux = dynamic_cast<DOMElement*>(auxList->item(j));

  				std::string type  = XMLString::transcode(aux->getAttribute(XMLString::transcode("auxtype")));
  				std::string value = XMLString::transcode(aux->getAttribute(XMLString::transcode("auxvalue")));

  				// Assign  Skin or Border Surfaces
  				if (type=="Surface"){
  					// Generate Skin or Border Surface
  					//createG4SkinSurface(volName,surface);
					if(physv1==nullptr) physv1=phyStore->GetVolume(value+"_PV");

  					detName=volName+"_PV";
  					createG4BorderSurface(physv1,detName,ArapucaSurface);
  					count++;
  				}

  				if (type=="PD" and value=="PhotonDetector")
  				{
  					//std::cout << "Attaching sensitive detector " << value << " to volume " << volName+"_PV" << std::endl ;


  					obtrHelpers.emplace(sid,sid);
  					DetectorIds.insert(std::pair<G4String,G4int>(volName+"_PV",sid++));
  				}
  			} // nested loop
  		}	// first loop
	} //else



 }


	//-------------------------------------------------------------------------//
	/*!
	* Initalize fast simulation.
	*/
	void OpticksInterface::beginJob()
	{
        mf::LogTrace("OpticksInterface::beginJob") << " Opticks Initialization";
		std::cout << "Begin Job" << std::endl;
        // Initialize the variables

		// initialize the parser
  		XMLPlatformUtils::Initialize();

 		//Getting the GDMLPATH incase we need it
  		fGeom = lar::providerFrom<geo::Geometry>();
	    GDMLPath = fGeom->GDMLFile();
        OpticksSensorIdentifier=nullptr;
		OpticksHits=nullptr;
		DetectorIds={};
	    Trackmps=nullptr;
  		PhotonGen=nullptr;

  	}
	//-------------------------------------------------------------------------//
	/*!
	* Simulate primary photons per art Event \c art::Event .
	*/
	OpticksInterface::UPVecBTR OpticksInterface::executeEvent(){
		std::cout << "OpticksInterface::executeEvent for primary photon production" << std::endl;
  		std::cout << "Number of Primary Photons " << fParticleList->size() << std::endl;

        auto records=std::make_unique<std::vector<sim::OpDetBacktrackerRecord>>();
        double vx,vy,vz,px,py,pz,mx,my,mz, wavelength;

        PhotonGen->setEventID(eventID);
        PhotonGen->setObtrHelpers(obtrHelpers);
        PhotonGen->CollectPhotons(fParticleList,0);

        if(obtrHelpers.size()>0){
		for (auto& opbtr: obtrHelpers)
			records->emplace_back(opbtr.second);
		} else std::cout << "obtrHelper seems empty ...." << std::endl;

		return records;
	}
	//-------------------------------------------------------------------------//
	/*!
	* Simulate scintilation photons per art Event \c art::Event .
	*/
    OpticksInterface::UPVecBTR OpticksInterface::executeEvent(VecSED const& edeps)
    {
		if(Trackmps==nullptr) initTracks();
		std::cout << "OpticksInterface::executeEvent" << std::endl;

        mf::LogTrace("OpticksInterface::executeEvent") << "Using Opticks tool";
		// Optical Back Tracker


		auto records=std::make_unique<std::vector<sim::OpDetBacktrackerRecord>>();

        int num_points = 0;
        int num_fastph = 0;
        int num_slowph = 0;
        int num_fastdp = 0;
        int num_slowdp = 0;


        mf::LogTrace("OpticksInterface::executeEvent")<< "Edep size " << edeps.size() << "\n";

  	    int nphot;
  		double edeposit;

		// Get The Parent Information
		std::cout << " Size of Energy Depositions " <<  edeps.size() << std::endl;
		G4Track * aTrack=nullptr;

		int tempTrackID=-1;


        for (auto const& edepi : edeps) {
            if (!(num_points % 10000))
			{

			  std::cout <<" Opticks "
                << "SimEnergyDeposit: " << num_points << " " << edepi.TrackID() << " " << edepi.Energy()
                << "\nStart: " << edepi.Start() << "\nEnd: " << edepi.End()
                << "\nNF: " << edepi.NumFPhotons() << "\nNS: " << edepi.NumSPhotons()
                << "\nSYR: " << edepi.ScintYieldRatio()
				<< "PDG: " << edepi.PdgCode()<<"\n"
            	<< std::endl;


            }
			if (tempTrackID != std::abs(edepi.TrackID())){ // Adding abs function to combine daughter tracks to parent since daughter tracks have negative ids
				tempTrackID = std::abs(edepi.TrackID());
				auto it = Trackmps->find(tempTrackID );
				if (it != Trackmps->end()){
					aTrack = it->second;
					//std::cout << "TempTrack_ID " << tempTrackID << " Track_ID " <<edepi.TrackID() << std::endl;
				}
				else {
					std::cout<<"No Track Found for TrkID " << tempTrackID <<std::endl;
					continue;
				}
			}
			CollectPhotons(aTrack,edepi);

            num_points++;

			nphot=edepi.NumPhotons();

        	edeposit = edeposit + edepi.Energy();
			nphot+=edepi.NumSPhotons();
        	num_fastph +=edepi.NumFPhotons();
        	num_slowph +=edepi.NumSPhotons();

		    mf::LogTrace("OpticksInterface:executeEvent")
		    << "Total points: " << num_points << ", total fast photons: " << num_fastph
		    << ", total slow photons: " << num_slowph << "\ndetected fast photons: " << num_fastdp
		    << ", detected slow photons: " << num_slowdp;
		}

		Simulate();
		if(obtrHelpers.size()>0){
			for (auto& opbtr: obtrHelpers)
				records->emplace_back(opbtr.second);

		} else std::cout << "obtrHelper seems empty ...." << std::endl;


		Trackmps->clear();
		delete Trackmps;
		Trackmps=nullptr;

		return records ;
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


	void OpticksInterface::initFileManager() {
    	// Get the analysis manager
     	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  		if (analysisManager)
  		  analysisManager->OpenFile("test.root");

  		//Opticks Hits
  		analysisManager->CreateNtuple("OpticksHits","Opticks Hits");
  		analysisManager->CreateNtupleIColumn("evtID");
  		analysisManager->CreateNtupleIColumn("parent_Id");
  		analysisManager->CreateNtupleIColumn("hit_Id");
  		analysisManager->CreateNtupleIColumn("SensorID");
  		analysisManager->CreateNtupleDColumn("x");
  		analysisManager->CreateNtupleDColumn("y");
  		analysisManager->CreateNtupleDColumn("z");
  		analysisManager->CreateNtupleDColumn("t");
  		analysisManager->CreateNtupleDColumn("wavelength");
  		analysisManager->FinishNtuple();
	}


	void OpticksInterface::initTracks(){
		std::cout << "OpticksInterface::initTracks" << std::endl;
		std::cout << "Setting up Tracks" << std::endl;
		std::cout << "Amount of Particles " << fParticleList->size() << std::endl;

		Trackmps = new std::map<int, G4Track*>();
		for (size_t ip=0; ip<fParticleList->size(); ip++){
			auto mp = fParticleList->at(ip);
		    G4ParticleDefinition* pdef = G4ParticleTable::GetParticleTable()->FindParticle(mp.PdgCode());

			G4DynamicParticle * DParticle= new G4DynamicParticle(pdef,G4ThreeVector(mp.Px(0),mp.Py(0),mp.Pz(0)),mp.E(0));
			auto trk =new G4Track(DParticle,mp.T(),G4ThreeVector(mp.Vx(0),mp.Vy(0),mp.Vz(0)));
			trackID=mp.TrackId();
			trk->SetTrackID(trackID);
			trk->SetParentID(mp.Mother());
  			G4TrackStatus status = static_cast<G4TrackStatus>(mp.StatusCode());
			trk->SetTrackStatus(status);
			ftracks.push_back(trk);
			fDynamicParticles.push_back(DParticle);
			Trackmps->insert( std::make_pair(mp.TrackId(), trk) );
			//std::cout << "TrackID " << trackID << " Pdgcode " << mp.PdgCode() << " parentID " << mp.Mother()<< std::endl;
		}
	}
	void OpticksInterface::createG4SkinSurface(std::string VolName, G4OpticalSurface* surface){
		mf::LogTrace("OpticksInterface::createG4SkinSurface")  << "Creating Skin Surface ..."<< std::endl;
  		// find the logical volume associated with the volume name
		auto lv=lvStore->GetVolume(VolName);

  		if(lv)  new G4LogicalSkinSurface(VolName+"_SkinSurface",lv,surface);
  		else std::cout << "Cant find logical volume for " << VolName << std::endl;
	}

	void OpticksInterface::createG4BorderSurface(G4VPhysicalVolume *phyv1, std::string v2, G4OpticalSurface* surface){
  		mf::LogTrace("OpticksInterface::createBorderSurface")  << "Creating Border Surface ..." << std::endl;
		if(phyv1==nullptr){
			std::cout << "Physical Volume pointer is empty!!" << std::endl;
			assert(false);
		}
  		auto phyv2=phyStore->GetVolume(v2);

  		if(phyv2) new G4LogicalBorderSurface(v2+"_BorderSurface", phyv1, phyv2, surface);
  		else std::cout << "Cant find physical volume for " << v2 << std::endl;
    }

	std::string OpticksInterface::GetVolumeName(const std::string& s){
		// Find the last '_' and get the any string before it.
		// Intension behind this to remove  the Cryostat ID from the names such as  VolumeName_CryoID
  		mf::LogTrace("OpticksInterface::GetVolumeName")  << "Removing cryostat id from detector names..."<< std::endl;

  		size_t pos = s.rfind('_');
  		return (pos == std::string::npos) ? s : s.substr(0, pos);
	}

	// Setting Some Global Parameters
	void OpticksInterface::setParticleList(std::vector<simb::MCParticle> const * plist) {
		fParticleList = plist;
	}
	void OpticksInterface::setEventID(int evt) {
		eventID = evt;
	}
	void OpticksInterface::setWorld(G4VPhysicalVolume * fWorld) {
		World = fWorld;
	}

}
