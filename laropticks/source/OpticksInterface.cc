
// LArSoft related
#include "G4Version.hh"
#include "laropticks/include/OpticksInterface.h"
#include "laropticks/include/AnalysisManagerHelper.h"
#include "laropticks/include/types.h"

using namespace xercesc;

namespace laropticks{
	OpticksInterface * OpticksInterface::instance = nullptr;

// Initialize Opticks and Its Libraries
  void OpticksInterface::init(){
	  if(World) return; // return if world exist since no need to re-initilize
  	  // Initialize Opticks Logs for Information and Debugging
  	  mf::LogInfo ("OpticksInterface") << "--- Initiation OpticksInterface ----" << std::endl;
	  mf::LogTrace("OpticksInterface::init") << "Initializing OpticksInterface";

	  // mf::LogInfo ("OpticksInterface) << "Number of Detectors " << fGeom.NAuxDets() ;
      int argc = 0; char** argv = nullptr;
      OPTICKS_LOG(argc, argv);
      cudaDeviceSynchronize();
  	  SEventConfig::Initialize();

      // Lets Parse the GDML to pass it to Opticks
	  World = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume();

	  // Assign Logical and Physical Store Instances
	  lvStore = G4LogicalVolumeStore::GetInstance();
	  phyStore = G4PhysicalVolumeStore::GetInstance();
      // Get instances of tools that we need
      OpticksHits = OpticksHitHandler::getInstance();
	  PhotonGen= GPUPrimaryPhoton::getInstance();

      // Initialize
      initPhotonDetectors();
	  if(DetectorIds.size()>0) {
        OpticksSensorIdentifier = new MySensorIdentifier(DetectorIds);

        if(IsSavePhotons() && GetSimTag()=="LightSource") OpticksHits->initSensorCounts(DetectorIds);
      }
	  else throw cet::exception("DetectorIds") << "Missing DetectorIds";



      // Set Geometry
      G4CXOpticks::SetSensorIdentifier(OpticksSensorIdentifier);
      G4CXOpticks::SetGeometry(World);

  }

  // Adjust this function
  void OpticksInterface::CollectPhotons(G4Track *track,sim::SimEnergyDeposit edep){
	  // mf::LogInfo ("OpticksInterface) << "Collecting Photons .." << std::endl;
      // Example of getting material properties
	  if(!track){
		 mf::LogInfo ("OpticksInterface") << "Null Track !! TrkID " << edep.TrackID()<< std::endl ;
		 return;
	 }

	  G4ThreeVector startPoint=G4ThreeVector(edep.StartX()*cm,edep.StartY()*cm,edep.StartZ()*cm);
	  G4ThreeVector endPoint=G4ThreeVector(edep.EndX()*cm,edep.EndY()*cm,edep.EndZ()*cm);

  	  auto touch = new G4TouchableHistory();
	  fTouchableHistories.push_back(touch);

      auto nav = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
	  nav->LocateGlobalPointAndUpdateTouchable(G4ThreeVector(edep.MidPointX()*cm,edep.MidPointY()*cm,edep.MidPointZ()*cm), touch);

	  G4Material * mat= G4Material::GetMaterial(touch->GetVolume()->GetLogicalVolume()->GetMaterial()->GetName());

	  auto pTable= mat->GetMaterialPropertiesTable();

	  if(!pTable){
			 mf::LogInfo ("OpticksInterface") << "Material Name " <<  mat->GetName() <<std::endl;
			 mf::LogInfo ("OpticksInterface") << "Null Material Properties Table" << std::endl;
			return;
		}

	  G4Step * astep = new G4Step();
	  G4StepPoint * preStep = new G4StepPoint();
	  G4StepPoint * postStep = new G4StepPoint();

	  astep->SetPostStepPoint(postStep);
	  astep->SetPreStepPoint(preStep);
	  astep->SetStepLength(edep.StepLength()*cm);
	  preStep->SetPosition(startPoint);
	  preStep->SetGlobalTime(edep.StartT()*ns);
	  preStep->SetMaterial(mat);
	  preStep->SetVelocity(startPoint.mag()/(edep.StartT()*ns));

  	  postStep->SetPosition(endPoint);
  	  postStep->SetGlobalTime(edep.EndT()*ns);
  	  postStep->SetMaterial(mat);
	  postStep->SetVelocity(endPoint.mag()/(edep.EndT()*ns));

      mf::LogTrace("OpticksInterface::CollectPhotons") << "Collecting Photons";
	  // Version 10.6.1
  	#if G4VERSION_NUMBER < 1100

      U4::CollectGenstep_DsG4Scintillation_r4695_LArSoft(track, astep, edep.NumFPhotons(), 0, pTable->GetConstProperty(kFASTTIMECONSTANT),edep.TrackID());
      U4::CollectGenstep_DsG4Scintillation_r4695_LArSoft(track, astep, edep.NumSPhotons(), 1, pTable->GetConstProperty(kSLOWTIMECONSTANT),edep.TrackID());
  	  // Version 11.2
    #else
  	  U4::CollectGenstep_DsG4Scintillation_r4695_LArSoft(track, astep, edep.NumFPhotons(), 0, pTable->GetConstProperty(kSCINTILLATIONTIMECONSTANT1),edep.TrackID());
      U4::CollectGenstep_DsG4Scintillation_r4695_LArSoft(track, astep, edep.NumSPhotons(), 1, pTable->GetConstProperty(kSCINTILLATIONTIMECONSTANT2),edep.TrackID());
    #endif

  	  int CollectedPhotons=SEvt::GetNumPhotonCollected(0);
      int maxPhoton=SEventConfig::MaxPhoton();

      // Simulate in batch
      if(CollectedPhotons>=maxPhoton) {
		 mf::LogInfo ("OpticksInterface") << "Simulating in Batch Mode ...." << std::endl;
			Simulate();
	  };

	  //fsteps.push_back(astep);
	  fstepPoints.push_back(preStep);
	  fstepPoints.push_back(postStep);
}


