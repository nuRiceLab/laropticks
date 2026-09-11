
#include "../include/OpticksHitHandler.h"
#include "laropticks/include/OpticksHitHandler.h"
#include "laropticks/include/OpticksHitHandler.h"


namespace laropticks{
  thread_local OpticksHitHandler OpticksHits;
  // Opticks Hit Collection
  // Handles getting hits from opticks to a file
  OpticksHitHandler::OpticksHitHandler():feventID(0),PhotonCount(0),fVoxelID(0)
  {

  }


  OpticksHitHandler::~OpticksHitHandler(){
		//mf::LogInfo("OpticksHitHandler") << "[OpticksHitHandler::~OpticksHitHandler] Destroying OpticksHitHandler instance" << std::endl;
		// Clear the hits
		hits.clear();
		hits.shrink_to_fit();
  }
	// For Primary Photon Generation
	void OpticksHitHandler::CollectHits(int eventID,std::map<int, sim::OBTRHelper> &obtrHelpers)
  {
	  //Collecting Opticks Photons
  	//mf::LogInfo("OpticksHitHandler") << "[OpticksHitHandler::CollectHits] Collecting Hits from GPU ...." << std::endl;

  	// --- Get Hits ----
  	SEvt* sev             = SEvt::Get_EGPU();
  	sphoton::Get(sphotons, sev->getHit());

  	feventID=eventID;
  	hits.reserve(sphotons.size());
  	int ttime=0;
  	for (auto & hit : sphotons) {
  		OpticksHit ohit= OpticksHit();
  		ohit.evtID=eventID;
  		ohit.hit_id=hit.iindex();
  		ohit.parent_id=hit.get_PId();
  		ohit.sensor_id=hit.get_identity() - 1;
  		double pos[3]={hit.pos.x  / cm ,hit.pos.y / cm, hit.pos.z / cm};
  		ohit.x=pos[0];
  		ohit.y=pos[1];
  		ohit.z=pos[2];
  		ohit.polx=hit.pol.x;
  		ohit.poly=hit.pol.y;
  		ohit.polz=hit.pol.z;
  		ohit.momx=hit.mom.x;
  		ohit.momy=hit.mom.y;
  		ohit.momz=hit.mom.z;
  		ohit.time=hit.time;
  		ohit.boundary=hit.boundary();
  		ohit.wavelength=hit.wavelength;
  		hits.push_back(ohit);
  		ttime=static_cast<int>(std::round(ohit.time));
  		obtrHelpers.at(ohit.sensor_id).AddScintillationPhotonsToMap(ohit.parent_id, ttime, 1,  pos , hit.wavelength);
  		// Increment Photon for Visibilities
  		if(fSensorCounts.size()>0) fSensorCounts.at(ohit.sensor_id)+=1;

  	}
  }
  void OpticksHitHandler::CollectHits(int eventID,std::map<int, sim::OBTRHelper> &obtrHelpers, std::map<int,OpticksBackTracker*> &OpticksBTRMap) {

      //Collecting Opticks Photons
  	  //mf::LogInfo("OpticksHitHandler") << "[OpticksHitHandler::CollectHits] Collecting Hits from GPU ...." << std::endl;

	  // --- Get Hits ----
      SEvt* sev             = SEvt::Get_EGPU();
      sphoton::Get(sphotons, sev->getHit());

	  feventID=eventID;
  	  hits.reserve(sphotons.size());
  	  int ttime=0;
      for (auto & hit : sphotons) {
          OpticksHit ohit= OpticksHit();
          ohit.evtID=eventID;
          ohit.hit_id=hit.iindex();

          ohit.sensor_id=hit.get_identity() - 1;
      	  double pos[3]={hit.pos.x  / cm ,hit.pos.y / cm, hit.pos.z / cm};
          ohit.x=pos[0];
          ohit.y=pos[1];
          ohit.z=pos[2];
          ohit.polx=hit.pol.x;
          ohit.poly=hit.pol.y;
          ohit.polz=hit.pol.z;
          ohit.momx=hit.mom.x;
          ohit.momy=hit.mom.y;
          ohit.momz=hit.mom.z;
          ohit.time=hit.time;
          ohit.boundary=hit.boundary();
          ohit.wavelength=hit.wavelength;

      	  ttime=static_cast<int>(std::round(ohit.time));
      	  auto btr = OpticksBTRMap.find(hit.get_PId());
      	   ohit.parent_id=btr->second->TrackID;
      	  //std::cout << "opChannel " << ohit.sensor_id << " time " << ttime << " parent Id " << ohit.parent_id << " edep " << edep<< std::endl ;
		  // Add Scintillation Photons to Map

		  obtrHelpers.at(ohit.sensor_id).AddScintillationPhotonsToMap(btr->second->TrackID, ttime, 1,  btr->second->pos , btr->second->edep);
      	  //std::cout << "OBTR Helper Size " << obtrHelpers.at(ohit.sensor_id).timePDclockSDPsMap()[0] << std::endl;
          // Increment Photon for Visibilities

          if(fSensorCounts.size()>0) fSensorCounts.at(ohit.sensor_id)+=1;
          hits.push_back(ohit);
      }



      if(fSensorCounts.size()>0) SaveVisibilities(); // Save Visibilities Before Hits
	  SaveHits();
      // clear the hits
      sphotons.clear();
      sphotons.shrink_to_fit();
      G4CXOpticks::Get()->reset(eventID);
      QSim::Get()->reset(eventID);
  }

  void OpticksHitHandler::SaveHits(){

      if (anaHelper.getOpticksHitTree()!=nullptr)
      {
      	 // mf::LogInfo("OpticksHitHandler") << "[OpticksHitHandler::SaveHits] Saving GPU Hits ..." << std::endl;
	      for (auto it : hits){
          	anaHelper.FillHitTree(it);
      	 }
      }


     // Handle Hits Here
     hits.clear();
     hits.shrink_to_fit();
     //G4CXOpticks::Get()->reset(feventID);
     //QSim::Get()->reset(feventID);
  }

  void OpticksHitHandler::SaveVisibilities(){


  	 // mf::LogInfo("OpticksHitHandler") << "[OpticksHitHandler::SaveVisibilities] Saving GPU Visibilities ..." << std::endl;

      Visibility fvis;
      for (auto &it : fSensorCounts)
      {

        // mf::LogInfo("OpticksHitHandler") << "Sid " <<it.first << " Count " << it.second << " PhotonCount " << PhotonCount << " vis " <<vis <<std::endl;

        fvis.id=fVoxelID;
        fvis.sensorid=it.first;
		double vis = (double (it.second) / double(PhotonCount));

        fvis.Visibility= vis;
		if(vis>0) anaHelper.FillVoxelTree(fvis);
        // Reseting for Next Event
        it.second=0;
      }
       // Reset
       PhotonCount=0;
  }

}
