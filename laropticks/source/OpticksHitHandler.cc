
#include "../include/OpticksHitHandler.h"
#include "laropticks/include/OpticksHitHandler.h"

namespace laropticks{

  // Opticks Hit Collection
  // Handles getting hits from opticks to a file
  OpticksHitHandler * OpticksHitHandler::instance = nullptr;
  OpticksHitHandler::~OpticksHitHandler(){
		mf::LogInfo("OpticksHitHandler") << "[OpticksHitHandler::~OpticksHitHandler] Destroying OpticksHitHandler instance" << std::endl;
		// Clear the hits
		hits.clear();
		hits.shrink_to_fit();
  }
  void OpticksHitHandler::CollectHits(int eventID,std::map<int, sim::OBTRHelper> obtrHelpers) {

      //Collecting Opticks Photons
	   mf::LogInfo("OpticksHitHandler") << "[OpticksHitHandler::CollectHits] Collecting Hits from GPU ...." << std::endl;

	  // --- Get Hits ----
      SEvt* sev             = SEvt::Get_EGPU();
      sphoton::Get(sphotons, sev->getHit());

	  feventID=eventID;
  	  hits.reserve(sphotons.size());
      for (auto & hit : sphotons) {
          OpticksHit ohit= OpticksHit();
          ohit.evtID=eventID;
          ohit.hit_id=hit.iindex();
          ohit.parent_id=hit.get_PId();
          ohit.sensor_id=hit.get_identity()-1;
          ohit.x=hit.pos.x;
          ohit.y=hit.pos.y;
          ohit.z=hit.pos.z;
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
		  double pos[3]={ohit.x,ohit.y,ohit.z};
		  obtrHelpers.at(ohit.sensor_id).AddScintillationPhotonsToMap(ohit.parent_id, ohit.time, 1,pos , ohit.wavelength);
          // Increment Photon for Visibilities
          if(fSensorCounts.size()>0) fSensorCounts.at(ohit.sensor_id)+=1;



      }

      if(fSensorCounts.size()>0) SaveVisibilities(); // Save Visibilities Before Hits

	  SaveHits();

      // clear the hits
      sphotons.clear();
      sphotons.shrink_to_fit();
      G4CXOpticks::Get()->reset(eventID);
      QSim::Get()->reset(eventID);

	  //
  }

  void OpticksHitHandler::SaveHits(){

      AnalysisManagerHelper * anaHelper= AnalysisManagerHelper::getInstance();
       mf::LogInfo("OpticksHitHandler") << "[OpticksHitHandler::SaveHits] Saving GPU Hits ..." << std::endl;
      for (auto it : hits){
          anaHelper->FillHitTree(it);
      }

     // Handle Hits Here
     hits.clear();
     hits.shrink_to_fit();
     //G4CXOpticks::Get()->reset(feventID);
     //QSim::Get()->reset(feventID);
  }

  void OpticksHitHandler::SaveVisibilities(){


      AnalysisManagerHelper * anaHelper= AnalysisManagerHelper::getInstance();
       mf::LogInfo("OpticksHitHandler") << "[OpticksHitHandler::SaveVisibilities] Saving GPU Visibilities ..." << std::endl;

      Visibility fvis;
      for (auto &it : fSensorCounts)
      {

        // mf::LogInfo("OpticksHitHandler") << "Sid " <<it.first << " Count " << it.second << " PhotonCount " << PhotonCount << " vis " <<vis <<std::endl;

        fvis.id=fVoxelID;
        fvis.sensorid=it.first;
		double vis = (double (it.second) / double(PhotonCount));

        fvis.Visibility= vis;
		if(vis>0) anaHelper->FillVoxelTree(fvis);
        // Reseting for Next Event
        it.second=0;
      }
       // Reset
       PhotonCount=0;
  }
}