  void OpticksInterface::Simulate(){

      mf::LogTrace("OpticksInterface::Simulate") << "Initiation GPU Simulation for Event Number " << eventID;
      G4CXOpticks * g4xc=G4CXOpticks::Get();
      //Event id needed in here

	  g4xc->simulate(eventID,0);
      cudaDeviceSynchronize();
  	  int hit_count = SEvt::GetNumHit(0);
      if(hit_count>0){
          OpticksHits->CollectHits(eventID,obtrHelpers);
      	  mf::LogInfo ("OpticksInterface") << "OpticksInterface::Simulate: "<< hit_count << " Hits" << std::endl;
      }else  mf::LogInfo ("OpticksInterface") << "OpticksInterface::Simulate: No Hits" << std::endl;
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
			if(!mpt)  mf::LogInfo ("OpticksInterface") << "ArapucaSurface, No Materials Table" << std::endl;
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
		 mf::LogInfo ("OpticksInterface") << "Collecting Detector Info From LArSoft ....."<< std::endl;
		 mf::LogInfo ("OpticksInterface") << "Number of PhotonDetectors " <<  fGeom->NOpDets() <<std::endl;
		// Getting the volume name that photons are comming from before reaching optical detector
		auto cryoID = fGeom->PositionToCryostatID(fGeom->OpDetGeoFromOpDet(0).GetCenter()); // Assuming this name common for all Cryos
		auto const& cryo = fGeom->Cryostat(cryoID);  // Get Cryo object
		auto cryoCenter = cryo.GetCenter();   // Get the Cryo Center
		CryoName=GetVolumeName(fGeom->VolumeName(cryoCenter))+"_PV";        // Obtain the volume name from the center
		physv1=phyStore->GetVolume(CryoName);  // Find the volume associated with the CryonName
		if(!physv1) {
			 mf::LogInfo ("OpticksInterface") << "could nt find the physical volume " << CryoName << std::endl;
			assert(false);
		}
		// Loop through each detector and generate surfaces for sensitive detectors
		// Map Detector IDs to Detector Names so Opticks knows about the sensitive detectors

		for (size_t i =0 ; i < nChannels; ++i) {
			geo::OpDetGeo const& opDet = fGeom->OpDetGeoFromOpDet(i);

			// mf::LogInfo ("OpticksInterface) << opDet.OpDetInfo() ;
			volName=GetVolumeName(fGeom->VolumeName(opDet.GetCenter()));// Get Detector Name by its position and remove the cryoID

			detName=volName+"_PV"; // Assign PV to end of the string since G4 likes this
			sid=opDet.ID().OpDet; // Assign the sensor ID
			obtrHelpers.emplace(sid,sid); // Define the channels for backtrackerhelper
			DetectorIds.insert(std::pair<G4String,G4int>(detName,sid)); // map detector ids to detector names

			// Generate Skin or Border Surface
			createG4SkinSurface(volName,ArapucaSurface);
			// mf::LogInfo ("OpticksInterface) << "CryoName " << physv1->GetName() << " DetName " << detName ;
			//createG4BorderSurface(physv1,detName,ArapucaSurface);
		}

	}
    else {
		 mf::LogInfo ("OpticksInterface") << "Detectors are nt initialized by LArSoft! We manually parse them here... " << std::endl;
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
  					createG4SkinSurface(volName,ArapucaSurface);
					if(physv1==nullptr) physv1=phyStore->GetVolume(value+"_PV");

  					detName=volName+"_PV";
  					//createG4BorderSurface(physv1,detName,ArapucaSurface);
  					count++;
  				}

  				if ((type=="PD" || type=="SensDet") and value=="PhotonDetector")
  				{
  					// mf::LogInfo ("OpticksInterface) << "Attaching sensitive detector " << value << " to volume " << volName+"_PV" << std::endl ;
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
		 //mf::LogInfo ("OpticksInterface") << "Begin Job" << std::endl;
        // Initialize the variables

		// initialize the parser
  		XMLPlatformUtils::Initialize();

 		//Getting the GDMLPATH incase we need it
  		fGeom = lar::providerFrom<geo::Geometry>();
	    GDMLPath = fGeom->GDMLFile();
        // Initialize root file (For Testing Purposes)
	    initFileManager();


	    // Note: ftracks and fDynamicParticles are reserved in initTracks() with exact size
		pt=new PerformanceTime();
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
	OpticksInterface::UPVecBTR OpticksInterface::executeEvent(int VoxelID){
		 mf::LogInfo ("OpticksInterface") << "OpticksInterface::executeEvent for primary photon production" << std::endl;
  		 mf::LogInfo ("OpticksInterface") << "Number of Primary Photons " << fParticleList->size() << std::endl;

        auto records=std::make_unique<std::vector<sim::OpDetBacktrackerRecord>>();
        //double vx,vy,vz,px,py,pz,mx,my,mz, wavelength;
		PhotonGen->setVoxelID(VoxelID);
        PhotonGen->setEventID(eventID);
        PhotonGen->setObtrHelpers(obtrHelpers);
        PhotonGen->CollectPhotonInfo(fParticleList,fph_save);
		PhotonGen->Batcher();
        if(obtrHelpers.size()>0){
		for (auto& opbtr: obtrHelpers)
			records->emplace_back(opbtr.second);
        	//std::cout < opbtr.second.OpDetNum() << " " << opbtr.second.timePDclockSDPsMap().size() << std::endl;
		} else  mf::LogInfo ("OpticksInterface") << "obtrHelper seems empty ...." << std::endl;
		PhotonGen->reset();
  		pt->PhotonAmount=fParticleList->size();
		return records;
	}



	//-------------------------------------------------------------------------//
	/*!
	* Simulate scintilation photons per art Event \c art::Event .
	*/
    OpticksInterface::UPVecBTR OpticksInterface::executeEvent(VecSED const& edeps)
    {
		if(Trackmps==nullptr) initTracks();
		//mf::LogInfo ("OpticksInterface") << "OpticksInterface::executeEvent" << std::endl;

        //mf::LogTrace("OpticksInterface::executeEvent") << "Using Opticks tool";
		// Optical Back Tracker


		auto records=std::make_unique<std::vector<sim::OpDetBacktrackerRecord>>();

        int num_points = 0;
        int num_fastph = 0;
        int num_slowph = 0;
        int num_fastdp = 0;
        int num_slowdp = 0;



        mf::LogTrace("OpticksInterface::executeEvent")<< "Edep size " << edeps.size() << "\n";

  	    int nphot=0;
  		double edeposit;

		// Get The Parent Information
		 mf::LogInfo ("OpticksInterface") << " Size of Energy Depositions " <<  edeps.size() ;
		G4Track * aTrack=nullptr;

		int tempTrackID=-99999;
		int edepTrackID=0;
		  // Pre-allocate vector capacity to avoid repeated allocations during event processing
	    // Prevents O(n log n) behavior with dynamic vector resizing
	    fTouchableHistories.reserve(edeps.size());
	    fstepPoints.reserve(2*edeps.size());  // 2x since we store preStep + postStep
		double photonE=0;
        for (auto const& edepi : edeps) {
        	num_points++;
        	nphot = nphot + edepi.NumPhotons();

            if (!(num_points % 100000))
			{
			   mf::LogInfo ("OpticksInterface") <<" Opticks "
                << "SimEnergyDeposit: " << num_points << " " << edepi.TrackID() << " " << edepi.Energy()
                << "\nStart: " << edepi.Start() << "\nEnd: " << edepi.End()
                << "\nNF: " << edepi.NumFPhotons() << "\nNS: " << edepi.NumSPhotons()
                << "\nSYR: " << edepi.ScintYieldRatio()
				<< "PDG: " << edepi.PdgCode()<<"\n" ;
            }
        	edepTrackID=edepi.TrackID();
        	//if (edepTrackID<0) edepTrackID=edepi.TrackID()+fParticleList->size();

			if (tempTrackID != edepTrackID){
				tempTrackID = edepTrackID;
				auto it = Trackmps->find(std::abs(tempTrackID) );
				if (it != Trackmps->end()){
					aTrack = it->second;
					 //mf::LogInfo ("OpticksInterface") << "TempTrack_ID " << tempTrackID << " pdg " <<edepi.PdgCode();
				}
				else {
					 mf::LogInfo ("OpticksInterface::executeEvent")<<" No Track Found for TrkID " << tempTrackID << " PDG "  << edepi.PdgCode()<<std::endl;
					continue;
				}
			}
        	if (IsSavePhotons())
        	{
        		G4LorentzVector ffpos = {edepi.StartX(),edepi.StartY(),edepi.StartZ()};
				analysisManager->FillEdepTree (eventID, ffpos, edepi.TrackID(), edepi.PdgCode(), edepi.NumPhotons(),  edepi.NumElectrons());
        	}
			CollectPhotons(aTrack,edepi);

        	//edeposit = edeposit + edepi.Energy();
        	//num_fastph +=edepi.NumFPhotons();
        	//num_slowph +=edepi.NumSPhotons();

		    /*mf::LogTrace("OpticksInterface:executeEvent")
		    << "Total points: " << num_points << ", total fast photons: " << num_fastph
		    << ", total slow photons: " << num_slowph << "\ndetected fast photons: " << num_fastdp
		    << ", detected slow photons: " << num_slowdp;*/
		}

		Simulate();
		if(obtrHelpers.size()>0){

			for (auto& opbtr: obtrHelpers)
			{
				records->emplace_back(opbtr.second);
				opbtr.second = sim::OBTRHelper(opbtr.first);	// Reinitilaze the obtrs
			}


		} else  mf::LogInfo ("OpticksInterface") << "obtrHelper seems empty ...." << std::endl;


		Trackmps->clear();
		delete Trackmps;
		Trackmps=nullptr;

  		// Release memory
  		ReleaseMemory(fstepPoints,"StepPoints");
  		ReleaseMemory(ftracks,"Tracks");
  		ReleaseMemory(fTouchableHistories,"TouchableHistories");

  		mf::LogInfo ("OpticksInterface::executeEvent")<<" Total Photons " << nphot  <<std::endl;
		/*std::cout << " Printing the backtracker records for primary photons " << std::endl;
  	    for (auto &btr : *records.get()) {
    	  for (auto const& [tick, sdps] : btr.timePDclockSDPsMap()) {
        	    std::cout << "opDet=" << btr.OpDetNum()
                  << " tick=" << tick          // should be ~O(1000) ns, not 0
                  << " nSDPs=" << sdps.size() << "\n";
    	  }
	   }*/
  	   pt->PhotonAmount=nphot;
  		return records ;
	}

	//-------------------------------------------------------------------------//
	/*
	* Finalize fast simulation.
	*/
	void OpticksInterface::endJob()
	{
  		 mf::LogInfo ("OpticksInterface") << "OpticksInterface::endJob" << std::endl;

		mf::LogTrace("OpticksInterface::endJob") << "Finalizing the job process for Opticks";



  	    XMLPlatformUtils::Terminate();

		// Release memory
		ReleaseMemory(fstepPoints,"StepPoints");
		//ReleaseMemory(fsteps,"Steps");  // Causing Seg Faults
		ReleaseMemory(fDynamicParticles,"DynamicParticles");
		ReleaseMemory(ftracks,"Tracks");
		ReleaseMemory(fTouchableHistories,"TouchableHistories");

		// Clean up OpticksSensorIdentifier to prevent singleton leak
		if(OpticksSensorIdentifier != nullptr) {
			delete OpticksSensorIdentifier;
			OpticksSensorIdentifier = nullptr;
			mf::LogInfo("OpticksInterface") << "Cleaned up OpticksSensorIdentifier" << std::endl;
		}
  	  // Clean up singleton instances to prevent memory leaks
      GPUPrimaryPhoton::deleteInstance();
	  OpticksHitHandler::deleteInstance();
  	  delete pt;

    }


	void OpticksInterface::initFileManager() {
    	// Get the analysis manager

     	analysisManager = AnalysisManagerHelper::getInstance();
	    analysisManager->setFileService(fTFileService);


	    //Opticks Hits
  		if (IsSavePhotons())
  		{
  			analysisManager->initOpticksHitTree();
  		}

  	    // Performance Tree
		analysisManager->initPerformanceTimeTree();


		//Initial Particle Info
		if(IsSavePhotons() && GetSimTag()=="LightSource"){
            // Initialize Photon Info
            analysisManager->initPhotonGenTree();
            // Estimating Visibilities for comparison with LightSource Module
            analysisManager->initVoxelTree();
		}
  		else if (IsSavePhotons() && GetSimTag()=="IonAndScint" )
  		{
  			analysisManager->initIonAndScintGenTree(); // Save edep info
  		}

}
	void OpticksInterface::initTracks(){
		 //mf::LogInfo ("OpticksInterface") << "OpticksInterface::initTracks" << std::endl;
		 //mf::LogInfo ("OpticksInterface") << "Setting up Tracks" << std::endl;
		 //mf::LogInfo ("OpticksInterface") << "Amount of Particles " << fParticleList->size()<< std::endl; ;

		Trackmps = new std::map<int, G4Track*>();
		// Pre-allocate vectors to avoid repeated allocations
		ftracks.reserve(ftracks.size()+fParticleList->size());
		fDynamicParticles.reserve(fDynamicParticles.size()+fParticleList->size());
		int MotherID=0;
		for (size_t ip=0; ip<fParticleList->size(); ip++){
			auto mp = fParticleList->at(ip);
		    G4ParticleDefinition* pdef = G4ParticleTable::GetParticleTable()->FindParticle(mp.PdgCode());

			G4DynamicParticle * DParticle= new G4DynamicParticle(pdef,G4ThreeVector(mp.Px(0)*GeV,mp.Py(0)*GeV,mp.Pz(0)*GeV),mp.E(0)*GeV);
			auto trk =new G4Track(DParticle,mp.T()*ns,G4ThreeVector(mp.Vx(0)*cm,mp.Vy(0)*cm,mp.Vz(0)*cm));
			trackID=mp.TrackId();
			//if (trackID<0) trackID=mp.TrackId()+fParticleList->size(); // G4 accepts positive tracks, so shift them
			trk->SetTrackID(trackID);
			MotherID=mp.Mother();
			//if (MotherID<0) MotherID=mp.Mother()+fParticleList->size(); // G4 accepts positive tracks, so shift them
			trk->SetParentID(MotherID);
  			G4TrackStatus status = static_cast<G4TrackStatus>(mp.StatusCode());
			trk->SetTrackStatus(status);
			ftracks.push_back(trk);
			fDynamicParticles.push_back(DParticle);
			Trackmps->insert( std::make_pair(trackID, trk) );
			//std::cout << "OpticksInterface " << "trackID " << trackID << " pdg "<< mp.PdgCode() << " parentID " << mp.Mother() << std::endl;
		}
	}
	void OpticksInterface::createG4SkinSurface(std::string VolName, G4OpticalSurface* surface){
		mf::LogTrace("OpticksInterface::createG4SkinSurface")  << "Creating Skin Surface ..."<< std::endl;
  		// find the logical volume associated with the volume name
		auto lv=lvStore->GetVolume(VolName);

  		if(lv)  new G4LogicalSkinSurface(VolName+"_SkinSurface",lv,surface);
  		else  mf::LogInfo ("OpticksInterface") << "Cant find logical volume for " << VolName << std::endl;
	}

	void OpticksInterface::createG4BorderSurface(G4VPhysicalVolume *phyv1, std::string v2, G4OpticalSurface* surface){
  		mf::LogTrace("OpticksInterface::createBorderSurface")  << "Creating Border Surface ..." << std::endl;
		if(phyv1==nullptr){
			 mf::LogInfo ("OpticksInterface") << "Physical Volume pointer is empty!!" << std::endl;
			assert(false);
		}
  		auto phyv2=phyStore->GetVolume(v2);

  		if(phyv2) new G4LogicalBorderSurface(v2+"_BorderSurface", phyv1, phyv2, surface);
  		else  mf::LogInfo ("OpticksInterface") << "Cant find physical volume for " << v2 ;
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
	void OpticksInterface::setSimTag(std::string tag) {
		ftag = tag;
	}
	void OpticksInterface::setSavePhotons(bool ph_save) {
		fph_save = ph_save;
	}
	std::string OpticksInterface::GetSimTag() {
		return ftag;
	}

	bool OpticksInterface::IsSavePhotons() {
		return fph_save;
	}
	void OpticksInterface::setDuration(double dr) {
		auto ana=AnalysisManagerHelper::getInstance();
  		if (pt)
  		{
  			pt->evtID=eventID;
  		    pt->time=dr;
  			ana->FillPerformanceTree(pt);
  		}else {std::cout << "Performance Tree is not initialized! " << std::endl;}
	}
}
